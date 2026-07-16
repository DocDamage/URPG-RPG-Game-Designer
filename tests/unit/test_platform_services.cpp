#include "engine/core/platform/platform_services.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Platform services fakes expose available unavailable and degraded capabilities",
          "[platform][services][pcq750][pcq751]") {
    const auto headless = urpg::platform::PlatformServicesProfile::headlessFake();
    REQUIRE(headless.audit().complete);
    REQUIRE(headless.audit().portable);
    REQUIRE(headless.status(urpg::platform::PlatformCapability::DisplayModes)->state ==
            urpg::platform::PlatformCapabilityState::Unavailable);

    const auto desktop = urpg::platform::PlatformServicesProfile::desktopFake();
    REQUIRE(desktop.audit().complete);
    REQUIRE(desktop.audit().portable);
    REQUIRE(desktop.status(urpg::platform::PlatformCapability::Presence)->state ==
            urpg::platform::PlatformCapabilityState::Degraded);
    REQUIRE(desktop.status(urpg::platform::PlatformCapability::Storage)->state ==
            urpg::platform::PlatformCapabilityState::Available);
}

TEST_CASE("Platform audit rejects missing required capabilities and direct desktop assumptions",
          "[platform][services][pcq750][pcq751]") {
    auto profile = urpg::platform::PlatformServicesProfile::desktopFake();
    REQUIRE(profile.setCapability({urpg::platform::PlatformCapability::Storage,
                                   urpg::platform::PlatformCapabilityState::Unavailable,
                                   "win32-path", "Storage unavailable.", true, true}));
    const auto report = profile.audit();
    REQUIRE(report.complete);
    REQUIRE_FALSE(report.portable);
    REQUIRE(report.diagnostics.size() == 2);
}
