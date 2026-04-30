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

TEST_CASE("creator package manifest rejects blank fields and duplicate dependencies",
          "[export][creator_package][robustness]") {
    const urpg::exporting::CreatorPackageManifest manifest{
        "  ",
        "\t",
        "   ",
        "",
        {"base-content", "  ", "base-content"},
        "\n",
    };

    const auto errors = urpg::exporting::ValidateCreatorPackageManifest(manifest);

    REQUIRE(errors == std::vector<std::string>{
                          "missing_package_id",
                          "missing_package_type",
                          "missing_license_evidence",
                          "missing_compatibility_target",
                          "missing_validation_summary",
                          "invalid_dependency",
                          "duplicate_dependency",
                      });
}

TEST_CASE("creator package manifest JSON emits sorted unique dependencies",
          "[export][creator_package][robustness]") {
    const urpg::exporting::CreatorPackageManifest manifest{
        "creator-pack-3",
        "tileset",
        "licenses/creator-pack-3.md",
        "URPG>=0.7",
        {"zeta-content", "base-content", "zeta-content"},
        "validated",
    };

    const auto json = urpg::exporting::CreatorPackageManifestToJson(manifest);

    REQUIRE(json["dependencies"] == nlohmann::json::array({"base-content", "zeta-content"}));
}
