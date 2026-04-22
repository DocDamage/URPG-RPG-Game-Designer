#include "engine/core/testing/visual_regression_harness.h"

#ifndef URPG_HEADLESS
#include "engine/core/engine_shell.h"
#include "engine/core/platform/opengl_renderer.h"
#include "engine/core/scene/scene_manager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <filesystem>

namespace urpg::testing {

#ifndef URPG_HEADLESS
namespace {

class CaptureSurface final : public IPlatformSurface {
public:
    explicit CaptureSurface(SDL_Window* window)
        : m_window(window) {}

    bool initialize(const WindowConfig& /*config*/) override {
        return true;
    }

    bool pollEvents() override {
        return true;
    }

    void present() override {}

    void shutdown() override {}

    void* getNativeHandle() const override {
        return m_window;
    }

private:
    SDL_Window* m_window = nullptr;
};

} // namespace
#endif

void VisualRegressionHarness::setGoldenRoot(const std::string& path) {
    m_goldenRoot = path;
}

std::string VisualRegressionHarness::buildGoldenPath(const std::string& testName,
                                                     const std::string& snapshotId) const {
    std::filesystem::path root(m_goldenRoot);
    std::string filename = testName + "_" + snapshotId + ".golden.json";
    return (root / filename).string();
}

std::optional<GoldenSnapshot> VisualRegressionHarness::loadGolden(const std::string& testName,
                                                                  const std::string& snapshotId) {
    std::string path = buildGoldenPath(testName, snapshotId);
    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (...) {
        return std::nullopt;
    }

    GoldenSnapshot golden;
    golden.testName = testName;
    golden.snapshotId = snapshotId;

    if (!j.contains("width") || !j.contains("height") || !j.contains("pixels")) {
        return std::nullopt;
    }

    golden.width = j["width"].get<int>();
    golden.height = j["height"].get<int>();

    const auto& pixels = j["pixels"];
    golden.pixels.reserve(pixels.size());
    for (const auto& p : pixels) {
        SnapshotPixel pixel;
        pixel.r = p.value("r", 0);
        pixel.g = p.value("g", 0);
        pixel.b = p.value("b", 0);
        pixel.a = p.value("a", 0);
        golden.pixels.push_back(pixel);
    }

    return golden;
}

bool VisualRegressionHarness::saveGolden(const std::string& testName,
                                         const std::string& snapshotId,
                                         const SceneSnapshot& snapshot) {
    std::string path = buildGoldenPath(testName, snapshotId);

    std::filesystem::path dir = std::filesystem::path(path).parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    nlohmann::json j;
    j["width"] = snapshot.width;
    j["height"] = snapshot.height;

    nlohmann::json pixels = nlohmann::json::array();
    for (const auto& p : snapshot.pixels) {
        nlohmann::json pixel;
        pixel["r"] = p.r;
        pixel["g"] = p.g;
        pixel["b"] = p.b;
        pixel["a"] = p.a;
        pixels.push_back(pixel);
    }
    j["pixels"] = pixels;

    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << j.dump(4);
    return file.good();
}

SnapshotComparisonResult VisualRegressionHarness::compareAgainstGolden(const std::string& testName,
                                                                       const std::string& snapshotId,
                                                                       const SceneSnapshot& current,
                                                                       float thresholdPercentage) {
    auto goldenOpt = loadGolden(testName, snapshotId);
    if (!goldenOpt.has_value()) {
        return {.matches = false, .errorPercentage = 100.0f};
    }

    const GoldenSnapshot& golden = goldenOpt.value();
    SceneSnapshot goldenScene;
    goldenScene.width = golden.width;
    goldenScene.height = golden.height;
    goldenScene.pixels = golden.pixels;

    return SnapshotValidator::compare(goldenScene, current, thresholdPercentage);
}

SceneSnapshot VisualRegressionHarness::generateDiffHeatmap(const SceneSnapshot& golden,
                                                           const SceneSnapshot& current) {
    SceneSnapshot heatmap;
    heatmap.width = golden.width;
    heatmap.height = golden.height;

    if (golden.width != current.width || golden.height != current.height ||
        golden.pixels.size() != current.pixels.size()) {
        heatmap.pixels.assign(static_cast<size_t>(golden.width) * golden.height, {255, 0, 0, 255});
        return heatmap;
    }

    heatmap.pixels.reserve(golden.pixels.size());
    for (size_t i = 0; i < golden.pixels.size(); ++i) {
        if (golden.pixels[i] == current.pixels[i]) {
            heatmap.pixels.push_back({0, 0, 0, 255});
        } else {
            heatmap.pixels.push_back({255, 0, 0, 255});
        }
    }

    return heatmap;
}

std::optional<SceneSnapshot> VisualRegressionHarness::captureOpenGLFrame(
    const std::vector<FrameRenderCommand>& commands,
    int width,
    int height,
    std::string* errorMessage) const {
#ifdef URPG_HEADLESS
    (void)commands;
    (void)width;
    (void)height;
    if (errorMessage != nullptr) {
        *errorMessage = "Renderer-backed OpenGL capture is unavailable in headless builds.";
    }
    return std::nullopt;
#else
    const int captureWidth = std::max(width, 1);
    const int captureHeight = std::max(height, 1);

    const bool initializedVideoHere = (SDL_WasInit(SDL_INIT_VIDEO) == 0);
    if (initializedVideoHere && SDL_Init(SDL_INIT_VIDEO) != 0) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_Init(SDL_INIT_VIDEO) failed: ") + SDL_GetError();
        }
        return std::nullopt;
    }

    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;

    auto cleanup = [&]() {
        if (glContext != nullptr) {
            SDL_GL_DeleteContext(glContext);
            glContext = nullptr;
        }
        if (window != nullptr) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        if (initializedVideoHere) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
    };

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow(
        "URPG Visual Regression Capture",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        captureWidth,
        captureHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (window == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_CreateWindow failed: ") + SDL_GetError();
        }
        cleanup();
        return std::nullopt;
    }

    glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_GL_CreateContext failed: ") + SDL_GetError();
        }
        cleanup();
        return std::nullopt;
    }

    if (SDL_GL_MakeCurrent(window, glContext) != 0) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_GL_MakeCurrent failed: ") + SDL_GetError();
        }
        cleanup();
        return std::nullopt;
    }

    SDL_GL_SetSwapInterval(0);

    CaptureSurface surface(window);
    OpenGLRenderer renderer;
    renderer.onResize(captureWidth, captureHeight);
    if (!renderer.initialize(&surface)) {
        if (errorMessage != nullptr) {
            *errorMessage = "OpenGLRenderer::initialize failed for renderer-backed capture.";
        }
        renderer.shutdown();
        cleanup();
        return std::nullopt;
    }

    renderer.beginFrame();
    renderer.processFrameCommands(commands);
    glFlush();
    glFinish();

    std::vector<uint8_t> rawPixels(static_cast<size_t>(captureWidth) * captureHeight * 4, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, captureWidth, captureHeight, GL_RGBA, GL_UNSIGNED_BYTE, rawPixels.data());

    SceneSnapshot snapshot;
    snapshot.width = captureWidth;
    snapshot.height = captureHeight;
    snapshot.pixels.resize(static_cast<size_t>(captureWidth) * captureHeight);

    for (int y = 0; y < captureHeight; ++y) {
        const int sourceRow = captureHeight - 1 - y;
        for (int x = 0; x < captureWidth; ++x) {
            const size_t sourceIndex = (static_cast<size_t>(sourceRow) * captureWidth + x) * 4;
            const size_t destIndex = static_cast<size_t>(y) * captureWidth + x;
            snapshot.pixels[destIndex] = SnapshotPixel{
                rawPixels[sourceIndex + 0],
                rawPixels[sourceIndex + 1],
                rawPixels[sourceIndex + 2],
                rawPixels[sourceIndex + 3],
            };
        }
    }

    renderer.shutdown();
    cleanup();
    return snapshot;
#endif
}

std::optional<SceneSnapshot> VisualRegressionHarness::captureOpenGLScene(
    const std::function<void(urpg::OpenGLRenderer&)>& renderCallback,
    int width,
    int height,
    std::string* errorMessage) const {
#ifdef URPG_HEADLESS
    (void)renderCallback;
    (void)width;
    (void)height;
    if (errorMessage != nullptr) {
        *errorMessage = "Renderer-backed OpenGL capture is unavailable in headless builds.";
    }
    return std::nullopt;
#else
    const int captureWidth = std::max(width, 1);
    const int captureHeight = std::max(height, 1);

    const bool initializedVideoHere = (SDL_WasInit(SDL_INIT_VIDEO) == 0);
    if (initializedVideoHere && SDL_Init(SDL_INIT_VIDEO) != 0) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_Init(SDL_INIT_VIDEO) failed: ") + SDL_GetError();
        }
        return std::nullopt;
    }

    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;

    auto cleanup = [&]() {
        if (glContext != nullptr) {
            SDL_GL_DeleteContext(glContext);
            glContext = nullptr;
        }
        if (window != nullptr) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        if (initializedVideoHere) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
    };

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow(
        "URPG Visual Regression Scene Capture",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        captureWidth,
        captureHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (window == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_CreateWindow failed: ") + SDL_GetError();
        }
        cleanup();
        return std::nullopt;
    }

    glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_GL_CreateContext failed: ") + SDL_GetError();
        }
        cleanup();
        return std::nullopt;
    }

    if (SDL_GL_MakeCurrent(window, glContext) != 0) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_GL_MakeCurrent failed: ") + SDL_GetError();
        }
        cleanup();
        return std::nullopt;
    }

    SDL_GL_SetSwapInterval(0);

    CaptureSurface surface(window);
    OpenGLRenderer renderer;
    renderer.onResize(captureWidth, captureHeight);
    if (!renderer.initialize(&surface)) {
        if (errorMessage != nullptr) {
            *errorMessage = "OpenGLRenderer::initialize failed for renderer-backed scene capture.";
        }
        renderer.shutdown();
        cleanup();
        return std::nullopt;
    }

    renderer.beginFrame();
    try {
        renderCallback(renderer);
    } catch (const std::exception& ex) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("Renderer-backed scene callback failed: ") + ex.what();
        }
        renderer.shutdown();
        cleanup();
        return std::nullopt;
    } catch (...) {
        if (errorMessage != nullptr) {
            *errorMessage = "Renderer-backed scene callback failed with a non-standard exception.";
        }
        renderer.shutdown();
        cleanup();
        return std::nullopt;
    }
    glFlush();
    glFinish();

    std::vector<uint8_t> rawPixels(static_cast<size_t>(captureWidth) * captureHeight * 4, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, captureWidth, captureHeight, GL_RGBA, GL_UNSIGNED_BYTE, rawPixels.data());

    SceneSnapshot snapshot;
    snapshot.width = captureWidth;
    snapshot.height = captureHeight;
    snapshot.pixels.resize(static_cast<size_t>(captureWidth) * captureHeight);

    for (int y = 0; y < captureHeight; ++y) {
        const int sourceRow = captureHeight - 1 - y;
        for (int x = 0; x < captureWidth; ++x) {
            const size_t sourceIndex = (static_cast<size_t>(sourceRow) * captureWidth + x) * 4;
            const size_t destIndex = static_cast<size_t>(y) * captureWidth + x;
            snapshot.pixels[destIndex] = SnapshotPixel{
                rawPixels[sourceIndex + 0],
                rawPixels[sourceIndex + 1],
                rawPixels[sourceIndex + 2],
                rawPixels[sourceIndex + 3],
            };
        }
    }

    renderer.shutdown();
    cleanup();
    return snapshot;
#endif
}

std::optional<SceneSnapshot> VisualRegressionHarness::captureOpenGLEngineTick(
    const std::function<void(urpg::EngineShell&)>& setupCallback,
    int width,
    int height,
    std::string* errorMessage) const {
#ifdef URPG_HEADLESS
    (void)setupCallback;
    (void)width;
    (void)height;
    if (errorMessage != nullptr) {
        *errorMessage = "Renderer-backed OpenGL capture is unavailable in headless builds.";
    }
    return std::nullopt;
#else
    const int captureWidth = std::max(width, 1);
    const int captureHeight = std::max(height, 1);

    const bool initializedVideoHere = (SDL_WasInit(SDL_INIT_VIDEO) == 0);
    if (initializedVideoHere && SDL_Init(SDL_INIT_VIDEO) != 0) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_Init(SDL_INIT_VIDEO) failed: ") + SDL_GetError();
        }
        return std::nullopt;
    }

    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;

    auto cleanup = [&]() {
        if (glContext != nullptr) {
            SDL_GL_DeleteContext(glContext);
            glContext = nullptr;
        }
        if (window != nullptr) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        if (initializedVideoHere) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
    };

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow(
        "URPG Visual Regression EngineShell Capture",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        captureWidth,
        captureHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (window == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_CreateWindow failed: ") + SDL_GetError();
        }
        cleanup();
        return std::nullopt;
    }

    glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_GL_CreateContext failed: ") + SDL_GetError();
        }
        cleanup();
        return std::nullopt;
    }

    if (SDL_GL_MakeCurrent(window, glContext) != 0) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("SDL_GL_MakeCurrent failed: ") + SDL_GetError();
        }
        cleanup();
        return std::nullopt;
    }

    SDL_GL_SetSwapInterval(0);

    auto& shell = urpg::EngineShell::getInstance();
    auto& sceneManager = urpg::scene::SceneManager::getInstance();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }
    urpg::RenderLayer::getInstance().flush();
    shell.shutdown();

    auto surface = std::make_unique<CaptureSurface>(window);
    auto renderer = std::make_unique<OpenGLRenderer>();
    renderer->onResize(captureWidth, captureHeight);
    if (!shell.startup(std::move(surface), std::move(renderer))) {
        if (errorMessage != nullptr) {
            *errorMessage = "EngineShell::startup failed for renderer-backed capture.";
        }
        shell.shutdown();
        cleanup();
        return std::nullopt;
    }

    try {
        setupCallback(shell);
        shell.tick();
    } catch (const std::exception& ex) {
        if (errorMessage != nullptr) {
            *errorMessage = std::string("Renderer-backed EngineShell callback failed: ") + ex.what();
        }
        shell.shutdown();
        while (sceneManager.stackSize() > 0) {
            sceneManager.popScene();
        }
        urpg::RenderLayer::getInstance().flush();
        cleanup();
        return std::nullopt;
    } catch (...) {
        if (errorMessage != nullptr) {
            *errorMessage = "Renderer-backed EngineShell callback failed with a non-standard exception.";
        }
        shell.shutdown();
        while (sceneManager.stackSize() > 0) {
            sceneManager.popScene();
        }
        urpg::RenderLayer::getInstance().flush();
        cleanup();
        return std::nullopt;
    }

    glFlush();
    glFinish();

    std::vector<uint8_t> rawPixels(static_cast<size_t>(captureWidth) * captureHeight * 4, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, captureWidth, captureHeight, GL_RGBA, GL_UNSIGNED_BYTE, rawPixels.data());

    SceneSnapshot snapshot;
    snapshot.width = captureWidth;
    snapshot.height = captureHeight;
    snapshot.pixels.resize(static_cast<size_t>(captureWidth) * captureHeight);

    for (int y = 0; y < captureHeight; ++y) {
        const int sourceRow = captureHeight - 1 - y;
        for (int x = 0; x < captureWidth; ++x) {
            const size_t sourceIndex = (static_cast<size_t>(sourceRow) * captureWidth + x) * 4;
            const size_t destIndex = static_cast<size_t>(y) * captureWidth + x;
            snapshot.pixels[destIndex] = SnapshotPixel{
                rawPixels[sourceIndex + 0],
                rawPixels[sourceIndex + 1],
                rawPixels[sourceIndex + 2],
                rawPixels[sourceIndex + 3],
            };
        }
    }

    shell.shutdown();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }
    urpg::RenderLayer::getInstance().flush();
    cleanup();
    return snapshot;
#endif
}

nlohmann::json VisualRegressionHarness::buildReportJson(const std::string& testName,
                                                        const SnapshotComparisonResult& result) const {
    nlohmann::json j;
    j["testName"] = testName;
    j["matches"] = result.matches;
    j["errorPercentage"] = result.errorPercentage;
    return j;
}

} // namespace urpg::testing
