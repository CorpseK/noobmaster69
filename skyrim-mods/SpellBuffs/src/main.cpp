// SpellBuffs - SKSE plugin (no ESP) that strengthens vanilla spells.
//
// Feature 1: Paralysis -> while the target is paralyzed, every pickpocket
//            attempt on it has a 100% success chance.

#include <Windows.h>
#include <detours/detours.h>

#include <spdlog/sinks/basic_file_sink.h>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

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
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 1, 0, 0 },
	.Name = "SpellBuffs"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("SpellBuffs v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	PickpocketHook::Install();
	return true;
}
