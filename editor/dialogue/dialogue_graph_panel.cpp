#include "editor/dialogue/dialogue_graph_panel.h"

#include <utility>

namespace urpg::editor {

void DialogueGraphPanel::setGraph(urpg::dialogue::DialogueGraph graph) {
    graph_ = std::move(graph);
}

void DialogueGraphPanel::render() {
    const auto route = graph_.previewRoute();
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
    snapshot_ = {
        {"panel", "dialogue_graph"},
        {"graph", graph_.serialize()},
        {"preview_route", route},
        {"node_count", graph_.nodes().size()},
        {"choice_count", choice_count},
        {"ending_count", ending_count},
        {"has_start_node", has_start},
        {"route_coverage", route_coverage},
        {"ux_focus_lane", has_start ? "route_preview" : "start_node"},
        {"primary_action", has_start ? "Preview route, choices, and ending coverage." : "Select a valid start node."},
    };
}

nlohmann::json DialogueGraphPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
