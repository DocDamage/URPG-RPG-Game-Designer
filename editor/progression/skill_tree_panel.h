#pragma once

#include "engine/core/progression/skill_tree.h"

#include <nlohmann/json.hpp>

#include <set>
#include <string>

namespace urpg::editor {

class SkillTreePanel {
public:
    void bindDocument(urpg::progression::SkillTreeDocument document);
    void setState(urpg::progression::SkillTreeState state);
    void setKnownAbilities(std::set<std::string> known_abilities);
    bool selectNode(const std::string& node_id);
    bool unlockSelectedNode();
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::progression::SkillTreeDocument document_;
    urpg::progression::SkillTreeState state_;
    std::set<std::string> known_abilities_;
    std::string selected_node_id_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
