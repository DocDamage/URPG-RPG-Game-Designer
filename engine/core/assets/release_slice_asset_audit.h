#pragma once

#include "engine/core/assets/asset_library.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::assets {

struct ReleaseSliceAssetAuditPolicy {
    std::vector<std::string> required_surfaces = {"title", "map", "battle", "ui", "audio", "icons", "fonts"};
    bool require_bundled_payload = true;
    bool require_final_quality_review = true;
    bool reject_placeholder_markers = true;
    bool require_complete_credits = true;
};

struct ReleaseSliceAssetDiagnostic {
    std::string code;
    std::string asset_id;
    std::string surface;
    std::string message;
};

struct ReleaseSliceAssetAuditResult {
    bool release_ready = true;
    std::vector<ReleaseSliceAssetDiagnostic> diagnostics;
    nlohmann::json rows = nlohmann::json::array();
    std::string credits_markdown;

    nlohmann::json toJson() const;
};

ReleaseSliceAssetAuditResult auditReleaseSliceAssets(
    const std::vector<AssetRecord>& assets,
    const ReleaseSliceAssetAuditPolicy& policy = {});

} // namespace urpg::assets
