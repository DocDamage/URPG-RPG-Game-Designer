#include "editor/dialogue/dialogue_graph_panel.h"

#include <utility>

namespace urpg::editor {

void DialogueGraphPanel::setGraph(urpg::dialogue::DialogueGraph graph) {
    graph_ = std::move(graph);
}

void DialogueGraphPanel::render() {
    snapshot_ = {
        {"panel", "dialogue_graph"},
        {"graph", graph_.serialize()},
        {"preview_route", graph_.previewRoute()},
    };
}

nlohmann::json DialogueGraphPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
