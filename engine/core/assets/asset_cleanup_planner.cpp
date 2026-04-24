#include "engine/core/assets/asset_cleanup_planner.h"

#include <algorithm>

namespace urpg::assets {

AssetCleanupPlan AssetCleanupPlanner::buildDuplicateCleanupPlan(const AssetLibrary& library) const {
    AssetCleanupPlan plan{};
    const auto& snapshot = library.snapshot();
    const auto& references = library.referencedAssets();

    for (const auto& group : snapshot.duplicate_groups) {
        for (const auto& entry : group.entries) {
            if (!entry.recommended_remove) {
                continue;
            }

            AssetCleanupAction action{};
            action.candidate_path = entry.path;
            action.keep_path = entry.recommended_keep;
            if (references.contains(entry.path)) {
                action.allowed = false;
                action.reason = "candidate_is_referenced";
                ++plan.refused_count;
            } else if (entry.recommended_keep.empty()) {
                action.allowed = false;
                action.reason = "missing_keep_path";
                ++plan.refused_count;
            } else {
                action.allowed = true;
                action.reason = "unreferenced_duplicate";
                ++plan.allowed_count;
            }
            plan.actions.push_back(std::move(action));
        }
    }

    std::sort(plan.actions.begin(), plan.actions.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.candidate_path < rhs.candidate_path;
    });
    return plan;
}

} // namespace urpg::assets
