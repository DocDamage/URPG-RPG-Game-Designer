#include "engine/core/assets/release_slice_asset_audit.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>

namespace urpg::assets {
namespace {

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool hasPlaceholderMarker(const AssetRecord& asset) {
    std::string searchable = asset.asset_id + " " + asset.path + " " + asset.category + " " +
                             asset.authored_metadata.value("notes", "") + " " +
                             asset.authored_metadata.value("quality_tier", "");
    searchable = lowercase(std::move(searchable));
    for (const auto* marker : {"placeholder", "prototype", "proof", "temporary", "temp_asset", "starter_only"}) {
        if (searchable.find(marker) != std::string::npos) return true;
    }
    return asset.authored_metadata.value("placeholder", false) || asset.authored_metadata.value("proof_only", false);
}

std::vector<std::string> releaseSurfaces(const AssetRecord& asset) {
    std::vector<std::string> surfaces;
    const auto found = asset.authored_metadata.find("release_surfaces");
    if (found != asset.authored_metadata.end() && found->is_array()) {
        for (const auto& value : *found) if (value.is_string()) surfaces.push_back(value.get<std::string>());
    } else {
        const auto single = asset.authored_metadata.value("release_surface", "");
        if (!single.empty()) surfaces.push_back(single);
    }
    std::sort(surfaces.begin(), surfaces.end());
    surfaces.erase(std::unique(surfaces.begin(), surfaces.end()), surfaces.end());
    return surfaces;
}

std::string sourceFor(const AssetRecord& asset) {
    return !asset.provenance.original_source.empty() ? asset.provenance.original_source : asset.source_path;
}

std::string licenseFor(const AssetRecord& asset) {
    return !asset.license_id.empty() ? asset.license_id : asset.provenance.license;
}

} // namespace

nlohmann::json ReleaseSliceAssetAuditResult::toJson() const {
    nlohmann::json issues = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics) {
        issues.push_back({{"code", diagnostic.code}, {"asset_id", diagnostic.asset_id},
                          {"surface", diagnostic.surface}, {"message", diagnostic.message}});
    }
    return {{"schema", "urpg.release_slice_asset_audit.v1"}, {"release_ready", release_ready},
            {"rows", rows}, {"diagnostics", std::move(issues)}, {"credits_markdown", credits_markdown}};
}

ReleaseSliceAssetAuditResult auditReleaseSliceAssets(const std::vector<AssetRecord>& assets,
                                                     const ReleaseSliceAssetAuditPolicy& policy) {
    ReleaseSliceAssetAuditResult result;
    std::vector<AssetRecord> ordered = assets;
    std::sort(ordered.begin(), ordered.end(), [](const auto& left, const auto& right) {
        return std::tie(left.asset_id, left.path) < std::tie(right.asset_id, right.path);
    });
    std::set<std::string> ids;
    std::set<std::string> paths;
    std::set<std::string> covered_surfaces;
    const auto issue = [&](std::string code, const AssetRecord& asset, std::string surface, std::string message) {
        result.release_ready = false;
        result.diagnostics.push_back({std::move(code), asset.asset_id, std::move(surface), std::move(message)});
    };

    std::ostringstream credits;
    credits << "# Release Vertical Slice Asset Credits\n\n";
    for (const auto& asset : ordered) {
        const auto surfaces = releaseSurfaces(asset);
        const auto source = sourceFor(asset);
        const auto license = licenseFor(asset);
        const auto credit_line = asset.authored_metadata.value("credit_line", "");
        const auto quality_tier = asset.authored_metadata.value("quality_tier", "");
        if (asset.asset_id.empty() || !ids.insert(asset.asset_id).second) {
            issue("release_asset_id_invalid", asset, "", "Release asset IDs must be non-empty and unique.");
        }
        if (asset.path.empty() || !paths.insert(lowercase(asset.path)).second) {
            issue("release_asset_path_invalid", asset, "", "Release asset paths must be non-empty and case-insensitively unique.");
        }
        if (!asset.required_for_release) {
            issue("release_asset_not_required", asset, "", "Every strict slice row must be marked required for release.");
        }
        if (surfaces.empty()) {
            issue("release_asset_surface_missing", asset, "", "Release asset has no declared release surface.");
        }
        for (const auto& surface : surfaces) covered_surfaces.insert(surface);
        if (policy.require_final_quality_review &&
            (quality_tier != "final" || !asset.authored_metadata.value("final_quality_reviewed", false))) {
            issue("release_asset_final_quality_unresolved", asset, surfaces.empty() ? "" : surfaces.front(),
                  "Asset lacks a completed final-quality review.");
        }
        if (policy.reject_placeholder_markers && hasPlaceholderMarker(asset)) {
            issue("release_asset_placeholder_forbidden", asset, surfaces.empty() ? "" : surfaces.front(),
                  "Placeholder, prototype, proof, or temporary assets cannot ship in the final slice.");
        }
        if (policy.require_bundled_payload) {
            if (asset.distribution != "bundled") {
                issue("release_asset_payload_not_bundled", asset, surfaces.empty() ? "" : surfaces.front(),
                      "Final slice assets must have a concrete bundled payload.");
            }
            const auto path = lowercase(asset.path);
            if (path.starts_with("imports/raw/") || path.starts_with("third_party/") || path.starts_with("vendor/")) {
                issue("release_asset_raw_path_forbidden", asset, surfaces.empty() ? "" : surfaces.front(),
                      "Raw and vendor paths cannot be package authorities.");
            }
            if (asset.size_bytes == 0 || asset.sha256.size() != 64 || asset.package_destination.empty()) {
                issue("release_asset_payload_evidence_missing", asset, surfaces.empty() ? "" : surfaces.front(),
                      "Bundled assets require byte size, SHA-256, and package destination evidence.");
            }
        }
        if (license.empty()) issue("release_asset_license_missing", asset, "", "License metadata is unresolved.");
        if (source.empty()) issue("release_asset_source_missing", asset, "", "Source provenance is unresolved.");
        if (!asset.authored_metadata.value("rights_reviewed", false)) {
            issue("release_asset_rights_review_missing", asset, "", "Rights review is unresolved.");
        }
        if (!asset.release_eligible && !asset.provenance.export_eligible) {
            issue("release_asset_export_ineligible", asset, "", "Asset is not approved for release export.");
        }
        if (policy.require_complete_credits && credit_line.empty()) {
            issue("release_asset_credit_missing", asset, "", "Every shipped asset requires an explicit credit line or waiver line.");
        }
        if (std::find(surfaces.begin(), surfaces.end(), "audio") != surfaces.end() &&
            (asset.authored_metadata.value("silent_policy", false) || asset.media_kind != "audio")) {
            issue("release_audio_payload_missing", asset, "audio", "The final audio surface requires an actual reviewed audio asset.");
        }

        result.rows.push_back({{"asset_id", asset.asset_id}, {"path", asset.path}, {"surfaces", surfaces},
                               {"quality_tier", quality_tier}, {"license_id", license}, {"source", source},
                               {"credit_line", credit_line}, {"sha256", asset.sha256}});
        credits << "- " << (credit_line.empty() ? asset.asset_id : credit_line) << " — License: "
                << (license.empty() ? "UNRESOLVED" : license) << "; Source: "
                << (source.empty() ? "UNRESOLVED" : source) << "\n";
    }

    AssetRecord missing_surface_asset;
    missing_surface_asset.asset_id = "release-slice";
    for (const auto& surface : policy.required_surfaces) {
        if (!covered_surfaces.contains(surface)) {
            issue("release_surface_uncovered", missing_surface_asset, surface,
                  "No final reviewed asset covers required surface '" + surface + "'.");
        }
    }
    std::sort(result.diagnostics.begin(), result.diagnostics.end(), [](const auto& left, const auto& right) {
        return std::tie(left.asset_id, left.code, left.surface) < std::tie(right.asset_id, right.code, right.surface);
    });
    result.credits_markdown = credits.str();
    return result;
}

} // namespace urpg::assets
