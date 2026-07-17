#include "engine/core/app_cli.h"
#include "engine/core/action/controller_binding_runtime.h"
#include "engine/core/accessibility/inclusive_runtime_policy.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/diagnostics/startup_diagnostics.h"
#include "engine/core/engine_shell.h"
#include "engine/core/map/grid_part_catalog_loader.h"
#include "engine/core/map/grid_part_runtime_compiler.h"
#include "engine/core/map/grid_part_serializer.h"
#include "engine/core/playtest/playtest_command_bridge.h"
#include "engine/core/playtest/playtest_event_debugger_runtime.h"
#include "engine/core/playtest/playtest_runtime_state_owner.h"
#include "engine/core/playtest/playtest_scenario_replay_runtime.h"
#include "engine/core/playtest/playtest_performance_runtime.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/save/runtime_save_startup.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/options_scene.h"
#include "engine/core/scene/runtime_title_scene.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/settings/app_settings_store.h"
#include "engine/core/version.h"

#ifndef URPG_HEADLESS
#include "engine/core/platform/opengl_renderer.h"
#include "engine/core/platform/sdl_surface.h"
#endif

#include <algorithm>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <thread>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif

namespace {

bool defaultHeadless() {
#ifdef URPG_HEADLESS
    return true;
#else
    return false;
#endif
}

uint64_t processMemoryBytes() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX counters{};
    counters.cb = sizeof(counters);
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),
                             sizeof(counters))) {
        return static_cast<uint64_t>(counters.PrivateUsage);
    }
#endif
    return 0;
}

void printVersion() {
    std::cout << "URPG Runtime " << urpg::versionString() << "\n";
}

bool applyRuntimeInclusiveSettings(urpg::EngineShell& shell,
                                   const urpg::settings::RuntimeSettings& settings) {
    urpg::RuntimeStartupServices::applyAudioSettings(shell.getAudio(), settings.audio);
    const auto inclusive = urpg::settings::inclusiveSettingsFromRuntime(settings);
    return urpg::accessibility::applyInclusiveRuntimePolicy(
        inclusive, nullptr, nullptr, nullptr, shell.getRenderer());
}

void clearSceneStack() {
    auto& sceneManager = urpg::scene::SceneManager::getInstance();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }
}

void printStartupDiagnostics(const urpg::RuntimeStartupReport& report) {
    for (const auto& subsystem : report.subsystems) {
        if (subsystem.status != urpg::RuntimeStartupSubsystemStatus::Warning &&
            subsystem.status != urpg::RuntimeStartupSubsystemStatus::Error) {
            continue;
        }

        std::cerr << "URPG runtime startup " << urpg::toString(subsystem.status) << " [" << subsystem.subsystem
                  << ":" << subsystem.code << "]: " << subsystem.message << "\n";
    }
}

void printStartupFailure(const urpg::diagnostics::StartupDiagnosticRecord& record,
                         const urpg::diagnostics::StartupDiagnosticWriteResult& writeResult) {
    std::cerr << "URPG runtime startup " << urpg::diagnostics::toString(record.severity) << " [" << record.code
              << "]: " << record.message << "\n";
    if (!writeResult.log_path.empty()) {
        std::cerr << "URPG runtime startup diagnostics log: " << writeResult.log_path.string() << "\n";
    }
    if (!writeResult.written && !writeResult.error.empty()) {
        std::cerr << "URPG runtime startup diagnostics write failed: " << writeResult.error << "\n";
    }
}

bool isPathUnderRoot(const std::filesystem::path& path, const std::filesystem::path& root) {
    std::error_code error;
    const auto normalizedPath = std::filesystem::weakly_canonical(path, error);
    if (error) return false;
    const auto normalizedRoot = std::filesystem::weakly_canonical(root, error);
    if (error) return false;
    const auto relative = normalizedPath.lexically_relative(normalizedRoot);
    if (relative.empty() || relative.is_absolute()) return false;
    return std::none_of(relative.begin(), relative.end(), [](const auto& component) { return component == ".."; });
}

bool pathsReferToSameProject(const std::filesystem::path& left, const std::filesystem::path& right) {
    std::error_code error;
    return std::filesystem::equivalent(left, right, error) && !error;
}

bool isSafePlaytestMapId(const std::string& mapId) {
    const std::filesystem::path path(mapId);
    return !mapId.empty() && !path.is_absolute() && path.filename().string() == mapId && mapId != "." && mapId != "..";
}

std::optional<urpg::events::EventDocument> loadPlaytestEventDocument(
    const std::filesystem::path& projectRoot) {
    std::ifstream input(projectRoot / "content" / "events" / "template_project.json", std::ios::binary);
    const auto value = input ? nlohmann::json::parse(input, nullptr, false) : nlohmann::json{};
    if (!value.is_object() || value.value("schema", "") != "urpg.event_template_project.v1" ||
        !value.contains("event_document")) {
        return std::nullopt;
    }
    auto document = urpg::events::EventDocument::fromJson(value["event_document"]);
    if (!document.validate().empty() || document.events().empty()) return std::nullopt;
    return document;
}

nlohmann::json liveRuntimeState(const std::shared_ptr<urpg::scene::MapScene>& scene) {
    nlohmann::json state{{"switches", nlohmann::json::object()},
                         {"variables", nlohmann::json::object()},
                         {"self_switches", nlohmann::json::object()},
                         {"entities", nlohmann::json::object()},
                         {"quests", nlohmann::json::object()},
                         {"inventory", nlohmann::json::object()}};
    if (!scene) return state;
    const auto authored = scene->authoredDialogueStateSnapshot();
    for (const auto& entry : authored.switches) state["switches"][entry.key] = entry.value == "true";
    for (const auto& entry : authored.self_switches) state["self_switches"][entry.key] = entry.value == "true";
    for (const auto& entry : authored.variables) {
        try {
            state["variables"][entry.key] = std::stoll(entry.value);
        } catch (const std::exception&) {
            state["variables"][entry.key] = 0;
        }
    }
    const auto& movement = scene->getPlayerMovement();
    state["entities"]["player"] = {
        {"type", "player"},
        {"fields", {{"map_id", urpg::diagnostics::g_ActiveMapId},
                    {"tile_x", movement.gridPos.x}, {"tile_y", movement.gridPos.y}}}};
    return state;
}

std::shared_ptr<urpg::scene::MapScene> makeRuntimeMapScene(const std::filesystem::path& projectRoot,
                                                           const std::string& mapName,
                                                           const urpg::settings::RuntimeSettings& settings,
                                                           const std::shared_ptr<urpg::audio::AudioCore>& audio,
                                                           const std::optional<urpg::localization::LocaleCatalog>& locale,
                                                           const std::filesystem::path& overlayRoot = {}) {
    const auto publishedPath = projectRoot / "content" / "maps" / (mapName + ".grid.json");
    const auto overlayPath = overlayRoot / "content" / "maps" / (mapName + ".grid.json");
    const auto mapPath = !overlayRoot.empty() && std::filesystem::is_regular_file(overlayPath) ? overlayPath : publishedPath;
    std::optional<urpg::map::GridPartDocument> gridDocument;
    if (std::filesystem::is_regular_file(mapPath)) {
        std::ifstream input(mapPath, std::ios::binary);
        const auto json = input ? nlohmann::json::parse(input, nullptr, false) : nlohmann::json{};
        if (json.is_object()) gridDocument = urpg::map::GridPartDocumentFromJson(json);
    }
    const int width = gridDocument ? gridDocument->width() : 16;
    const int height = gridDocument ? gridDocument->height() : 12;
    auto map = std::make_shared<urpg::scene::MapScene>(mapName, width, height);
    map->setAudioCore(audio);
    map->setDialogueLocaleCatalog(locale);
    if (!map->setDialogueInclusiveSettings(urpg::settings::inclusiveSettingsFromRuntime(settings))) {
        urpg::diagnostics::RuntimeDiagnostics::error(
            "runtime.settings", "runtime.dialogue_accessibility_settings_invalid",
            "Runtime dialogue accessibility settings could not be applied; safe defaults remain active.");
    }
    map->setAssetReferences(urpg::scene::loadRuntimeMapAssetReferences(projectRoot, mapName));
    // A starter map may intentionally be blank.  It still has a valid map
    // canvas and can be launched with the editor-selected fallback spawn; it
    // simply has no Grid Parts that require the optional catalog to compile.
    if (gridDocument && !gridDocument->parts().empty()) {
        urpg::map::GridPartCatalog catalog;
        std::string catalogError;
        if (urpg::map::LoadGridPartCatalogFromProject(
                projectRoot, catalog, std::filesystem::path("content") / "part_catalogs" / "base_jrpg_parts.json",
                &catalogError)) {
            const auto compiled = urpg::map::CompileGridPartRuntime(*gridDocument, catalog);
            if (!compiled.ok || !urpg::map::ApplyGridPartRuntimeToMapScene(compiled, *map)) {
                urpg::diagnostics::RuntimeDiagnostics::error(
                    "runtime.playtest", "playtest.map_compile_failed",
                    "The current-map playtest overlay failed runtime compilation.", mapName, {}, mapPath.generic_string());
            }
        } else {
            urpg::diagnostics::RuntimeDiagnostics::error(
                "runtime.playtest", "playtest.catalog_load_failed",
                "The Grid Parts catalog required by the current-map playtest could not be loaded: " + catalogError,
                mapName, {}, mapPath.generic_string());
        }
    }
    return map;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto cli = urpg::cli::parseRuntimeCli(urpg::cli::argvToViews(argc, argv), defaultHeadless());
        if (!cli.ok()) {
            std::cerr << "URPG runtime: " << cli.error << "\n" << urpg::cli::runtimeHelpText();
            return 2;
        }
        if (cli.action == urpg::cli::CliAction::Help) {
            std::cout << urpg::cli::runtimeHelpText();
            return 0;
        }
        if (cli.action == urpg::cli::CliAction::Version) {
            printVersion();
            return 0;
        }

        const urpg::cli::RuntimeCliOptions options = cli.options;
        std::filesystem::path playtestOverlay;
        std::string targetMap = options.map;
        std::string targetSpawn = options.spawn;
        std::string targetSelectedObject;
        std::string playtestProjectRevision;
        uint64_t playtestReplaySeed = 0;
        if (!options.session_manifest.empty()) {
            std::ifstream manifestFile(options.session_manifest, std::ios::binary);
            const auto manifest = manifestFile ? nlohmann::json::parse(manifestFile, nullptr, false) : nlohmann::json{};
            if (!manifest.is_object()) {
                std::cerr << "URPG runtime: invalid playtest session manifest.\n";
                return 2;
            }
            playtestOverlay = manifest.value("overlay_dir", std::string{});
            const auto manifestProjectRoot = std::filesystem::path(manifest.value("project_root", std::string{}));
            if (manifestProjectRoot.empty() || !pathsReferToSameProject(manifestProjectRoot, options.project_root) ||
                !isPathUnderRoot(playtestOverlay, options.project_root / ".urpg" / "playtest")) {
                std::cerr << "URPG runtime: playtest session manifest is outside this project's private overlay root.\n";
                return 2;
            }
            if (targetMap.empty()) targetMap = manifest.value("map_id", std::string{});
            if (targetSpawn.empty()) targetSpawn = manifest.value("spawn", std::string{});
            targetSelectedObject = manifest.value("selected_object_id", std::string{});
            playtestProjectRevision = manifest.value("project_revision", std::string{});
            playtestReplaySeed = manifest.value("replay_seed", uint64_t{0});
            urpg::diagnostics::g_DiagnosticsFilePath = manifest.value("diagnostics_path", std::string{});
            if (!isPathUnderRoot(urpg::diagnostics::g_DiagnosticsFilePath, playtestOverlay)) {
                std::cerr << "URPG runtime: playtest diagnostics path is outside the private overlay.\n";
                return 2;
            }
        }
        if (!targetMap.empty() && !isSafePlaytestMapId(targetMap)) {
            std::cerr << "URPG runtime: invalid playtest map ID.\n";
            return 2;
        }
        urpg::diagnostics::g_ActiveMapId = targetMap;
        const auto settingsPaths = urpg::settings::appSettingsPaths(options.project_root);
        auto settingsLoad = urpg::settings::loadRuntimeSettings(settingsPaths.runtime_settings);
        if (settingsLoad.report.recovered_from_malformed) {
            const auto quarantine = urpg::settings::quarantineMalformedSettings(settingsPaths.runtime_settings);
            std::string recoveryError;
            if (!quarantine.success ||
                !urpg::settings::saveRuntimeSettings(settingsPaths.runtime_settings, settingsLoad.settings,
                                                      &recoveryError)) {
                std::cerr << "URPG runtime settings quarantine failed: "
                          << (quarantine.success ? recoveryError : quarantine.message) << "\n";
            }
        }
        for (const auto& warning : settingsLoad.report.warnings) {
            std::cerr << "URPG runtime settings warning: " << warning << "\n";
        }
        if (options.width_provided) {
            settingsLoad.settings.window.width = options.width;
        }
        if (options.height_provided) {
            settingsLoad.settings.window.height = options.height;
        }

        if (const auto startupFailure = urpg::diagnostics::validateStartupInputs(
                "runtime", options.project_root, settingsLoad.settings.window.width, settingsLoad.settings.window.height,
                options.headless)) {
            const auto writeResult = urpg::diagnostics::writeStartupDiagnostic(*startupFailure);
            printStartupFailure(*startupFailure, writeResult);
            return 1;
        }
        if (const auto preflightFailure =
                urpg::diagnostics::validateRuntimeProjectPreflight("runtime", options.project_root, options.headless)) {
            const auto writeResult = urpg::diagnostics::writeStartupDiagnostic(*preflightFailure);
            printStartupFailure(*preflightFailure, writeResult);
            return 1;
        }

        urpg::WindowConfig config;
        config.title = "URPG Runtime";
        config.width = settingsLoad.settings.window.width;
        config.height = settingsLoad.settings.window.height;
        config.fullscreen = settingsLoad.settings.window.fullscreen;
        config.resizable = settingsLoad.settings.window.resizable;

        std::unique_ptr<urpg::IPlatformSurface> surface;
        std::unique_ptr<urpg::RendererBackend> renderer;

        if (options.headless) {
            surface = std::make_unique<urpg::HeadlessSurface>();
            renderer = std::make_unique<urpg::HeadlessRenderer>();
        } else {
#ifdef URPG_HEADLESS
            std::cerr << "URPG runtime was built headless; use --headless.\n";
            return 2;
#else
            auto sdlSurface = std::make_unique<urpg::SDLSurface>();
            urpg::action::ControllerBindingRuntime controllerBindings;
            const auto controllerBindingsPath =
                options.project_root / settingsLoad.settings.controller_mapping_path;
            if (std::filesystem::is_regular_file(controllerBindingsPath)) {
                std::string controllerBindingsError;
                if (!controllerBindings.loadFromFile(controllerBindingsPath, &controllerBindingsError)) {
                    urpg::diagnostics::RuntimeDiagnostics::warning(
                        "runtime.input", "runtime.controller_bindings_invalid",
                        "Controller bindings could not be loaded; safe defaults remain active: " +
                            controllerBindingsError);
                }
            }
            if (!sdlSurface->setControllerBindings(controllerBindings)) {
                urpg::diagnostics::RuntimeDiagnostics::warning(
                    "runtime.input", "runtime.controller_bindings_rejected",
                    "Controller bindings were incomplete; safe defaults remain active.");
            }
            surface = std::move(sdlSurface);
            renderer = std::make_unique<urpg::OpenGLRenderer>();
#endif
        }

        if (!surface->initialize(config)) {
            std::cerr << "URPG runtime failed to initialize platform surface.\n";
            return 1;
        }

        auto& shell = urpg::EngineShell::getInstance();
        if (!shell.startup(std::move(surface), std::move(renderer),
                           urpg::EngineShell::StartupOptions(options.project_root))) {
            std::cerr << "URPG runtime startup failed.\n";
            return 1;
        }
        if (!applyRuntimeInclusiveSettings(shell, settingsLoad.settings)) {
            urpg::diagnostics::RuntimeDiagnostics::error(
                "runtime.settings", "runtime.accessibility_renderer_settings_invalid",
                "Runtime accessibility renderer settings were invalid; safe renderer defaults remain active.");
        }
#ifndef URPG_HEADLESS
        if (auto* openGl = dynamic_cast<urpg::OpenGLRenderer*>(shell.getRenderer())) {
            openGl->setFontProfileRegistry(shell.getRuntimeStartupReport().font_profiles);
        }
#endif
        printStartupDiagnostics(shell.getRuntimeStartupReport());
        auto runtimeAudio = std::shared_ptr<urpg::audio::AudioCore>(&shell.getAudio(), [](auto*) {});
        const auto runtimeLocale = shell.getRuntimeStartupReport().locale_catalog;

        clearSceneStack();
        std::shared_ptr<urpg::scene::MapScene> activePlaytestScene;
        if (!targetMap.empty()) {
            activePlaytestScene = makeRuntimeMapScene(options.project_root, targetMap, settingsLoad.settings,
                                                      runtimeAudio, runtimeLocale, playtestOverlay);
            const auto comma = targetSpawn.find(',');
            if (comma != std::string::npos) {
                try {
                    auto& movement = activePlaytestScene->getPlayerMovement();
                    movement.gridPos = {std::stoi(targetSpawn.substr(0, comma)), std::stoi(targetSpawn.substr(comma + 1))};
                    movement.lastGridPos = movement.gridPos;
                } catch (const std::exception&) {
                    std::cerr << "URPG runtime: ignored invalid playtest spawn '" << targetSpawn << "'.\n";
                }
            }
            urpg::scene::SceneManager::getInstance().gotoScene(activePlaytestScene);
            urpg::diagnostics::RuntimeDiagnostics::info(
                "runtime.playtest", "playtest.session_started", "Started the editor-owned current-map playtest session.",
                targetMap, {}, options.session_manifest.generic_string());
        } else {
            const auto startupSaveState = urpg::discoverRuntimeSaves(options.project_root);
            auto titleScene = urpg::scene::makeDefaultRuntimeTitleScene({
            [&options, &settingsLoad, &runtimeAudio, &runtimeLocale] {
                urpg::scene::SceneManager::getInstance().gotoScene(
                    makeRuntimeMapScene(options.project_root, "RuntimeBoot", settingsLoad.settings,
                                        runtimeAudio, runtimeLocale));
            },
            [&shell] { shell.shutdown(); },
            [startupSaveState, &options, &settingsLoad, &runtimeAudio, &runtimeLocale] {
                const auto result = urpg::continueNewestRuntimeSave(startupSaveState);
                if (!result.ok) {
                    std::cerr << "URPG runtime continue failed: " << result.error << "\n";
                    return urpg::scene::RuntimeTitleCommandResult{
                        true, false, "continue_load_failed", result.error.empty() ? "Save load failed." : result.error};
                }

                const auto mapName =
                    result.active_meta.map_display_name.empty() ? std::string("RuntimeBoot") : result.active_meta.map_display_name;
                if (result.loaded_from_recovery) {
                    std::cout << "URPG runtime continue recovered slot " << result.slot_id << " with tier "
                              << static_cast<int>(result.recovery_tier) << ".\n";
                } else {
                    std::cout << "URPG runtime continue loaded slot " << result.slot_id << ".\n";
                }

                urpg::scene::SceneManager::getInstance().gotoScene(
                    makeRuntimeMapScene(options.project_root, mapName, settingsLoad.settings,
                                        runtimeAudio, runtimeLocale));
                return urpg::scene::RuntimeTitleCommandResult{
                    true, true, "continue_loaded", "Continue loaded the newest save slot."};
            },
            [&settingsLoad, &settingsPaths, &shell] {
                urpg::scene::SceneManager::getInstance().pushScene(urpg::scene::makeRuntimeOptionsScene(
                    settingsLoad.settings, settingsPaths.runtime_settings,
                    {
                        [] { urpg::scene::SceneManager::getInstance().popScene(); },
                        [&settingsLoad, &shell](const urpg::settings::RuntimeSettings& savedSettings) {
                            settingsLoad.settings = savedSettings;
                            (void)applyRuntimeInclusiveSettings(shell, settingsLoad.settings);
                        },
                        {},
                        [&shell](const urpg::settings::RuntimeSettings& previewSettings) {
                            (void)applyRuntimeInclusiveSettings(shell, previewSettings);
                        },
                        [&shell](const urpg::settings::RuntimeSettings& baselineSettings) {
                            (void)applyRuntimeInclusiveSettings(shell, baselineSettings);
                        },
                    }));
            },
            });
            titleScene->setContinueAvailability(startupSaveState.hasLoadableSave(), startupSaveState.continueDisabledReason());
            titleScene->setStartupReport(shell.getRuntimeStartupReport());
            urpg::scene::SceneManager::getInstance().gotoScene(titleScene);
        }

        std::optional<urpg::playtest::PlaytestCommandBridge> playtestCommands;
        if (!playtestOverlay.empty()) playtestCommands.emplace(playtestOverlay);
        std::optional<urpg::playtest::PlaytestEventDebuggerRuntime> playtestEventDebugger;
        if (!playtestOverlay.empty()) {
            if (const auto eventDocument = loadPlaytestEventDocument(options.project_root)) {
                std::string eventId = eventDocument->events().front().id;
                const auto selected = std::find_if(eventDocument->events().begin(), eventDocument->events().end(),
                                                   [&](const auto& event) {
                                                       return event.id == targetSelectedObject;
                                                   });
                if (selected != eventDocument->events().end()) eventId = selected->id;
                playtestEventDebugger.emplace(playtestOverlay);
                std::string debuggerDiagnostic;
                if (!playtestEventDebugger->start(playtestOverlay.filename().string(), *eventDocument,
                                                  eventId, {}, &debuggerDiagnostic)) {
                    urpg::diagnostics::RuntimeDiagnostics::warning(
                        "runtime.playtest", "playtest.event_debugger_start_failed",
                        "The optional playtest event-debugger lane could not start: " + debuggerDiagnostic,
                        eventId, {}, options.session_manifest.generic_string());
                    playtestEventDebugger.reset();
                }
            }
        }
        std::optional<urpg::playtest::PlaytestRuntimeStateOwner> playtestRuntimeState;
        if (!playtestOverlay.empty() && activePlaytestScene) {
            playtestRuntimeState.emplace(playtestOverlay);
            std::string stateDiagnostic;
            if (!playtestRuntimeState->start(playtestOverlay.filename().string(), "launch",
                                             liveRuntimeState(activePlaytestScene), &stateDiagnostic)) {
                urpg::diagnostics::RuntimeDiagnostics::warning(
                    "runtime.playtest", "playtest.runtime_state_start_failed",
                    "The disposable live state inspector could not start: " + stateDiagnostic,
                    targetMap, {}, options.session_manifest.generic_string());
                playtestRuntimeState.reset();
            }
        }
        std::optional<urpg::playtest::PlaytestScenarioReplayRuntime> playtestScenarioReplay;
        if (!playtestOverlay.empty() && activePlaytestScene && !playtestProjectRevision.empty()) {
            playtestScenarioReplay.emplace(
                playtestOverlay,
                urpg::replay::ScenarioReplayCaptureConfig{playtestReplaySeed, playtestProjectRevision,
                                                           urpg::versionString(), {}, 100000, 64});
            std::string replayDiagnostic;
            if (!playtestScenarioReplay->start(playtestOverlay.filename().string(),
                                               liveRuntimeState(activePlaytestScene), &replayDiagnostic)) {
                urpg::diagnostics::RuntimeDiagnostics::warning(
                    "runtime.playtest", "playtest.scenario_replay_start_failed",
                    "The semantic scenario recorder could not start: " + replayDiagnostic,
                    targetMap, {}, options.session_manifest.generic_string());
                playtestScenarioReplay.reset();
            } else {
                shell.getInput().addHandler([&playtestScenarioReplay](const auto action, const auto state) {
                    if (playtestScenarioReplay) playtestScenarioReplay->observeInput(action, state);
                });
            }
        }
        std::optional<urpg::playtest::PlaytestPerformanceRuntime> playtestPerformance;
        const auto performanceStartedAt = std::chrono::steady_clock::now();
        if (!playtestOverlay.empty()) {
            playtestPerformance.emplace(playtestOverlay, urpg::perf::PerformanceCaptureConfig{3600, 33333});
            std::string performanceDiagnostic;
            if (!playtestPerformance->start(playtestOverlay.filename().string(), &performanceDiagnostic)) {
                urpg::diagnostics::RuntimeDiagnostics::warning(
                    "runtime.playtest", "playtest.performance_capture_start_failed",
                    "The live performance capture could not start: " + performanceDiagnostic,
                    targetMap, {}, options.session_manifest.generic_string());
                playtestPerformance.reset();
            }
        }
        uint64_t playtestMapReloadRevision = 0;
        int frame = 0;
        while (shell.isRunning() && (options.frames < 0 || frame < options.frames)) {
            if (playtestCommands) {
                const auto poll = playtestCommands->poll([&](const urpg::playtest::EditorCommand& command) {
                    if (command.command == "hot_reload_map") {
                        if (command.map_id != targetMap || command.expected_revision != playtestMapReloadRevision ||
                            command.state_policy != "reset_affected") return false;
                        auto replacement = makeRuntimeMapScene(options.project_root, command.map_id,
                                                               settingsLoad.settings, runtimeAudio, runtimeLocale,
                                                               playtestOverlay);
                        if (!replacement) return false;
                        activePlaytestScene = std::move(replacement);
                        urpg::scene::SceneManager::getInstance().gotoScene(activePlaytestScene);
                        ++playtestMapReloadRevision;
                        urpg::diagnostics::RuntimeDiagnostics::info(
                            "runtime.playtest", "playtest.map_hot_reload_applied",
                            "Applied a bounded editor-owned Map reload and reset affected map state.", command.map_id,
                            command.selected_object_id, options.session_manifest.generic_string());
                        return true;
                    }
                    if (command.map_id != targetMap) {
                        activePlaytestScene = makeRuntimeMapScene(options.project_root, command.map_id,
                                                                  settingsLoad.settings, runtimeAudio, runtimeLocale,
                                                                  playtestOverlay);
                        if (!activePlaytestScene) return false;
                        urpg::scene::SceneManager::getInstance().gotoScene(activePlaytestScene);
                        targetMap = command.map_id;
                        urpg::diagnostics::g_ActiveMapId = targetMap;
                    }
                    if (!activePlaytestScene) return false;
                    auto& movement = activePlaytestScene->getPlayerMovement();
                    movement.gridPos = {command.tile_x, command.tile_y};
                    movement.lastGridPos = movement.gridPos;
                    urpg::diagnostics::RuntimeDiagnostics::info(
                        "runtime.playtest", "playtest.teleport_applied",
                        "Applied an editor-owned live playtest teleport.", command.map_id,
                        command.selected_object_id, options.session_manifest.generic_string());
                    return true;
                });
                if (poll.io_error) {
                    urpg::diagnostics::RuntimeDiagnostics::warning(
                        "runtime.playtest", "playtest.command_bridge_io_failed",
                        "The live editor command bridge could not read or acknowledge a command: " + poll.error,
                        targetMap, {}, options.session_manifest.generic_string());
                }
            }
            if (playtestEventDebugger) {
                std::string debuggerDiagnostic;
                if (!playtestEventDebugger->tick(&debuggerDiagnostic)) {
                    urpg::diagnostics::RuntimeDiagnostics::warning(
                        "runtime.playtest", "playtest.event_debugger_bridge_failed",
                        "The live playtest event-debugger bridge failed: " + debuggerDiagnostic,
                        targetMap, {}, options.session_manifest.generic_string());
                    playtestEventDebugger.reset();
                }
            }
            if (playtestRuntimeState) {
                std::string stateDiagnostic;
                if (!playtestRuntimeState->synchronize(liveRuntimeState(activePlaytestScene), &stateDiagnostic) ||
                    !playtestRuntimeState->poll(&stateDiagnostic)) {
                    urpg::diagnostics::RuntimeDiagnostics::warning(
                        "runtime.playtest", "playtest.runtime_state_bridge_failed",
                        "The disposable live state inspector bridge failed: " + stateDiagnostic,
                        targetMap, {}, options.session_manifest.generic_string());
                    playtestRuntimeState.reset();
                }
            }
            if (playtestScenarioReplay) {
                std::string replayDiagnostic;
                const bool replayOk = playtestScenarioReplay->poll(
                    [&](const urpg::replay::ReplayArtifact& artifact,
                        const urpg::replay::ReplayExecutionMode mode) {
                        activePlaytestScene = makeRuntimeMapScene(options.project_root, targetMap,
                                                                  settingsLoad.settings, runtimeAudio, runtimeLocale,
                                                                  playtestOverlay);
                        if (!activePlaytestScene) {
                            return urpg::replay::ReplayExecutionResult{
                                false, "replay_scene_reset_failed",
                                "Replay could not recreate the captured MapScene checkpoint.", mode,
                                0, std::nullopt};
                        }
                        const auto comma = targetSpawn.find(',');
                        if (comma != std::string::npos) {
                            try {
                                auto& movement = activePlaytestScene->getPlayerMovement();
                                movement.gridPos = {std::stoi(targetSpawn.substr(0, comma)),
                                                    std::stoi(targetSpawn.substr(comma + 1))};
                                movement.lastGridPos = movement.gridPos;
                            } catch (const std::exception&) {
                                // The launch path already validated the optional spawn; replay keeps the safe map default.
                            }
                        }
                        urpg::scene::SceneManager::getInstance().gotoScene(activePlaytestScene);
                        return urpg::replay::ScenarioReplayRunner::run(
                            {artifact, mode, playtestProjectRevision, urpg::versionString()},
                            [&](const urpg::replay::ReplayInput& input, uint64_t,
                                urpg::replay::ReplayExecutionMode) -> std::optional<nlohmann::json> {
                                const auto action = urpg::playtest::PlaytestScenarioReplayRuntime::inputActionFor(
                                    input.action);
                                if (!action) return std::nullopt;
                                shell.getInput().clearActionStates();
                                shell.getInput().updateActionState(*action, urpg::input::ActionState::Pressed);
                                shell.tick(1.0F / 60.0F);
                                const auto state = liveRuntimeState(activePlaytestScene);
                                shell.getInput().updateActionState(*action, urpg::input::ActionState::Released);
                                return state;
                            });
                    }, &replayDiagnostic);
                if (!replayOk) {
                    urpg::diagnostics::RuntimeDiagnostics::warning(
                        "runtime.playtest", "playtest.scenario_replay_bridge_failed",
                        "The semantic replay bridge failed: " + replayDiagnostic,
                        targetMap, {}, options.session_manifest.generic_string());
                    playtestScenarioReplay.reset();
                }
            }
            if (playtestPerformance) {
                std::string performanceDiagnostic;
                if (!playtestPerformance->poll(&performanceDiagnostic)) {
                    urpg::diagnostics::RuntimeDiagnostics::warning(
                        "runtime.playtest", "playtest.performance_bridge_failed",
                        "The live performance control bridge failed: " + performanceDiagnostic,
                        targetMap, {}, options.session_manifest.generic_string());
                    playtestPerformance.reset();
                }
            }
            shell.tick();
            if (playtestScenarioReplay) {
                std::string replayDiagnostic;
                if (!playtestScenarioReplay->finishFrame(liveRuntimeState(activePlaytestScene), &replayDiagnostic)) {
                    urpg::diagnostics::RuntimeDiagnostics::warning(
                        "runtime.playtest", "playtest.scenario_capture_failed",
                        "The semantic input capture failed: " + replayDiagnostic,
                        targetMap, {}, options.session_manifest.generic_string());
                    playtestScenarioReplay.reset();
                }
            }
            if (playtestPerformance) {
                const auto timing = shell.getLastFrameTimings();
                const auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now() - performanceStartedAt).count();
                urpg::perf::PerformanceFrameSample sample;
                sample.frame_index = static_cast<uint64_t>(frame + 1);
                sample.timestamp_us = static_cast<uint64_t>(std::max<int64_t>(0, timestamp));
                sample.frame_time_us = std::max<uint32_t>(1, timing.total_us);
                sample.subsystem_time_us = {
                    {urpg::perf::PerformanceSubsystem::Render, timing.render_us},
                    {urpg::perf::PerformanceSubsystem::ScriptEvent, timing.scene_update_us},
                    {urpg::perf::PerformanceSubsystem::AssetStreaming, 0},
                    {urpg::perf::PerformanceSubsystem::Audio, 0},
                    {urpg::perf::PerformanceSubsystem::Other, timing.input_us}};
                sample.memory_bytes = processMemoryBytes();
                std::string performanceDiagnostic;
                if (!playtestPerformance->record(std::move(sample), &performanceDiagnostic)) {
                    urpg::diagnostics::RuntimeDiagnostics::warning(
                        "runtime.playtest", "playtest.performance_sample_failed",
                        "The live performance sample was rejected: " + performanceDiagnostic,
                        targetMap, {}, options.session_manifest.generic_string());
                    playtestPerformance.reset();
                }
            }
            ++frame;
            if (options.headless) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        shell.shutdown();
        clearSceneStack();

        if (!targetMap.empty()) {
            urpg::diagnostics::RuntimeDiagnostics::info(
                "runtime.playtest", "playtest.session_exited", "The editor-owned current-map playtest session exited.",
                targetMap, {}, options.session_manifest.generic_string());
        }

        settingsLoad.settings.window.width = config.width;
        settingsLoad.settings.window.height = config.height;
        settingsLoad.settings.window.fullscreen = config.fullscreen;
        settingsLoad.settings.window.resizable = config.resizable;
        std::string settingsError;
        if (!urpg::settings::saveRuntimeSettings(settingsPaths.runtime_settings, settingsLoad.settings, &settingsError)) {
            std::cerr << "URPG runtime failed to save settings: " << settingsError << "\n";
        }

        std::cout << "URPG runtime exited after " << frame << " frame(s).\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "URPG runtime exception: " << ex.what() << "\n";
        return 2;
    }
}
