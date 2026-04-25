#include "editor/platform/device_profile_panel.h"

namespace urpg::editor::platform {

std::string DeviceProfilePanel::snapshotLabel(const urpg::platform::DeviceProfileReport& report) {
    return report.ready ? "device:ready" : "device:over-budget";
}

} // namespace urpg::editor::platform
