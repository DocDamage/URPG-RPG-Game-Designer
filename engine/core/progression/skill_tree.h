#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace urpg::progression {

struct SkillTreeCost {
    std::string currency = "skill_point";
    int32_t amount = 1;
};

struct SkillTreeNode {
    std::string id;
    std::string title;
    std::string class_id;
    std::string ability_id;
    std::vector<std::string> prerequisites;
    SkillTreeCost cost;
};

struct SkillTreeDiagnostic {
    std::string code;
    std::string message;
    std::string node_id;
};

struct SkillTreeState {
    int32_t available_points = 0;
    std::set<std::string> learned_nodes;
};

struct SkillTreePreview {
    std::string tree_id;
    std::vector<std::string> unlockable_node_ids;
    std::vector<std::string> locked_node_ids;
    std::vector<std::string> learned_ability_ids;
    std::vector<SkillTreeDiagnostic> diagnostics;
};

class SkillTreeDocument {
public:
    std::string tree_id;
    std::string title;
    std::vector<SkillTreeNode> nodes;

    static SkillTreeDocument fromJson(const nlohmann::json& json);
    nlohmann::json toJson() const;
    std::vector<SkillTreeDiagnostic> validate(const std::set<std::string>& known_abilities = {}) const;
    SkillTreePreview preview(const SkillTreeState& state, const std::set<std::string>& known_abilities = {}) const;
    bool unlockNode(const std::string& node_id, SkillTreeState& state,
                    const std::set<std::string>& known_abilities = {}) const;
};

nlohmann::json skillTreeDiagnosticToJson(const SkillTreeDiagnostic& diagnostic);
nlohmann::json skillTreePreviewToJson(const SkillTreePreview& preview);

} // namespace urpg::progression
