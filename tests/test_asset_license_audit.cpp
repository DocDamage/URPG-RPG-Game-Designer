#include "engine/core/asset/asset_license_audit.h"
#include <catch2/catch_test_macros.hpp>
#include <fstream>

using namespace urpg::asset;

TEST_CASE("AssetLicenseAuditor validates asset for export safety", "[asset]") {
    SECTION("Fails on unknown license type") {
        AssetLicense unknown;
        AssetAuditResult result = AssetLicenseAuditor::auditAsset("sprites/hero.png", unknown);

        REQUIRE_FALSE(result.isExportSafe);
        REQUIRE(result.warningMessage.find("no declared license") != std::string::npos);
    }

    SECTION("Succeeds on common open-source licenses") {
        AssetLicense mit;
        mit.type = LicenseType::MIT;
        AssetAuditResult result = AssetLicenseAuditor::auditAsset("lib/utils.js", mit);

        REQUIRE(result.isExportSafe);
    }

    SECTION("Flags proprietary assets with warnings") {
        AssetLicense prop;
        prop.type = LicenseType::Proprietary;
        AssetAuditResult result = AssetLicenseAuditor::auditAsset("content/music/boss_leak.ogg", prop);

        REQUIRE(result.isExportSafe);
        REQUIRE(result.warningMessage.find("Proprietary asset") != std::string::npos);
    }
}

TEST_CASE("Release asset manifest excludes source-only and vendor baggage from release assumptions",
          "[asset][release][license]") {
    std::ifstream stream(
#ifdef URPG_SOURCE_DIR
        std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" / "release_asset_promotion_manifest.json"
#else
        std::filesystem::current_path() / "content" / "fixtures" / "release_asset_promotion_manifest.json"
#endif
    );
    REQUIRE(stream.is_open());
    nlohmann::json manifest;
    stream >> manifest;

    const auto result = auditReleaseAssetManifest(manifest);

    REQUIRE(result.exportSafe);
    REQUIRE(result.releaseRequiredCount == 3);
    REQUIRE(result.bundledCount == 3);
    REQUIRE(result.issues.empty());
}

TEST_CASE("Release asset manifest fails closed on raw release paths or missing attribution",
          "[asset][release][license]") {
    const nlohmann::json manifest = {
        {"schema", "urpg.release_asset_promotion_manifest.v1"},
        {"assets", nlohmann::json::array({
            {
                {"id", "bad_source_asset"},
                {"path", "third_party/source_pack/PSD/raw_button.psd"},
                {"release_required", true},
                {"distribution", "bundled"},
                {"license_cleared", false},
                {"attribution", ""},
                {"source_url", ""}
            }
        })}
    };

    const auto result = auditReleaseAssetManifest(manifest);

    REQUIRE_FALSE(result.exportSafe);
    REQUIRE(result.releaseRequiredCount == 1);
    REQUIRE(result.issues.size() == 4);
    REQUIRE(result.issues[0].code == "license_not_cleared");
    REQUIRE(result.issues[1].code == "missing_attribution");
    REQUIRE(result.issues[2].code == "missing_source_url");
    REQUIRE(result.issues[3].code == "raw_or_vendor_release_path");
}
