#include "engine/core/engine_shell.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/scene_manager.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

namespace {

std::optional<std::filesystem::path> parseMarkerPath(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--write-marker" && i + 1 < argc) {
            return std::filesystem::path(argv[i + 1]);
        }
    }

    return std::nullopt;
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
        const auto markerPath = parseMarkerPath(argc, argv);

        clearSceneStack();

        auto& shell = urpg::EngineShell::getInstance();
        auto surface = std::make_unique<urpg::HeadlessSurface>();
        auto renderer = std::make_unique<urpg::HeadlessRenderer>();

        if (!shell.startup(std::move(surface), std::move(renderer))) {
            std::cerr << "URPG export smoke startup failed.\n";
            return 1;
        }

        auto smokeScene = std::make_shared<urpg::scene::MapScene>("ExportSmoke", 4, 4);
        urpg::scene::SceneManager::getInstance().gotoScene(smokeScene);
        shell.tick();
        shell.shutdown();

        clearSceneStack();

        if (markerPath.has_value()) {
            std::filesystem::create_directories(markerPath->parent_path());
            std::ofstream marker(*markerPath, std::ios::binary);
            marker << "URPG_EXPORT_SMOKE_OK\n";
        }

        std::cout << "URPG export smoke OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "URPG export smoke exception: " << ex.what() << "\n";
        return 2;
    }
}
