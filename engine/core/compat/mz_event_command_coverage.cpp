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
        {"schema", "urpg.mz_event_command_coverage.v2"},
        {"authority", "runtimes/compat_js"},
        {"native_command_matrix_authority", "engine/core/events/native_event_command_matrix"},
        {"total_command_count", total_command_count},
        {"supported_command_count", supported_command_count},
        {"unsupported_command_count", unsupported_command_count},
        {"supported_commands", supported_commands},
        {"unsupported_commands", unsupported_commands},
        {"diagnostics", std::move(diagnostic_json)},
        {"fallback_records", fallback_records},
    };
}

const std::vector<MzEventCommandCapability>& MzEventCommandCompatibilityMatrix() {
    using events::EventCommandKind;
    static const std::vector<MzEventCommandCapability> matrix = {
        {101, "show_text", true, "map_parameters_to_message", "compat_message_bridge", "", EventCommandKind::Message},
        {111, "conditional_branch", true, "map_branch_parameters", "compat_branch_bridge", "", EventCommandKind::Condition},
        {117, "call_common_event", true, "resolve_mz_common_event_id", "compat_common_event_bridge", "", EventCommandKind::CommonEvent},
        {121, "change_switch", true, "map_switch_range", "compat_switch_bridge", "", EventCommandKind::Switch},
        {122, "change_variable", true, "map_variable_operand", "compat_variable_bridge", "", EventCommandKind::Variable},
        {123, "change_self_switch", true, "map_self_switch_key", "compat_self_switch_bridge", "", EventCommandKind::SelfSwitch},
        {125, "change_gold", true, "map_operand", "compat_gold_bridge", "", EventCommandKind::Gold},
        {126, "change_item", true, "map_item_operand", "compat_item_bridge", "", EventCommandKind::Item},
        {201, "transfer_player", true, "resolve_mz_map_position", "compat_transfer_bridge", "", EventCommandKind::Transfer},
        {205, "move_route", true, "preserve_route_list", "compat_movement_route_bridge", "", EventCommandKind::MovementRoute},
        {261, "play_movie", false, "preserve_raw", "diagnostic_noop", "preserve_raw_command", EventCommandKind::Unsupported},
        {355, "script_eval", false, "preserve_raw", "controlled_compat_runtime_only", "preserve_raw_command", EventCommandKind::Unsupported},
        {357, "plugin_command", false, "preserve_raw", "compat_plugin_dispatch_or_diagnostic", "preserve_raw_command", EventCommandKind::Unsupported},
    };
    return matrix;
}

std::set<std::string> SupportedMzEventCommands() {
    std::set<std::string> result;
    for (const auto& row : MzEventCommandCompatibilityMatrix()) if (row.supported) result.insert(row.command);
    return result;
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
        const auto found = std::find_if(MzEventCommandCompatibilityMatrix().begin(),
                                        MzEventCommandCompatibilityMatrix().end(),
                                        [&](const auto& row) { return row.command == command; });
        report.fallback_records.push_back({
            {"schema", "urpg.mz_event_command_fallback.v1"},
            {"command", command},
            {"opcode", found == MzEventCommandCompatibilityMatrix().end() ? 0 : found->opcode},
            {"policy", "preserve_raw_command"},
            {"execution", found == MzEventCommandCompatibilityMatrix().end() ? "diagnostic_noop" : found->execution_strategy},
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
    std::sort(report.fallback_records.begin(), report.fallback_records.end(), [](const auto& a, const auto& b) {
        return a.value("command", "") < b.value("command", "");
    });
    report.supported_command_count = report.supported_commands.size();
    report.unsupported_command_count = report.unsupported_commands.size();
    return report;
}

MzEventCommandImportResult ImportMzEventCommand(std::string command_id, const std::string& command,
                                                const nlohmann::json& raw_command) {
    const auto found = std::find_if(MzEventCommandCompatibilityMatrix().begin(),
                                    MzEventCommandCompatibilityMatrix().end(),
                                    [&](const auto& row) { return row.command == command; });
    MzEventCommandImportResult result;
    result.command.id = std::move(command_id);
    result.command.payload = {{"compat_source", "rpg_maker_mz"}, {"mz_command", command}, {"raw", raw_command}};
    if (found != MzEventCommandCompatibilityMatrix().end() && found->supported) {
        result.supported = true;
        result.command.kind = found->native_kind;
        result.command.payload["import_strategy"] = found->import_strategy;
        result.command.payload["execution_strategy"] = found->execution_strategy;
        return result;
    }
    result.command.kind = events::EventCommandKind::Unsupported;
    result.command.compat_fallback = {
        {"schema", "urpg.mz_event_command_fallback.v1"},
        {"source", "rpg_maker_mz"},
        {"command", command},
        {"opcode", found == MzEventCommandCompatibilityMatrix().end() ? raw_command.value("code", 0) : found->opcode},
        {"policy", "preserve_raw_command"},
        {"execution", found == MzEventCommandCompatibilityMatrix().end() ? "diagnostic_noop" : found->execution_strategy},
        {"raw", raw_command},
    };
    result.diagnostics.push_back({"mz_event_command_unsupported",
                                  "MZ command is preserved for compatibility and will not be executed as a native command.",
                                  command});
    return result;
}

} // namespace urpg::compat
