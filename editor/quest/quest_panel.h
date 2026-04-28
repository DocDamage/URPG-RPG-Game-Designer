#pragma once

#include "engine/core/quest/quest_objective_graph.h"
#include "engine/core/quest/quest_registry.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace urpg::editor {

class QuestPanel {
public:
    void setRegistry(urpg::quest::QuestRegistry registry);
    void bindObjectiveGraph(urpg::quest::QuestObjectiveGraphDocument graph);
    void clearObjectiveGraph();
    void setPreviewWorldState(urpg::quest::QuestWorldState world);
    bool selectGraphNode(const std::string& node_id);
    bool applyReadyGraphObjectives(const std::string& timestamp);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::quest::QuestRegistry registry_;
    std::optional<urpg::quest::QuestObjectiveGraphDocument> graph_;
    urpg::quest::QuestWorldState preview_world_;
    std::string selected_node_id_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
