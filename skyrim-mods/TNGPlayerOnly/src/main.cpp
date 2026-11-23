// TNGPlayerOnly - SKSE plugin (no ESP), add-on for The New Gentleman (TNG).
// Keeps TNG's genital addon on the PLAYER only; every NPC keeps its original skin.
//
// How TNG distributes (read from its MIT source, github.com/ModiLogist/TheNewGentleman):
//   * At kDataLoaded it duplicates each race's skin ARMO, adds the addon ARMA +
//     slot 52, tags the copy with keyword TNG_TngSkin [TheNewGentleman.esp 0xFF6]
//     and sets race->skin to the copy. NPCs with a custom skin get the same
//     treatment on npc->skin.
//   * At runtime (Character::Load3D hook + equip/load/race-switch events) it calls
//     UpdateActor, which can give NPCs a TNG skin again (saved choices, random
//     "Gentlewoman" chance, MCM changes).
//
// What we do:
//   1. One frame after kDataLoaded (TNG is done by then): give the player base a
//      TNG skin of its own, then put every race and every non-player NPC back on
//      its original skin. TNG never edits the original skin, so nothing is lost.
//   2. Wrap TNG's Load3D hook and listen to the same events; whenever an NPC (or
//      its race) ends up with a TNG skin again, revert it and refresh its 3D.
//   The player is never touched, so TNG (MCM, size, addon choice) works as usual.
//
// The TNG skins are never modified (the player still uses them). Nothing is saved.
//
// Config: Data/SKSE/Plugins/TNGPlayerOnly.ini

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <algorithm>
#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

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
		auto path = *logDir / "TNGPlayerOnly.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%H:%M:%S] [%l] %v");
	}

	struct Config
	{
		bool enabled = true;
		bool keepPlayer = true;  // make sure the player keeps a TNG skin
		bool verbose = false;

		void Load()
		{
			const auto path =
				(std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "TNGPlayerOnly.ini").string();
			enabled = GetPrivateProfileIntA("General", "bEnabled", 1, path.c_str()) != 0;
			keepPlayer = GetPrivateProfileIntA("General", "bKeepPlayer", 1, path.c_str()) != 0;
			verbose = GetPrivateProfileIntA("General", "bVerboseLog", 0, path.c_str()) != 0;
		}
	};

	Config g_config;

	constexpr auto kTngFile = "TheNewGentleman.esp"sv;
	constexpr RE::FormID kTngSkinKw = 0xFF6;  // TNG_TngSkin
	constexpr auto kGenitalSlot = RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary;  // slot 52

	RE::BGSKeyword* g_tngSkinKw = nullptr;
	bool g_ready = false;

	std::mutex g_lock;  // Load3D can run on a background loading thread
	// Original-skin index: (race, sorted non-genital ARMA list) -> original ARMO.
	std::map<std::pair<RE::TESRace*, std::vector<RE::TESObjectARMA*>>, RE::TESObjectARMO*> g_ogIndex;
	std::unordered_map<RE::TESObjectARMO*, RE::TESObjectARMO*> g_ogCache;  // TNG skin -> original
	std::unordered_map<RE::TESRace*, RE::TESObjectARMO*> g_raceTngSkin;      // last TNG skin seen per race

	bool IsTngSkin(const RE::TESObjectARMO* a_skin)
	{
		return a_skin && g_tngSkinKw && a_skin->HasKeyword(g_tngSkinKw);
	}

	std::vector<RE::TESObjectARMA*> BodyAddons(const RE::TESObjectARMO* a_skin)
	{
		std::vector<RE::TESObjectARMA*> res;
		for (auto* aa : a_skin->armorAddons) {
			if (aa && !aa->HasPartOf(kGenitalSlot)) {
				res.push_back(aa);
			}
		}
		std::sort(res.begin(), res.end());
		res.erase(std::unique(res.begin(), res.end()), res.end());
		return res;
	}

	void BuildOgIndex()
	{
		for (auto* armo : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESObjectARMO>()) {
			if (!armo || IsTngSkin(armo) || armo->HasPartOf(kGenitalSlot) || armo->armorAddons.empty()) {
				continue;
			}
			g_ogIndex.try_emplace({ armo->race, BodyAddons(armo) }, armo);
		}
		SKSE::log::info("Indexed {} candidate original skins", g_ogIndex.size());
	}

	// TNG copies the original skin (same race, same ARMAs) and only appends the genital ARMA.
	RE::TESObjectARMO* OriginalOf(RE::TESObjectARMO* a_tngSkin)
	{
		if (auto it = g_ogCache.find(a_tngSkin); it != g_ogCache.end()) {
			return it->second;
		}
		RE::TESObjectARMO* og = nullptr;
		if (auto it = g_ogIndex.find({ a_tngSkin->race, BodyAddons(a_tngSkin) }); it != g_ogIndex.end()) {
			og = it->second;
		}
		if (!og) {
			SKSE::log::warn("No original skin found for TNG skin {:08X} '{}' - left as is", a_tngSkin->GetFormID(),
				a_tngSkin->GetName());
		}
		g_ogCache.emplace(a_tngSkin, og);
		return og;
	}

	bool IsPlayerBase(const RE::TESNPC* a_npc)
	{
		return a_npc && a_npc->GetFormID() == 0x7;
	}

	// Returns true if something changed. Caller holds g_lock.
	bool StripRace(RE::TESRace* a_race)
	{
		if (!a_race || !IsTngSkin(a_race->skin)) {
			return false;
		}
		auto* og = OriginalOf(a_race->skin);
		if (!og) {
			return false;
		}
		g_raceTngSkin[a_race] = a_race->skin;
		a_race->skin = og;
		return true;
	}

	bool StripNPC(RE::TESNPC* a_npc)
	{
		if (!a_npc || IsPlayerBase(a_npc)) {
			return false;
		}
		bool changed = StripRace(a_npc->race);
		if (IsTngSkin(a_npc->skin)) {
			if (auto* og = OriginalOf(a_npc->skin)) {
				a_npc->skin = (a_npc->race && og == a_npc->race->skin) ? nullptr : og;
				changed = true;
			}
		}
		return changed;
	}

	bool EnsurePlayer()
	{
		if (!g_config.keepPlayer) {
			return false;
		}
		auto* pc = RE::PlayerCharacter::GetSingleton();
		auto* base = pc ? pc->GetActorBase() : nullptr;
		if (!base || !base->race || IsTngSkin(base->skin)) {
			return false;
		}
		auto it = g_raceTngSkin.find(base->race);
		// Only replace an empty skin: a custom (non-TNG) player skin belongs to TNG to handle.
		if (it == g_raceTngSkin.end() || base->skin) {
			return false;
		}
		base->skin = it->second;
		SKSE::log::info("Player base now uses TNG skin {:08X}", it->second->GetFormID());
		return true;
	}

	void Refresh3D(RE::ObjectRefHandle a_handle)
	{
		SKSE::GetTaskInterface()->AddTask([a_handle] {
			auto ref = a_handle.get();
			auto* actor = ref ? ref->As<RE::Actor>() : nullptr;
			if (actor && actor->Is3DLoaded()) {
				actor->Update3DModel();
			}
		});
	}

	void FixActor(RE::Actor* a_actor, bool a_refresh)
	{
		if (!g_ready || !a_actor) {
			return;
		}
		bool changed = false;
		{
			std::scoped_lock guard(g_lock);
			if (a_actor == RE::PlayerCharacter::GetSingleton()) {
				changed = EnsurePlayer();
			} else {
				changed = StripNPC(a_actor->GetActorBase());
			}
		}
		if (changed) {
			if (g_config.verbose) {
				SKSE::log::info("Reverted TNG skin on {:08X} '{}'", a_actor->GetFormID(), a_actor->GetName());
			}
			if (a_refresh) {
				Refresh3D(a_actor->GetHandle());
			}
		}
	}

	void InitialPass()
	{
		std::scoped_lock guard(g_lock);
		BuildOgIndex();

		// Remember the race skins first so the player can keep one.
		std::size_t races = 0, npcs = 0;
		for (auto* race : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESRace>()) {
			if (race && IsTngSkin(race->skin)) {
				g_raceTngSkin[race] = race->skin;
			}
		}
		if (g_config.keepPlayer) {
			if (auto* pcBase = RE::TESForm::LookupByID<RE::TESNPC>(0x7); pcBase && pcBase->race && !pcBase->skin) {
				if (auto it = g_raceTngSkin.find(pcBase->race); it != g_raceTngSkin.end()) {
					pcBase->skin = it->second;
				}
			}
		}
		for (auto* race : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESRace>()) {
			races += StripRace(race) ? 1 : 0;
		}
		for (auto* npc : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESNPC>()) {
			npcs += StripNPC(npc) ? 1 : 0;
		}
		SKSE::log::info("Initial pass: reverted {} race skins and {} NPC skins", races, npcs);
	}

	// --- Hook: Character::Load3D (vtable 0x6A), installed after TNG's -----------
	namespace Load3DHook
	{
		RE::NiAVObject* thunk(RE::Character* a_actor, bool a_backgroundLoading);
		REL::Relocation<decltype(thunk)> func;

		RE::NiAVObject* thunk(RE::Character* a_actor, bool a_backgroundLoading)
		{
			auto* res = func(a_actor, a_backgroundLoading);  // vanilla + TNG's UpdateActor
			FixActor(a_actor, true);
			return res;
		}

		void Install()
		{
			REL::Relocation<std::uintptr_t> vt{ RE::Character::VTABLE[0] };
			func = vt.write_vfunc(0x6A, thunk);
			SKSE::log::info("Hooked Character::Load3D");
		}
	}

	// --- Events: TNG reacts to these too; we fix up one frame later -------------
	void DeferFix(RE::Actor* a_actor)
	{
		if (!a_actor) {
			return;
		}
		auto handle = a_actor->GetHandle();
		SKSE::GetTaskInterface()->AddTask([handle] {
			auto ref = handle.get();
			FixActor(ref ? ref->As<RE::Actor>() : nullptr, true);
		});
	}

	class EventSink :
		public RE::BSTEventSink<RE::TESEquipEvent>,
		public RE::BSTEventSink<RE::TESObjectLoadedEvent>,
		public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent>
	{
	public:
		static EventSink* GetSingleton()
		{
			static EventSink singleton;
			return &singleton;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event,
			RE::BSTEventSource<RE::TESEquipEvent>*) override
		{
			if (a_event && a_event->actor) {
				DeferFix(a_event->actor->As<RE::Actor>());
			}
			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* a_event,
			RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override
		{
			if (a_event && a_event->loaded) {
				DeferFix(RE::TESForm::LookupByID<RE::Actor>(a_event->formID));
			}
			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* a_event,
			RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override
		{
			if (a_event && a_event->subject) {
				DeferFix(a_event->subject->As<RE::Actor>());
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void Init()
	{
		auto* dh = RE::TESDataHandler::GetSingleton();
		const auto* file = dh ? dh->LookupModByName(kTngFile) : nullptr;
		if (!file || file->GetCompileIndex() == 0xFF) {
			SKSE::log::warn("{} not loaded -> TNGPlayerOnly does nothing", kTngFile);
			return;
		}
		g_tngSkinKw = dh->LookupForm<RE::BGSKeyword>(kTngSkinKw, kTngFile);
		if (!g_tngSkinKw) {
			SKSE::log::error("Keyword TNG_TngSkin (0xFF6) not found in {} -> TNGPlayerOnly does nothing", kTngFile);
			return;
		}

		InitialPass();
		g_ready = true;

		Load3DHook::Install();
		auto* holder = RE::ScriptEventSourceHolder::GetSingleton();
		holder->AddEventSink<RE::TESEquipEvent>(EventSink::GetSingleton());
		holder->AddEventSink<RE::TESObjectLoadedEvent>(EventSink::GetSingleton());
		holder->AddEventSink<RE::TESSwitchRaceCompleteEvent>(EventSink::GetSingleton());
		SKSE::log::info("TNGPlayerOnly ready");
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kDataLoaded:
			// Run one frame later so TNG's own kDataLoaded work (and its Load3D hook) is done.
			SKSE::GetTaskInterface()->AddTask(Init);
			break;
		case SKSE::MessagingInterface::kPostLoadGame:
		case SKSE::MessagingInterface::kNewGame:
			if (g_ready) {
				auto* pc = RE::PlayerCharacter::GetSingleton();
				FixActor(pc, true);
			}
			break;
		default:
			break;
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 1, 0, 0 },
	.Name = "TNGPlayerOnly"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("TNGPlayerOnly v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	g_config.Load();
	SKSE::log::info("Config: enabled={} keepPlayer={} verbose={}", g_config.enabled, g_config.keepPlayer,
		g_config.verbose);
	if (!g_config.enabled) {
		return true;
	}
	if (auto* messaging = SKSE::GetMessagingInterface()) {
		messaging->RegisterListener(OnMessage);
	}
	return true;
}
