#include "engine/core/progression/skill_tree.h"

#include <algorithm>
#include <map>

namespace urpg::progression {
namespace {

SkillTreeCost costFromJson(const nlohmann::json& json) {
    return {json.value("currency", "skill_point"), json.value("amount", 1)};
}

nlohmann::json costToJson(const SkillTreeCost& cost) {
    return {{"currency", cost.currency}, {"amount", cost.amount}};
}

bool hasCycleFrom(const std::string& id, const std::map<std::string, SkillTreeNode>& nodes,
                  std::vector<std::string>& stack) {
    if (std::find(stack.begin(), stack.end(), id) != stack.end()) {
        return true;
    }
    const auto it = nodes.find(id);
    if (it == nodes.end()) {
        return false;
    }
    stack.push_back(id);
    for (const auto& prerequisite : it->second.prerequisites) {
        if (hasCycleFrom(prerequisite, nodes, stack)) {
            return true;
        }
    }
    stack.pop_back();
    return false;
}

} // namespace

SkillTreeDocument SkillTreeDocument::fromJson(const nlohmann::json& json) {
    SkillTreeDocument document;
    document.tree_id = json.value("tree_id", "");
    document.title = json.value("title", "");
    if (json.contains("nodes") && json["nodes"].is_array()) {
        for (const auto& node_json : json["nodes"]) {
            SkillTreeNode node;
            node.id = node_json.value("id", "");
            node.title = node_json.value("title", "");
            node.class_id = node_json.value("class_id", "");
            node.ability_id = node_json.value("ability_id", "");
            if (node_json.contains("prerequisites") && node_json["prerequisites"].is_array()) {
                for (const auto& prerequisite_json : node_json["prerequisites"]) {
                    node.prerequisites.push_back(prerequisite_json.get<std::string>());
                }
            }
            if (node_json.contains("cost") && node_json["cost"].is_object()) {
                node.cost = costFromJson(node_json["cost"]);
            }
            document.nodes.push_back(std::move(node));
        }
    }
    return document;
}

nlohmann::json SkillTreeDocument::toJson() const {
    nlohmann::json node_array = nlohmann::json::array();
    for (const auto& node : nodes) {
        node_array.push_back({{"id", node.id},
                              {"title", node.title},
                              {"class_id", node.class_id},
                              {"ability_id", node.ability_id},
                              {"prerequisites", node.prerequisites},
                              {"cost", costToJson(node.cost)}});
    }
    return {{"schema_version", "urpg.skill_tree.v1"},
            {"tree_id", tree_id},
            {"title", title},
            {"nodes", std::move(node_array)}};
}

std::vector<SkillTreeDiagnostic> SkillTreeDocument::validate(const std::set<std::string>& known_abilities) const {
    std::vector<SkillTreeDiagnostic> diagnostics;
    if (tree_id.empty()) {
        diagnostics.push_back({"missing_tree_id", "Skill tree requires a tree_id.", ""});
    }

    std::map<std::string, SkillTreeNode> by_id;
    for (const auto& node : nodes) {
        if (node.id.empty()) {
            diagnostics.push_back({"missing_node_id", "Skill tree node requires an id.", ""});
            continue;
        }
        if (by_id.contains(node.id)) {
            diagnostics.push_back({"duplicate_node_id", "Skill tree node id is duplicated.", node.id});
        }
        by_id[node.id] = node;
        if (node.ability_id.empty()) {
            diagnostics.push_back({"missing_ability_id", "Skill tree node requires an ability id.", node.id});
        } else if (!known_abilities.empty() && !known_abilities.contains(node.ability_id)) {
            diagnostics.push_back({"missing_ability_reference", "Skill tree node references an unknown ability.",
                                   node.id});
        }
        if (node.cost.amount <= 0) {
            diagnostics.push_back({"invalid_unlock_cost", "Skill tree node cost must be positive.", node.id});
        }
    }

    for (const auto& node : nodes) {
        for (const auto& prerequisite : node.prerequisites) {
            if (!by_id.contains(prerequisite)) {
                diagnostics.push_back({"missing_prerequisite", "Skill tree prerequisite node does not exist.", node.id});
            }
        }
        std::vector<std::string> stack;
        if (hasCycleFrom(node.id, by_id, stack)) {
            diagnostics.push_back({"skill_tree_cycle", "Skill tree prerequisite graph contains a cycle.", node.id});
            break;
        }
    }
    return diagnostics;
}

SkillTreePreview SkillTreeDocument::preview(const SkillTreeState& state,
                                            const std::set<std::string>& known_abilities) const {
    SkillTreePreview preview;
    preview.tree_id = tree_id;
    preview.diagnostics = validate(known_abilities);
    if (!preview.diagnostics.empty()) {
        return preview;
    }

    for (const auto& node : nodes) {
        if (state.learned_nodes.contains(node.id)) {
            preview.learned_ability_ids.push_back(node.ability_id);
            continue;
        }
        const bool prerequisites_met = std::all_of(node.prerequisites.begin(), node.prerequisites.end(),
                                                   [&](const std::string& id) {
                                                       return state.learned_nodes.contains(id);
                                                   });
        if (prerequisites_met && state.available_points >= node.cost.amount) {
            preview.unlockable_node_ids.push_back(node.id);
        } else {
            preview.locked_node_ids.push_back(node.id);
        }
    }
    return preview;
}

bool SkillTreeDocument::unlockNode(const std::string& node_id, SkillTreeState& state,
                                   const std::set<std::string>& known_abilities) const {
    const auto preview_state = preview(state, known_abilities);
    if (std::find(preview_state.unlockable_node_ids.begin(), preview_state.unlockable_node_ids.end(), node_id) ==
        preview_state.unlockable_node_ids.end()) {
        return false;
    }
    const auto it = std::find_if(nodes.begin(), nodes.end(), [&](const SkillTreeNode& node) {
        return node.id == node_id;
    });
    if (it == nodes.end()) {
        return false;
    }
    state.available_points -= it->cost.amount;
    state.learned_nodes.insert(node_id);
    return true;
}

nlohmann::json skillTreeDiagnosticToJson(const SkillTreeDiagnostic& diagnostic) {
    return {{"code", diagnostic.code}, {"message", diagnostic.message}, {"node_id", diagnostic.node_id}};
}

nlohmann::json skillTreePreviewToJson(const SkillTreePreview& preview) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        diagnostics.push_back(skillTreeDiagnosticToJson(diagnostic));
    }
    return {{"tree_id", preview.tree_id},
            {"unlockable_node_ids", preview.unlockable_node_ids},
            {"locked_node_ids", preview.locked_node_ids},
            {"learned_ability_ids", preview.learned_ability_ids},
            {"diagnostics", std::move(diagnostics)}};
}

} // namespace urpg::progression
