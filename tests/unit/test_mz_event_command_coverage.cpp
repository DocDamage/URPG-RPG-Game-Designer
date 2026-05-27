#include "engine/core/compat/mz_event_command_coverage.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MZ event command coverage separates supported and unsupported commands",
          "[compat][mz_event_commands]") {
    const std::vector<std::string> commands = {
        "show_text",
        "transfer_player",
        "change_switch",
        "conditional_branch",
        "script_eval",
    };

    const auto report = urpg::compat::AnalyzeMzEventCommandCoverage(commands);

    REQUIRE(report.total_command_count == 5);
    REQUIRE(report.supported_command_count == 4);
    REQUIRE(report.unsupported_command_count == 1);
    REQUIRE(report.unsupported_commands == std::vector<std::string>{"script_eval"});
    REQUIRE(report.diagnostics.size() == 1);
    REQUIRE(report.diagnostics[0].code == "mz_event_command_unsupported");
    REQUIRE(report.diagnostics[0].target == "script_eval");
    REQUIRE(report.toJson()["supported_command_count"] == 4);
    REQUIRE(report.toJson()["diagnostics"][0]["code"] == "mz_event_command_unsupported");
}

TEST_CASE("MZ event command support table includes current P2D command slice",
          "[compat][mz_event_commands]") {
    const auto supported = urpg::compat::SupportedMzEventCommands();

    for (const std::string command : {
             "show_text",
             "transfer_player",
             "change_switch",
             "change_variable",
             "change_self_switch",
             "change_gold",
             "change_item",
             "move_route",
             "call_common_event",
             "conditional_branch",
         }) {
        REQUIRE(supported.contains(command));
    }
}
