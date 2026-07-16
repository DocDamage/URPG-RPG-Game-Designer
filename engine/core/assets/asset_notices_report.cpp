#include "engine/core/assets/asset_notices_report.h"

#include <algorithm>
#include <sstream>

namespace urpg::assets {

nlohmann::json AssetNoticesReport::toJson() const {
    nlohmann::json issues = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics) {
        issues.push_back({{"code", diagnostic.code}, {"asset_id", diagnostic.asset_id},
                          {"message", diagnostic.message}});
    }
    return {{"schema", "urpg/third_party_asset_notices/v1"}, {"package_allowed", package_allowed},
            {"rows", rows}, {"diagnostics", std::move(issues)}};
}

std::string AssetNoticesReport::toNoticeText() const {
    std::ostringstream output;
    output << "URPG Third-Party Asset Notices\n";
    for (const auto& row : rows) {
        output << "\n" << row.value("asset_id", "") << " | " << row.value("path", "") << "\n"
               << "License: " << row.value("license_id", "") << "\n"
               << "Attribution: " << row.value("attribution", "") << "\n"
               << "Source: " << row.value("source", "") << "\n";
        const auto notice = row.value("notice", "");
        if (!notice.empty()) output << "Notice: " << notice << "\n";
    }
    return output.str();
}

AssetNoticesReport buildAssetNoticesReport(const std::vector<AssetRecord>& assets, const AssetRightsPolicy& policy) {
    AssetNoticesReport report;
    std::vector<AssetRecord> ordered;
    for (const auto& asset : assets) {
        if (policy.include_non_runtime_assets || asset.include_in_runtime || asset.required_for_release) ordered.push_back(asset);
    }
    std::sort(ordered.begin(), ordered.end(), [](const auto& left, const auto& right) {
        if (left.asset_id != right.asset_id) return left.asset_id < right.asset_id;
        return left.path < right.path;
    });
    const auto issue = [&](const AssetRecord& asset, std::string code, std::string message) {
        report.package_allowed = false;
        report.diagnostics.push_back({std::move(code), asset.asset_id, std::move(message)});
    };
    for (const auto& asset : ordered) {
        const auto source = !asset.provenance.original_source.empty() ? asset.provenance.original_source : asset.source_path;
        const auto license = !asset.license_id.empty() ? asset.license_id : asset.provenance.license;
        const auto attribution = asset.authored_metadata.value("attribution", "");
        const auto reviewed = asset.authored_metadata.value("rights_reviewed", false);
        if (policy.require_license_id && license.empty()) issue(asset, "asset_rights_license_missing", "Required license metadata is unresolved.");
        if (policy.require_source && source.empty()) issue(asset, "asset_rights_source_missing", "Required source metadata is unresolved.");
        if (policy.require_review && !reviewed) issue(asset, "asset_rights_review_missing", "Required rights review is unresolved.");
        if (policy.require_export_eligible && !asset.provenance.export_eligible && !asset.release_eligible) {
            issue(asset, "asset_rights_export_ineligible", "Asset provenance is not approved for package export.");
        }
        report.rows.push_back({{"asset_id", asset.asset_id}, {"path", asset.path}, {"license_id", license},
                               {"attribution", attribution}, {"source", source}, {"rights_reviewed", reviewed},
                               {"notice", asset.authored_metadata.value("notice", "")}});
    }
    std::sort(report.diagnostics.begin(), report.diagnostics.end(), [](const auto& left, const auto& right) {
        if (left.asset_id != right.asset_id) return left.asset_id < right.asset_id;
        return left.code < right.code;
    });
    return report;
}

} // namespace urpg::assets
