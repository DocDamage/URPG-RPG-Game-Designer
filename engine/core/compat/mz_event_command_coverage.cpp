#include "engine/core/compat/mz_event_command_coverage.h"

#include <algorithm>

namespace urpg::compat {

namespace {

void sortUnique(std::vector<std::string>& values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

} // namespace

nlohmann::json MzEventCommandDiagnostic::toJson() const {
    return {
        {"code", code},
        {"message", message},
        {"target", target},
    };
}

nlohmann::json MzEventCommandCoverageReport::toJson() const {
    nlohmann::json diagnostic_json = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics) {
        diagnostic_json.push_back(diagnostic.toJson());
    }

    return {
        {"total_command_count", total_command_count},
        {"supported_command_count", supported_command_count},
        {"unsupported_command_count", unsupported_command_count},
        {"supported_commands", supported_commands},
        {"unsupported_commands", unsupported_commands},
        {"diagnostics", std::move(diagnostic_json)},
    };
}

std::set<std::string> SupportedMzEventCommands() {
    return {
        "call_common_event",
        "change_gold",
        "change_item",
        "change_self_switch",
        "change_switch",
        "change_variable",
        "conditional_branch",
        "move_route",
        "show_text",
        "transfer_player",
    };
}

MzEventCommandCoverageReport AnalyzeMzEventCommandCoverage(const std::vector<std::string>& commands) {
    const auto supported_table = SupportedMzEventCommands();
    MzEventCommandCoverageReport report;
    report.total_command_count = commands.size();

    for (const auto& command : commands) {
        if (supported_table.contains(command)) {
            report.supported_commands.push_back(command);
            continue;
        }

        report.unsupported_commands.push_back(command);
        report.diagnostics.push_back({
            "mz_event_command_unsupported",
            "RPG Maker MZ event command is not covered by the current native compatibility command set.",
            command,
        });
    }

    sortUnique(report.supported_commands);
    sortUnique(report.unsupported_commands);
    std::sort(report.diagnostics.begin(), report.diagnostics.end(), [](const auto& a, const auto& b) {
        if (a.target != b.target) {
            return a.target < b.target;
        }
        return a.code < b.code;
    });
    report.supported_command_count = report.supported_commands.size();
    report.unsupported_command_count = report.unsupported_commands.size();
    return report;
}

} // namespace urpg::compat
