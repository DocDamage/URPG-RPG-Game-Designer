#pragma once

#include "engine/core/platform/device_profile.h"

#include <string>

namespace urpg::editor::platform {

class DeviceProfilePanel {
public:
    [[nodiscard]] static std::string snapshotLabel(const urpg::platform::DeviceProfileReport& report);
};

} // namespace urpg::editor::platform
