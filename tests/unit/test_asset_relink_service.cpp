#include "engine/core/assets/asset_relink_service.h"
#include "engine/core/assets/asset_promotion_manifest.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

namespace {

void writeText(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << text;
}

} // namespace

TEST_CASE("asset relinking service scan and apply matches", "[assets][relink]") {
    const auto project_root = std::filesystem::temp_directory_path() / "urpg_relink_test_proj";
    std::filesystem::remove_all(project_root);

    // Create a mock manifest for a missing asset
    urpg::assets::AssetPromotionManifest manifest;
    manifest.assetId = "test_character_sprite";
    manifest.promotedPath = "content/assets/imported/test_character_sprite/hero.png";
    manifest.sourceSha256 = "c0ffee256sha";
    manifest.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    manifest.package.includeInRuntime = true;

    const auto manifest_path = project_root / "content" / "assets" / "manifests" / "test_character_sprite.json";
    writeText(manifest_path, urpg::assets::serializeAssetPromotionManifest(manifest).dump(2));

    // The payload "content/assets/imported/test_character_sprite/hero.png" is missing at this stage.
    urpg::assets::AssetRelinkService service;
    auto missing = service.scanMissingAssets(project_root);
    REQUIRE(missing.size() == 1);
    REQUIRE(missing[0].asset_id == "test_character_sprite");
    REQUIRE(missing[0].candidates.empty()); // No candidates yet
    REQUIRE(missing[0].affected_reference_paths == std::vector<std::filesystem::path>{manifest_path});

    // Let's create a candidate file matching the hash
    const auto candidate_path = project_root / "content" / "assets" / "imported" / "some_moved_dir" / "new_hero.png";
    // Use the real SHA-256 implementation and update the fixture manifest to match it.
    writeText(candidate_path, "hero_pixel_bytes_here");
    std::string expected_hash = urpg::assets::AssetRelinkService::calculateSha256(candidate_path);

    // Update manifest sourceSha256 to match the real hash of the candidate
    manifest.sourceSha256 = expected_hash;
    writeText(manifest_path, urpg::assets::serializeAssetPromotionManifest(manifest).dump(2));

    // Scan again
    missing = service.scanMissingAssets(project_root);
    REQUIRE(missing.size() == 1);
    REQUIRE(missing[0].candidates.size() == 1);
    REQUIRE(missing[0].candidates[0].confidence == urpg::assets::RelinkConfidence::High);
    REQUIRE(missing[0].candidates[0].path == candidate_path);

    const auto external_candidate = project_root.parent_path() / "unpromoted_hero.png";
    writeText(external_candidate, "hero_pixel_bytes_here");
    REQUIRE_FALSE(service.applyRelink(project_root, "test_character_sprite", external_candidate));

    // Apply relink
    REQUIRE(service.applyRelink(project_root, "test_character_sprite", candidate_path));
    REQUIRE(std::filesystem::exists(manifest_path.string() + ".backup"));

    // Verify scan returns no missing assets
    missing = service.scanMissingAssets(project_root);
    REQUIRE(missing.empty());

    REQUIRE(service.undoLastRelink(project_root, "test_character_sprite"));
    missing = service.scanMissingAssets(project_root);
    REQUIRE(missing.size() == 1);

    std::filesystem::remove_all(project_root);
    std::filesystem::remove(external_candidate);
}

TEST_CASE("asset relink rejects traversal identifiers and raw external sources", "[assets][relink]") {
    const auto project_root = std::filesystem::temp_directory_path() / "urpg_relink_rejection_proj";
    std::filesystem::remove_all(project_root);
    const auto external = project_root.parent_path() / "raw_external_asset.png";
    writeText(external, "raw bytes");

    urpg::assets::AssetRelinkService service;
    REQUIRE_FALSE(service.applyRelink(project_root, "../escape", external));

    std::filesystem::remove_all(project_root);
    std::filesystem::remove(external);
}
