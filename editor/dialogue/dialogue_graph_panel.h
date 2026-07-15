#pragma once

#include "engine/core/dialogue/dialogue_graph.h"

#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <vector>

namespace urpg::editor {

class DialogueGraphPanel {
public:
    void setGraph(urpg::dialogue::DialogueGraph graph);
    void setPreviewValues(std::map<std::string, int> values);
    bool beginInteractivePreview();
    bool choosePreviewChoice(const std::string& choice_id);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::dialogue::DialogueGraph graph_;
    std::map<std::string, int> preview_values_;
    std::string preview_node_id_;
    std::vector<std::string> preview_trace_;
    std::vector<urpg::dialogue::DialogueGraphDiagnostic> preview_diagnostics_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
