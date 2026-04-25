#include "engine/core/export/patch_manifest.h"

#include <catch2/catch_test_macros.hpp>

#include <set>

TEST_CASE("patch manifest builder records changed data and assets", "[export][patch][ffs07]") {
    urpg::exporting::ExportArtifactCompareResult diff;
    diff.changed_schemas = {"data/schemas/save.schema.json"};
    diff.changed_assets = {"assets/hero.png"};

    const auto manifest = urpg::exporting::BuildPatchManifestFromExportDiff(
        "patch-1", "1.0.0", "1.1.0", diff, {"base-content"});

    REQUIRE(manifest.changed_data == std::vector<std::string>{"data/schemas/save.schema.json"});
    REQUIRE(manifest.changed_assets == std::vector<std::string>{"assets/hero.png"});
    REQUIRE(manifest.dependencies == std::vector<std::string>{"base-content"});
    REQUIRE(urpg::exporting::ValidatePatchManifest(manifest, {"base-content"}).empty());
}

TEST_CASE("patch manifest rejects missing dependency", "[export][patch][ffs07]") {
    urpg::exporting::PatchManifest manifest;
    manifest.id = "patch-2";
    manifest.base_version = "1.0.0";
    manifest.target_version = "1.0.1";
    manifest.dependencies = {"missing-base"};

    const auto errors = urpg::exporting::ValidatePatchManifest(manifest, std::set<std::string>{"base-content"});

    REQUIRE(errors == std::vector<std::string>{"missing_patch_dependency:missing-base"});
}
