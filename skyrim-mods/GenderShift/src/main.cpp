// GenderShift - SKSE plugin (no ESP): changes the male/female ratio of NPCs
// that the game picks from leveled lists (bandits, Forsworn, Vigilants, ...).
//
// How it works:
//   Leveled actors (an ACHR whose base is a LVLN such as LCharForswornMelee)
//   are resolved by TESLeveledList::CalculateCurrentFormList. We hook it and,
//   after the game has picked an NPC, decide the wanted sex. If the pick has the
//   wrong sex we replace it with an opposite-sex NPC taken from the *same*
//   leveled list tree (same race, same class, level <= the original pick).
//   Lists that only contain one sex (e.g. Briarheart) are therefore untouched.
//   Placed unique NPCs never go through a leveled list and are never touched.
//
//   Nothing is written to the save: once an actor is resolved the game keeps
//   it; after removing the plugin, new resolutions are vanilla again.
//
// Config: Data/SKSE/Plugins/GenderShift.ini

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

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
		auto path = *logDir / "GenderShift.log";
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
	enum class Mode : int
	{
		kPerNPC = 0,    // every spawned NPC rolls on its own
		kPerGroup = 1,  // NPCs spawned close together in time (one camp) share one roll
	};

	struct Config
	{
		Mode mode = Mode::kPerGroup;
		float femaleChance = 50.0f;  // 0..100
		int groupWindowMs = 2000;
		bool sameClass = true;
		bool onlyHumanoid = true;
		bool verbose = false;

		void Load()
		{
			const auto path =
				(std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "GenderShift.ini").string();
			auto readInt = [&](const char* a_key, int a_default) {
				return static_cast<int>(GetPrivateProfileIntA("General", a_key, a_default, path.c_str()));
			};
			auto readFloat = [&](const char* a_key, float a_default) {
				char buf[64]{};
				GetPrivateProfileStringA("General", a_key, "", buf, sizeof(buf), path.c_str());
				if (buf[0] == '\0') {
					return a_default;
				}
				try {
					return std::stof(buf);
				} catch (...) {
					return a_default;
				}
			};

			mode = readInt("iMode", 1) == 0 ? Mode::kPerNPC : Mode::kPerGroup;
			femaleChance = std::clamp(readFloat("fFemaleChance", femaleChance), 0.0f, 100.0f);
			groupWindowMs = std::max(0, readInt("iGroupWindowMs", groupWindowMs));
			sameClass = readInt("bSameClass", 1) != 0;
			onlyHumanoid = readInt("bOnlyHumanoid", 1) != 0;
			verbose = readInt("bVerboseLog", 0) != 0;
		}
	};

	Config g_config;

	// ------------------------------------------------------------------------
	// NPC helpers
	// ------------------------------------------------------------------------
	using TPL = RE::ACTOR_BASE_DATA::TEMPLATE_USE_FLAG;

	// Follows "Use Traits"/"Use Stats" templates to the NPC that really owns the data.
	// Returns nullptr when the template is a leveled list (data unknown until spawn).
	RE::TESNPC* ResolveTemplate(RE::TESNPC* a_npc, TPL a_flag)
	{
		for (int i = 0; a_npc && i < 16; ++i) {
			if (!a_npc->UsesTemplate() || !a_npc->actorData.templateUseFlags.all(a_flag)) {
				return a_npc;
			}
			auto* tpl = a_npc->baseTemplateForm;
			if (!tpl) {
				return a_npc;
			}
			a_npc = tpl->As<RE::TESNPC>();
		}
		return nullptr;
	}

	struct Candidate
	{
		RE::TESNPC* npc = nullptr;
		RE::TESRace* race = nullptr;
		RE::TESClass* npcClass = nullptr;
		std::uint16_t level = 0;  // effective entry level inside the list tree
		bool female = false;
	};

	bool Describe(RE::TESNPC* a_npc, std::uint16_t a_level, Candidate& a_out)
	{
		if (!a_npc || a_npc->IsUnique()) {
			return false;
		}
		auto* traits = ResolveTemplate(a_npc, TPL::kTraits);
		if (!traits || !traits->race) {
			return false;
		}
		if (g_config.onlyHumanoid) {
			// ActorTypeNPC [KYWD:00013794] - humanoid races (no animals, children have it too
			// but they are matched by race so they never swap with adults).
			if (!traits->race->HasKeywordID(0x00013794)) {
				return false;
			}
		}
		auto* stats = ResolveTemplate(a_npc, TPL::kStats);
		a_out.npc = a_npc;
		a_out.race = traits->race;
		a_out.npcClass = stats ? stats->npcClass : nullptr;
		a_out.level = a_level;
		a_out.female = traits->IsFemale();
		return true;
	}

	// ------------------------------------------------------------------------
	// Candidate pool per top-level leveled list (built lazily, cached)
	// ------------------------------------------------------------------------
	std::mutex g_lock;  // guards everything below
	std::unordered_map<const RE::TESLeveledList*, std::vector<Candidate>> g_pools;
	std::mt19937 g_rng{ std::random_device{}() };
	std::chrono::steady_clock::time_point g_groupLast{};
	bool g_groupFemale = false;
	bool g_groupValid = false;

	void Collect(const RE::TESLeveledList* a_list, std::uint16_t a_gate, std::vector<Candidate>& a_out,
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
			if (auto* sub = entry.form->As<RE::TESLevCharacter>()) {
				Collect(sub, level, a_out, a_seen);
			} else if (auto* npc = entry.form->As<RE::TESNPC>()) {
				Candidate c;
				if (Describe(npc, level, c)) {
					a_out.push_back(c);
				}
			}
		}
	}

	const std::vector<Candidate>& GetPool(const RE::TESLeveledList* a_list)
	{
		auto it = g_pools.find(a_list);
		if (it != g_pools.end()) {
			return it->second;
		}
		std::vector<Candidate> pool;
		std::unordered_set<const RE::TESLeveledList*> seen;
		Collect(a_list, 0, pool, seen);
		return g_pools.emplace(a_list, std::move(pool)).first->second;
	}

	bool RollFemale()
	{
		std::uniform_real_distribution<float> dist(0.0f, 100.0f);
		if (g_config.mode == Mode::kPerNPC) {
			return dist(g_rng) < g_config.femaleChance;
		}
		const auto now = std::chrono::steady_clock::now();
		const auto window = std::chrono::milliseconds(g_config.groupWindowMs);
		if (!g_groupValid || now - g_groupLast > window) {
			g_groupFemale = dist(g_rng) < g_config.femaleChance;
			g_groupValid = true;
			if (g_config.verbose) {
				SKSE::log::info("New group: {}", g_groupFemale ? "female" : "male");
			}
		}
		g_groupLast = now;  // sliding window: a camp loading over several frames stays one group
		return g_groupFemale;
	}

	RE::TESNPC* PickReplacement(const std::vector<Candidate>& a_pool, const Candidate& a_orig, bool a_wantFemale)
	{
		// Best level <= the original pick's level so the swap never spawns a stronger enemy.
		int bestLevel = -1;
		std::vector<RE::TESNPC*> best;
		for (const auto& c : a_pool) {
			if (c.female != a_wantFemale || c.race != a_orig.race || c.level > a_orig.level) {
				continue;
			}
			if (g_config.sameClass && c.npcClass != a_orig.npcClass) {
				continue;
			}
			if (c.level > bestLevel) {
				bestLevel = c.level;
				best.clear();
			}
			if (c.level == bestLevel) {
				best.push_back(c.npc);
			}
		}
		if (best.empty()) {
			return nullptr;
		}
		std::uniform_int_distribution<std::size_t> dist(0, best.size() - 1);
		return best[dist(g_rng)];
	}

	void ProcessResults(RE::TESLevCharacter* a_lev, RE::BSScrapArray<RE::CALCED_OBJECT>& a_results,
		std::uint32_t a_from)
	{
		std::scoped_lock guard(g_lock);
		const auto& pool = GetPool(a_lev);
		if (pool.empty()) {
			return;
		}
		bool hasMale = false, hasFemale = false;
		for (const auto& c : pool) {
			(c.female ? hasFemale : hasMale) = true;
		}
		if (!hasMale || !hasFemale) {
			return;  // single-sex list (Briarheart, ...): nothing to do
		}

		for (auto i = a_from; i < a_results.size(); ++i) {
			auto& result = a_results[i];
			auto* npc = result.form ? result.form->As<RE::TESNPC>() : nullptr;
			if (!npc) {
				continue;
			}
			// Find the pick's own entry in the pool (gives its level inside this tree).
			const Candidate* orig = nullptr;
			for (const auto& c : pool) {
				if (c.npc == npc && (!orig || c.level > orig->level)) {
					orig = &c;
				}
			}
			if (!orig) {
				continue;
			}
			const bool wantFemale = RollFemale();
			if (orig->female == wantFemale) {
				continue;
			}
			auto* repl = PickReplacement(pool, *orig, wantFemale);
			if (!repl) {
				if (g_config.verbose) {
					SKSE::log::info("{:08X} '{}' kept: no {} counterpart in list {:08X}", npc->GetFormID(),
						npc->GetName(), wantFemale ? "female" : "male", a_lev->GetFormID());
				}
				continue;
			}
			if (g_config.verbose) {
				SKSE::log::info("{:08X} '{}' -> {:08X} '{}' ({})", npc->GetFormID(), npc->GetName(),
					repl->GetFormID(), repl->GetName(), wantFemale ? "female" : "male");
			}
			result.form = repl;
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

			if (t_depth != 0 || !a_list || a_results.size() == before) {
				return;
			}
			// Only leveled *character* lists (items/spells also use this function).
			auto* lev = skyrim_cast<RE::TESLevCharacter*>(a_list);
			if (!lev) {
				return;
			}
			ProcessResults(lev, a_results, before);
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
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 1, 0, 0 },
	.Name = "GenderShift"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("GenderShift v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	g_config.Load();
	SKSE::log::info("Config: mode={} femaleChance={} groupWindowMs={} sameClass={} onlyHumanoid={} verbose={}",
		g_config.mode == Mode::kPerNPC ? "per-NPC" : "per-group", g_config.femaleChance, g_config.groupWindowMs,
		g_config.sameClass, g_config.onlyHumanoid, g_config.verbose);

	LevListHook::Install();
	return true;
}
