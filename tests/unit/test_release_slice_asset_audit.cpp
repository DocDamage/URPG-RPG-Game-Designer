#include "engine/core/assets/release_slice_asset_audit.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

namespace {

urpg::assets::AssetRecord finalAsset(std::string id, std::string surface, std::string media_kind = "image") {
    urpg::assets::AssetRecord asset;
    asset.asset_id = std::move(id);
    asset.path = "content/release/" + asset.asset_id + (media_kind == "audio" ? ".ogg" : ".png");
    asset.source_path = "repo:" + asset.path;
    asset.media_kind = std::move(media_kind);
    asset.category = surface;
    asset.package_destination = "share/urpg/" + asset.path;
    asset.distribution = "bundled";
    asset.size_bytes = 4096;
    asset.sha256 = std::string(64, static_cast<char>('a' + (asset.asset_id.size() % 6)));
    asset.license_id = "CC0-1.0";
    asset.required_for_release = true;
    asset.release_eligible = true;
    asset.provenance.original_source = "https://example.invalid/" + asset.asset_id;
    asset.authored_metadata = {{"release_surfaces", {surface}}, {"quality_tier", "final"},
                               {"final_quality_reviewed", true}, {"rights_reviewed", true},
                               {"credit_line", "Original " + asset.asset_id + " by Example Artist"}};
    return asset;
}

bool hasCode(const urpg::assets::ReleaseSliceAssetAuditResult& result, const std::string& code) {
    return std::any_of(result.diagnostics.begin(), result.diagnostics.end(),
                       [&](const auto& diagnostic) { return diagnostic.code == code; });
}

} // namespace

TEST_CASE("Final release slice audit accepts complete reviewed assets and emits deterministic credits",
          "[assets][release_slice][pcq605]") {
    using namespace urpg::assets;
    std::vector<AssetRecord> assets = {
        finalAsset("title_art", "title"), finalAsset("map_tiles", "map"),
        finalAsset("battle_art", "battle"), finalAsset("ui_skin", "ui"),
        finalAsset("sound_motifs", "audio", "audio"), finalAsset("app_icons", "icons"),
        finalAsset("ui_font", "fonts")};
    const auto first = auditReleaseSliceAssets(assets);
    std::reverse(assets.begin(), assets.end());
    const auto second = auditReleaseSliceAssets(assets);
    REQUIRE(first.release_ready);
    REQUIRE(first.diagnostics.empty());
    REQUIRE(first.rows.size() == 7);
    REQUIRE(first.toJson() == second.toJson());
    REQUIRE(first.credits_markdown == second.credits_markdown);
    REQUIRE(first.credits_markdown.find("Original sound_motifs") != std::string::npos);
    REQUIRE(first.toJson()["schema"] == "urpg.release_slice_asset_audit.v1");
}

TEST_CASE("Final release slice audit rejects starter placeholders silent policy and incomplete credits",
          "[assets][release_slice][pcq605]") {
    using namespace urpg::assets;
    auto prototype = finalAsset("prototype_actor", "map");
    prototype.authored_metadata["quality_tier"] = "starter_only";
    prototype.authored_metadata["final_quality_reviewed"] = false;
    prototype.authored_metadata["placeholder"] = true;
    prototype.authored_metadata["credit_line"] = "";
    prototype.distribution = "system_fallback";
    prototype.size_bytes = 0;
    prototype.sha256.clear();
    prototype.package_destination.clear();
    auto silent = finalAsset("silent_audio_policy", "audio", "policy");
    silent.authored_metadata["silent_policy"] = true;
    const auto result = auditReleaseSliceAssets({prototype, silent});
    REQUIRE_FALSE(result.release_ready);
    REQUIRE(hasCode(result, "release_asset_final_quality_unresolved"));
    REQUIRE(hasCode(result, "release_asset_placeholder_forbidden"));
    REQUIRE(hasCode(result, "release_asset_credit_missing"));
    REQUIRE(hasCode(result, "release_asset_payload_not_bundled"));
    REQUIRE(hasCode(result, "release_asset_payload_evidence_missing"));
    REQUIRE(hasCode(result, "release_audio_payload_missing"));
    REQUIRE(hasCode(result, "release_surface_uncovered"));
}

TEST_CASE("Final release slice audit fails closed on duplicate raw and unreviewed assets",
          "[assets][release_slice][pcq605]") {
    using namespace urpg::assets;
    auto first = finalAsset("duplicate", "title");
    auto second = finalAsset("duplicate", "map");
    second.path = "imports/raw/unreviewed.png";
    second.license_id.clear();
    second.provenance.license.clear();
    second.provenance.original_source.clear();
    second.source_path.clear();
    second.authored_metadata["rights_reviewed"] = false;
    second.release_eligible = false;
    const auto result = auditReleaseSliceAssets({first, second});
    REQUIRE_FALSE(result.release_ready);
    REQUIRE(hasCode(result, "release_asset_id_invalid"));
    REQUIRE(hasCode(result, "release_asset_raw_path_forbidden"));
    REQUIRE(hasCode(result, "release_asset_license_missing"));
    REQUIRE(hasCode(result, "release_asset_source_missing"));
    REQUIRE(hasCode(result, "release_asset_rights_review_missing"));
    REQUIRE(hasCode(result, "release_asset_export_ineligible"));
}
