#include "editor/dialogue/dialogue_graph_panel.h"

#include <utility>

namespace urpg::editor {

void DialogueGraphPanel::setGraph(urpg::dialogue::DialogueGraph graph) {
    graph_ = std::move(graph);
    preview_node_id_.clear();
    preview_trace_.clear();
    preview_diagnostics_.clear();
}

void DialogueGraphPanel::setPreviewValues(std::map<std::string, int> values) {
    preview_values_ = std::move(values);
    preview_diagnostics_.clear();
}

bool DialogueGraphPanel::beginInteractivePreview() {
    if (graph_.startNode().empty() || graph_.findNode(graph_.startNode()) == nullptr) {
        preview_diagnostics_ = {{"preview_start_node_missing", "Select a valid start node before previewing choices.", "", ""}};
        return false;
    }
    preview_node_id_ = graph_.startNode();
    preview_trace_ = {preview_node_id_};
    preview_diagnostics_.clear();
    return true;
}

bool DialogueGraphPanel::choosePreviewChoice(const std::string& choice_id) {
    if (preview_node_id_.empty() && !beginInteractivePreview()) {
        return false;
    }
    if (preview_trace_.size() >= 64) {
        preview_diagnostics_ = {{"preview_step_limit_reached",
                                 "Dialogue preview stopped after 64 selected choices.", preview_node_id_, choice_id}};
        return false;
    }
    const auto transition = graph_.previewChoice(preview_node_id_, choice_id, preview_values_);
    preview_diagnostics_ = transition.diagnostics;
    if (!transition.applied) {
        return false;
    }
    preview_values_ = transition.values;
    preview_node_id_ = transition.next_node_id;
    preview_trace_.push_back(preview_node_id_);
    return true;
}

void DialogueGraphPanel::render() {
    const auto route = graph_.previewRoute();
    auto diagnostics = graph_.validate();
    const auto flow_diagnostics = graph_.analyzeFlow();
    diagnostics.insert(diagnostics.end(), flow_diagnostics.begin(), flow_diagnostics.end());
    std::size_t choice_count = 0;
    std::size_t ending_count = 0;
    for (const auto& [id, node] : graph_.nodes()) {
        (void)id;
        choice_count += node.choices.size();
        if (node.ending) {
            ++ending_count;
        }
    }
    const bool has_start = !graph_.startNode().empty() && graph_.findNode(graph_.startNode()) != nullptr;
    const float route_coverage = graph_.nodes().empty()
        ? 0.0f
        : static_cast<float>(route.size()) / static_cast<float>(graph_.nodes().size());
    nlohmann::json diagnostic_rows = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics) {
        diagnostic_rows.push_back({{"code", diagnostic.code},
                                   {"message", diagnostic.message},
                                   {"node_id", diagnostic.node_id},
                                   {"choice_id", diagnostic.choice_id}});
    }
    nlohmann::json preview_values = nlohmann::json::object();
    for (const auto& [key, value] : preview_values_) {
        preview_values[key] = value;
    }
    nlohmann::json interactive_choices = nlohmann::json::array();
    if (!preview_node_id_.empty()) {
        for (const auto& choice : graph_.previewChoices(preview_node_id_, preview_values_)) {
            nlohmann::json choice_diagnostics = nlohmann::json::array();
            for (const auto& diagnostic : choice.diagnostics) {
                choice_diagnostics.push_back({{"code", diagnostic.code}, {"message", diagnostic.message}});
            }
            interactive_choices.push_back({{"id", choice.id},
                                           {"label", choice.label},
                                           {"localization_key", choice.localization_key},
                                           {"target_node_id", choice.target_node_id},
                                           {"enabled", choice.enabled},
                                           {"diagnostics", std::move(choice_diagnostics)}});
        }
    }
    nlohmann::json interactive_diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : preview_diagnostics_) {
        interactive_diagnostics.push_back({{"code", diagnostic.code},
                                           {"message", diagnostic.message},
                                           {"node_id", diagnostic.node_id},
                                           {"choice_id", diagnostic.choice_id}});
    }
    snapshot_ = {
        {"panel", "dialogue_graph"},
        {"graph", graph_.serialize()},
        {"preview_route", route},
        {"node_count", graph_.nodes().size()},
        {"choice_count", choice_count},
        {"ending_count", ending_count},
        {"has_start_node", has_start},
        {"route_coverage", route_coverage},
        {"diagnostics", std::move(diagnostic_rows)},
        {"diagnostic_count", diagnostics.size()},
        {"ux_focus_lane", has_start ? "route_preview" : "start_node"},
        {"primary_action", has_start ? "Preview route, choices, and ending coverage." : "Select a valid start node."},
        {"interactive_preview", {{"active_node_id", preview_node_id_},
                                  {"trace", preview_trace_},
                                  {"values", std::move(preview_values)},
                                  {"choices", std::move(interactive_choices)},
                                  {"diagnostics", std::move(interactive_diagnostics)},
                                  {"non_persistent", true}}},
    };
}

nlohmann::json DialogueGraphPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
