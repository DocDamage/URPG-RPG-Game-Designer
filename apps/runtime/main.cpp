#include "engine/core/engine_shell.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/version.h"

#ifndef URPG_HEADLESS
#include "engine/core/platform/opengl_renderer.h"
#include "engine/core/platform/sdl_surface.h"
#endif

#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace {

struct RuntimeOptions {
    bool headless =
#ifdef URPG_HEADLESS
        true;
#else
        false;
#endif
    int frames = -1;
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    std::filesystem::path project_root = std::filesystem::current_path();
};

RuntimeOptions parseOptions(int argc, char** argv) {
    RuntimeOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--headless") {
            options.headless = true;
        } else if (arg == "--frames" && i + 1 < argc) {
            options.frames = std::stoi(argv[++i]);
        } else if (arg == "--width" && i + 1 < argc) {
            options.width = static_cast<std::uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--height" && i + 1 < argc) {
            options.height = static_cast<std::uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--project-root" && i + 1 < argc) {
            options.project_root = argv[++i];
        }
    }
    return options;
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

} // namespace

int main(int argc, char** argv) {
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--version") {
                printVersion();
                return 0;
            }
        }

        const RuntimeOptions options = parseOptions(argc, argv);

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

        clearSceneStack();
        urpg::scene::SceneManager::getInstance().gotoScene(
            std::make_shared<urpg::scene::MapScene>("RuntimeBoot", 16, 12));

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
