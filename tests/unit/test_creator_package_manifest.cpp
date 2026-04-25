#include "engine/core/export/creator_package_manifest.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("creator package manifest includes marketplace validation fields", "[export][creator_package][ffs07]") {
    const urpg::exporting::CreatorPackageManifest manifest{
        "creator-pack-1",
        "tileset",
        "licenses/creator-pack-1.md",
        "URPG>=0.7",
        {"base-content"},
        "validated against export package fixture",
    };

    const auto json = urpg::exporting::CreatorPackageManifestToJson(manifest);

    REQUIRE(urpg::exporting::ValidateCreatorPackageManifest(manifest).empty());
    REQUIRE(json["type"] == "tileset");
    REQUIRE(json["license_evidence"] == "licenses/creator-pack-1.md");
    REQUIRE(json["compatibility_target"] == "URPG>=0.7");
    REQUIRE(json["dependencies"] == nlohmann::json::array({"base-content"}));
    REQUIRE(json["validation_summary"] == "validated against export package fixture");
}

TEST_CASE("creator package manifest rejects absent license evidence", "[export][creator_package][ffs07]") {
    const urpg::exporting::CreatorPackageManifest manifest{
        "creator-pack-2",
        "plugin",
        "",
        "URPG>=0.7",
        {},
        "validated",
    };

    const auto errors = urpg::exporting::ValidateCreatorPackageManifest(manifest);

    REQUIRE(errors == std::vector<std::string>{"missing_license_evidence"});
}
