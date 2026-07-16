#include "engine/core/platform/platform_services.h"

#include <algorithm>
#include <array>
#include <set>
#include <utility>

namespace urpg::platform {
namespace {

constexpr std::array<PlatformCapability, 13> kCapabilities = {
    PlatformCapability::Lifecycle, PlatformCapability::Users, PlatformCapability::Storage,
    PlatformCapability::InputDevices, PlatformCapability::DisplayModes, PlatformCapability::Achievements,
    PlatformCapability::Presence, PlatformCapability::NetworkStatus, PlatformCapability::VirtualKeyboard,
    PlatformCapability::Locale, PlatformCapability::Clock, PlatformCapability::PowerSuspend,
    PlatformCapability::ErrorPresentation};

} // namespace

const char* platformCapabilityName(const PlatformCapability capability) {
    switch (capability) {
    case PlatformCapability::Lifecycle: return "lifecycle";
    case PlatformCapability::Users: return "users";
    case PlatformCapability::Storage: return "storage";
    case PlatformCapability::InputDevices: return "input_devices";
    case PlatformCapability::DisplayModes: return "display_modes";
    case PlatformCapability::Achievements: return "achievements";
    case PlatformCapability::Presence: return "presence";
    case PlatformCapability::NetworkStatus: return "network_status";
    case PlatformCapability::VirtualKeyboard: return "virtual_keyboard";
    case PlatformCapability::Locale: return "locale";
    case PlatformCapability::Clock: return "clock";
    case PlatformCapability::PowerSuspend: return "power_suspend";
    case PlatformCapability::ErrorPresentation: return "error_presentation";
    }
    return "unknown";
}

const char* platformCapabilityStateName(const PlatformCapabilityState state) {
    switch (state) {
    case PlatformCapabilityState::Available: return "available";
    case PlatformCapabilityState::Unavailable: return "unavailable";
    case PlatformCapabilityState::Degraded: return "degraded";
    }
    return "unknown";
}

PlatformServicesProfile::PlatformServicesProfile(std::string id) : id_(std::move(id)) {}

bool PlatformServicesProfile::setCapability(PlatformCapabilityStatus status) {
    if (status.adapter_id.empty() ||
        (status.state != PlatformCapabilityState::Available && status.diagnostic.empty())) return false;
    const auto existing = std::find_if(capabilities_.begin(), capabilities_.end(), [&](const auto& row) {
        return row.capability == status.capability;
    });
    if (existing != capabilities_.end()) *existing = std::move(status);
    else capabilities_.push_back(std::move(status));
    return true;
}

const PlatformCapabilityStatus* PlatformServicesProfile::status(const PlatformCapability capability) const {
    const auto row = std::find_if(capabilities_.begin(), capabilities_.end(), [&](const auto& value) {
        return value.capability == capability;
    });
    return row == capabilities_.end() ? nullptr : &*row;
}

PlatformServicesReport PlatformServicesProfile::audit() const {
    PlatformServicesReport report;
    if (id_.empty()) report.diagnostics.push_back("platform_profile_id_missing");
    for (const auto capability : kCapabilities) {
        const auto* row = status(capability);
        if (row == nullptr) {
            report.diagnostics.push_back(std::string("platform_capability_missing:") + platformCapabilityName(capability));
            continue;
        }
        if (row->runtime_required && row->state == PlatformCapabilityState::Unavailable) {
            report.diagnostics.push_back(std::string("platform_required_capability_unavailable:") + platformCapabilityName(capability));
        }
        if (row->runtime_required && row->direct_desktop_assumption) {
            report.diagnostics.push_back(std::string("platform_direct_desktop_assumption:") + platformCapabilityName(capability));
        }
    }
    report.complete = capabilities_.size() == kCapabilities.size();
    report.portable = report.complete && report.diagnostics.empty();
    return report;
}

PlatformServicesProfile PlatformServicesProfile::headlessFake() {
    PlatformServicesProfile profile("headless_fake");
    for (const auto capability : kCapabilities) {
        const bool required = capability == PlatformCapability::Lifecycle || capability == PlatformCapability::Storage ||
                              capability == PlatformCapability::Locale || capability == PlatformCapability::Clock ||
                              capability == PlatformCapability::ErrorPresentation;
        const bool available = required || capability == PlatformCapability::NetworkStatus;
        profile.setCapability({capability, available ? PlatformCapabilityState::Available : PlatformCapabilityState::Unavailable,
                               "headless_fake:" + std::string(platformCapabilityName(capability)),
                               available ? "" : "Capability intentionally unavailable in headless mode.", required, false});
    }
    return profile;
}

PlatformServicesProfile PlatformServicesProfile::desktopFake() {
    PlatformServicesProfile profile("desktop_fake");
    for (const auto capability : kCapabilities) {
        const bool degraded = capability == PlatformCapability::Presence || capability == PlatformCapability::VirtualKeyboard;
        profile.setCapability({capability, degraded ? PlatformCapabilityState::Degraded : PlatformCapabilityState::Available,
                               "desktop_fake:" + std::string(platformCapabilityName(capability)),
                               degraded ? "Fake exposes a deterministic degraded capability." : "", true, false});
    }
    return profile;
}

} // namespace urpg::platform
