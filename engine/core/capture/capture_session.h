#pragma once

#include <string>

namespace urpg::capture {

struct CaptureConfig {
    bool headless{false};
    std::string outputDirectory;
};

struct SceneCaptureState {
    std::string sceneId;
    int frame{0};
    int width{0};
    int height{0};
};

struct CaptureResult {
    bool supported{true};
    std::string status;
    std::string outputName;
};

class CaptureSession {
public:
    explicit CaptureSession(CaptureConfig config);
    [[nodiscard]] CaptureResult captureScreenshot(const SceneCaptureState& state) const;

private:
    CaptureConfig config_;
};

} // namespace urpg::capture
