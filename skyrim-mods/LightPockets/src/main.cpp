// LightPockets - SKSE plugin (no ESP) that makes small item categories
// lighter (or weightless). Weights are changed in memory after the game data
// loads, so nothing is written to the save: remove the DLL and every item gets
// its original weight back on the next launch.
//
// Config: Data/SKSE/Plugins/LightPockets.ini (multipliers, 1.0 = unchanged).

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
		auto path = *logDir / "LightPockets.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%H:%M:%S] [%l] %v");
	}

	std::filesystem::path IniPath()
	{
		return std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "LightPockets.ini";
	}

	float ReadFloat(const char* a_key, float a_default)
	{
		char buf[64]{};
		const auto path = IniPath().string();
		GetPrivateProfileStringA("Weights", a_key, "", buf, sizeof(buf), path.c_str());
		if (buf[0] == '\0') {
			return a_default;
		}
		try {
			return std::max(0.0f, std::stof(buf));
		} catch (...) {
			SKSE::log::warn("Bad value for {}: '{}', using {}", a_key, buf, a_default);
			return a_default;
		}
	}

	struct Config
	{
		float ingredients = 0.0f;
		float potions = 0.0f;
		float poisons = 0.0f;
		float food = 1.0f;
		float soulGems = 0.0f;
		float lockpicks = 0.0f;
		float keys = 0.0f;
		float scrolls = 0.0f;
		float books = 1.0f;

		void Load()
		{
			ingredients = ReadFloat("fIngredients", ingredients);
			potions = ReadFloat("fPotions", potions);
			poisons = ReadFloat("fPoisons", poisons);
			food = ReadFloat("fFood", food);
			soulGems = ReadFloat("fSoulGems", soulGems);
			lockpicks = ReadFloat("fLockpicks", lockpicks);
			keys = ReadFloat("fKeys", keys);
			scrolls = ReadFloat("fScrolls", scrolls);
			books = ReadFloat("fBooks", books);
		}
	};

	// Scales the weight of every form in a_forms for which a_mult(form) != 1.
	template <class T, class MultFn>
	std::size_t ScaleBy(RE::BSTArray<T*>& a_forms, MultFn&& a_mult)
	{
		std::size_t count = 0;
		for (auto* form : a_forms) {
			if (!form) {
				continue;
			}
			const float mult = a_mult(form);
			if (mult == 1.0f) {
				continue;
			}
			form->weight *= mult;
			++count;
		}
		return count;
	}

	template <class T>
	std::size_t Scale(RE::BSTArray<T*>& a_forms, float a_mult)
	{
		return ScaleBy(a_forms, [a_mult](T*) { return a_mult; });
	}

	void Apply()
	{
		Config cfg;
		cfg.Load();
		SKSE::log::info(
			"Config: ingredients={} potions={} poisons={} food={} soulGems={} lockpicks={} keys={} scrolls={} books={}",
			cfg.ingredients, cfg.potions, cfg.poisons, cfg.food, cfg.soulGems, cfg.lockpicks, cfg.keys, cfg.scrolls,
			cfg.books);

		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			SKSE::log::error("TESDataHandler not available");
			return;
		}

		const auto ingredients = Scale(dh->GetFormArray<RE::IngredientItem>(), cfg.ingredients);
		const auto alchemy = ScaleBy(dh->GetFormArray<RE::AlchemyItem>(), [&](RE::AlchemyItem* a_item) -> float {
			if (a_item->IsPoison()) {
				return cfg.poisons;
			}
			if (a_item->IsFood()) {
				return cfg.food;
			}
			return cfg.potions;
		});
		const auto soulGems = Scale(dh->GetFormArray<RE::TESSoulGem>(), cfg.soulGems);
		const auto keys = Scale(dh->GetFormArray<RE::TESKey>(), cfg.keys);
		const auto scrolls = Scale(dh->GetFormArray<RE::ScrollItem>(), cfg.scrolls);
		const auto books = Scale(dh->GetFormArray<RE::TESObjectBOOK>(), cfg.books);

		std::size_t lockpicks = 0;
		// Skyrim.esm "Lockpick" (MISC 0000000A).
		if (auto* lockpick = RE::TESForm::LookupByID<RE::TESObjectMISC>(0x0000000A); lockpick && cfg.lockpicks != 1.0f) {
			lockpick->weight *= cfg.lockpicks;
			lockpicks = 1;
		}

		SKSE::log::info(
			"Changed weights: {} ingredients, {} potions/poisons/food, {} soul gems, {} keys, {} scrolls, {} books, {} lockpick",
			ingredients, alchemy, soulGems, keys, scrolls, books, lockpicks);
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
			Apply();
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 0, 1, 0, 0 },
	.Name = "LightPockets"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("LightPockets v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
	return true;
}
