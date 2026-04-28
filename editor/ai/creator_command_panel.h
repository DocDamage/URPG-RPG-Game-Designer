#pragma once

#include "engine/core/ai/creator_command_planner.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class CreatorCommandPanel {
public:
    void setRequest(urpg::ai::CreatorCommandRequest request);
    void setProjectData(nlohmann::json projectData);
    void setTransportConfig(urpg::ai::CreatorProviderTransportConfig transportConfig);
    void render();
    bool applyCurrentPlan();
    const nlohmann::json& lastRenderSnapshot() const;

private:
    urpg::ai::CreatorCommandRequest request_;
    urpg::ai::CreatorProviderTransportConfig transport_config_;
    nlohmann::json project_data_ = nlohmann::json::object();
    urpg::ai::CreatorCommandPlan current_plan_;
    nlohmann::json last_render_snapshot_ = nlohmann::json::object();
};

} // namespace urpg::editor
