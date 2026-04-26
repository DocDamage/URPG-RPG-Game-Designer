#include "engine/core/engine_shell.h"
#include "engine/core/export/runtime_bundle_loader.h"
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
#include <vector>

namespace {

struct SmokeOptions {
    std::optional<std::filesystem::path> markerPath;
    std::optional<std::filesystem::path> bundlePath;
    urpg::tools::ExportTarget target = urpg::tools::ExportTarget::Windows_x64;
};

std::optional<urpg::tools::ExportTarget> parseTarget(const std::string& value) {
    if (value == "Windows_x64") {
        return urpg::tools::ExportTarget::Windows_x64;
    }
    if (value == "Linux_x64") {
        return urpg::tools::ExportTarget::Linux_x64;
    }
    if (value == "macOS_Universal") {
        return urpg::tools::ExportTarget::macOS_Universal;
    }
    if (value == "Web_WASM") {
        return urpg::tools::ExportTarget::Web_WASM;
    }
    return std::nullopt;
}

SmokeOptions parseOptions(int argc, char** argv) {
    SmokeOptions options;

#if defined(_WIN32)
    options.target = urpg::tools::ExportTarget::Windows_x64;
#elif defined(__APPLE__)
    options.target = urpg::tools::ExportTarget::macOS_Universal;
#else
    options.target = urpg::tools::ExportTarget::Linux_x64;
#endif

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--write-marker" && i + 1 < argc) {
            options.markerPath = std::filesystem::path(argv[++i]);
        } else if (arg == "--bundle" && i + 1 < argc) {
            options.bundlePath = std::filesystem::path(argv[++i]);
        } else if (arg == "--target" && i + 1 < argc) {
            const auto parsedTarget = parseTarget(argv[++i]);
            if (parsedTarget.has_value()) {
                options.target = *parsedTarget;
            }
        }
    }

    return options;
}

std::filesystem::path executablePath(char** argv) {
    if (argv != nullptr && argv[0] != nullptr) {
        std::error_code canonicalError;
        auto path = std::filesystem::weakly_canonical(std::filesystem::path(argv[0]), canonicalError);
        if (!canonicalError) {
            return path;
        }
        return std::filesystem::absolute(std::filesystem::path(argv[0]));
    }
    return std::filesystem::current_path();
}

std::filesystem::path defaultBundlePath(char** argv) {
    const auto exe = executablePath(argv);
    const auto exeDir = exe.parent_path();
    const std::vector<std::filesystem::path> candidates = {
        exeDir / "data.pck",
        std::filesystem::current_path() / "data.pck",
        exeDir.parent_path().parent_path().parent_path() / "data.pck",
    };

    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    return exeDir / "data.pck";
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
        const auto options = parseOptions(argc, argv);
        const auto bundlePath = options.bundlePath.value_or(defaultBundlePath(argv));

        const auto bundleLoad = urpg::exporting::LoadRuntimeBundle(bundlePath, options.target);
        if (!bundleLoad.loaded) {
            std::cerr << "URPG export smoke bundle load rejected: " << bundlePath.string() << "\n";
            for (const auto& error : bundleLoad.errors) {
                std::cerr << error << "\n";
            }
            return 3;
        }

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

        if (options.markerPath.has_value()) {
            if (!options.markerPath->parent_path().empty()) {
                std::filesystem::create_directories(options.markerPath->parent_path());
            }
            std::ofstream marker(*options.markerPath, std::ios::binary);
            marker << "URPG_EXPORT_SMOKE_OK\n";
        }

        std::cout << "URPG export smoke OK\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "URPG export smoke exception: " << ex.what() << "\n";
        return 2;
    }
}
