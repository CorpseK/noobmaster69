// EasyRecruit - SKSE plugin (no ESP): any NPC whose relationship with the
// player reaches Friend (or better) becomes a potential follower, so the
// vanilla "Follow me" dialogue shows up (and follower frameworks such as NFF
// pick it up) without opening an MCM.
//
// How: after every conversation, check the speaker's relationship rank with
// the player. If it is Friend+ and the NPC is not yet in
// PotentialFollowerFaction (Skyrim.esm 0005C84D), add them to it (rank 0).
//
// Config: Data/SKSE/Plugins/EasyRecruit.ini

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

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
		auto path = *logDir / "EasyRecruit.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%H:%M:%S] [%l] %v");
	}

	using Level = RE::BGSRelationship::RELATIONSHIP_LEVEL;

	// Papyrus-style rank: 4 Lover, 3 Ally, 2 Confidant, 1 Friend, 0 Acquaintance,
	// -1 Rival, -2 Foe, -3 Enemy, -4 Archnemesis.
	int ToPapyrusRank(Level a_level)
	{
		return 4 - static_cast<int>(a_level);
	}

	std::string Trim(std::string_view a_str)
	{
		const auto first = a_str.find_first_not_of(" \t\r\n");
		if (first == std::string_view::npos) {
			return {};
		}
		const auto last = a_str.find_last_not_of(" \t\r\n");
		return std::string(a_str.substr(first, last - first + 1));
	}

	struct Config
	{
		int minRank = 1;  // Friend
		bool notify = true;
		bool skipChildren = true;
		bool skipHostile = true;
		bool verbose = false;
		std::unordered_set<RE::FormID> exclude;  // base NPC IDs: never touched

		static std::string Path()
		{
			return (std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "EasyRecruit.ini").string();
		}

		// Section lines look like: AnyName=Plugin.esp|0x00ABCD (load-order independent local form ID).
		static std::unordered_set<RE::FormID> ReadNPCList(const char* a_section)
		{
			std::unordered_set<RE::FormID> result;
			const auto path = Path();
			std::vector<char> buf(32 * 1024);
			const auto len =
				GetPrivateProfileSectionA(a_section, buf.data(), static_cast<DWORD>(buf.size()), path.c_str());
			auto* dh = RE::TESDataHandler::GetSingleton();

			for (std::size_t pos = 0; pos < len;) {
				std::string_view line(buf.data() + pos);
				pos += line.size() + 1;
				if (line.empty() || line.front() == ';' || line.front() == '#') {
					continue;
				}
				const auto eq = line.find('=');
				const auto value = Trim(eq == std::string_view::npos ? line : line.substr(eq + 1));
				const auto bar = value.find('|');
				if (bar == std::string::npos) {
					SKSE::log::warn("[{}] bad entry '{}' (expected Plugin.esp|0xID)", a_section, line);
					continue;
				}
				const auto plugin = Trim(std::string_view(value).substr(0, bar));
				RE::FormID localID = 0;
				try {
					localID = static_cast<RE::FormID>(std::stoul(Trim(std::string_view(value).substr(bar + 1)), nullptr, 16));
				} catch (...) {
					SKSE::log::warn("[{}] bad form ID in '{}'", a_section, line);
					continue;
				}
				auto* npc = dh ? dh->LookupForm<RE::TESNPC>(localID & 0x00FFFFFF, plugin) : nullptr;
				if (!npc) {
					// ESL local IDs are only 12 bits.
					npc = dh ? dh->LookupForm<RE::TESNPC>(localID & 0x00000FFF, plugin) : nullptr;
				}
				if (!npc) {
					SKSE::log::warn("[{}] NPC not found: {}|{:X}", a_section, plugin, localID);
					continue;
				}
				result.insert(npc->GetFormID());
				SKSE::log::info("[{}] {} ({:08X})", a_section, npc->GetName(), npc->GetFormID());
			}
			return result;
		}

		void Load()
		{
			const auto path = Path();
			minRank = std::clamp(static_cast<int>(GetPrivateProfileIntA("General", "iMinRank", minRank, path.c_str())), -4, 4);
			notify = GetPrivateProfileIntA("General", "bNotify", notify ? 1 : 0, path.c_str()) != 0;
			skipChildren = GetPrivateProfileIntA("General", "bSkipChildren", skipChildren ? 1 : 0, path.c_str()) != 0;
			skipHostile = GetPrivateProfileIntA("General", "bSkipHostile", skipHostile ? 1 : 0, path.c_str()) != 0;
			verbose = GetPrivateProfileIntA("General", "bVerboseLog", verbose ? 1 : 0, path.c_str()) != 0;
			exclude = ReadNPCList("Exclude");
		}
	};

	Config g_config;
	RE::TESFaction* g_potentialFollower = nullptr;
	RE::ObjectRefHandle g_lastSpeaker;

	// Relationship rank of a_npc toward the player, Papyrus scale. 0 if no record.
	int GetRankWithPlayer(RE::TESNPC* a_npc, RE::TESNPC* a_playerBase)
	{
		auto* rel = RE::BGSRelationship::GetRelationship(a_npc, a_playerBase);
		if (!rel) {
			rel = RE::BGSRelationship::GetRelationship(a_playerBase, a_npc);
		}
		return rel ? ToPapyrusRank(rel->level.get()) : 0;
	}

	void CheckActor(RE::Actor* a_actor)
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!a_actor || !player || a_actor == player || !g_potentialFollower) {
			return;
		}
		auto* npc = a_actor->GetActorBase();
		auto* playerBase = player->GetActorBase();
		if (!npc || !playerBase) {
			return;
		}

		const auto baseID = npc->GetFormID();
		const char* name = a_actor->GetName();

		if (g_config.exclude.contains(baseID)) {
			if (g_config.verbose) {
				SKSE::log::info("{} ({:08X}) is in [Exclude]", name, baseID);
			}
			return;
		}
		if (a_actor->IsInFaction(g_potentialFollower)) {
			if (g_config.verbose) {
				SKSE::log::info("{} ({:08X}) is already a potential follower", name, baseID);
			}
			return;
		}
		if (a_actor->IsDead() || (g_config.skipChildren && a_actor->IsChild()) ||
			(g_config.skipHostile && a_actor->IsHostileToActor(player))) {
			if (g_config.verbose) {
				SKSE::log::info("{} ({:08X}) skipped (dead/child/hostile)", name, baseID);
			}
			return;
		}

		const int rank = GetRankWithPlayer(npc, playerBase);
		if (rank < g_config.minRank) {
			if (g_config.verbose) {
				SKSE::log::info("{} ({:08X}) relationship rank {} < {}", name, baseID, rank, g_config.minRank);
			}
			return;
		}

		a_actor->AddToFaction(g_potentialFollower, 0);
		SKSE::log::info("{} ({:08X}) added to PotentialFollowerFaction (relationship rank {})", name, baseID, rank);
		if (g_config.notify) {
			const auto msg = std::format("{} can now be recruited as a follower.", name);
			RE::DebugNotification(msg.c_str());
		}
	}

	void QueueCheck(RE::ObjectRefHandle a_handle)
	{
		if (!a_handle) {
			return;
		}
		SKSE::GetTaskInterface()->AddTask([a_handle]() {
			auto ref = a_handle.get();
			if (ref) {
				CheckActor(ref->As<RE::Actor>());
			}
		});
	}

	class MenuSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static MenuSink* GetSingleton()
		{
			static MenuSink singleton;
			return &singleton;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
			RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
		{
			if (!a_event || a_event->menuName != RE::DialogueMenu::MENU_NAME) {
				return RE::BSEventNotifyControl::kContinue;
			}

			if (a_event->opening) {
				// Remember who we are talking to; also catches NPCs that were already
				// Friend+ before the mod was installed (recruitable on the next talk).
				if (auto* topics = RE::MenuTopicManager::GetSingleton()) {
					g_lastSpeaker = topics->speaker;
				}
				QueueCheck(g_lastSpeaker);
			} else {
				// Quests/favors usually raise the relationship during the conversation.
				QueueCheck(g_lastSpeaker);
				g_lastSpeaker.reset();
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (a_msg->type != SKSE::MessagingInterface::kDataLoaded) {
			return;
		}

		g_potentialFollower = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x05C84D, "Skyrim.esm"sv);
		if (!g_potentialFollower) {
			SKSE::log::error("PotentialFollowerFaction (Skyrim.esm 0005C84D) not found - mod disabled");
			return;
		}

		g_config.Load();
		SKSE::log::info("Config: minRank={} notify={} skipChildren={} skipHostile={} exclude={}",
			g_config.minRank, g_config.notify, g_config.skipChildren, g_config.skipHostile, g_config.exclude.size());

		if (auto* ui = RE::UI::GetSingleton()) {
			ui->AddEventSink<RE::MenuOpenCloseEvent>(MenuSink::GetSingleton());
			SKSE::log::info("Registered MenuOpenCloseEvent sink");
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 1, 0, 0 },
	.Name = "EasyRecruit"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("EasyRecruit v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
	return true;
}
