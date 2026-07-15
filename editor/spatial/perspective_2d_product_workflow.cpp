#include "editor/spatial/perspective_2d_product_workflow.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <set>

namespace urpg::editor {

namespace {

bool isSupportedCommand(const std::string& code) {
    static const std::set<std::string> supported = {"show_text",
                                                    "show_choice",
                                                    "transfer_player",
                                                    "change_switch",
                                                    "change_variable",
                                                    "change_self_switch",
                                                    "change_gold",
                                                    "change_item",
                                                    "move_route",
                                                    "call_common_event",
                                                    "start_battle",
                                                    "open_vendor",
                                                    "conditional_branch"};
    return supported.find(code) != supported.end();
}

void countCommands(const nlohmann::json& commands, Perspective2DProductWorkflowReport& report) {
    if (!commands.is_array()) {
        return;
    }
    for (const auto& command : commands) {
        const std::string code = command.value("code", "");
        if (isSupportedCommand(code)) {
            ++report.supported_command_count;
        } else if (!code.empty()) {
            ++report.unsupported_command_count;
        }

        if (code == "transfer_player") {
            ++report.transfer_edge_count;
        }
        countCommands(command.value("true_commands", nlohmann::json::array()), report);
        countCommands(command.value("false_commands", nlohmann::json::array()), report);
    }
}

bool parseJson(const std::string& text, nlohmann::json& out_json, std::vector<std::string>& blockers,
               const std::string& blocker_code) {
    if (text.empty()) {
        blockers.push_back(blocker_code + "_missing");
        return false;
    }
    try {
        out_json = nlohmann::json::parse(text);
    } catch (const nlohmann::json::exception&) {
        blockers.push_back(blocker_code + "_invalid_json");
        return false;
    }
    return true;
}

void addBlockerIf(bool condition, std::vector<std::string>& blockers, const std::string& code) {
    if (condition && std::find(blockers.begin(), blockers.end(), code) == blockers.end()) {
        blockers.push_back(code);
    }
}

} // namespace

Perspective2DProductWorkflowReport
Perspective2DProductWorkflow::Analyze(const std::string& draft_document_json,
                                      const std::string& runtime_manifest_json,
                                      const std::string& export_package_manifest_json) {
    Perspective2DProductWorkflowReport report;

    nlohmann::json draft;
    nlohmann::json runtime;
    nlohmann::json package;
    const bool has_draft = parseJson(draft_document_json, draft, report.blockers, "p2d_draft");
    const bool has_runtime = parseJson(runtime_manifest_json, runtime, report.blockers, "p2d_runtime_manifest");
    const bool has_package = parseJson(export_package_manifest_json, package, report.blockers, "p2d_package_manifest");

    if (has_draft) {
        report.map_id = draft.value("map_id", "");
        report.draft_layer_count = draft.value("layers", nlohmann::json::array()).size();
        report.draft_tile_count = draft.value("tiles", nlohmann::json::array()).size();
        report.draft_event_count = draft.value("events", nlohmann::json::array()).size();
    }

    if (has_runtime) {
        if (report.map_id.empty()) {
            report.map_id = runtime.value("map_id", "");
        }
        report.runtime_layer_count = runtime.value("layers", nlohmann::json::array()).size();
        report.runtime_tile_count = runtime.value("tiles", nlohmann::json::array()).size();
        const auto events = runtime.value("events", nlohmann::json::array());
        report.runtime_event_count = events.size();
        for (const auto& event : events) {
            countCommands(event.value("commands", nlohmann::json::array()), report);
        }
    }

    if (has_package) {
        report.export_package_file_count = package.value("files", nlohmann::json::array()).size();
        report.package_signature_present = !package.value("package_signature", "").empty();
    }

    addBlockerIf(report.map_id.empty(), report.blockers, "p2d_product_missing_map_id");
    addBlockerIf(report.draft_layer_count == 0, report.blockers, "p2d_product_missing_draft_layers");
    addBlockerIf(report.draft_tile_count == 0, report.blockers, "p2d_product_missing_draft_tiles");
    addBlockerIf(report.runtime_layer_count == 0, report.blockers, "p2d_product_missing_runtime_layers");
    addBlockerIf(report.runtime_tile_count == 0, report.blockers, "p2d_product_missing_runtime_tiles");
    addBlockerIf(report.export_package_file_count == 0, report.blockers, "p2d_product_missing_package_files");
    addBlockerIf(!report.package_signature_present, report.blockers, "p2d_product_missing_package_signature");
    addBlockerIf(report.unsupported_command_count > 0, report.blockers, "p2d_product_unsupported_commands");

    report.ready = report.blockers.empty();
    return report;
}

} // namespace urpg::editor
