#pragma once

#include "engine/core/progression/stat_allocation.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::editor {

class StatAllocationPanel {
public:
    void bindDocument(urpg::progression::StatAllocationDocument document);
    void setCurrentStats(urpg::progression::ActorStatBlock stats);
    void setRequest(urpg::progression::StatAllocationRequest request);
    void setPostLoadActorId(std::string actor_id);
    void setLoadedAllocations(std::vector<urpg::progression::AppliedStatAllocation> allocations);
    void render();
    [[nodiscard]] nlohmann::json lastRenderSnapshot() const;

private:
    urpg::progression::StatAllocationDocument document_;
    urpg::progression::ActorStatBlock current_stats_;
    urpg::progression::StatAllocationRequest request_;
    std::string post_load_actor_id_;
    std::vector<urpg::progression::AppliedStatAllocation> loaded_allocations_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
