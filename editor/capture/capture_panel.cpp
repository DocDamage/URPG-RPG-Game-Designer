#include "editor/capture/capture_panel.h"

namespace urpg::editor::capture {

std::string CapturePanel::snapshotLabel(const urpg::capture::CaptureResult& result) {
    return result.supported ? "capture:ready" : "capture:" + result.status;
}

} // namespace urpg::editor::capture
