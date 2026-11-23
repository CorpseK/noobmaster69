// KillRestore - SKSE plugin (no ESP): every time the player kills an actor,
// restore a percentage of the player's max Health / Magicka / Stamina.
// Purely event-driven, nothing is stored in the save.
//
// Config: Data/SKSE/Plugins/KillRestore.ini

#include <Windows.h>

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
		auto path = *logDir / "KillRestore.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%H:%M:%S] [%l] %v");
	}

	struct Config
	{
		float healthPct = 10.0f;
		float magickaPct = 10.0f;
		float staminaPct = 10.0f;
		bool onlyHostile = true;
		bool verbose = false;

		void Load()
		{
			const auto path =
				(std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "KillRestore.ini").string();
			auto readFloat = [&](const char* a_key, float a_default) {
				char buf[64]{};
				GetPrivateProfileStringA("General", a_key, "", buf, sizeof(buf), path.c_str());
				if (buf[0] == '\0') {
					return a_default;
				}
				try {
					return std::clamp(std::stof(buf), 0.0f, 100.0f);
				} catch (...) {
					return a_default;
				}
			};
			auto readBool = [&](const char* a_key, bool a_default) {
				return GetPrivateProfileIntA("General", a_key, a_default ? 1 : 0, path.c_str()) != 0;
			};

			healthPct = readFloat("fHealthPercent", healthPct);
			magickaPct = readFloat("fMagickaPercent", magickaPct);
			staminaPct = readFloat("fStaminaPercent", staminaPct);
			onlyHostile = readBool("bOnlyHostile", onlyHostile);
			verbose = readBool("bVerboseLog", verbose);
		}
	};

	Config g_config;

	void Restore(RE::Actor* a_actor, RE::ActorValue a_av, float a_pct)
	{
		if (a_pct <= 0.0f) {
			return;
		}
		auto* avOwner = a_actor->AsActorValueOwner();
		const float max = avOwner->GetPermanentActorValue(a_av);
		if (max <= 0.0f) {
			return;
		}
		// Restores damage only: never goes above the current maximum.
		avOwner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, a_av, max * a_pct / 100.0f);
	}

	class DeathSink : public RE::BSTEventSink<RE::TESDeathEvent>
	{
	public:
		static DeathSink* GetSingleton()
		{
			static DeathSink singleton;
			return &singleton;
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* a_event,
			RE::BSTEventSource<RE::TESDeathEvent>*) override
		{
			// The event fires twice per death (dying, then dead). React once.
			if (!a_event || !a_event->dead) {
				return RE::BSEventNotifyControl::kContinue;
			}

			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player || a_event->actorKiller.get() != player) {
				return RE::BSEventNotifyControl::kContinue;
			}

			auto* victim = a_event->actorDying ? a_event->actorDying->As<RE::Actor>() : nullptr;
			if (!victim || victim == player) {
				return RE::BSEventNotifyControl::kContinue;
			}
			if (g_config.onlyHostile && !victim->IsHostileToActor(player)) {
				if (g_config.verbose) {
					SKSE::log::info("Kill of non-hostile {:08X} ignored", victim->GetFormID());
				}
				return RE::BSEventNotifyControl::kContinue;
			}

			Restore(player, RE::ActorValue::kHealth, g_config.healthPct);
			Restore(player, RE::ActorValue::kMagicka, g_config.magickaPct);
			Restore(player, RE::ActorValue::kStamina, g_config.staminaPct);

			if (g_config.verbose) {
				SKSE::log::info("Player killed {:08X} '{}' -> restored", victim->GetFormID(), victim->GetName());
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
			g_config.Load();
			SKSE::log::info("Config: health={}% magicka={}% stamina={}% onlyHostile={} verbose={}",
				g_config.healthPct, g_config.magickaPct, g_config.staminaPct, g_config.onlyHostile, g_config.verbose);

			if (auto* events = RE::ScriptEventSourceHolder::GetSingleton()) {
				events->AddEventSink<RE::TESDeathEvent>(DeathSink::GetSingleton());
				SKSE::log::info("Registered TESDeathEvent sink");
			} else {
				SKSE::log::error("ScriptEventSourceHolder not available");
			}
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 1, 0, 0 },
	.Name = "KillRestore"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("KillRestore v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
	return true;
}
