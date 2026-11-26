// ImmersiveRecruit - SKSE plugin (no ESP): any NPC whose relationship with the
// player reaches Friend (or better) becomes a potential follower, so the
// vanilla "Follow me" dialogue shows up (and follower frameworks such as NFF
// pick it up) without opening an MCM.
//
// How: every time the player activates (talks to) an NPC, and again when a
// conversation closes, check the NPC's relationship rank with the player. If it
// is Friend+ and the NPC is not yet in PotentialFollowerFaction
// (Skyrim.esm 0005C84D), add them to it (rank 0).
// Activation matters: many NPCs have no dialogue left after their quest and
// only say a greeting, so the dialogue menu never opens for them.
//
// Config: Data/SKSE/Plugins/ImmersiveRecruit.ini

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <chrono>
#include <format>
#include <string>
#include <thread>
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
		auto path = *logDir / "ImmersiveRecruit.log";
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

		// [Follower] - applied once an NPC actually follows the player.
		bool scaleWithPlayer = true;   // same as NFF's "setlevel 1000 0 1 0"
		int levelMultPercent = 100;    // follower level = player level * this / 100
		bool keepLevelAsMin = true;    // never drop below the level they had before
		int maxLevel = 80;             // 0 = no cap
		bool removeLevelCap = true;    // NPCs that already scale but stop at a low level
		bool makeProtected = true;     // only the player can kill them
		bool onlyUnique = true;        // never edit a base shared by many NPCs

		static std::string Path()
		{
			return (std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "ImmersiveRecruit.ini").string();
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

			scaleWithPlayer = GetPrivateProfileIntA("Follower", "bScaleWithPlayer", 1, path.c_str()) != 0;
			levelMultPercent =
				std::clamp(static_cast<int>(GetPrivateProfileIntA("Follower", "iLevelMultPercent", 100, path.c_str())), 10, 500);
			keepLevelAsMin = GetPrivateProfileIntA("Follower", "bKeepCurrentLevelAsMin", 1, path.c_str()) != 0;
			maxLevel = std::clamp(static_cast<int>(GetPrivateProfileIntA("Follower", "iMaxLevel", 80, path.c_str())), 0, 65535);
			removeLevelCap = GetPrivateProfileIntA("Follower", "bRemoveLevelCap", 1, path.c_str()) != 0;
			makeProtected = GetPrivateProfileIntA("Follower", "bProtected", 1, path.c_str()) != 0;
			onlyUnique = GetPrivateProfileIntA("Follower", "bOnlyUnique", 1, path.c_str()) != 0;
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

	// --- Followers: scale with the player + protected ---------------------------
	// Ordinary NPCs have a fixed level, so as followers they fall behind quickly.
	// Same idea as NFF's nfflevel.txt ("setlevel 1000 0 1 0"), done natively when
	// the NPC is actually following the player. Saved in the save game (base data
	// change flag), so it stays after the mod is removed.
	constexpr std::uint32_t kChangeActorBaseData = 1 << 1;  // CHANGE_ACTOR_BASE_DATA

	bool IsFollowingPlayer(RE::Actor* a_actor)
	{
		return a_actor && !a_actor->IsDead() && a_actor->IsPlayerTeammate() && g_potentialFollower &&
		       a_actor->IsInFaction(g_potentialFollower);
	}

	void StrengthenFollower(RE::Actor* a_actor)
	{
		if (!IsFollowingPlayer(a_actor) || (!g_config.scaleWithPlayer && !g_config.makeProtected)) {
			return;
		}
		auto* npc = a_actor->GetActorBase();
		if (!npc || g_config.exclude.contains(npc->GetFormID())) {
			return;
		}
		if (g_config.onlyUnique && !npc->IsUnique()) {
			if (g_config.verbose) {
				SKSE::log::info("{} ({:08X}) not unique - follower stats left alone", a_actor->GetName(), npc->GetFormID());
			}
			return;
		}

		using Flag = RE::ACTOR_BASE_DATA::Flag;
		auto& data = npc->actorData;
		bool changed = false;

		// The cap never goes below the NPC's minimum level (a level 90 NPC keeps 90 with a cap of 80).
		const auto CapFor = [](std::uint16_t a_min) {
			if (g_config.maxLevel == 0) {
				return std::uint16_t{ 0 };
			}
			return std::max(static_cast<std::uint16_t>(g_config.maxLevel), a_min);
		};
		const bool statsFromTemplate =
			npc->UsesTemplate() && data.templateUseFlags.any(RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG::kStats);
		// Already scales but stops early (e.g. Avrusa Sarethi at 25): lift the cap.
		const bool capTooLow = data.calcLevelMax != 0 &&
		                       (g_config.maxLevel == 0 || data.calcLevelMax < g_config.maxLevel);

		if (g_config.scaleWithPlayer && npc->HasPCLevelMult() && g_config.removeLevelCap && capTooLow) {
			if (statsFromTemplate) {
				SKSE::log::info("{} ({:08X}) takes stats from a template - cannot change level cap", a_actor->GetName(),
					npc->GetFormID());
			} else {
				const auto oldMax = data.calcLevelMax;
				data.calcLevelMax = CapFor(data.calcLevelMin);
				changed = true;
				SKSE::log::info("{} ({:08X}) level cap {} -> {} (level {}..)", a_actor->GetName(), npc->GetFormID(),
					oldMax, data.calcLevelMax ? std::to_string(data.calcLevelMax) : "no cap"s, data.calcLevelMin);
			}
		} else if (g_config.scaleWithPlayer && !npc->HasPCLevelMult()) {
			if (statsFromTemplate) {
				SKSE::log::info("{} ({:08X}) takes stats from a template - cannot scale level", a_actor->GetName(),
					npc->GetFormID());
			} else {
				const auto oldLevel = a_actor->GetLevel();
				data.actorBaseFlags.set(Flag::kPCLevelMult);
				data.level = static_cast<std::uint16_t>(g_config.levelMultPercent * 10);  // 1000 = x1.0
				data.calcLevelMin = static_cast<std::uint16_t>(g_config.keepLevelAsMin ? std::max<std::uint16_t>(oldLevel, 1) : 1);
				data.calcLevelMax = CapFor(data.calcLevelMin);
				changed = true;
				SKSE::log::info("{} ({:08X}) now scales with the player: x{:.2f}, level {}..{} (was fixed {})",
					a_actor->GetName(), npc->GetFormID(), g_config.levelMultPercent / 100.0, data.calcLevelMin,
					data.calcLevelMax ? std::to_string(data.calcLevelMax) : "no cap"s, oldLevel);
			}
		}
		if (g_config.makeProtected && !npc->IsEssential() && !npc->IsProtected()) {
			data.actorBaseFlags.set(Flag::kProtected);
			changed = true;
			SKSE::log::info("{} ({:08X}) is now protected", a_actor->GetName(), npc->GetFormID());
		}
		if (changed) {
			npc->AddChange(kChangeActorBaseData);
			if (g_config.notify) {
				RE::DebugNotification(std::format("{} will now grow stronger with you.", a_actor->GetName()).c_str());
			}
		}
	}

	// The follow flag is set by the dialogue script after the menu closes.
	void QueueFollowerCheck(RE::ObjectRefHandle a_handle, std::chrono::milliseconds a_delay)
	{
		if (!a_handle) {
			return;
		}
		std::thread([a_handle, a_delay]() {
			std::this_thread::sleep_for(a_delay);
			SKSE::GetTaskInterface()->AddTask([a_handle]() {
				auto ref = a_handle.get();
				StrengthenFollower(ref ? ref->As<RE::Actor>() : nullptr);
			});
		}).detach();
	}

	// Followers recruited elsewhere (NFF power, before this version...) on game load.
	void CheckLoadedFollowers()
	{
		std::thread([]() {
			std::this_thread::sleep_for(5s);
			SKSE::GetTaskInterface()->AddTask([]() {
				if (auto* lists = RE::ProcessLists::GetSingleton()) {
					for (auto& handle : lists->highActorHandles) {
						auto actor = handle.get();
						StrengthenFollower(actor.get());
					}
				}
			});
		}).detach();
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

	// Player activates an NPC: fires even when the NPC only greets and no dialogue menu opens.
	class ActivateSink : public RE::BSTEventSink<RE::TESActivateEvent>
	{
	public:
		static ActivateSink* GetSingleton()
		{
			static ActivateSink singleton;
			return &singleton;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* a_event,
			RE::BSTEventSource<RE::TESActivateEvent>*) override
		{
			if (!a_event || !a_event->objectActivated || !a_event->actionRef ||
				a_event->actionRef.get() != RE::PlayerCharacter::GetSingleton()) {
				return RE::BSEventNotifyControl::kContinue;
			}
			auto* target = a_event->objectActivated.get();
			if (target->GetFormType() != RE::FormType::ActorCharacter) {
				return RE::BSEventNotifyControl::kContinue;
			}
			if (g_config.verbose) {
				SKSE::log::info("Player activated {}", target->GetName());
			}
			QueueCheck(target->GetHandle());
			return RE::BSEventNotifyControl::kContinue;
		}
	};

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
				// Remember who we are talking to (the activation check already ran).
				if (auto* topics = RE::MenuTopicManager::GetSingleton()) {
					g_lastSpeaker = topics->speaker;
				}
			} else {
				// Quests/favors usually raise the relationship during the conversation.
				QueueCheck(g_lastSpeaker);
				QueueFollowerCheck(g_lastSpeaker, 3s);
				g_lastSpeaker.reset();
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (a_msg->type == SKSE::MessagingInterface::kPostLoadGame) {
			if (g_potentialFollower) {
				CheckLoadedFollowers();
			}
			return;
		}
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
		SKSE::log::info(
			"Follower: scale={} mult={}% keepLevelAsMin={} maxLevel={} removeLevelCap={} protected={} onlyUnique={}",
			g_config.scaleWithPlayer, g_config.levelMultPercent, g_config.keepLevelAsMin, g_config.maxLevel,
			g_config.removeLevelCap, g_config.makeProtected, g_config.onlyUnique);

		if (auto* ui = RE::UI::GetSingleton()) {
			ui->AddEventSink<RE::MenuOpenCloseEvent>(MenuSink::GetSingleton());
			SKSE::log::info("Registered MenuOpenCloseEvent sink");
		}
		if (auto* events = RE::ScriptEventSourceHolder::GetSingleton()) {
			events->AddEventSink<RE::TESActivateEvent>(ActivateSink::GetSingleton());
			SKSE::log::info("Registered TESActivateEvent sink");
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 1, 1, 0, 0 },
	.Name = "ImmersiveRecruit"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("ImmersiveRecruit v1.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
	return true;
}
