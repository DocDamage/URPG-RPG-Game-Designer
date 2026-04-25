#pragma once

#include "engine/core/capture/capture_session.h"

#include <string>

namespace urpg::editor::capture {

class CapturePanel {
public:
    [[nodiscard]] static std::string snapshotLabel(const urpg::capture::CaptureResult& result);
};

} // namespace urpg::editor::capture
