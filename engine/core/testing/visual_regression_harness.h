#pragma once

#include "engine/core/render/render_layer.h"
#include "engine/core/testing/snapshot_validator.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace urpg {
class OpenGLRenderer;
class EngineShell;
class RendererBackend;
}

namespace urpg::testing {

enum class CaptureBackend : uint8_t {
    OpenGL = 0,
    Headless = 1,
};

struct GoldenSnapshot {
    int width = 0;
    int height = 0;
    std::vector<SnapshotPixel> pixels;
    std::string testName;
    std::string snapshotId;
};

class VisualRegressionHarness {
public:
    void setGoldenRoot(const std::string& path);

    static std::string captureBackendToString(CaptureBackend backend);

    std::optional<GoldenSnapshot> loadGolden(const std::string& testName,
                                             const std::string& snapshotId);

    bool saveGolden(const std::string& testName,
                    const std::string& snapshotId,
                    const SceneSnapshot& snapshot);

    SnapshotComparisonResult compareAgainstGolden(const std::string& testName,
                                                  const std::string& snapshotId,
                                                  const SceneSnapshot& current,
                                                  float thresholdPercentage = 0.0f);

    static SceneSnapshot generateDiffHeatmap(const SceneSnapshot& golden,
                                             const SceneSnapshot& current);

    std::optional<SceneSnapshot> captureFrame(CaptureBackend backend,
                                              const std::vector<FrameRenderCommand>& commands,
                                              int width,
                                              int height,
                                              std::string* errorMessage = nullptr) const;

    std::optional<SceneSnapshot> captureScene(CaptureBackend backend,
                                              const std::function<void(urpg::RendererBackend&)>& renderCallback,
                                              int width,
                                              int height,
                                              std::string* errorMessage = nullptr) const;

    std::optional<SceneSnapshot> captureEngineTick(CaptureBackend backend,
                                                   const std::function<void(urpg::EngineShell&)>& setupCallback,
                                                   int width,
                                                   int height,
                                                   std::string* errorMessage = nullptr) const;

    std::optional<SceneSnapshot> captureOpenGLFrame(const std::vector<FrameRenderCommand>& commands,
                                                    int width,
                                                    int height,
                                                    std::string* errorMessage = nullptr) const;

    std::optional<SceneSnapshot> captureOpenGLScene(const std::function<void(urpg::OpenGLRenderer&)>& renderCallback,
                                                    int width,
                                                    int height,
                                                    std::string* errorMessage = nullptr) const;

    std::optional<SceneSnapshot> captureOpenGLEngineTick(const std::function<void(urpg::EngineShell&)>& setupCallback,
                                                         int width,
                                                         int height,
                                                         std::string* errorMessage = nullptr) const;

    nlohmann::json buildReportJson(const std::string& testName,
                                   const SnapshotComparisonResult& result) const;

private:
    std::string m_goldenRoot;

    std::string buildGoldenPath(const std::string& testName,
                                const std::string& snapshotId) const;
};

} // namespace urpg::testing
