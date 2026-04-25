#include "engine/core/capture/capture_session.h"

#include <iomanip>
#include <sstream>
#include <utility>

namespace urpg::capture {

namespace {

std::string stableName(const std::string& scene_id, int frame) {
    std::ostringstream out;
    out << scene_id << "_" << std::setw(6) << std::setfill('0') << frame << ".png";
    return out.str();
}

} // namespace

CaptureSession::CaptureSession(CaptureConfig config)
    : config_(std::move(config)) {}

CaptureResult CaptureSession::captureScreenshot(const SceneCaptureState& state) const {
    const auto output_name = stableName(state.sceneId, state.frame);
    if (config_.headless) {
        return {false, "unsupported_headless", output_name};
    }
    if (config_.outputDirectory.empty()) {
        return {false, "unwritable_output", output_name};
    }
    return {true, "captured", output_name};
}

} // namespace urpg::capture
