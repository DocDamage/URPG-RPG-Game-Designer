#pragma once

#include "engine/core/assets/asset_library.h"

#include <string>
#include <vector>

namespace urpg::assets {

struct AssetCleanupAction {
    std::string candidate_path;
    std::string keep_path;
    std::string reason;
    bool allowed = false;
};

struct AssetCleanupPlan {
    std::vector<AssetCleanupAction> actions;
    size_t allowed_count = 0;
    size_t refused_count = 0;
};

class AssetCleanupPlanner {
public:
    AssetCleanupPlan buildDuplicateCleanupPlan(const AssetLibrary& library) const;
};

} // namespace urpg::assets
