// RelicHunter - SKSE plugin (no ESP) for Legacy of the Dragonborn + The Curator's Companion.
//
// Makes random loot (chests, corpses, NPC gear, merchants - anything that comes
// from a *leveled item list*) more likely to be a museum relic the player still
// needs.
//
// How it works:
//   Leveled item lists (LVLI) are resolved by TESLeveledList::CalculateCurrentFormList.
//   After the game picks an item, if that item is NOT a wanted relic and the same
//   list tree contains wanted relics allowed at this level, we roll fRerollChance
//   and on success replace the pick with one of those relics.
//   => Only items that could already drop from that list are ever produced.
//      Quest/unique artifacts that are not in leveled lists are never spawned.
//
// "Wanted" comes from The Curator's Companion (DBM_RelicNotifications.esp), the
// same lists moreHUD reads (local FormIDs, measured by the grid-inventory project):
//   dbmMaster 0x609634  every relic   | dbmNew  0x558285  never found
//   dbmFound  0x558286  found, not displayed | dbmDisp 0x558287  displayed
// They are filled by Papyrus at runtime, so we re-read them (on the main thread)
// whenever their size changes.
//
// Config: Data/SKSE/Plugins/RelicHunter.ini

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Windows API headers must come after CommonLib.
#include <Windows.h>
#include <detours/detours.h>

using namespace std::literals;

namespace
{
	void SetupLog()
	{
		auto logDir = SKSE::log::log_directory();
		if (!logDir) {
			return;
		}
		auto path = *logDir / "RelicHunter.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%H:%M:%S] [%l] %v");
	}

	// ------------------------------------------------------------------------
	// Config
	// ------------------------------------------------------------------------
	struct Config
	{
		float rerollChance = 25.0f;  // 0..100, per picked item
		bool includeFound = false;   // also boost "found but not displayed" relics
		bool sameType = true;        // armor -> armor, weapon -> weapon, misc -> misc...
		bool verbose = false;

		void Load()
		{
			const auto path =
				(std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "RelicHunter.ini").string();
			char buf[64]{};
			GetPrivateProfileStringA("General", "fRerollChance", "", buf, sizeof(buf), path.c_str());
			if (buf[0] != '\0') {
				try {
					rerollChance = std::stof(buf);
				} catch (...) {
				}
			}
			rerollChance = std::clamp(rerollChance, 0.0f, 100.0f);
			includeFound = GetPrivateProfileIntA("General", "bIncludeFound", 0, path.c_str()) != 0;
			sameType = GetPrivateProfileIntA("General", "bSameType", 1, path.c_str()) != 0;
			verbose = GetPrivateProfileIntA("General", "bVerboseLog", 0, path.c_str()) != 0;
		}
	};

	Config g_config;

	// ------------------------------------------------------------------------
	// Curator's Companion lists
	// ------------------------------------------------------------------------
	constexpr auto kCurator = "DBM_RelicNotifications.esp"sv;

	enum ListIdx : std::size_t
	{
		kMaster,
		kNew,
		kFound,
		kDisp,
		kListCount
	};

	constexpr std::pair<RE::FormID, std::string_view> kLists[kListCount] = {
		{ 0x609634, "dbmMaster"sv },
		{ 0x558285, "dbmNew"sv },
		{ 0x558286, "dbmFound"sv },
		{ 0x558287, "dbmDisp"sv },
	};

	RE::BGSListForm* g_lists[kListCount]{};
	std::size_t g_lastSizes[kListCount]{};

	std::mutex g_lock;                         // guards g_wanted, g_pools, g_rng
	std::unordered_set<RE::FormID> g_wanted;   // relics to boost
	std::atomic<bool> g_refreshQueued{ false };
	std::atomic<std::int64_t> g_lastRefreshMs{ 0 };

	std::int64_t NowMs()
	{
		using namespace std::chrono;
		return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
	}

	std::size_t ListSize(const RE::BGSListForm* a_list)
	{
		if (!a_list) {
			return 0;
		}
		return a_list->forms.size() + (a_list->scriptAddedTempForms ? a_list->scriptAddedTempForms->size() : 0);
	}

	void CollectList(const RE::BGSListForm* a_list, std::unordered_set<RE::FormID>& a_out, int a_depth = 0)
	{
		if (!a_list || a_depth > 4) {
			return;
		}
		a_list->ForEachForm([&](RE::TESForm* a_form) {
			if (a_form) {
				if (auto* sub = a_form->As<RE::BGSListForm>()) {
					CollectList(sub, a_out, a_depth + 1);
				} else {
					a_out.insert(a_form->GetFormID());
				}
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});
	}

	// Main thread only (Papyrus edits these lists).
	void RefreshWanted(bool a_force)
	{
		g_lastRefreshMs = NowMs();
		if (!g_lists[kNew]) {
			return;
		}
		bool moved = a_force;
		std::size_t sizes[kListCount]{};
		for (std::size_t i = 0; i < kListCount; ++i) {
			sizes[i] = ListSize(g_lists[i]);
			moved = moved || sizes[i] != g_lastSizes[i];
		}
		if (!moved) {
			return;
		}
		std::copy(std::begin(sizes), std::end(sizes), std::begin(g_lastSizes));

		std::unordered_set<RE::FormID> wanted;
		CollectList(g_lists[kNew], wanted);
		if (g_config.includeFound) {
			CollectList(g_lists[kFound], wanted);
		}
		// Safety net: never boost something already on a pedestal.
		std::unordered_set<RE::FormID> displayed;
		CollectList(g_lists[kDisp], displayed);
		for (auto id : displayed) {
			wanted.erase(id);
		}

		SKSE::log::info("Relic lists: master={} new={} found={} disp={} -> {} wanted", sizes[kMaster], sizes[kNew],
			sizes[kFound], sizes[kDisp], wanted.size());

		std::scoped_lock guard(g_lock);
		g_wanted = std::move(wanted);
	}

	void QueueRefresh()
	{
		// At most one pending task, and at most one check every 3 s.
		if (NowMs() - g_lastRefreshMs < 3000 || g_refreshQueued.exchange(true)) {
			return;
		}
		if (auto* tasks = SKSE::GetTaskInterface()) {
			tasks->AddTask([] {
				RefreshWanted(false);
				g_refreshQueued = false;
			});
		} else {
			g_refreshQueued = false;
		}
	}

	void ResolveLists()
	{
		auto* dh = RE::TESDataHandler::GetSingleton();
		const auto* file = dh ? dh->LookupModByName(kCurator) : nullptr;
		if (!file || file->GetCompileIndex() == 0xFF) {
			SKSE::log::warn("{} not loaded -> RelicHunter does nothing", kCurator);
			return;
		}
		for (std::size_t i = 0; i < kListCount; ++i) {
			g_lists[i] = dh->LookupForm<RE::BGSListForm>(kLists[i].first, kCurator);
			if (!g_lists[i]) {
				// Needs an EditorID map (powerofthree's Tweaks) - fallback if the IDs ever move.
				g_lists[i] = RE::TESForm::LookupByEditorID<RE::BGSListForm>(kLists[i].second);
			}
			SKSE::log::info("{} {}", kLists[i].second, g_lists[i] ? "found" : "NOT FOUND");
		}
		if (!g_lists[kNew] || !g_lists[kDisp]) {
			SKSE::log::error("dbmNew/dbmDisp missing -> RelicHunter does nothing");
			for (auto*& l : g_lists) {
				l = nullptr;
			}
		}
	}

	// ------------------------------------------------------------------------
	// Leveled item trees (static data, cached per list)
	// ------------------------------------------------------------------------
	struct Leaf
	{
		RE::TESForm* form = nullptr;
		std::uint16_t level = 0;
		std::uint16_t count = 1;
	};

	std::unordered_map<const RE::TESLeveledList*, std::vector<Leaf>> g_pools;
	std::mt19937 g_rng{ std::random_device{}() };

	void Collect(const RE::TESLeveledList* a_list, std::uint16_t a_gate, std::vector<Leaf>& a_out,
		std::unordered_set<const RE::TESLeveledList*>& a_seen)
	{
		if (!a_list || !a_seen.insert(a_list).second) {
			return;
		}
		for (const auto& entry : a_list->entries) {
			if (!entry.form) {
				continue;
			}
			const auto level = std::max(a_gate, entry.level);
			if (auto* sub = entry.form->As<RE::TESLevItem>()) {
				Collect(sub, level, a_out, a_seen);
			} else {
				a_out.push_back({ entry.form, level, std::max<std::uint16_t>(entry.count, 1) });
			}
		}
	}

	const std::vector<Leaf>& GetPool(const RE::TESLeveledList* a_list)
	{
		auto it = g_pools.find(a_list);
		if (it != g_pools.end()) {
			return it->second;
		}
		std::vector<Leaf> pool;
		std::unordered_set<const RE::TESLeveledList*> seen;
		Collect(a_list, 0, pool, seen);
		return g_pools.emplace(a_list, std::move(pool)).first->second;
	}

	void ProcessResults(RE::TESLevItem* a_lev, std::uint16_t a_level, RE::BSScrapArray<RE::CALCED_OBJECT>& a_results,
		std::uint32_t a_from)
	{
		std::scoped_lock guard(g_lock);
		if (g_wanted.empty()) {
			return;
		}
		const auto& pool = GetPool(a_lev);

		std::vector<const Leaf*> relics;
		for (const auto& leaf : pool) {
			if (leaf.level <= a_level && g_wanted.contains(leaf.form->GetFormID())) {
				relics.push_back(&leaf);
			}
		}
		if (relics.empty()) {
			return;
		}

		std::uniform_real_distribution<float> chance(0.0f, 100.0f);
		for (auto i = a_from; i < a_results.size(); ++i) {
			auto& result = a_results[i];
			if (!result.form || g_wanted.contains(result.form->GetFormID())) {
				continue;  // already a relic (or nothing)
			}
			if (chance(g_rng) >= g_config.rerollChance) {
				continue;
			}
			std::vector<const Leaf*> options;
			for (auto* r : relics) {
				if (!g_config.sameType || r->form->GetFormType() == result.form->GetFormType()) {
					options.push_back(r);
				}
			}
			if (options.empty()) {
				continue;
			}
			const auto* pick = options[std::uniform_int_distribution<std::size_t>(0, options.size() - 1)(g_rng)];
			if (g_config.verbose) {
				SKSE::log::info("LVLI {:08X}: {:08X} '{}' -> relic {:08X} '{}'", a_lev->GetFormID(),
					result.form->GetFormID(), result.form->GetName(), pick->form->GetFormID(), pick->form->GetName());
			}
			result.form = pick->form;
			result.count = pick->count;
		}
	}

	// ------------------------------------------------------------------------
	// Hook: TESLeveledList::CalculateCurrentFormList
	// ------------------------------------------------------------------------
	namespace LevListHook
	{
		using Func_t = void (*)(RE::TESLeveledList*, std::uint16_t, std::int16_t,
			RE::BSScrapArray<RE::CALCED_OBJECT>&, std::uint32_t, bool);

		Func_t original = nullptr;
		thread_local int t_depth = 0;  // the function recurses into sub-lists; act on the outer call only

		void Hooked(RE::TESLeveledList* a_list, std::uint16_t a_level, std::int16_t a_count,
			RE::BSScrapArray<RE::CALCED_OBJECT>& a_results, std::uint32_t a_arg5, bool a_usePlayerLevel)
		{
			const auto before = a_results.size();
			++t_depth;
			original(a_list, a_level, a_count, a_results, a_arg5, a_usePlayerLevel);
			--t_depth;

			if (t_depth != 0 || !a_list || !g_lists[kNew]) {
				return;
			}
			QueueRefresh();
			if (a_results.size() == before) {
				return;
			}
			auto* lev = skyrim_cast<RE::TESLevItem*>(a_list);
			if (!lev) {
				return;  // leveled characters / spells
			}
			ProcessResults(lev, a_level, a_results, before);
		}

		bool Install()
		{
			// SE 1.5.97: 14579, AE 1.6.x: 14751 (Address Library IDs, from CommonLibSSE-NG).
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(14579, 14751) };
			original = reinterpret_cast<Func_t>(target.address());

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(reinterpret_cast<PVOID*>(&original), reinterpret_cast<PVOID>(&Hooked));
			const auto result = DetourTransactionCommit();
			if (result != NO_ERROR) {
				SKSE::log::error("Failed to hook CalculateCurrentFormList (Detours error {})", result);
				return false;
			}
			SKSE::log::info("Hooked CalculateCurrentFormList at 0x{:X}", target.address());
			return true;
		}
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kDataLoaded:
			ResolveLists();
			break;
		case SKSE::MessagingInterface::kPostLoadGame:
		case SKSE::MessagingInterface::kNewGame:
			RefreshWanted(true);
			break;
		default:
			break;
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 1, 0, 0 },
	.Name = "RelicHunter"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("RelicHunter v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	g_config.Load();
	SKSE::log::info("Config: rerollChance={} includeFound={} sameType={} verbose={}", g_config.rerollChance,
		g_config.includeFound, g_config.sameType, g_config.verbose);

	if (auto* messaging = SKSE::GetMessagingInterface()) {
		messaging->RegisterListener(OnMessage);
	}
	LevListHook::Install();
	return true;
}
