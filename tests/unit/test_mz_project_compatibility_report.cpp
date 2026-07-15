#include "engine/core/compat/mz_project_compatibility_report.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MZ project compatibility report aggregates lane scores and blockers",
          "[compat][mz_project_report]") {
    urpg::compat::MzProjectCompatibilityInput input;
    input.maps = {100, 1, 0, "maps"};
    input.events = {35, 10, 2, "events"};
    input.plugins = {65, 10, 1, "plugins"};
    input.saves = {80, 1, 0, "saves"};
    input.assets = {80, 0, 0, "assets"};
    input.visual_parity = {0, 0, 0, "visual_parity"};
    input.runtime_parity = {75, 0, 0, "runtime_parity"};
    input.unsupported_event_command_count = 2;

    const auto report = urpg::compat::BuildMzProjectCompatibilityReport(input);

    REQUIRE(report.project_score == 72);
    REQUIRE(report.blockers == std::vector<std::string>{"unsupported_event_commands"});
    REQUIRE(report.lanes.at("plugins").score == 65);
    REQUIRE(report.lanes.at("events").unsupported_count == 2);
    REQUIRE(report.toJson()["project_score"] == 72);
    REQUIRE(report.toJson()["lanes"]["plugins"]["score"] == 65);
    REQUIRE(report.toJson()["blockers"][0] == "unsupported_event_commands");
    REQUIRE(report.release_authoritative == false);
}

TEST_CASE("MZ project compatibility report flags zero-score present lanes",
          "[compat][mz_project_report]") {
    urpg::compat::MzProjectCompatibilityInput input;
    input.maps = {100, 1, 0, "maps"};
    input.assets = {0, 3, 3, "assets"};

    const auto report = urpg::compat::BuildMzProjectCompatibilityReport(input);

    REQUIRE(report.project_score == 50);
    REQUIRE(report.blockers == std::vector<std::string>{"assets_unready"});
    REQUIRE(report.lanes.at("assets").present);
}
