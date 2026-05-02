#include "engine/core/testing/visual_regression_harness.h"

#include "engine/core/engine_shell.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/platform/renderer_backend.h"
#include "engine/core/scene/scene_manager.h"

#ifndef URPG_HEADLESS
#include "engine/core/platform/opengl_renderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <limits>
#include <memory>

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

namespace {

constexpr const char* kCompactGoldenFormat = "urpg.visual_golden.rle.v1";
constexpr int kRleRunFieldCount = 5;
constexpr uint64_t kFnvOffsetBasis = 14695981039346656037ull;
constexpr uint64_t kFnvPrime = 1099511628211ull;

uint8_t colorChannel(float value) {
    return static_cast<uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
}

void fillRect(SceneSnapshot& snapshot,
              int x,
              int y,
              int width,
              int height,
              const SnapshotPixel& pixel) {
    const int x0 = std::clamp(x, 0, snapshot.width);
    const int y0 = std::clamp(y, 0, snapshot.height);
    const int x1 = std::clamp(x + std::max(width, 0), 0, snapshot.width);
    const int y1 = std::clamp(y + std::max(height, 0), 0, snapshot.height);

    for (int row = y0; row < y1; ++row) {
        for (int col = x0; col < x1; ++col) {
            snapshot.pixels[static_cast<size_t>(row) * snapshot.width + col] = pixel;
        }
    }
}

void clearEngineShellCaptureState() {
    auto& sceneManager = urpg::scene::SceneManager::getInstance();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }
    urpg::RenderLayer::getInstance().flush();
}

SceneSnapshot rasterizeReferenceFrame(const std::vector<FrameRenderCommand>& commands, int width, int height) {
    SceneSnapshot snapshot;
    snapshot.width = std::max(width, 1);
    snapshot.height = std::max(height, 1);
    snapshot.pixels.assign(static_cast<size_t>(snapshot.width) * snapshot.height, {0, 0, 0, 255});

    for (const auto& command : commands) {
        switch (command.type) {
        case RenderCmdType::Clear:
            snapshot.pixels.assign(static_cast<size_t>(snapshot.width) * snapshot.height, SnapshotPixel{0, 0, 0, 255});
            break;
        case RenderCmdType::Rect: {
            const auto* rect = command.tryGet<RectRenderData>();
            if (rect == nullptr) {
                break;
            }
            fillRect(snapshot,
                     static_cast<int>(command.x),
                     static_cast<int>(command.y),
                     static_cast<int>(rect->w),
                     static_cast<int>(rect->h),
                     {colorChannel(rect->r),
                      colorChannel(rect->g),
                      colorChannel(rect->b),
                      colorChannel(rect->a)});
            break;
        }
        case RenderCmdType::Text: {
            const auto* text = command.tryGet<TextRenderData>();
            if (text == nullptr) {
                break;
            }
            fillRect(snapshot,
                     static_cast<int>(command.x),
                     static_cast<int>(command.y),
                     std::max<int>(static_cast<int>(text->text.size()) * std::max(text->fontSize / 3, 1), 1),
                     std::max(text->fontSize, 1),
                     {text->r, text->g, text->b, text->a});
            break;
        }
        case RenderCmdType::Sprite: {
            const auto* sprite = command.tryGet<SpriteRenderData>();
            if (sprite == nullptr) {
                break;
            }
            fillRect(snapshot,
                     static_cast<int>(command.x),
                     static_cast<int>(command.y),
                     static_cast<int>(sprite->width),
                     static_cast<int>(sprite->height),
                     {96, 160, 224, 255});
            break;
        }
        case RenderCmdType::Tile:
            fillRect(snapshot,
                     static_cast<int>(command.x),
                     static_cast<int>(command.y),
                     48,
                     48,
                     {112, 176, 104, 255});
            break;
        }
    }

    return snapshot;
}

uint64_t stableSnapshotHash(const SceneSnapshot& snapshot, const std::vector<FrameRenderCommand>& commands) {
    uint64_t hash = kFnvOffsetBasis;
    const auto mixByte = [&](uint8_t value) {
        hash ^= value;
        hash *= kFnvPrime;
    };
    const auto mixSize = [&](size_t value) {
        for (size_t i = 0; i < sizeof(size_t); ++i) {
            mixByte(static_cast<uint8_t>((value >> (i * 8)) & 0xffu));
        }
    };

    mixSize(static_cast<size_t>(snapshot.width));
    mixSize(static_cast<size_t>(snapshot.height));
    mixSize(commands.size());
    for (const auto& pixel : snapshot.pixels) {
        mixByte(pixel.r);
        mixByte(pixel.g);
        mixByte(pixel.b);
        mixByte(pixel.a);
    }
    return hash == 0 ? 1 : hash;
}

bool decodeLegacyPixels(const nlohmann::json& pixels, GoldenSnapshot& golden) {
    if (!pixels.is_array()) {
        return false;
    }

    golden.pixels.reserve(pixels.size());
    for (const auto& p : pixels) {
        SnapshotPixel pixel;
        pixel.r = p.value("r", 0);
        pixel.g = p.value("g", 0);
        pixel.b = p.value("b", 0);
        pixel.a = p.value("a", 0);
        golden.pixels.push_back(pixel);
    }

    return true;
}

bool decodeRunLengthPixels(const nlohmann::json& runs, GoldenSnapshot& golden) {
    if (!runs.is_array()) {
        return false;
    }

    const auto expectedPixels = static_cast<size_t>(std::max(golden.width, 0)) *
                                static_cast<size_t>(std::max(golden.height, 0));
    golden.pixels.clear();
    golden.pixels.reserve(expectedPixels);

    for (const auto& run : runs) {
        if (!run.is_array() || run.size() != kRleRunFieldCount || !run[0].is_number_unsigned()) {
            return false;
        }

        const auto count = run[0].get<size_t>();
        if (count == 0 || count > expectedPixels - golden.pixels.size()) {
            return false;
        }

        const SnapshotPixel pixel{
            run[1].get<uint8_t>(),
            run[2].get<uint8_t>(),
            run[3].get<uint8_t>(),
            run[4].get<uint8_t>(),
        };
        golden.pixels.insert(golden.pixels.end(), count, pixel);
    }

    return golden.pixels.size() == expectedPixels;
}

nlohmann::json encodeRunLengthPixels(const SceneSnapshot& snapshot) {
    nlohmann::json runs = nlohmann::json::array();
    if (snapshot.pixels.empty()) {
        return runs;
    }

    SnapshotPixel current = snapshot.pixels.front();
    size_t count = 0;
    const auto flushRun = [&]() {
        runs.push_back(nlohmann::json::array({
            count,
            current.r,
            current.g,
            current.b,
            current.a,
        }));
    };

    for (const auto& pixel : snapshot.pixels) {
        if (pixel == current && count < std::numeric_limits<uint32_t>::max()) {
            ++count;
            continue;
        }

        flushRun();
        current = pixel;
        count = 1;
    }
    flushRun();

    return runs;
}

} // namespace

std::string VisualRegressionHarness::captureBackendToString(CaptureBackend backend) {
    switch (backend) {
        case CaptureBackend::OpenGL:
            return "OpenGL";
        case CaptureBackend::Headless:
            return "Headless";
        case CaptureBackend::SoftwareReference:
            return "software_reference";
        default:
            return "Unknown";
    }
}

std::vector<RendererBackendParityEntry> VisualRegressionHarness::buildLocalBackendParityMatrix() {
    std::vector<RendererBackendParityEntry> entries;
#ifdef URPG_HEADLESS
    entries.push_back({CaptureBackend::OpenGL,
                       false,
                       false,
                       false,
                       false,
                       "OpenGL capture is unavailable in headless builds."});
#else
    entries.push_back({CaptureBackend::OpenGL,
                       true,
                       true,
                       true,
                       true,
                       "OpenGL renderer-backed capture supports frame, scene, and EngineShell permutations."});
#endif
    entries.push_back({CaptureBackend::Headless,
                       true,
                       true,
                       true,
                       true,
                       "Headless backend records command streams and uses deterministic reference rasterization for capture."});
    entries.push_back({CaptureBackend::SoftwareReference,
                       true,
                       true,
                       true,
                       true,
                       "Software reference capture deterministically rasterizes frame commands without platform graphics."});
    return entries;
}

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

    if (!j.contains("width") || !j.contains("height")) {
        return std::nullopt;
    }

    golden.width = j["width"].get<int>();
    golden.height = j["height"].get<int>();
    const auto expectedPixels = static_cast<size_t>(std::max(golden.width, 0)) *
                                static_cast<size_t>(std::max(golden.height, 0));

    bool decoded = false;
    if (j.value("format", std::string{}) == kCompactGoldenFormat && j.contains("pixelRuns")) {
        decoded = decodeRunLengthPixels(j["pixelRuns"], golden);
    } else if (j.contains("pixels")) {
        decoded = decodeLegacyPixels(j["pixels"], golden);
    }

    if (!decoded || golden.pixels.size() != expectedPixels) {
        return std::nullopt;
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
    j["format"] = kCompactGoldenFormat;
    j["width"] = snapshot.width;
    j["height"] = snapshot.height;
    j["encoding"] = {
        {"kind", "rgba_rle"},
        {"runFields", {"count", "r", "g", "b", "a"}},
    };
    j["pixelRuns"] = encodeRunLengthPixels(snapshot);

    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << j.dump();
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

std::optional<SceneSnapshot> VisualRegressionHarness::captureFrame(
    CaptureBackend backend,
    const std::vector<FrameRenderCommand>& commands,
    int width,
    int height,
    std::string* errorMessage) const {
    switch (backend) {
        case CaptureBackend::OpenGL:
            return captureOpenGLFrame(commands, width, height, errorMessage);
        case CaptureBackend::Headless:
        case CaptureBackend::SoftwareReference:
            return rasterizeReferenceFrame(commands, width, height);
        default:
            if (errorMessage != nullptr) {
                *errorMessage = "Unknown renderer-backed capture backend.";
            }
            return std::nullopt;
    }
}

std::optional<CaptureFrameResult> VisualRegressionHarness::captureFrameResult(
    CaptureBackend backend,
    const std::vector<FrameRenderCommand>& commands,
    int width,
    int height,
    std::string* errorMessage) const {
    auto snapshot = captureFrame(backend, commands, width, height, errorMessage);
    if (!snapshot.has_value()) {
        return std::nullopt;
    }

    CaptureFrameResult result;
    result.snapshot = std::move(*snapshot);
    result.backendId = captureBackendToString(backend);
    result.commandCount = commands.size();
    result.stableHash = stableSnapshotHash(result.snapshot, commands);
    return result;
}

std::optional<SceneSnapshot> VisualRegressionHarness::captureScene(
    CaptureBackend backend,
    const std::function<void(urpg::RendererBackend&)>& renderCallback,
    int width,
    int height,
    std::string* errorMessage) const {
    switch (backend) {
        case CaptureBackend::OpenGL:
#ifdef URPG_HEADLESS
            (void)renderCallback;
            (void)width;
            (void)height;
            if (errorMessage != nullptr) {
                *errorMessage = "Renderer-backed OpenGL scene capture is unavailable in headless builds.";
            }
            return std::nullopt;
#else
            return captureOpenGLScene(
                [&](urpg::OpenGLRenderer& renderer) {
                    renderCallback(renderer);
                },
                width,
                height,
                errorMessage);
#endif
        case CaptureBackend::Headless:
        case CaptureBackend::SoftwareReference: {
            HeadlessRenderer renderer;
            renderer.onResize(width, height);
            if (!renderer.initialize(nullptr)) {
                if (errorMessage != nullptr) {
                    *errorMessage = "HeadlessRenderer::initialize failed for deterministic scene capture.";
                }
                return std::nullopt;
            }
            try {
                renderCallback(renderer);
            } catch (const std::exception& ex) {
                if (errorMessage != nullptr) {
                    *errorMessage = std::string("Deterministic scene callback failed: ") + ex.what();
                }
                renderer.shutdown();
                return std::nullopt;
            } catch (...) {
                if (errorMessage != nullptr) {
                    *errorMessage = "Deterministic scene callback failed with a non-standard exception.";
                }
                renderer.shutdown();
                return std::nullopt;
            }
            const auto commands = renderer.lastFrameCommands();
            renderer.shutdown();
            return rasterizeReferenceFrame(commands, width, height);
        }
        default:
            if (errorMessage != nullptr) {
                *errorMessage = "Unknown renderer-backed capture backend.";
            }
            return std::nullopt;
    }
}

std::optional<SceneSnapshot> VisualRegressionHarness::captureEngineTick(
    CaptureBackend backend,
    const std::function<void(urpg::EngineShell&)>& setupCallback,
    int width,
    int height,
    std::string* errorMessage) const {
    switch (backend) {
        case CaptureBackend::OpenGL:
            return captureOpenGLEngineTick(setupCallback, width, height, errorMessage);
        case CaptureBackend::Headless:
        case CaptureBackend::SoftwareReference: {
            auto& shell = urpg::EngineShell::getInstance();
            clearEngineShellCaptureState();
            shell.shutdown();

            auto surface = std::make_unique<HeadlessSurface>();
            auto renderer = std::make_unique<HeadlessRenderer>();
            auto* rendererPtr = renderer.get();
            renderer->onResize(width, height);
            if (!shell.startup(std::move(surface), std::move(renderer))) {
                if (errorMessage != nullptr) {
                    *errorMessage = "EngineShell::startup failed for deterministic headless capture.";
                }
                clearEngineShellCaptureState();
                return std::nullopt;
            }

            try {
                setupCallback(shell);
                shell.tick();
            } catch (const std::exception& ex) {
                if (errorMessage != nullptr) {
                    *errorMessage = std::string("Deterministic EngineShell callback failed: ") + ex.what();
                }
                shell.shutdown();
                clearEngineShellCaptureState();
                return std::nullopt;
            } catch (...) {
                if (errorMessage != nullptr) {
                    *errorMessage = "Deterministic EngineShell callback failed with a non-standard exception.";
                }
                shell.shutdown();
                clearEngineShellCaptureState();
                return std::nullopt;
            }

            const auto commands = rendererPtr->lastFrameCommands();
            shell.shutdown();
            clearEngineShellCaptureState();
            return rasterizeReferenceFrame(commands, width, height);
        }
        default:
            if (errorMessage != nullptr) {
                *errorMessage = "Unknown renderer-backed capture backend.";
            }
            return std::nullopt;
    }
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
