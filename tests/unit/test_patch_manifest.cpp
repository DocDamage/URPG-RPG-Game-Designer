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

TEST_CASE("patch manifest JSON emits deterministic dependency and change ordering",
          "[export][patch][robustness]") {
    urpg::exporting::PatchManifest manifest;
    manifest.id = "patch-3";
    manifest.base_version = "1.0.0";
    manifest.target_version = "1.0.1";
    manifest.changed_data = {"data/z.json", "data/a.json", "data/a.json"};
    manifest.changed_assets = {"assets/z.png", "assets/a.png", "assets/z.png"};
    manifest.dependencies = {"z-base", "base-content", "z-base"};

    const auto json = urpg::exporting::PatchManifestToJson(manifest);

    REQUIRE(json["changed_data"] == nlohmann::json::array({"data/a.json", "data/z.json"}));
    REQUIRE(json["changed_assets"] == nlohmann::json::array({"assets/a.png", "assets/z.png"}));
    REQUIRE(json["dependencies"] == nlohmann::json::array({"base-content", "z-base"}));
}

TEST_CASE("patch manifest JSON parser rejects malformed arrays without throwing",
          "[export][patch][robustness]") {
    const nlohmann::json json = {
        {"id", "patch-4"},
        {"base_version", "1.0.0"},
        {"target_version", "1.0.1"},
        {"changed_data", nlohmann::json::array({"data/a.json", 7})},
        {"changed_assets", nlohmann::json::array({"assets/a.png"})},
        {"dependencies", nlohmann::json::array({"base-content"})},
    };

    const auto manifest = urpg::exporting::PatchManifestFromJson(json);

    REQUIRE(manifest.id.empty());
    REQUIRE(manifest.changed_data.empty());
    REQUIRE(manifest.changed_assets.empty());
    REQUIRE(manifest.dependencies.empty());
}

TEST_CASE("patch manifest validation rejects blank and duplicate dependencies",
          "[export][patch][robustness]") {
    urpg::exporting::PatchManifest manifest;
    manifest.id = "patch-5";
    manifest.base_version = "1.0.0";
    manifest.target_version = "1.0.1";
    manifest.dependencies = {"base-content", " ", "base-content"};

    const auto errors = urpg::exporting::ValidatePatchManifest(manifest, std::set<std::string>{"base-content"});

    REQUIRE(errors == std::vector<std::string>{"invalid_patch_dependency", "duplicate_patch_dependency"});
}
