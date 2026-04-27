#include "engine/core/audio/audio_core.h"
#include "engine/core/audio/audio_core_fwd.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/engine_shell.h"
#include "engine/core/input/input_core.h"
#include "engine/core/input/input_remap_store.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/runtime_startup_services.h"
#include "engine/core/settings/app_settings_store.h"
#include "engine/core/tools/export_packager.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TempProject {
  public:
    TempProject() {
        const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        root_ = std::filesystem::temp_directory_path() / ("urpg_runtime_startup_services_" + unique);
        std::filesystem::create_directories(root_);
    }

    ~TempProject() {
        std::error_code ec;
        std::filesystem::remove_all(root_, ec);
    }

    const std::filesystem::path& root() const { return root_; }

    void writeFile(const std::filesystem::path& relative_path, const std::string& payload) const {
        const auto path = root_ / relative_path;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream out(path, std::ios::binary);
        out << payload;
    }

  private:
    std::filesystem::path root_;
};

} // namespace

TEST_CASE("RuntimeStartupServices reports required subsystem startup decisions",
          "[integration][runtime][startup]") {
    urpg::diagnostics::RuntimeDiagnostics::clear();
    const TempProject project;
    urpg::audio::AudioCore audio;
    urpg::input::InputCore input;

    const auto report = urpg::RuntimeStartupServices::initialize(project.root(), audio, input, true);

    REQUIRE_FALSE(report.hasErrors());
    REQUIRE(report.hasSubsystemCode("AudioCore", "audio.ready"));
    REQUIRE(report.hasSubsystemCode("AssetLoader", "asset_loader.cache_ready"));
    REQUIRE(report.hasSubsystemCode("RuntimeBundleLoader", "runtime_bundle.not_present"));
    REQUIRE(report.hasSubsystemCode("LocaleCatalog", "localization.catalog_missing"));
    REQUIRE(report.hasSubsystemCode("PerfProfiler", "profiler.ready"));
    REQUIRE(report.hasSubsystemCode("InputManager", "input.default_map_ready"));
    REQUIRE(report.input_mapping_count >= 20);
    REQUIRE(input.hasMappingFor('W', urpg::input::InputAction::MoveUp));
    REQUIRE(input.hasMappingFor(13, urpg::input::InputAction::Confirm));

    const auto diagnostics = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    REQUIRE_FALSE(diagnostics.empty());
}

TEST_CASE("RuntimeStartupServices loads a project locale catalog when present",
          "[integration][runtime][startup][localization]") {
    const TempProject project;
    project.writeFile("content/localization/en-US.json", R"({
  "locale": "en-US",
  "keys": {
    "title.new_game": "New Game",
    "title.continue": "Continue"
  }
})");
    urpg::audio::AudioCore audio;
    urpg::input::InputCore input;

    const auto report = urpg::RuntimeStartupServices::initialize(project.root(), audio, input, true);

    REQUIRE_FALSE(report.hasErrors());
    REQUIRE(report.hasSubsystemCode("LocaleCatalog", "localization.ready"));
    REQUIRE(report.locale_code == "en-US");
    REQUIRE(report.locale_key_count == 2);
}

TEST_CASE("RuntimeStartupServices rejects tampered runtime bundles before project content use",
          "[integration][runtime][startup][runtime bundle][tamper]") {
    const TempProject project;

    urpg::tools::ExportPackager packager;
    urpg::tools::ExportConfig config{};
    config.target = urpg::tools::ExportTarget::Windows_x64;
    config.outputDir = project.root().string();
    config.compressAssets = true;

    const auto exportResult = packager.runExport(config);
    INFO(exportResult.log);
    REQUIRE(exportResult.success);

    {
        std::fstream bundle(project.root() / "data.pck", std::ios::binary | std::ios::in | std::ios::out);
        REQUIRE(bundle.good());
        bundle.seekp(-1, std::ios::end);
        const char tampered = '\x42';
        bundle.write(&tampered, 1);
    }

    urpg::audio::AudioCore audio;
    urpg::input::InputCore input;
    const auto report = urpg::RuntimeStartupServices::initialize(project.root(), audio, input, true);

    REQUIRE(report.hasErrors());
    REQUIRE(report.hasSubsystemCode("RuntimeBundleLoader", "runtime_bundle.validation_failed"));
    const auto runtimeBundleEntry = std::find_if(report.subsystems.begin(), report.subsystems.end(), [](const auto& entry) {
        return entry.subsystem == "RuntimeBundleLoader" && entry.code == "runtime_bundle.validation_failed";
    });
    REQUIRE(runtimeBundleEntry != report.subsystems.end());
    REQUIRE(runtimeBundleEntry->message.find("Runtime asset bundle validation failed") != std::string::npos);
    REQUIRE(runtimeBundleEntry->message.find("entry_integrity_mismatch") != std::string::npos);
}

TEST_CASE("RuntimeStartupServices surfaces invalid locale catalogs as targeted diagnostics",
          "[integration][runtime][startup][localization]") {
    const TempProject project;
    project.writeFile("content/localization/en-US.json", R"({"locale": "en-US",)");
    urpg::audio::AudioCore audio;
    urpg::input::InputCore input;

    const auto report = urpg::RuntimeStartupServices::initialize(project.root(), audio, input, true);

    REQUIRE(report.hasErrors());
    REQUIRE(report.hasSubsystemCode("LocaleCatalog", "localization.json_parse_failed"));
}

TEST_CASE("RuntimeStartupServices loads runtime input remap from project-relative settings path",
          "[integration][runtime][startup][input][remap][settings]") {
    const TempProject project;
    const auto settingsPaths = urpg::settings::appSettingsPaths(project.root());

    auto runtimeSettings = urpg::settings::defaultRuntimeSettings();
    runtimeSettings.input_mapping_path = "config/custom_input.json";
    REQUIRE(urpg::settings::saveRuntimeSettings(settingsPaths.runtime_settings, runtimeSettings));

    urpg::input::InputRemapStore remapStore;
    remapStore.clear();
    remapStore.setMapping('F', urpg::input::InputAction::Confirm);
    project.writeFile("config/custom_input.json", remapStore.saveToJson().dump(2));

    urpg::audio::AudioCore audio;
    urpg::input::InputCore input;

    const auto report = urpg::RuntimeStartupServices::initialize(project.root(), audio, input, true);

    REQUIRE_FALSE(report.hasErrors());
    REQUIRE(report.hasSubsystemCode("InputManager", "input.default_map_ready"));
    REQUIRE(report.hasSubsystemCode("InputManager", "input.remap_loaded"));
    REQUIRE(report.input_mapping_count == 1);
    REQUIRE(input.hasMappingFor('F', urpg::input::InputAction::Confirm));
    REQUIRE_FALSE(input.hasMappingFor('Z', urpg::input::InputAction::Confirm));
}

TEST_CASE("RuntimeStartupServices warns and keeps defaults when configured input remap is missing",
          "[integration][runtime][startup][input][remap][settings]") {
    const TempProject project;
    const auto settingsPaths = urpg::settings::appSettingsPaths(project.root());

    auto runtimeSettings = urpg::settings::defaultRuntimeSettings();
    runtimeSettings.input_mapping_path = "config/missing_input.json";
    REQUIRE(urpg::settings::saveRuntimeSettings(settingsPaths.runtime_settings, runtimeSettings));

    urpg::audio::AudioCore audio;
    urpg::input::InputCore input;

    const auto report = urpg::RuntimeStartupServices::initialize(project.root(), audio, input, true);

    REQUIRE_FALSE(report.hasErrors());
    REQUIRE(report.hasSubsystemCode("InputManager", "input.remap_missing"));
    REQUIRE(input.hasMappingFor('W', urpg::input::InputAction::MoveUp));
    REQUIRE(input.hasMappingFor('Z', urpg::input::InputAction::Confirm));
}

TEST_CASE("RuntimeStartupServices warns and keeps defaults when input remap is malformed",
          "[integration][runtime][startup][input][remap][settings]") {
    const TempProject project;
    const auto settingsPaths = urpg::settings::appSettingsPaths(project.root());

    auto runtimeSettings = urpg::settings::defaultRuntimeSettings();
    runtimeSettings.input_mapping_path = "config/bad_input.json";
    REQUIRE(urpg::settings::saveRuntimeSettings(settingsPaths.runtime_settings, runtimeSettings));
    project.writeFile("config/bad_input.json", "{ not valid json");

    urpg::audio::AudioCore audio;
    urpg::input::InputCore input;

    const auto report = urpg::RuntimeStartupServices::initialize(project.root(), audio, input, true);

    REQUIRE_FALSE(report.hasErrors());
    REQUIRE(report.hasSubsystemCode("InputManager", "input.remap_json_parse_failed"));
    REQUIRE(input.hasMappingFor('W', urpg::input::InputAction::MoveUp));
    REQUIRE(input.hasMappingFor('Z', urpg::input::InputAction::Confirm));
}

TEST_CASE("RuntimeStartupServices applies persisted audio settings to AudioCore",
          "[integration][runtime][startup][audio][settings][persistence]") {
    const TempProject project;
    const auto settingsPaths = urpg::settings::appSettingsPaths(project.root());

    auto runtimeSettings = urpg::settings::defaultRuntimeSettings();
    runtimeSettings.audio.master_volume = 0.5f;
    runtimeSettings.audio.bgm_volume = 0.8f;
    runtimeSettings.audio.bgs_volume = 0.6f;
    runtimeSettings.audio.se_volume = 0.4f;
    runtimeSettings.audio.me_volume = 0.2f;
    runtimeSettings.audio.system_volume = 1.0f;
    REQUIRE(urpg::settings::saveRuntimeSettings(settingsPaths.runtime_settings, runtimeSettings));

    const auto loaded = urpg::settings::loadRuntimeSettings(settingsPaths.runtime_settings);
    REQUIRE(loaded.report.loaded);

    urpg::audio::AudioCore audio;
    urpg::RuntimeStartupServices::applyAudioSettings(audio, loaded.settings.audio);

    REQUIRE(audio.getCategoryVolume(urpg::audio::AudioCategory::BGM) == 0.4f);
    REQUIRE(audio.getCategoryVolume(urpg::audio::AudioCategory::BGS) == 0.3f);
    REQUIRE(audio.getCategoryVolume(urpg::audio::AudioCategory::SE) == 0.2f);
    REQUIRE(audio.getCategoryVolume(urpg::audio::AudioCategory::ME) == 0.1f);
    REQUIRE(audio.getCategoryVolume(urpg::audio::AudioCategory::System) == 0.5f);
}

TEST_CASE("EngineShell startup invokes RuntimeStartupServices",
          "[integration][runtime][startup]") {
    const TempProject project;
    auto& shell = urpg::EngineShell::getInstance();

    REQUIRE(shell.startup(std::make_unique<urpg::HeadlessSurface>(), std::make_unique<urpg::HeadlessRenderer>(),
                          urpg::EngineShell::StartupOptions(project.root())));

    const auto& report = shell.getRuntimeStartupReport();
    REQUIRE(report.hasSubsystem("AudioCore"));
    REQUIRE(report.hasSubsystem("AssetLoader"));
    REQUIRE(report.hasSubsystem("RuntimeBundleLoader"));
    REQUIRE(report.hasSubsystem("LocaleCatalog"));
    REQUIRE(report.hasSubsystem("PerfProfiler"));
    REQUIRE(report.hasSubsystem("InputManager"));
    REQUIRE(shell.getInput().hasMappingFor('Z', urpg::input::InputAction::Confirm));

    shell.shutdown();
}
