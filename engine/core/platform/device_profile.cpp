#include "engine/core/platform/device_profile.h"

#include <algorithm>

namespace urpg::platform {

DeviceProfileReport DeviceProfile::evaluate(const DeviceProfileRequest& request) const {
    return evaluateDeviceProfile(*this, request);
}

DeviceProfileReport evaluateDeviceProfile(const DeviceProfile& profile, const DeviceProfileRequest& request) {
    DeviceProfileReport report;
    const auto addIssue = [&](std::string issue) {
        report.ready = false;
        report.issues.push_back(std::move(issue));
    };
    if (request.frame_time_ms > profile.frame_time_budget_ms) {
        addIssue("frame_time_over_budget");
    }
    if (request.memory_mb > profile.memory_budget_mb) {
        addIssue("memory_over_budget");
    }
    if (request.resolution.width > profile.max_resolution.width || request.resolution.height > profile.max_resolution.height) {
        addIssue("resolution_over_budget");
    }
    if (request.storage_mb > profile.storage_budget_mb) {
        addIssue("storage_over_budget");
    }
    for (const auto& device : request.required_input_devices) {
        if (std::ranges::find(profile.input_devices, device) == profile.input_devices.end()) {
            addIssue("missing_input_device");
        }
    }
    return report;
}

} // namespace urpg::platform
