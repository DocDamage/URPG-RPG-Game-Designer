#include "engine/core/asset/asset_license_audit.h"
#include <catch2/catch_test_macros.hpp>

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
