#include "engine/core/runtime_startup_services.h"

#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/export/runtime_bundle_loader.h"
#include "engine/core/input/input_remap_store.h"
#include "engine/core/localization/locale_catalog.h"
#include "engine/core/perf/perf_profiler.h"
#include "engine/core/render/asset_loader.h"
#include "engine/core/settings/app_settings_store.h"
#include "engine/core/tools/export_packager.h"

#include <array>
#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <system_error>

namespace urpg {

namespace {

diagnostics::DiagnosticSeverity diagnosticSeverity(RuntimeStartupSubsystemStatus status) {
    switch (status) {
    case RuntimeStartupSubsystemStatus::Initialized:
    case RuntimeStartupSubsystemStatus::Skipped:
        return diagnostics::DiagnosticSeverity::Info;
    case RuntimeStartupSubsystemStatus::Warning:
        return diagnostics::DiagnosticSeverity::Warning;
    case RuntimeStartupSubsystemStatus::Error:
        return diagnostics::DiagnosticSeverity::Error;
    }
    return diagnostics::DiagnosticSeverity::Info;
}

void addSubsystem(RuntimeStartupReport& report, std::string subsystem, RuntimeStartupSubsystemStatus status,
                  std::string code, std::string message) {
    diagnostics::RuntimeDiagnostics::emit(diagnosticSeverity(status), subsystem, code, message);
    report.subsystems.push_back({
        std::move(subsystem),
        status,
        std::move(code),
        std::move(message),
    });
}

std::filesystem::path normalizeProjectRoot(const std::filesystem::path& project_root) {
    if (project_root.empty()) {
        return std::filesystem::current_path();
    }
    return project_root;
}

std::optional<nlohmann::json> readJsonFile(const std::filesystem::path& path, std::string& error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "file_open_failed";
        return std::nullopt;
    }

    const auto parsed = nlohmann::json::parse(in, nullptr, false);
    if (parsed.is_discarded()) {
        error = "json_parse_failed";
        return std::nullopt;
    }
    return parsed;
}

void initializeAudio(RuntimeStartupReport& report, const std::filesystem::path& project_root, audio::AudioCore& audio,
                     bool audio_compat_bound) {
    audio.clearAudioDiagnostics();
    audio.setAssetRoot(project_root);

    if (!audio_compat_bound) {
        addSubsystem(report, "AudioCore", RuntimeStartupSubsystemStatus::Error, "audio.compat_not_bound",
                     "AudioCore was configured, but the RPG Maker AudioManager compatibility bridge is not bound.");
        return;
    }

    addSubsystem(report, "AudioCore", RuntimeStartupSubsystemStatus::Initialized, "audio.ready",
                 "AudioCore asset root and compatibility bridge are configured.");
}

void initializeAssets(RuntimeStartupReport& report, const std::filesystem::path& project_root) {
    AssetLoader::clearCaches();
    addSubsystem(report, "AssetLoader", RuntimeStartupSubsystemStatus::Initialized, "asset_loader.cache_ready",
                 "AssetLoader caches were reset for the runtime startup domain.");

    const std::array<std::filesystem::path, 4> bundle_candidates = {
        project_root / "data.pck",
        project_root / "export" / "data.pck",
        project_root / "dist" / "data.pck",
        project_root / "build" / "export" / "data.pck",
    };

    for (const auto& candidate : bundle_candidates) {
        std::error_code ec;
        if (!std::filesystem::is_regular_file(candidate, ec) || ec) {
            continue;
        }

        const auto bundle = exporting::LoadRuntimeBundle(candidate, tools::ExportTarget::Windows_x64);
        if (!bundle.loaded) {
            std::string message = "Runtime asset bundle validation failed for " + candidate.string();
            if (!bundle.errors.empty()) {
                message += ": " + bundle.errors.front();
            }
            addSubsystem(report, "RuntimeBundleLoader", RuntimeStartupSubsystemStatus::Error,
                         "runtime_bundle.validation_failed", std::move(message));
            return;
        }

        addSubsystem(report, "RuntimeBundleLoader", RuntimeStartupSubsystemStatus::Initialized,
                     "runtime_bundle.validated", "Runtime asset bundle validated: " + candidate.string());
        return;
    }

    addSubsystem(report, "RuntimeBundleLoader", RuntimeStartupSubsystemStatus::Skipped, "runtime_bundle.not_present",
                 "No exported runtime asset bundle was found; startup will use project-root content directly.");
}

void initializeLocalization(RuntimeStartupReport& report, const std::filesystem::path& project_root) {
    const std::array<std::filesystem::path, 4> locale_candidates = {
        project_root / "content" / "localization" / "en-US.json",
        project_root / "content" / "localization" / "en.json",
        project_root / "localization" / "en-US.json",
        project_root / "data" / "localization" / "en-US.json",
    };

    for (const auto& candidate : locale_candidates) {
        std::error_code ec;
        if (!std::filesystem::is_regular_file(candidate, ec) || ec) {
            continue;
        }

        std::string error;
        const auto payload = readJsonFile(candidate, error);
        if (!payload) {
            addSubsystem(report, "LocaleCatalog", RuntimeStartupSubsystemStatus::Error,
                         "localization." + error, "Locale catalog could not be read: " + candidate.string());
            return;
        }

        localization::LocaleCatalog catalog;
        try {
            catalog.loadFromJson(*payload);
        } catch (const std::exception& ex) {
            addSubsystem(report, "LocaleCatalog", RuntimeStartupSubsystemStatus::Error,
                         "localization.invalid_catalog",
                         "Locale catalog is invalid at " + candidate.string() + ": " + ex.what());
            return;
        }

        report.locale_code = catalog.getLocaleCode();
        report.locale_key_count = catalog.keyCount();
        addSubsystem(report, "LocaleCatalog", RuntimeStartupSubsystemStatus::Initialized, "localization.ready",
                     "Loaded locale catalog " + report.locale_code + " with " +
                         std::to_string(report.locale_key_count) + " key(s).");
        return;
    }

    addSubsystem(report, "LocaleCatalog", RuntimeStartupSubsystemStatus::Warning, "localization.catalog_missing",
                 "No locale catalog was found under project content; runtime text will use authored fallback strings.");
}

void initializeProfiler(RuntimeStartupReport& report) {
    perf::PerfProfiler profiler;
    profiler.beginFrame();
    profiler.endFrame();
    addSubsystem(report, "PerfProfiler", RuntimeStartupSubsystemStatus::Initialized, "profiler.ready",
                 "PerfProfiler was initialized and accepted a startup frame sample.");
}

void applyDefaultInputMappings(input::InputCore& input) {
    input.clearKeyMappings();
    input.mapKey('W', input::InputAction::MoveUp);
    input.mapKey('w', input::InputAction::MoveUp);
    input.mapKey('S', input::InputAction::MoveDown);
    input.mapKey('s', input::InputAction::MoveDown);
    input.mapKey('A', input::InputAction::MoveLeft);
    input.mapKey('a', input::InputAction::MoveLeft);
    input.mapKey('D', input::InputAction::MoveRight);
    input.mapKey('d', input::InputAction::MoveRight);
    input.mapKey(38, input::InputAction::MoveUp);
    input.mapKey(40, input::InputAction::MoveDown);
    input.mapKey(37, input::InputAction::MoveLeft);
    input.mapKey(39, input::InputAction::MoveRight);
    input.mapKey(13, input::InputAction::Confirm);
    input.mapKey(32, input::InputAction::Confirm);
    input.mapKey('Z', input::InputAction::Confirm);
    input.mapKey('z', input::InputAction::Confirm);
    input.mapKey(27, input::InputAction::Cancel);
    input.mapKey('X', input::InputAction::Cancel);
    input.mapKey('x', input::InputAction::Cancel);
    input.mapKey(9, input::InputAction::Menu);
    input.mapKey('C', input::InputAction::Menu);
    input.mapKey('c', input::InputAction::Menu);
    input.mapKey('`', input::InputAction::Debug);
    input.mapKey('~', input::InputAction::Debug);
}

std::filesystem::path resolveProjectPath(const std::filesystem::path& project_root, const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute()) {
        return path;
    }
    return project_root / path;
}

void initializeInput(RuntimeStartupReport& report, const std::filesystem::path& project_root, input::InputCore& input) {
    applyDefaultInputMappings(input);

    report.input_mapping_count = input.mappedKeyCount();
    addSubsystem(report, "InputManager", RuntimeStartupSubsystemStatus::Initialized, "input.default_map_ready",
                 "Runtime default keyboard mappings were registered.");

    const auto settings_paths = settings::appSettingsPaths(project_root);
    const auto runtime_settings = settings::loadRuntimeSettings(settings_paths.runtime_settings);
    for (const auto& warning : runtime_settings.report.warnings) {
        addSubsystem(report, "InputManager", RuntimeStartupSubsystemStatus::Warning, "input.runtime_settings_warning",
                     "Runtime settings warning while resolving input mappings: " + warning);
    }

    const bool custom_mapping_path_configured =
        runtime_settings.report.loaded &&
        runtime_settings.settings.input_mapping_path != settings::defaultRuntimeSettings().input_mapping_path;
    const auto mapping_path = resolveProjectPath(project_root, runtime_settings.settings.input_mapping_path);
    std::error_code ec;
    if (!std::filesystem::is_regular_file(mapping_path, ec) || ec) {
        if (custom_mapping_path_configured) {
            addSubsystem(report, "InputManager", RuntimeStartupSubsystemStatus::Warning, "input.remap_missing",
                         "Configured input mapping file was not found: " + mapping_path.string() +
                             "; runtime default mappings remain active.");
        }
        return;
    }

    std::string error;
    const auto payload = readJsonFile(mapping_path, error);
    if (!payload) {
        addSubsystem(report, "InputManager", RuntimeStartupSubsystemStatus::Warning, "input.remap_" + error,
                     "Input mapping file could not be read: " + mapping_path.string() +
                         "; runtime default mappings remain active.");
        return;
    }

    input::InputRemapStore remap_store;
    try {
        remap_store.loadFromJson(*payload);
    } catch (const std::exception& ex) {
        addSubsystem(report, "InputManager", RuntimeStartupSubsystemStatus::Warning, "input.remap_invalid",
                     "Input mapping file is invalid at " + mapping_path.string() + ": " + ex.what() +
                         "; runtime default mappings remain active.");
        return;
    }

    input.clearKeyMappings();
    for (const auto& [key_code, action] : remap_store.getAllMappings()) {
        if (action != input::InputAction::None) {
            input.mapKey(key_code, action);
        }
    }
    report.input_mapping_count = input.mappedKeyCount();
    addSubsystem(report, "InputManager", RuntimeStartupSubsystemStatus::Initialized, "input.remap_loaded",
                 "Runtime input mappings loaded from " + mapping_path.string() + " with " +
                     std::to_string(report.input_mapping_count) + " binding(s).");
}

} // namespace

bool RuntimeStartupReport::hasErrors() const {
    for (const auto& subsystem : subsystems) {
        if (subsystem.status == RuntimeStartupSubsystemStatus::Error) {
            return true;
        }
    }
    return false;
}

bool RuntimeStartupReport::hasSubsystem(const std::string& subsystem) const {
    for (const auto& entry : subsystems) {
        if (entry.subsystem == subsystem) {
            return true;
        }
    }
    return false;
}

bool RuntimeStartupReport::hasSubsystemCode(const std::string& subsystem, const std::string& code) const {
    for (const auto& entry : subsystems) {
        if (entry.subsystem == subsystem && entry.code == code) {
            return true;
        }
    }
    return false;
}

RuntimeStartupReport RuntimeStartupServices::initialize(const std::filesystem::path& project_root,
                                                        audio::AudioCore& audio, input::InputCore& input,
                                                        bool audio_compat_bound) {
    RuntimeStartupReport report;
    report.project_root = normalizeProjectRoot(project_root);

    initializeAudio(report, report.project_root, audio, audio_compat_bound);
    initializeAssets(report, report.project_root);
    initializeLocalization(report, report.project_root);
    initializeProfiler(report);
    initializeInput(report, report.project_root, input);

    return report;
}

void RuntimeStartupServices::applyAudioSettings(audio::AudioCore& audio, const settings::AudioSettings& settings) {
    audio.setCategoryVolume(audio::AudioCategory::BGM, settings.bgm_volume * settings.master_volume);
    audio.setCategoryVolume(audio::AudioCategory::BGS, settings.bgs_volume * settings.master_volume);
    audio.setCategoryVolume(audio::AudioCategory::SE, settings.se_volume * settings.master_volume);
    audio.setCategoryVolume(audio::AudioCategory::ME, settings.me_volume * settings.master_volume);
    audio.setCategoryVolume(audio::AudioCategory::System, settings.system_volume * settings.master_volume);
}

const char* toString(RuntimeStartupSubsystemStatus status) {
    switch (status) {
    case RuntimeStartupSubsystemStatus::Initialized:
        return "initialized";
    case RuntimeStartupSubsystemStatus::Skipped:
        return "skipped";
    case RuntimeStartupSubsystemStatus::Warning:
        return "warning";
    case RuntimeStartupSubsystemStatus::Error:
        return "error";
    }
    return "unknown";
}

} // namespace urpg
