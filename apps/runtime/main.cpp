#include "engine/core/app_cli.h"
#include "engine/core/diagnostics/startup_diagnostics.h"
#include "engine/core/engine_shell.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/save/runtime_save_startup.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/options_scene.h"
#include "engine/core/scene/runtime_title_scene.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/settings/app_settings_store.h"
#include "engine/core/version.h"
#include "engine/core/map/grid_part_catalog_loader.h"
#include "engine/core/map/grid_part_runtime_compiler.h"
#include "engine/core/map/grid_part_serializer.h"
#include "engine/core/platform/process_runner.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/save/save_journal.h"
#include <fstream>
#include <nlohmann/json.hpp>

#ifndef URPG_HEADLESS
#include "engine/core/platform/opengl_renderer.h"
#include "engine/core/platform/sdl_surface.h"
#endif

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace {

bool defaultHeadless() {
#ifdef URPG_HEADLESS
    return true;
#else
    return false;
#endif
}

void printVersion() {
    std::cout << "URPG Runtime " << urpg::versionString() << "\n";
}

bool isSafeReloadResourcePath(const std::string& value, const std::string& kind) {
    const std::filesystem::path path(value);
    if (path.empty() || path.is_absolute()) return false;
    for (const auto& part : path.lexically_normal()) {
        if (part == "..") return false;
    }
    const auto normalized = path.lexically_normal().generic_string();
    if (kind == "map") return normalized.starts_with("content/maps/");
    if (kind == "ability") return normalized.starts_with("content/abilities/");
    if (kind == "dialogue") {
        return normalized == "content/dialogue_sequences.json" || normalized.starts_with("content/dialogue/") ||
            normalized.starts_with("content/data/");
    }
    return false;
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

std::shared_ptr<urpg::scene::MapScene> makeRuntimeMapScene(const std::filesystem::path& projectRoot,
                                                           const std::string& mapName) {
    int width = 16;
    int height = 12;

    const auto gridPath = urpg::platform::resolvePlaytestPath(projectRoot / "content" / "maps" / (mapName + ".grid.json"));

    std::optional<urpg::map::GridPartDocument> doc;
    if (std::filesystem::exists(gridPath)) {
        std::ifstream input(gridPath, std::ios::binary);
        if (input) {
            nlohmann::json gridJson = nlohmann::json::parse(input, nullptr, false);
            if (gridJson.is_object()) {
                doc = urpg::map::GridPartDocumentFromJson(gridJson);
                if (doc) {
                    width = doc->width();
                    height = doc->height();
                }
            }
        }
        if (!doc) {
            urpg::diagnostics::RuntimeDiagnostics::error(
                "runtime.playtest", "playtest.map_overlay_invalid",
                "The current-map playtest overlay could not be parsed.", mapName, "", gridPath.generic_string());
        }
    }

    auto map = std::make_shared<urpg::scene::MapScene>(mapName, width, height);
    map->setProjectRoot(projectRoot);
    map->setAssetReferences(urpg::scene::loadRuntimeMapAssetReferences(projectRoot, mapName));

    if (doc) {
        urpg::map::GridPartCatalog catalog;
        std::string error_msg;
        if (urpg::map::LoadGridPartCatalogFromProject(projectRoot, catalog,
                                                     std::filesystem::path("content") / "part_catalogs" / "base_jrpg_parts.json",
                                                     &error_msg)) {
            auto compileResult = urpg::map::CompileGridPartRuntime(*doc, catalog);
            if (!compileResult.ok || !urpg::map::ApplyGridPartRuntimeToMapScene(compileResult, *map)) {
                urpg::diagnostics::RuntimeDiagnostics::error(
                    "runtime.playtest", "playtest.map_compile_failed",
                    "The current-map playtest overlay failed runtime compilation.", mapName, "",
                    gridPath.generic_string());
            }
        } else {
            urpg::diagnostics::RuntimeDiagnostics::error(
                "runtime.playtest", "playtest.catalog_load_failed",
                "The grid-part catalog required by the current map could not be loaded: " + error_msg, mapName, "",
                gridPath.generic_string());
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
        std::string targetMap = options.map;
        std::string targetSpawn = options.spawn;
        if (!options.session_manifest.empty()) {
            std::ifstream in(options.session_manifest);
            if (in) {
                nlohmann::json manifest = nlohmann::json::parse(in, nullptr, false);
                if (manifest.is_object()) {
                    if (manifest.contains("overlay_dir") && manifest["overlay_dir"].is_string()) {
                        urpg::platform::g_PlaytestOverlayDir = manifest["overlay_dir"].get<std::string>();
                    }
                    if (manifest.contains("diagnostics_path") && manifest["diagnostics_path"].is_string()) {
                        urpg::diagnostics::g_DiagnosticsFilePath = manifest["diagnostics_path"].get<std::string>();
                    } else {
                        urpg::diagnostics::g_DiagnosticsFilePath = options.session_manifest.parent_path() / "diagnostics.jsonl";
                    }
                    if (targetMap.empty() && manifest.contains("map_id") && manifest["map_id"].is_string()) {
                        targetMap = manifest["map_id"].get<std::string>();
                    }
                    if (targetSpawn.empty() && manifest.contains("spawn") && manifest["spawn"].is_string()) {
                        targetSpawn = manifest["spawn"].get<std::string>();
                    }
                }
            }
        }
        urpg::diagnostics::g_ActiveMapId = targetMap;
        const auto settingsPaths = urpg::settings::appSettingsPaths(options.project_root);
        auto settingsLoad = urpg::settings::loadRuntimeSettings(settingsPaths.runtime_settings);
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
            surface = std::make_unique<urpg::SDLSurface>();
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
        urpg::RuntimeStartupServices::applyAudioSettings(shell.getAudio(), settingsLoad.settings.audio);
        printStartupDiagnostics(shell.getRuntimeStartupReport());

        clearSceneStack();
        if (!targetMap.empty()) {
            auto mapScene = makeRuntimeMapScene(options.project_root, targetMap);
            if (!targetSpawn.empty()) {
                size_t comma = targetSpawn.find(',');
                if (comma != std::string::npos) {
                    try {
                        int x = std::stoi(targetSpawn.substr(0, comma));
                        int y = std::stoi(targetSpawn.substr(comma + 1));
                        auto& mv = mapScene->getPlayerMovement();
                        mv.gridPos = {x, y};
                        mv.lastGridPos = {x, y};
                    } catch (const std::exception& ex) {
                        urpg::diagnostics::RuntimeDiagnostics::warning(
                            "runtime.playtest", "playtest.spawn_invalid",
                            "Ignored invalid playtest spawn '" + targetSpawn + "': " + ex.what(), targetMap, "",
                            options.session_manifest.generic_string());
                    }
                }
            }
            urpg::scene::SceneManager::getInstance().gotoScene(mapScene);
        } else {
            const auto startupSaveState = urpg::discoverRuntimeSaves(options.project_root);
            auto titleScene = urpg::scene::makeDefaultRuntimeTitleScene({
                [&options] {
                    urpg::scene::SceneManager::getInstance().gotoScene(
                        makeRuntimeMapScene(options.project_root, "RuntimeBoot"));
                },
                [&shell] { shell.shutdown(); },
                [startupSaveState, &options] {
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
                        makeRuntimeMapScene(options.project_root, mapName));
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
                                urpg::RuntimeStartupServices::applyAudioSettings(shell.getAudio(),
                                                                                 settingsLoad.settings.audio);
                            },
                        }));
                },
            });
            titleScene->setContinueAvailability(startupSaveState.hasLoadableSave(), startupSaveState.continueDisabledReason());
            titleScene->setStartupReport(shell.getRuntimeStartupReport());
            urpg::scene::SceneManager::getInstance().gotoScene(titleScene);
        }

        int frame = 0;
        while (shell.isRunning() && (options.frames < 0 || frame < options.frames)) {
            if (!options.session_manifest.empty()) {
                std::filesystem::path requestPath = options.session_manifest.parent_path() / "reload_request.json";
                std::error_code readEc;
                if (std::filesystem::exists(requestPath, readEc)) {
                    std::ifstream in(requestPath, std::ios::binary);
                    nlohmann::json request = nlohmann::json::parse(in, nullptr, false);
                    in.close();

                    std::error_code removeEc;
                    std::filesystem::remove(requestPath, removeEc);

                    if (request.is_object() && request.contains("version") && request.contains("resources")) {
                        int version = request.value("version", 0);
                        nlohmann::json response;
                        response["version"] = version;
                        response["results"] = nlohmann::json::array();

                        for (const auto& resource : request["resources"]) {
                            if (!resource.is_object() || !resource.contains("id") || !resource.contains("kind") || !resource.contains("path")) {
                                continue;
                            }
                            std::string id = resource["id"];
                            std::string kind = resource["kind"];
                            std::string path = resource["path"];

                            nlohmann::json resultItem;
                            resultItem["id"] = id;

                            if (!isSafeReloadResourcePath(path, kind)) {
                                resultItem["status"] = "rejected";
                                resultItem["reason"] = "Resource path is outside the supported project data roots";
                            } else if (kind == "map") {
                                const auto fullGridPath = urpg::platform::resolvePlaytestPath(options.project_root / path);
                                std::ifstream mapInput(fullGridPath, std::ios::binary);
                                nlohmann::json mapJson = nlohmann::json::parse(mapInput, nullptr, false);
                                auto reloadedDoc = urpg::map::GridPartDocumentFromJson(mapJson);

                                auto activeScene = urpg::scene::SceneManager::getInstance().getActiveScene();
                                if (activeScene && activeScene->getType() == urpg::scene::SceneType::MAP && reloadedDoc) {
                                    auto* mapScene = static_cast<urpg::scene::MapScene*>(activeScene.get());
                                    if (reloadedDoc->width() != mapScene->getWidth() || reloadedDoc->height() != mapScene->getHeight()) {
                                        resultItem["status"] = "rejected";
                                        resultItem["reason"] = "Incompatible map topology changes (width/height change requires relaunch)";
                                    } else {
                                        urpg::map::GridPartCatalog catalog;
                                        std::string error_msg;
                                        if (urpg::map::LoadGridPartCatalogFromProject(options.project_root, catalog,
                                                                                     std::filesystem::path("content") / "part_catalogs" / "base_jrpg_parts.json",
                                                                                     &error_msg)) {
                                            auto compileResult = urpg::map::CompileGridPartRuntime(*reloadedDoc, catalog);
                                            if (compileResult.ok &&
                                                urpg::map::ApplyGridPartRuntimeToMapScene(compileResult, *mapScene)) {
                                                resultItem["status"] = "accepted";
                                            } else {
                                                resultItem["status"] = "rejected";
                                                resultItem["reason"] = "Failed to apply map compilation to active scene";
                                            }
                                        } else {
                                            resultItem["status"] = "rejected";
                                            resultItem["reason"] = "Failed to load grid parts catalog: " + error_msg;
                                        }
                                    }
                                } else {
                                    resultItem["status"] = "rejected";
                                    resultItem["reason"] = "Active scene is not map or map document invalid";
                                }
                            } else if (kind == "ability") {
                                const auto fullPath = urpg::platform::resolvePlaytestPath(options.project_root / path);
                                auto reloadedAsset = urpg::ability::loadAuthoredAbilityAssetFromFile(fullPath);
                                if (reloadedAsset) {
                                    auto activeScene = urpg::scene::SceneManager::getInstance().getActiveScene();
                                    if (activeScene && activeScene->getType() == urpg::scene::SceneType::MAP) {
                                        auto* mapScene = static_cast<urpg::scene::MapScene*>(activeScene.get());
                                        mapScene->grantPlayerAbility(*reloadedAsset);
                                        resultItem["status"] = "accepted";
                                    } else {
                                        resultItem["status"] = "rejected";
                                        resultItem["reason"] = "No active map scene to grant player ability";
                                    }
                                } else {
                                    resultItem["status"] = "rejected";
                                    resultItem["reason"] = "Failed to parse authored ability asset from file";
                                }
                            } else if (kind == "dialogue") {
                                auto& dialogueRegistry = urpg::message::DialogueRegistry::getInstance();
                                const auto previousConversations = dialogueRegistry.getConversations();
                                dialogueRegistry.clear();
                                auto loadResult = urpg::message::DialogueProjectLoader::loadProject(dialogueRegistry, options.project_root);
                                bool hasError = false;
                                std::string reasons;
                                for (const auto& diag : loadResult.diagnostics) {
                                    if (diag.severity == urpg::message::DialogueLoadSeverity::Error) {
                                        hasError = true;
                                        reasons += diag.message + "; ";
                                    }
                                }
                                if (hasError) {
                                    dialogueRegistry.clear();
                                    for (const auto& [conversationId, nodes] : previousConversations) {
                                        dialogueRegistry.registerConversation(conversationId, nodes);
                                    }
                                    resultItem["status"] = "rejected";
                                    resultItem["reason"] = reasons;
                                } else {
                                    resultItem["status"] = "accepted";
                                }
                            } else {
                                resultItem["status"] = "rejected";
                                resultItem["reason"] = "Unsupported resource kind for safe-point reload";
                            }
                            response["results"].push_back(resultItem);
                        }

                        std::filesystem::path responsePath = options.session_manifest.parent_path() / "reload_response.json";
                        std::string writeError;
                        (void)urpg::SaveJournal::WriteAtomically(responsePath, response.dump() + "\n", &writeError);
                    }
                }
            }
            shell.tick();
            ++frame;
            if (options.headless) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        shell.shutdown();
        clearSceneStack();

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
