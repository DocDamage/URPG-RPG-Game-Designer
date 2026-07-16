#include "engine/core/assets/asset_notices_report.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Asset notices report is deterministic and provenance governed", "[assets][notices][bom]") {
    using namespace urpg::assets;
    AssetRecord hero;
    hero.asset_id = "asset.hero"; hero.path = "images/hero.png"; hero.include_in_runtime = true;
    hero.license_id = "CC-BY-4.0"; hero.provenance.original_source = "https://example.invalid/hero";
    hero.provenance.export_eligible = true;
    hero.authored_metadata = {{"attribution", "Example Artist"}, {"rights_reviewed", true}, {"notice", "Changes made."}};
    AssetRecord music;
    music.asset_id = "asset.music"; music.path = "audio/theme.ogg"; music.required_for_release = true;
    music.license_id = "MIT"; music.provenance.original_source = "https://example.invalid/music";
    music.release_eligible = true; music.authored_metadata = {{"attribution", "Composer"}, {"rights_reviewed", true}};
    AssetRecord sourceOnly; sourceOnly.asset_id = "source.psd"; sourceOnly.path = "source.psd";
    const auto first = buildAssetNoticesReport({music, sourceOnly, hero});
    const auto second = buildAssetNoticesReport({hero, music, sourceOnly});
    REQUIRE(first.package_allowed);
    REQUIRE(first.rows.size() == 2);
    REQUIRE(first.rows[0]["asset_id"] == "asset.hero");
    REQUIRE(first.toJson() == second.toJson());
    REQUIRE(first.toNoticeText() == second.toNoticeText());
    REQUIRE(first.toNoticeText().find("Example Artist") != std::string::npos);
    REQUIRE(first.toJson()["schema"] == "urpg/third_party_asset_notices/v1");
}

TEST_CASE("Asset notices report blocks unresolved rights under configured policy", "[assets][notices][bom]") {
    using namespace urpg::assets;
    AssetRecord unresolved;
    unresolved.asset_id = "asset.unresolved"; unresolved.path = "images/unresolved.png";
    unresolved.required_for_release = true;
    const auto strict = buildAssetNoticesReport({unresolved});
    REQUIRE_FALSE(strict.package_allowed);
    REQUIRE(strict.diagnostics.size() == 4);
    REQUIRE(strict.diagnostics[0].code == "asset_rights_export_ineligible");
    REQUIRE(strict.diagnostics[1].code == "asset_rights_license_missing");
    REQUIRE(strict.diagnostics[2].code == "asset_rights_review_missing");
    REQUIRE(strict.diagnostics[3].code == "asset_rights_source_missing");
    AssetRightsPolicy relaxed;
    relaxed.require_license_id = false; relaxed.require_source = false; relaxed.require_review = false;
    relaxed.require_export_eligible = false;
    REQUIRE(buildAssetNoticesReport({unresolved}, relaxed).package_allowed);
}
