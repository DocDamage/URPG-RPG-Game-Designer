#pragma once

#include "engine/core/dialogue/dialogue_graph.h"

#include <nlohmann/json.hpp>
#include <string>

namespace urpg::editor {

class DialogueGraphPanel {
public:
    void setGraph(urpg::dialogue::DialogueGraph graph);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::dialogue::DialogueGraph graph_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
