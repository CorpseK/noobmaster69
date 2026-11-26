// SpellBuffs - SKSE plugin (no ESP) that strengthens vanilla spells.
//
// Feature 1: Paralysis -> while the target is paralyzed, every pickpocket
//            attempt on it has a 100% success chance.

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

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
		auto path = *logDir / "SpellBuffs.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%H:%M:%S] [%l] %v");
	}

	bool IsParalyzed(RE::Actor* a_actor)
	{
		if (!a_actor) {
			return false;
		}
		// The Paralysis magic effect archetype drives the Paralysis actor value above 0.
		return a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kParalysis) > 0.0f;
	}

	// --- Hook: AIFormulas::ComputePickpocketSuccess ---------------------------
	// Returns the pickpocket success chance (0-100). It is used both for the
	// "Steal X%" number in the menu and for the actual roll.
	namespace PickpocketHook
	{
		using Func_t = std::int32_t (*)(float a_thiefSkill, float a_targetSkill, std::uint32_t a_valueStolen,
			float a_weightStolen, RE::Actor* a_thief, RE::Actor* a_target, bool a_isDetected,
			RE::TESForm* a_item);

		Func_t original = nullptr;

		std::int32_t Hooked(float a_thiefSkill, float a_targetSkill, std::uint32_t a_valueStolen,
			float a_weightStolen, RE::Actor* a_thief, RE::Actor* a_target, bool a_isDetected,
			RE::TESForm* a_item)
		{
			if (a_thief && a_thief == RE::PlayerCharacter::GetSingleton() && IsParalyzed(a_target)) {
				return 100;
			}
			return original(a_thiefSkill, a_targetSkill, a_valueStolen, a_weightStolen, a_thief, a_target,
				a_isDetected, a_item);
		}

		bool Install()
		{
			// SE 1.5.97: 25822, AE 1.6.x: 26379 (Address Library IDs, from CommonLibSSE-NG).
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(25822, 26379) };
			original = reinterpret_cast<Func_t>(target.address());

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(reinterpret_cast<PVOID*>(&original), reinterpret_cast<PVOID>(&Hooked));
			const auto result = DetourTransactionCommit();
			if (result != NO_ERROR) {
				SKSE::log::error("Failed to hook ComputePickpocketSuccess (Detours error {})", result);
				return false;
			}
			SKSE::log::info("Hooked ComputePickpocketSuccess at 0x{:X}", target.address());
			return true;
		}
	}

	// --- Hook: TESNPC::Activate / GetActivateText (vtable 0x37 / 0x4C) ----------
	// The game refuses to activate an actor that is ragdolled by paralysis, so the
	// pickpocket menu never opens. While the player sneaks, open it ourselves.
	namespace ActivateHook
	{
		bool CanPickpocketParalyzed(RE::Actor* a_target)
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			return a_target && player && a_target != player && !a_target->IsDead() && IsParalyzed(a_target) &&
			       player->IsSneaking();
		}

		RE::Actor* TargetFromArg(RE::TESObjectREFR* a_ref)
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (a_ref && a_ref != player) {
				return a_ref->As<RE::Actor>();
			}
			// Some callers pass the activator; fall back to what the crosshair points at.
			if (auto* pick = RE::CrosshairPickData::GetSingleton()) {
				auto ref = pick->target.get();
				if (!ref) {
					ref = pick->targetActor.get();
				}
				return ref ? ref->As<RE::Actor>() : nullptr;
			}
			return nullptr;
		}

		bool Activate(RE::TESNPC* a_this, RE::TESObjectREFR* a_targetRef, RE::TESObjectREFR* a_activatorRef,
			std::uint8_t a_arg3, RE::TESBoundObject* a_object, std::int32_t a_targetCount);
		REL::Relocation<decltype(Activate)> activateFunc;

		bool Activate(RE::TESNPC* a_this, RE::TESObjectREFR* a_targetRef, RE::TESObjectREFR* a_activatorRef,
			std::uint8_t a_arg3, RE::TESBoundObject* a_object, std::int32_t a_targetCount)
		{
			if (a_activatorRef && a_activatorRef == RE::PlayerCharacter::GetSingleton()) {
				auto* target = a_targetRef ? a_targetRef->As<RE::Actor>() : nullptr;
				if (CanPickpocketParalyzed(target)) {
					SKSE::log::info("Pickpocket on paralyzed {:08X} '{}'", target->GetFormID(), target->GetName());
					RE::ContainerMenu::OpenMenu(target, RE::ContainerMenu::ContainerMode::kPickpocket);
					return true;
				}
			}
			return activateFunc(a_this, a_targetRef, a_activatorRef, a_arg3, a_object, a_targetCount);
		}

		bool GetActivateText(RE::TESNPC* a_this, RE::TESObjectREFR* a_ref, RE::BSString& a_dst);
		REL::Relocation<decltype(GetActivateText)> textFunc;

		bool GetActivateText(RE::TESNPC* a_this, RE::TESObjectREFR* a_ref, RE::BSString& a_dst)
		{
			const bool res = textFunc(a_this, a_ref, a_dst);
			if (auto* target = TargetFromArg(a_ref); target && target->GetActorBase() == a_this &&
			                                         CanPickpocketParalyzed(target)) {
				const char* label = "Pickpocket";
				if (auto* gmst = RE::GameSettingCollection::GetSingleton()) {
					if (auto* s = gmst->GetSetting("sPickpocket"); s && s->GetString() && *s->GetString()) {
						label = s->GetString();
					}
				}
				a_dst = (std::string(label) + "\n" + target->GetName()).c_str();
				return true;
			}
			return res;
		}

		void Install()
		{
			REL::Relocation<std::uintptr_t> vt{ RE::TESNPC::VTABLE[0] };
			activateFunc = vt.write_vfunc(0x37, Activate);
			textFunc = vt.write_vfunc(0x4C, GetActivateText);
			SKSE::log::info("Hooked TESNPC::Activate / GetActivateText");
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 2, 0, 0 },
	.Name = "SpellBuffs"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("SpellBuffs v0.2.0 loading (game {})", a_skse->RuntimeVersion().string());

	PickpocketHook::Install();
	ActivateHook::Install();
	return true;
}
