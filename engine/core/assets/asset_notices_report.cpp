#include "engine/core/assets/asset_notices_report.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

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

AssetNoticesWriteResult writeAssetNoticesReport(const std::filesystem::path& package_root,
                                                const AssetNoticesReport& report) {
    AssetNoticesWriteResult result;
    if (!report.package_allowed) {
        result.code = "asset_notices_rights_blocked";
        result.message = "Package notices were not emitted because required rights metadata is unresolved.";
        return result;
    }
    if (package_root.empty()) {
        result.code = "asset_notices_package_root_missing";
        result.message = "Package root is required.";
        return result;
    }

    std::error_code error;
    const auto noticesDirectory = package_root / "licenses";
    std::filesystem::create_directories(noticesDirectory, error);
    if (error) {
        result.code = "asset_notices_directory_failed";
        result.message = error.message();
        return result;
    }
    result.json_path = noticesDirectory / "third_party_assets.json";
    result.notice_path = noticesDirectory / "THIRD_PARTY_ASSETS.txt";

    const auto publish = [&](const std::filesystem::path& path, const std::string& content) {
        const auto temporary = path.parent_path() / ("." + path.filename().string() + ".tmp");
        {
            std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
            output << content;
            if (!output) {
                return false;
            }
        }
#ifdef _WIN32
        if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            std::filesystem::remove(temporary, error);
            return false;
        }
#else
        std::filesystem::rename(temporary, path, error);
        if (error) {
            std::filesystem::remove(temporary, error);
            return false;
        }
#endif
        return true;
    };

    if (!publish(result.json_path, report.toJson().dump(2) + "\n") ||
        !publish(result.notice_path, report.toNoticeText())) {
        std::filesystem::remove(result.json_path, error);
        std::filesystem::remove(result.notice_path, error);
        result.code = "asset_notices_publish_failed";
        result.message = "Unable to atomically publish the package notices.";
        return result;
    }
    result.success = true;
    result.code = "asset_notices_published";
    result.message = "Deterministic package notices were published.";
    return result;
}

} // namespace urpg::assets
