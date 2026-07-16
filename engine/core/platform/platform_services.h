#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::platform {

enum class PlatformCapability : uint8_t {
    Lifecycle,
    Users,
    Storage,
    InputDevices,
    DisplayModes,
    Achievements,
    Presence,
    NetworkStatus,
    VirtualKeyboard,
    Locale,
    Clock,
    PowerSuspend,
    ErrorPresentation
};

enum class PlatformCapabilityState : uint8_t { Available, Unavailable, Degraded };

struct PlatformCapabilityStatus {
    PlatformCapability capability = PlatformCapability::Lifecycle;
    PlatformCapabilityState state = PlatformCapabilityState::Unavailable;
    std::string adapter_id;
    std::string diagnostic;
    bool runtime_required = false;
    bool direct_desktop_assumption = false;
};

struct PlatformServicesReport {
    bool complete = false;
    bool portable = false;
    std::vector<std::string> diagnostics;
};

class PlatformServicesProfile {
public:
    explicit PlatformServicesProfile(std::string id = {});
    bool setCapability(PlatformCapabilityStatus status);
    const PlatformCapabilityStatus* status(PlatformCapability capability) const;
    PlatformServicesReport audit() const;

    const std::string& id() const { return id_; }
    const std::vector<PlatformCapabilityStatus>& capabilities() const { return capabilities_; }

    static PlatformServicesProfile headlessFake();
    static PlatformServicesProfile desktopFake();

private:
    std::string id_;
    std::vector<PlatformCapabilityStatus> capabilities_;
};

const char* platformCapabilityName(PlatformCapability capability);
const char* platformCapabilityStateName(PlatformCapabilityState state);

} // namespace urpg::platform
