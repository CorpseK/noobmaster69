// BendWillRecruit - SKSE plugin (no ESP): the Bend Will shout (Dragonborn DLC)
// turns any humanoid NPC it hits into a follower.
//
// How:
//  1. After data load, find the Bend Will shout (TESShout from Dragonborn.esm,
//     by name or by an INI override) and collect the magic effects of its 3 words.
//  2. Listen to TESMagicEffectApplyEvent. When the player's Bend Will effect lands
//     on an NPC with the ActorTypeNPC keyword, recruit them:
//       - stop combat, raise relationship to Ally (Papyrus Actor.SetRelationshipRank),
//       - add to PotentialFollowerFaction (so "Follow me" works after dismissing),
//       - call DialogueFollowerScript.SetFollower on the vanilla DialogueFollower
//         quest, the same call the "Follow me" dialogue makes. Follower frameworks
//         such as NFF hook this call, so they take over as usual.
//
// Config: Data/SKSE/Plugins/BendWillRecruit.ini

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <format>
#include <unordered_map>
#include <unordered_set>

// Windows API headers must come after CommonLib.
#include <Windows.h>

using namespace std::literals;

namespace
{
	void SetupLog()
	{
		auto logDir = SKSE::log::log_directory();
		if (!logDir) {
			return;
		}
		auto path = *logDir / "BendWillRecruit.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%H:%M:%S] [%l] %v");
	}

	// --- Skyrim.esm forms ----------------------------------------------------
	constexpr RE::FormID kPotentialFollowerFaction = 0x05C84D;
	constexpr RE::FormID kCurrentFollowerFaction = 0x05C84E;
	constexpr RE::FormID kDialogueFollowerQuest = 0x0750BA;
	constexpr RE::FormID kActorTypeNPC = 0x013794;

	std::string Trim(std::string_view a_str)
	{
		const auto first = a_str.find_first_not_of(" \t\r\n");
		if (first == std::string_view::npos) {
			return {};
		}
		const auto last = a_str.find_last_not_of(" \t\r\n");
		return std::string(a_str.substr(first, last - first + 1));
	}

	// "Plugin.esp|0x123456" -> form, or nullptr.
	template <class T>
	T* LookupPluginForm(std::string_view a_entry)
	{
		const auto bar = a_entry.find('|');
		if (bar == std::string_view::npos) {
			return nullptr;
		}
		const auto plugin = Trim(a_entry.substr(0, bar));
		RE::FormID localID = 0;
		try {
			localID = static_cast<RE::FormID>(std::stoul(Trim(a_entry.substr(bar + 1)), nullptr, 16));
		} catch (...) {
			return nullptr;
		}
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return nullptr;
		}
		auto* form = dh->LookupForm<T>(localID & 0x00FFFFFF, plugin);
		if (!form) {
			form = dh->LookupForm<T>(localID & 0x00000FFF, plugin);  // ESL
		}
		return form;
	}

	struct Config
	{
		std::string shoutName = "Bend Will";
		std::string shoutOverride;  // "Plugin.esp|0xID", empty = search by name
		int minWords = 1;
		bool recruitHostile = true;
		bool skipChildren = true;
		bool notify = true;
		bool verbose = false;
		std::unordered_set<RE::FormID> exclude;

		static std::string Path()
		{
			return (std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "BendWillRecruit.ini").string();
		}

		static std::string ReadString(const char* a_key, const std::string& a_default)
		{
			char buf[256]{};
			GetPrivateProfileStringA("General", a_key, a_default.c_str(), buf, sizeof(buf), Path().c_str());
			return Trim(buf);
		}

		static bool ReadBool(const char* a_key, bool a_default)
		{
			return GetPrivateProfileIntA("General", a_key, a_default ? 1 : 0, Path().c_str()) != 0;
		}

		void Load()
		{
			const auto path = Path();
			shoutName = ReadString("sShoutName", shoutName);
			shoutOverride = ReadString("sShoutForm", shoutOverride);
			minWords = std::clamp(static_cast<int>(GetPrivateProfileIntA("General", "iMinWords", minWords, path.c_str())), 1, 3);
			recruitHostile = ReadBool("bRecruitHostile", recruitHostile);
			skipChildren = ReadBool("bSkipChildren", skipChildren);
			notify = ReadBool("bNotify", notify);
			verbose = ReadBool("bVerboseLog", verbose);

			std::vector<char> buf(32 * 1024);
			const auto len = GetPrivateProfileSectionA("Exclude", buf.data(), static_cast<DWORD>(buf.size()), path.c_str());
			for (std::size_t pos = 0; pos < len;) {
				std::string_view line(buf.data() + pos);
				pos += line.size() + 1;
				if (line.empty() || line.front() == ';' || line.front() == '#') {
					continue;
				}
				const auto eq = line.find('=');
				const auto value = Trim(eq == std::string_view::npos ? line : line.substr(eq + 1));
				if (auto* npc = LookupPluginForm<RE::TESNPC>(value)) {
					exclude.insert(npc->GetFormID());
					SKSE::log::info("[Exclude] {} ({:08X})", npc->GetName(), npc->GetFormID());
				} else {
					SKSE::log::warn("[Exclude] NPC not found: '{}' (expected Plugin.esp|0xID)", line);
				}
			}
		}
	};

	Config g_config;
	RE::TESFaction* g_potentialFollower = nullptr;
	RE::TESFaction* g_currentFollower = nullptr;
	RE::TESQuest* g_dialogueFollower = nullptr;
	RE::BGSKeyword* g_actorTypeNPC = nullptr;
	std::unordered_map<RE::FormID, int> g_shoutEffects;  // magic effect -> lowest word index (1-3)

	// --- Find the shout --------------------------------------------------------
	RE::TESShout* FindShout()
	{
		if (!g_config.shoutOverride.empty()) {
			if (auto* shout = LookupPluginForm<RE::TESShout>(g_config.shoutOverride)) {
				return shout;
			}
			SKSE::log::warn("sShoutForm '{}' not found, searching by name", g_config.shoutOverride);
		}

		RE::TESShout* found = nullptr;
		for (auto* shout : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESShout>()) {
			if (!shout) {
				continue;
			}
			const char* name = shout->GetName();
			const auto* file = shout->GetFile(0);
			if (g_config.verbose) {
				SKSE::log::info("Shout {:08X} '{}' from {}", shout->GetFormID(), name ? name : "",
					file ? file->GetFilename() : "?"sv);
			}
			if (name && _stricmp(name, g_config.shoutName.c_str()) == 0) {
				// Prefer the one from Dragonborn.esm if several share the name.
				if (!found || (file && file->GetFilename() == "Dragonborn.esm"sv)) {
					found = shout;
				}
			}
		}
		return found;
	}

	bool CollectShoutEffects()
	{
		auto* shout = FindShout();
		if (!shout) {
			SKSE::log::error("Shout '{}' not found. Set sShoutName (localized name) or sShoutForm in the INI.",
				g_config.shoutName);
			return false;
		}
		SKSE::log::info("Using shout {:08X} '{}'", shout->GetFormID(), shout->GetName());

		for (int i = 0; i < 3; ++i) {
			auto* spell = shout->variations[i].spell;
			if (!spell) {
				continue;
			}
			for (auto* effect : spell->effects) {
				if (!effect || !effect->baseEffect) {
					continue;
				}
				const auto id = effect->baseEffect->GetFormID();
				const int word = i + 1;
				auto [it, inserted] = g_shoutEffects.try_emplace(id, word);
				if (!inserted) {
					it->second = std::min(it->second, word);
				}
				SKSE::log::info("  word {}: spell {:08X} '{}' effect {:08X} '{}'", word, spell->GetFormID(),
					spell->GetName(), id, effect->baseEffect->GetName());
			}
		}
		if (g_shoutEffects.empty()) {
			SKSE::log::error("Shout has no magic effects");
			return false;
		}
		return true;
	}

	// --- Papyrus calls ---------------------------------------------------------
	void CallPapyrus(RE::TESForm* a_form, const char* a_class, const char* a_function,
		RE::BSScript::IFunctionArguments* a_args)
	{
		auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!vm || !a_form) {
			delete a_args;
			return;
		}
		auto* policy = vm->GetObjectHandlePolicy();
		const auto handle = policy->GetHandleForObject(a_form->GetFormType(), a_form);
		if (handle == policy->EmptyHandle()) {
			SKSE::log::error("No VM handle for {:08X} ({}.{})", a_form->GetFormID(), a_class, a_function);
			delete a_args;
			return;
		}
		RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
		if (!vm->DispatchMethodCall(handle, a_class, a_function, a_args, callback)) {
			SKSE::log::error("Papyrus call {}.{} on {:08X} failed", a_class, a_function, a_form->GetFormID());
		}
	}

	bool IsFollower(RE::Actor* a_actor)
	{
		return a_actor->IsPlayerTeammate() ||
		       (g_currentFollower && a_actor->IsInFaction(g_currentFollower) &&
				   a_actor->GetFactionRank(g_currentFollower, false) >= 0);
	}

	void Recruit(RE::Actor* a_actor, int a_word)
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!a_actor || !player || a_actor == player) {
			return;
		}
		auto* npc = a_actor->GetActorBase();
		const char* name = a_actor->GetName();
		const auto baseID = npc ? npc->GetFormID() : 0;

		auto skip = [&](std::string_view a_reason) {
			if (g_config.verbose) {
				SKSE::log::info("{} ({:08X}) not recruited: {}", name, baseID, a_reason);
			}
		};

		if (a_word < g_config.minWords) {
			return skip(std::format("word {} < iMinWords {}", a_word, g_config.minWords));
		}
		if (!npc) {
			return skip("no base NPC");
		}
		if (g_config.exclude.contains(baseID)) {
			return skip("in [Exclude]");
		}
		if (a_actor->IsDead()) {
			return skip("dead");
		}
		if (!g_actorTypeNPC || !a_actor->HasKeyword(g_actorTypeNPC)) {
			return skip("not humanoid (no ActorTypeNPC)");
		}
		if (g_config.skipChildren && a_actor->IsChild()) {
			return skip("child");
		}
		if (IsFollower(a_actor)) {
			return skip("already a follower");
		}
		const bool hostile = a_actor->IsHostileToActor(player);
		if (hostile && !g_config.recruitHostile) {
			return skip("hostile (bRecruitHostile=0)");
		}

		if (a_actor->IsInCombat()) {
			a_actor->StopCombat();
		}
		// Ally (3). The vanilla SetFollower only raises ranks that are already >= 0.
		CallPapyrus(a_actor, "Actor", "SetRelationshipRank",
			RE::MakeFunctionArguments(static_cast<RE::Actor*>(player), static_cast<std::int32_t>(3)));
		if (g_potentialFollower && !a_actor->IsInFaction(g_potentialFollower)) {
			a_actor->AddToFaction(g_potentialFollower, 0);
		}
		CallPapyrus(g_dialogueFollower, "DialogueFollowerScript", "SetFollower",
			RE::MakeFunctionArguments(static_cast<RE::TESObjectREFR*>(a_actor)));

		SKSE::log::info("{} ({:08X}) recruited by Bend Will word {}{}", name, baseID, a_word, hostile ? " (was hostile)" : "");
		if (g_config.notify) {
			RE::SendHUDMessage::ShowHUDMessage(std::format("{} bows to your Voice and follows you.", name).c_str());
		}
	}

	class MagicEffectSink : public RE::BSTEventSink<RE::TESMagicEffectApplyEvent>
	{
	public:
		static MagicEffectSink* GetSingleton()
		{
			static MagicEffectSink singleton;
			return &singleton;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event,
			RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*) override
		{
			if (!a_event) {
				return RE::BSEventNotifyControl::kContinue;
			}
			const auto it = g_shoutEffects.find(a_event->magicEffect);
			if (it == g_shoutEffects.end()) {
				return RE::BSEventNotifyControl::kContinue;
			}
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player || a_event->caster.get() != player || !a_event->target) {
				return RE::BSEventNotifyControl::kContinue;
			}
			auto* target = a_event->target->As<RE::Actor>();
			if (!target) {
				return RE::BSEventNotifyControl::kContinue;
			}

			// A shout may apply several effects; recruit once per hit, on the game's main thread.
			const auto handle = target->GetHandle();
			const int word = it->second;
			SKSE::GetTaskInterface()->AddTask([handle, word]() {
				if (auto ref = handle.get()) {
					Recruit(ref->As<RE::Actor>(), word);
				}
			});
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (a_msg->type != SKSE::MessagingInterface::kDataLoaded) {
			return;
		}

		auto* dh = RE::TESDataHandler::GetSingleton();
		g_potentialFollower = dh->LookupForm<RE::TESFaction>(kPotentialFollowerFaction, "Skyrim.esm"sv);
		g_currentFollower = dh->LookupForm<RE::TESFaction>(kCurrentFollowerFaction, "Skyrim.esm"sv);
		g_dialogueFollower = dh->LookupForm<RE::TESQuest>(kDialogueFollowerQuest, "Skyrim.esm"sv);
		g_actorTypeNPC = dh->LookupForm<RE::BGSKeyword>(kActorTypeNPC, "Skyrim.esm"sv);
		if (!g_dialogueFollower || !g_actorTypeNPC) {
			SKSE::log::error("DialogueFollower quest or ActorTypeNPC keyword not found - mod disabled");
			return;
		}

		g_config.Load();
		SKSE::log::info("Config: shout='{}' form='{}' minWords={} recruitHostile={} skipChildren={} notify={} exclude={}",
			g_config.shoutName, g_config.shoutOverride, g_config.minWords, g_config.recruitHostile,
			g_config.skipChildren, g_config.notify, g_config.exclude.size());

		if (!CollectShoutEffects()) {
			return;
		}
		if (auto* events = RE::ScriptEventSourceHolder::GetSingleton()) {
			events->AddEventSink<RE::TESMagicEffectApplyEvent>(MagicEffectSink::GetSingleton());
			SKSE::log::info("Registered TESMagicEffectApplyEvent sink");
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 1, 0, 0 },
	.Name = "BendWillRecruit"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("BendWillRecruit v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
	return true;
}
