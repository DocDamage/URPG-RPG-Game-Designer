#include "editor/progression/skill_tree_panel.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

void SkillTreePanel::bindDocument(urpg::progression::SkillTreeDocument document) {
    document_ = std::move(document);
    selected_node_id_.clear();
}

void SkillTreePanel::setState(urpg::progression::SkillTreeState state) {
    state_ = std::move(state);
}

void SkillTreePanel::setKnownAbilities(std::set<std::string> known_abilities) {
    known_abilities_ = std::move(known_abilities);
}

bool SkillTreePanel::selectNode(const std::string& node_id) {
    const auto it = std::find_if(document_.nodes.begin(), document_.nodes.end(),
                                 [&](const urpg::progression::SkillTreeNode& node) { return node.id == node_id; });
    if (it == document_.nodes.end()) {
        return false;
    }
    selected_node_id_ = node_id;
    return true;
}

bool SkillTreePanel::unlockSelectedNode() {
    if (selected_node_id_.empty()) {
        return false;
    }
    return document_.unlockNode(selected_node_id_, state_, known_abilities_);
}

void SkillTreePanel::render() {
    nlohmann::json nodes = nlohmann::json::array();
    nlohmann::json selected = nullptr;
    for (const auto& node : document_.nodes) {
        nlohmann::json node_json = {{"id", node.id},
                                    {"title", node.title},
                                    {"class_id", node.class_id},
                                    {"ability_id", node.ability_id},
                                    {"prerequisites", node.prerequisites},
                                    {"cost", {{"currency", node.cost.currency}, {"amount", node.cost.amount}}},
                                    {"learned", state_.learned_nodes.contains(node.id)}};
        if (node.id == selected_node_id_) {
            selected = node_json;
        }
        nodes.push_back(std::move(node_json));
    }

    snapshot_ = {{"panel", "skill_tree"},
                 {"tree_id", document_.tree_id},
                 {"title", document_.title},
                 {"available_points", state_.available_points},
                 {"selected_node_id", selected_node_id_.empty() ? nlohmann::json(nullptr)
                                                                : nlohmann::json(selected_node_id_)},
                 {"selected_node", selected},
                 {"nodes", std::move(nodes)},
                 {"preview", urpg::progression::skillTreePreviewToJson(document_.preview(state_, known_abilities_))}};
}

nlohmann::json SkillTreePanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
