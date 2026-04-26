#include "engine/core/app_cli.h"
#include "engine/core/engine_shell.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/save/runtime_save_startup.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/runtime_title_scene.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/version.h"

#ifndef URPG_HEADLESS
#include "engine/core/platform/opengl_renderer.h"
#include "engine/core/platform/sdl_surface.h"
#endif

#include <chrono>
#include <exception>
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

        urpg::WindowConfig config;
        config.title = "URPG Runtime";
        config.width = options.width;
        config.height = options.height;

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
        printStartupDiagnostics(shell.getRuntimeStartupReport());

        clearSceneStack();
        const auto startupSaveState = urpg::discoverRuntimeSaves(options.project_root);
        auto titleScene = urpg::scene::makeDefaultRuntimeTitleScene({
            [] {
                urpg::scene::SceneManager::getInstance().gotoScene(
                    std::make_shared<urpg::scene::MapScene>("RuntimeBoot", 16, 12));
            },
            [&shell] { shell.shutdown(); },
            [startupSaveState] {
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
                    std::make_shared<urpg::scene::MapScene>(mapName, 16, 12));
                return urpg::scene::RuntimeTitleCommandResult{
                    true, true, "continue_loaded", "Continue loaded the newest save slot."};
            },
        });
        titleScene->setContinueAvailability(startupSaveState.hasLoadableSave(), startupSaveState.continueDisabledReason());
        urpg::scene::SceneManager::getInstance().gotoScene(titleScene);

        int frame = 0;
        while (shell.isRunning() && (options.frames < 0 || frame < options.frames)) {
            shell.tick();
            ++frame;
            if (options.headless) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        shell.shutdown();
        clearSceneStack();

        std::cout << "URPG runtime exited after " << frame << " frame(s).\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "URPG runtime exception: " << ex.what() << "\n";
        return 2;
    }
}
