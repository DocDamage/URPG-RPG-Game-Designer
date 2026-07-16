#pragma once

#include "engine/core/assets/asset_library.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::assets {

struct AssetRightsPolicy {
    bool require_license_id = true;
    bool require_source = true;
    bool require_review = true;
    bool require_export_eligible = true;
    bool include_non_runtime_assets = false;
};

struct AssetNoticeDiagnostic {
    std::string code;
    std::string asset_id;
    std::string message;
};

struct AssetNoticesReport {
    bool package_allowed = true;
    std::vector<AssetNoticeDiagnostic> diagnostics;
    nlohmann::json rows = nlohmann::json::array();

    nlohmann::json toJson() const;
    std::string toNoticeText() const;
};

AssetNoticesReport buildAssetNoticesReport(const std::vector<AssetRecord>& assets,
                                           const AssetRightsPolicy& policy = {});

} // namespace urpg::assets
