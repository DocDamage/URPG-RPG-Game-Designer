#pragma once

#include "engine/core/progression/stat_allocation.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class StatAllocationPanel {
public:
    void bindDocument(urpg::progression::StatAllocationDocument document);
    void setCurrentStats(urpg::progression::ActorStatBlock stats);
    void setRequest(urpg::progression::StatAllocationRequest request);
    void render();
    [[nodiscard]] nlohmann::json lastRenderSnapshot() const;

private:
    urpg::progression::StatAllocationDocument document_;
    urpg::progression::ActorStatBlock current_stats_;
    urpg::progression::StatAllocationRequest request_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
