#include "engine/core/compat/mz_event_command_coverage.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

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
    REQUIRE(report.toJson()["schema"] == "urpg.mz_event_command_coverage.v2");
    REQUIRE(report.toJson()["authority"] == "runtimes/compat_js");
    REQUIRE(report.fallback_records[0]["policy"] == "preserve_raw_command");
}

TEST_CASE("MZ compatibility matrix imports supported commands and preserves unsupported raw fallbacks",
          "[compat][mz_event_commands][matrix][pcq407]") {
    const auto& matrix = urpg::compat::MzEventCommandCompatibilityMatrix();
    REQUIRE(matrix.size() == 13);
    REQUIRE(std::all_of(matrix.begin(), matrix.end(), [](const auto& row) {
        return row.opcode != 0 && !row.command.empty() && !row.import_strategy.empty() &&
               !row.execution_strategy.empty() && (row.supported || row.unsupported_fallback == "preserve_raw_command");
    }));

    const auto supported = urpg::compat::ImportMzEventCommand(
        "mz121", "change_switch", {{"code", 121}, {"parameters", {1, 1, 0}}});
    REQUIRE(supported.supported);
    REQUIRE(supported.command.kind == urpg::events::EventCommandKind::Switch);
    REQUIRE(supported.command.payload["compat_source"] == "rpg_maker_mz");
    REQUIRE(supported.command.compat_fallback.empty());

    const auto unsupported = urpg::compat::ImportMzEventCommand(
        "mz355", "script_eval", {{"code", 355}, {"parameters", {"dangerousCall()"}}});
    REQUIRE_FALSE(unsupported.supported);
    REQUIRE(unsupported.command.kind == urpg::events::EventCommandKind::Unsupported);
    REQUIRE(unsupported.command.compat_fallback["schema"] == "urpg.mz_event_command_fallback.v1");
    REQUIRE(unsupported.command.compat_fallback["execution"] == "controlled_compat_runtime_only");
    REQUIRE(unsupported.command.compat_fallback["raw"]["parameters"][0] == "dangerousCall()");
    REQUIRE(unsupported.diagnostics[0].code == "mz_event_command_unsupported");

    const auto unknown = urpg::compat::ImportMzEventCommand(
        "mz777", "unknown_777", {{"code", 777}, {"parameters", {"opaque"}}});
    REQUIRE_FALSE(unknown.supported);
    REQUIRE(unknown.command.compat_fallback["opcode"] == 777);
    REQUIRE(unknown.command.compat_fallback["execution"] == "diagnostic_noop");

    urpg::events::EventDocument native_document;
    native_document.addMap({"town", 10, 10});
    native_document.addEvent({"compat", "town", 0, 0,
                              {{"page", 0, urpg::events::EventTrigger::ActionButton, {},
                                {supported.command, unsupported.command, unknown.command}}}});
    const auto round_trip = urpg::events::EventDocument::fromJson(native_document.toJson()).toJson();
    REQUIRE(round_trip["events"][0]["pages"][0]["commands"][1]["kind"] == "unsupported");
    REQUIRE(round_trip["events"][0]["pages"][0]["commands"][1]["_compat_command_fallbacks"]["raw"]["code"] == 355);
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
