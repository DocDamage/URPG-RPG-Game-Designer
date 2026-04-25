#pragma once

#include <string>
#include <vector>

namespace urpg::platform {

struct Resolution {
    int width{0};
    int height{0};
};

struct DeviceProfileRequest {
    double frame_time_ms{0.0};
    int memory_mb{0};
    Resolution resolution;
    int storage_mb{0};
    std::vector<std::string> required_input_devices;
};

struct DeviceProfileReport {
    bool ready{true};
    std::vector<std::string> issues;
};

struct DeviceProfile {
    std::string id;
    double frame_time_budget_ms{16.6};
    int memory_budget_mb{0};
    Resolution max_resolution;
    int storage_budget_mb{0};
    std::vector<std::string> input_devices;

    [[nodiscard]] DeviceProfileReport evaluate(const DeviceProfileRequest& request) const;
};

[[nodiscard]] DeviceProfileReport evaluateDeviceProfile(const DeviceProfile& profile, const DeviceProfileRequest& request);

} // namespace urpg::platform
