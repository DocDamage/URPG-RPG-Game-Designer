#include "runtimes/compat_js/mz_runtime_parity_mode.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MZ runtime parity mode defaults to bridge-only and requires explicit opt-in",
          "[compat][mz_runtime_parity]") {
    const auto defaults = urpg::compat_js::MzRuntimeParityConfig{};

    REQUIRE(defaults.mode == urpg::compat_js::MzRuntimeParityMode::BridgeOnly);
    REQUIRE_FALSE(defaults.runtimeParityEnabled());
    REQUIRE(defaults.toJson()["mode"] == "bridge_only");
    REQUIRE(defaults.toJson()["release_authoritative"] == false);
    REQUIRE(defaults.toJson()["diagnostic"] ==
            "mz_runtime_parity_is_experimental_and_not_part_of_compat_bridge_exit_ready_scope");
}

TEST_CASE("MZ runtime parity mode reports experimental opt-in without promoting release claim",
          "[compat][mz_runtime_parity]") {
    urpg::compat_js::MzRuntimeParityConfig config;
    config.mode = urpg::compat_js::MzRuntimeParityMode::RuntimeParityExperimental;

    REQUIRE(config.runtimeParityEnabled());
    REQUIRE(config.toJson()["mode"] == "runtime_parity_experimental");
    REQUIRE(config.toJson()["release_authoritative"] == false);
}
