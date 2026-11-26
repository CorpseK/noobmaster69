// GMSTTweaks - SKSE plugin (no ESP) that overrides Game Settings (GMSTs) from
// an INI file after all plugins have loaded. Nothing is stored in the save:
// remove the DLL (or the INI line) and the vanilla/ESP value is used again on
// the next launch.
//
// Config: Data/SKSE/Plugins/GMSTTweaks.ini, section [GameSettings], Name=Value.

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
		auto path = *logDir / "GMSTTweaks.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_pattern("[%H:%M:%S] [%l] %v");
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

	// Returns every "key=value" pair of the [GameSettings] section, in file order.
	std::vector<std::pair<std::string, std::string>> ReadEntries()
	{
		const auto path = (std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "GMSTTweaks.ini").string();
		std::vector<char> buf(64 * 1024);
		const auto len = GetPrivateProfileSectionA("GameSettings", buf.data(), static_cast<DWORD>(buf.size()), path.c_str());

		std::vector<std::pair<std::string, std::string>> entries;
		// The buffer holds NUL-separated lines and ends with a double NUL.
		for (std::size_t pos = 0; pos < len;) {
			std::string_view line(buf.data() + pos);
			pos += line.size() + 1;
			if (line.empty() || line.front() == ';' || line.front() == '#') {
				continue;
			}
			const auto eq = line.find('=');
			if (eq == std::string_view::npos) {
				continue;
			}
			auto key = Trim(line.substr(0, eq));
			auto value = Trim(line.substr(eq + 1));
			// Allow trailing comments: fFoo=1.5 ; comment
			if (const auto semi = value.find(';'); semi != std::string::npos) {
				value = Trim(std::string_view(value).substr(0, semi));
			}
			if (!key.empty() && !value.empty()) {
				entries.emplace_back(std::move(key), std::move(value));
			}
		}
		SKSE::log::info("Read {} entries from {}", entries.size(), path);
		return entries;
	}

	bool ApplyOne(RE::Setting* a_setting, const std::string& a_name, const std::string& a_value)
	{
		try {
			switch (a_setting->GetType()) {
			case RE::Setting::Type::kFloat:
				{
					const float old = a_setting->data.f;
					a_setting->data.f = std::stof(a_value);
					SKSE::log::info("{}: {} -> {}", a_name, old, a_setting->data.f);
					return true;
				}
			case RE::Setting::Type::kInteger:
				{
					const auto old = a_setting->data.i;
					a_setting->data.i = static_cast<std::int32_t>(std::stol(a_value));
					SKSE::log::info("{}: {} -> {}", a_name, old, a_setting->data.i);
					return true;
				}
			case RE::Setting::Type::kUnsignedInteger:
				{
					const auto old = a_setting->data.u;
					a_setting->data.u = static_cast<std::uint32_t>(std::stoul(a_value));
					SKSE::log::info("{}: {} -> {}", a_name, old, a_setting->data.u);
					return true;
				}
			case RE::Setting::Type::kBool:
				{
					const bool old = a_setting->data.b;
					const bool value = a_value == "1" || _stricmp(a_value.c_str(), "true") == 0;
					a_setting->data.b = value;
					SKSE::log::info("{}: {} -> {}", a_name, old, value);
					return true;
				}
			default:
				SKSE::log::warn("{}: type {} not supported (only f/i/u/b)", a_name,
					static_cast<int>(a_setting->GetType()));
				return false;
			}
		} catch (...) {
			SKSE::log::warn("{}: cannot parse value '{}'", a_name, a_value);
			return false;
		}
	}

	void Apply()
	{
		auto* gmsts = RE::GameSettingCollection::GetSingleton();
		if (!gmsts) {
			SKSE::log::error("GameSettingCollection not available");
			return;
		}

		std::size_t applied = 0;
		const auto entries = ReadEntries();
		for (const auto& [name, value] : entries) {
			auto* setting = gmsts->GetSetting(name.c_str());
			if (!setting) {
				SKSE::log::warn("{}: no such game setting", name);
				continue;
			}
			if (ApplyOne(setting, name, value)) {
				++applied;
			}
		}
		SKSE::log::info("Applied {}/{} game settings", applied, entries.size());
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
	.Name = "GMSTTweaks"sv,
	.Author = "CorpseK"sv,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("GMSTTweaks v0.1.0 loading (game {})", a_skse->RuntimeVersion().string());

	SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
	return true;
}
