#include "editor/quest/quest_panel.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

void QuestPanel::setRegistry(urpg::quest::QuestRegistry registry) {
    registry_ = std::move(registry);
}

void QuestPanel::bindObjectiveGraph(urpg::quest::QuestObjectiveGraphDocument graph) {
    graph_ = std::move(graph);
    selected_node_id_.clear();
}

void QuestPanel::clearObjectiveGraph() {
    graph_.reset();
    selected_node_id_.clear();
}

void QuestPanel::setPreviewWorldState(urpg::quest::QuestWorldState world) {
    preview_world_ = std::move(world);
}

bool QuestPanel::selectGraphNode(const std::string& node_id) {
    if (!graph_.has_value()) {
        return false;
    }
    const auto it = std::find_if(graph_->nodes.begin(), graph_->nodes.end(),
                                 [&](const urpg::quest::QuestGraphNode& node) { return node.id == node_id; });
    if (it == graph_->nodes.end()) {
        return false;
    }
    selected_node_id_ = node_id;
    return true;
}

bool QuestPanel::applyReadyGraphObjectives(const std::string& timestamp) {
    if (!graph_.has_value()) {
        return false;
    }
    const auto result = graph_->applyReadyObjectives(registry_, preview_world_, timestamp);
    return result.diagnostics.empty() && !result.completed_objective_ids.empty();
}

void QuestPanel::render() {
    snapshot_ = {{"panel", "quest"}, {"registry", registry_.serialize()}};
    if (!graph_.has_value()) {
        snapshot_["graph_bound"] = false;
        return;
    }

    nlohmann::json nodes = nlohmann::json::array();
    nlohmann::json selected = nullptr;
    for (const auto& node : graph_->nodes) {
        nlohmann::json rewards = nlohmann::json::array();
        for (const auto& reward : node.rewards) {
            rewards.push_back({{"type", reward.type}, {"id", reward.id}, {"value", reward.value}});
        }
        nlohmann::json node_json = {{"id", node.id},
                                    {"type", node.type},
                                    {"title", node.title},
                                    {"objective_id", node.objective_id},
                                    {"localization_key", node.localization_key},
                                    {"condition_count", node.conditions.size()},
                                    {"reward_count", node.rewards.size()},
                                    {"rewards", std::move(rewards)}};
        if (node.id == selected_node_id_) {
            selected = node_json;
        }
        nodes.push_back(std::move(node_json));
    }

    nlohmann::json links = nlohmann::json::array();
    for (const auto& link : graph_->links) {
        links.push_back({{"from", link.from}, {"to", link.to}});
    }

    snapshot_["graph_bound"] = true;
    snapshot_["graph"] = {{"quest_id", graph_->quest_id},
                          {"title", graph_->title},
                          {"nodes", std::move(nodes)},
                          {"links", std::move(links)},
                          {"selected_node_id", selected_node_id_.empty() ? nlohmann::json(nullptr)
                                                                          : nlohmann::json(selected_node_id_)},
                          {"selected_node", selected},
                          {"preview", urpg::quest::questGraphPreviewToJson(graph_->preview(preview_world_))}};
}

nlohmann::json QuestPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
