// DOMTalkGate - SKSE plugin (no ESP), add-on for Diary of Mine (DiaryOfMine.esm).
//
// DOM's dialogue for NPCs that are NOT captured (the "serious chat" / order / praise branches)
// is shown to every NPC, picked by GetRelationshipRank. This plugin makes those lines appear only
// for a short window after you cast DOM's Training Info spell (or power) on that NPC.
//
// How (all in memory, nothing saved, nothing edited on disk):
//   1. DOM's info spells apply DOMTrainingInfoEffect [DiaryOfMine.esm 0x5BD55A] for 1 second.
//      We raise that duration to fWindowSeconds (default 30). The effect's script still runs once
//      (it shows the training message as before); the effect simply stays on the NPC longer.
//   2. For every top-level dialogue branch of DiaryOfMine.esm, each opening line that is meant for
//      non-captured NPCs (condition: Subject.GetInFaction(DOMPlayerSlaveFaction) == 0, and not a
//      quest line using GetIsID/GetStage) gets one extra condition at the front:
//          Subject.HasMagicEffect(DOMTrainingInfoEffect) == 1   (AND)
//      The relationship-rank conditions stay, so DOM still picks the line that fits the NPC.
//
// Config: Data/SKSE/Plugins/DOMTalkGate.ini

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
		auto path = *logDir / "DOMTalkGate.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%H:%M:%S] [%l] %v");
	}

	struct Config
	{
		bool gateDialogue = true;
		int windowSeconds = 30;
		bool verbose = false;

		void Load()
		{
			const auto path =
				(std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "DOMTalkGate.ini").string();
			gateDialogue = GetPrivateProfileIntA("General", "bGateNPCDialogue", 1, path.c_str()) != 0;
			windowSeconds = std::clamp(
				static_cast<int>(GetPrivateProfileIntA("General", "iWindowSeconds", windowSeconds, path.c_str())), 1,
				3600);
			verbose = GetPrivateProfileIntA("General", "bVerboseLog", 0, path.c_str()) != 0;
		}
	};

	Config g_config;

	constexpr auto kDomFile = "DiaryOfMine.esm"sv;
	constexpr RE::FormID kInfoEffect = 0x5BD55A;          // DOMTrainingInfoEffect (MGEF)
	constexpr RE::FormID kPlayerSlaveFaction = 0x56C505;  // DOMPlayerSlaveFaction

	using FID = RE::FUNCTION_DATA::FunctionID;
	using OpCode = RE::CONDITION_ITEM_DATA::OpCode;
	using Obj = RE::CONDITIONITEMOBJECT;

	// Raise the duration of every spell effect that uses DOM's info effect.
	std::size_t ExtendInfoSpells(RE::EffectSetting* a_effect)
	{
		std::size_t changed = 0;
		for (auto* spell : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::SpellItem>()) {
			if (!spell) {
				continue;
			}
			for (auto* eff : spell->effects) {
				if (eff && eff->baseEffect == a_effect) {
					if (g_config.verbose) {
						SKSE::log::info("Spell {:08X} '{}': info effect duration {} -> {}", spell->GetFormID(),
							spell->GetName(), eff->effectItem.duration, g_config.windowSeconds);
					}
					eff->effectItem.duration = static_cast<std::uint32_t>(g_config.windowSeconds);
					++changed;
				}
			}
		}
		return changed;
	}

	// An opening line for non-captured NPCs: Subject not in DOMPlayerSlaveFaction, not a quest line.
	bool IsFreeNPCLine(const RE::TESTopicInfo* a_info, const RE::TESFaction* a_slaveFaction)
	{
		bool notSlave = false;
		for (auto* item = a_info->objConditions.head; item; item = item->next) {
			const auto& d = item->data;
			const auto fn = d.functionData.function.get();
			if (fn == FID::kGetIsID || fn == FID::kGetStage) {
				return false;  // quest-specific line (Kimli, Agnis...) - leave alone
			}
			if (fn == FID::kGetInFaction && d.object.get() == Obj::kSelf && !d.flags.global &&
				d.flags.opCode == OpCode::kEqualTo && d.comparisonValue.f == 0.0f &&
				d.functionData.params[0] == a_slaveFaction) {
				notSlave = true;
			}
		}
		return notSlave;
	}

	bool AlreadyGated(const RE::TESTopicInfo* a_info, const RE::EffectSetting* a_effect)
	{
		for (auto* item = a_info->objConditions.head; item; item = item->next) {
			if (item->data.functionData.function.get() == FID::kHasMagicEffect &&
				item->data.functionData.params[0] == a_effect) {
				return true;
			}
		}
		return false;
	}

	void PrependGate(RE::TESTopicInfo* a_info, RE::EffectSetting* a_effect)
	{
		auto* gate = new RE::TESConditionItem();  // game heap (TES_HEAP_REDEFINE_NEW)
		gate->data.object = Obj::kSelf;
		gate->data.functionData.function = FID::kHasMagicEffect;
		gate->data.functionData.params[0] = a_effect;
		gate->data.flags.opCode = OpCode::kEqualTo;
		gate->data.flags.isOR = false;  // AND with everything that follows
		gate->data.comparisonValue.f = 1.0f;
		gate->next = a_info->objConditions.head;
		a_info->objConditions.head = gate;
	}

	std::size_t GateDialogue(const RE::TESFile* a_domFile, RE::EffectSetting* a_effect, RE::TESFaction* a_slaveFaction)
	{
		std::size_t gated = 0;
		for (auto* branch : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::BGSDialogueBranch>()) {
			if (!branch || branch->GetFile(0) != a_domFile ||
				!branch->flags.all(RE::BGSDialogueBranch::Flag::kTopLevel) || !branch->startingTopic) {
				continue;
			}
			auto* topic = branch->startingTopic;
			std::size_t here = 0;
			for (std::uint32_t i = 0; i < topic->numTopicInfos; ++i) {
				auto* info = topic->topicInfos ? topic->topicInfos[i] : nullptr;
				if (!info || !IsFreeNPCLine(info, a_slaveFaction) || AlreadyGated(info, a_effect)) {
					continue;
				}
				PrependGate(info, a_effect);
				++here;
			}
			if (here > 0) {
				SKSE::log::info("Branch {:08X} '{}': gated {} opening line(s)", branch->GetFormID(),
					branch->GetFormEditorID(), here);
			}
			gated += here;
		}
		return gated;
	}

	void Init()
	{
		auto* dh = RE::TESDataHandler::GetSingleton();
		const auto* file = dh ? dh->LookupModByName(kDomFile) : nullptr;
		if (!file || file->GetCompileIndex() == 0xFF) {
			SKSE::log::warn("{} not loaded -> DOMTalkGate does nothing", kDomFile);
			return;
		}
		auto* effect = dh->LookupForm<RE::EffectSetting>(kInfoEffect, kDomFile);
		auto* slaveFaction = dh->LookupForm<RE::TESFaction>(kPlayerSlaveFaction, kDomFile);
		if (!effect || !slaveFaction) {
			SKSE::log::error("DOMTrainingInfoEffect ({}) or DOMPlayerSlaveFaction ({}) not found in {} - "
							 "different DOM version? DOMTalkGate does nothing",
				effect ? "ok" : "missing", slaveFaction ? "ok" : "missing", kDomFile);
			return;
		}

		const auto spells = ExtendInfoSpells(effect);
		SKSE::log::info("Info effect now lasts {} s on {} spell effect(s)", g_config.windowSeconds, spells);

		if (g_config.gateDialogue) {
			const auto gated = GateDialogue(file, effect, slaveFaction);
			SKSE::log::info("Gated {} DOM opening line(s) behind the Training Info spell", gated);
		}
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
			Init();
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 1, 0, 0 },
	.Name = "DOMTalkGate"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("DOMTalkGate v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	g_config.Load();
	SKSE::log::info("Config: gateNPCDialogue={} windowSeconds={} verbose={}", g_config.gateDialogue,
		g_config.windowSeconds, g_config.verbose);

	if (auto* messaging = SKSE::GetMessagingInterface()) {
		messaging->RegisterListener(OnMessage);
	}
	return true;
}
