#include "export_packager_test_helpers.h"

#include "engine/core/export/export_validator.h"
#include "engine/core/security/resource_protector.h"
#include "engine/core/tools/export_packager.h"
#include "engine/core/tools/export_packager_bundle_writer.h"
#include "engine/core/tools/export_packager_payload_builder.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

using namespace urpg::tools;
using urpg::exporting::ExportValidator;
using namespace urpg::tests::export_packager;

TEST_CASE("ExportPackager writes deterministic bounded asset bundles for identical inputs", "[export][packager]") {
    const auto baseA = std::filesystem::temp_directory_path() / "urpg_export_packager_deterministic_a";
    const auto baseB = std::filesystem::temp_directory_path() / "urpg_export_packager_deterministic_b";
    std::filesystem::remove_all(baseA);
    std::filesystem::remove_all(baseB);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Web_WASM;
    config.compressAssets = true;

    config.outputDir = baseA.string();
    const auto first = packager.runExport(config);
    INFO(first.log);
    REQUIRE(first.success);

    config.outputDir = baseB.string();
    const auto second = packager.runExport(config);
    INFO(second.log);
    REQUIRE(second.success);

    REQUIRE(ReadFileBytes(baseA / "data.pck") == ReadFileBytes(baseB / "data.pck"));

    std::filesystem::remove_all(baseA);
    std::filesystem::remove_all(baseB);
}

TEST_CASE("ExportPackager stages bounded repo-owned content roots into data.pck", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_repo_owned_bundle";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);

    const auto bundlePath = base / "data.pck";
    const auto manifest = ReadBundleManifest(bundlePath);

    const auto findEntry = [&](const std::string& path) -> nlohmann::json {
        for (const auto& entry : manifest["entries"]) {
            if (entry["path"] == path) {
                return entry;
            }
        }
        INFO(path);
        REQUIRE(false);
        return nlohmann::json::object();
    };

    const auto readinessEntry = findEntry("content/readiness/readiness_status.json");
    const auto readinessSchemaEntry = findEntry("content/schemas/readiness_status.schema.json");
    const auto starterDungeonEntry = findEntry("content/level_libraries/starter_dungeon.json");
    const auto modernUiEntry = findEntry("content/ui/modern/modern_ui_style_1_48x48.png");
    const auto discoveryManifestEntry = findEntry(kAssetDiscoveryManifestPath);

    const auto readinessBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, readinessEntry);
    const auto readinessSchemaBytes =
        DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, readinessSchemaEntry);
    const auto starterDungeonBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, starterDungeonEntry);
    const auto modernUiBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, modernUiEntry);
    const auto discoveryManifestBytes =
        DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, discoveryManifestEntry);

    const std::string readinessText(readinessBytes.begin(), readinessBytes.end());
    const std::string readinessSchemaText(readinessSchemaBytes.begin(), readinessSchemaBytes.end());
    const std::string starterDungeonText(starterDungeonBytes.begin(), starterDungeonBytes.end());
    const std::string discoveryManifestText(discoveryManifestBytes.begin(), discoveryManifestBytes.end());

    REQUIRE(readinessEntry["kind"] == "readiness");
    REQUIRE(readinessSchemaEntry["kind"] == "schema");
    REQUIRE(starterDungeonEntry["kind"] == "level_library");
    REQUIRE(modernUiEntry["kind"] == "ui_theme_asset");
    REQUIRE(discoveryManifestEntry["kind"] == "asset_discovery_manifest");
    REQUIRE(readinessText.find("\"schemaVersion\": \"1.0.0\"") != std::string::npos);
    REQUIRE(readinessSchemaText.find("\"title\"") != std::string::npos);
    REQUIRE(starterDungeonText.find("\"libraryName\": \"Starter Dungeon Kit\"") != std::string::npos);
    REQUIRE(modernUiBytes.size() > 8);
    REQUIRE(modernUiBytes[0] == 0x89);
    REQUIRE(modernUiBytes[1] == 'P');
    REQUIRE(modernUiBytes[2] == 'N');
    REQUIRE(modernUiBytes[3] == 'G');
    REQUIRE(discoveryManifestText.find("\"format\": \"URPG_PROJECT_ASSET_DISCOVERY_V1\"") != std::string::npos);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager stages promoted asset bundles from governed manifests", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_promoted_asset_bundles";
    const auto manifestRoot = base / "asset_bundles";
    const auto normalizedRoot = base / "normalized";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(manifestRoot);
    std::filesystem::create_directories(normalizedRoot);
    WritePromotedSourceManifest(base, "SRC-002");

    WriteFile(manifestRoot / "BND-900.json",
              R"({
  "bundle_id": "BND-900",
  "bundle_name": "test_promoted_assets",
  "source_id": "SRC-002",
  "bundle_state": "promoted",
  "assets": [
    {
      "original_relative_path": "sprites/hero.svg",
      "promoted_relative_path": "prototype_sprites/hero_placeholder.svg",
      "category": "prototype_sprite",
      "status": "promoted"
    },
    {
      "original_relative_path": "sprites/missing.svg",
      "promoted_relative_path": "prototype_sprites/missing_placeholder.svg",
      "category": "prototype_sprite",
      "status": "promoted"
    },
    {
      "original_relative_path": "sprites/not_promoted.svg",
      "promoted_relative_path": "prototype_sprites/not_promoted.svg",
      "category": "prototype_sprite",
      "status": "normalized"
    }
  ]
}
)");
    WriteFile(manifestRoot / "BND-901.json",
              R"({
  "bundle_id": "BND-901",
  "bundle_name": "planned_assets",
  "source_id": "SRC-002",
  "bundle_state": "planned",
  "assets": [
    {
      "original_relative_path": "sprites/planned.svg",
      "promoted_relative_path": "prototype_sprites/planned.svg",
      "category": "prototype_sprite",
      "status": "promoted"
    }
  ]
}
)");
    WriteFile(normalizedRoot / "prototype_sprites" / "hero_placeholder.svg",
              "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\"><rect width=\"16\" height=\"16\" "
              "fill=\"#3a7\"/></svg>");
    WriteFile(normalizedRoot / "prototype_sprites" / "planned.svg",
              "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\"><circle cx=\"8\" cy=\"8\" r=\"6\" "
              "fill=\"#a73\"/></svg>");

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = (base / "out").string();
    config.compressAssets = true;
    config.assetBundleManifestRootOverride = manifestRoot.string();
    config.normalizedAssetRootOverride = normalizedRoot.string();

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);

    const auto bundlePath = base / "out" / "data.pck";
    const auto manifest = ReadBundleManifest(bundlePath);

    const auto findEntry = [&](const std::string& path) -> nlohmann::json {
        for (const auto& entry : manifest["entries"]) {
            if (entry["path"] == path) {
                return entry;
            }
        }
        INFO(path);
        REQUIRE(false);
        return nlohmann::json::object();
    };

    const auto bundleManifestEntry = findEntry("imports/manifests/asset_bundles/BND-900.json");
    const auto promotedAssetEntry = findEntry("imports/normalized/prototype_sprites/hero_placeholder.svg");
    const auto bundleManifestBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, bundleManifestEntry);
    const auto promotedAssetBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, promotedAssetEntry);

    const std::string bundleManifestText(bundleManifestBytes.begin(), bundleManifestBytes.end());
    const std::string promotedAssetText(promotedAssetBytes.begin(), promotedAssetBytes.end());

    REQUIRE(bundleManifestEntry["kind"] == "asset_bundle_manifest");
    REQUIRE(promotedAssetEntry["kind"] == "promoted_asset");
    REQUIRE(bundleManifestText.find("\"bundle_id\": \"BND-900\"") != std::string::npos);
    REQUIRE(promotedAssetText.find("<svg") != std::string::npos);

    bool foundMissing = false;
    bool foundPlannedManifest = false;
    bool foundPlannedAsset = false;
    for (const auto& entry : manifest["entries"]) {
        if (entry["path"] == "imports/normalized/prototype_sprites/missing_placeholder.svg") {
            foundMissing = true;
        }
        if (entry["path"] == "imports/manifests/asset_bundles/BND-901.json") {
            foundPlannedManifest = true;
        }
        if (entry["path"] == "imports/normalized/prototype_sprites/planned.svg") {
            foundPlannedAsset = true;
        }
    }
    REQUIRE_FALSE(foundMissing);
    REQUIRE_FALSE(foundPlannedManifest);
    REQUIRE_FALSE(foundPlannedAsset);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager fails closed when promoted asset source license evidence is missing",
          "[export][packager][security]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_missing_source_license";
    const auto manifestRoot = base / "asset_bundles";
    const auto normalizedRoot = base / "normalized";
    std::filesystem::remove_all(base);

    WriteFile(manifestRoot / "BND-910.json",
              R"({
  "bundle_id": "BND-910",
  "bundle_name": "missing_source_license",
  "source_id": "SRC-910",
  "bundle_state": "promoted",
  "assets": [
    {
      "original_relative_path": "sprites/hero.svg",
      "promoted_relative_path": "prototype_sprites/hero.svg",
      "category": "prototype_sprite",
      "status": "promoted"
    }
  ]
}
)");
    WriteFile(normalizedRoot / "prototype_sprites" / "hero.svg", "<svg></svg>");

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = (base / "out").string();
    config.assetBundleManifestRootOverride = manifestRoot.string();
    config.normalizedAssetRootOverride = normalizedRoot.string();

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.generatedFiles.empty());
    REQUIRE(result.log.find("Asset License Audit failed") != std::string::npos);
    REQUIRE(result.log.find("missing source license evidence") != std::string::npos);
    REQUIRE_FALSE(std::filesystem::exists(base / "out" / "data.pck"));

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager fails closed on disallowed promoted asset legal disposition",
          "[export][packager][security]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_disallowed_source_license";
    const auto manifestRoot = base / "asset_bundles";
    const auto normalizedRoot = base / "normalized";
    std::filesystem::remove_all(base);
    WritePromotedSourceManifest(base, "SRC-911", "mixed_asset_pack_reference_only_until_attribution_is_captured");

    WriteFile(manifestRoot / "BND-911.json",
              R"({
  "bundle_id": "BND-911",
  "bundle_name": "disallowed_source_license",
  "source_id": "SRC-911",
  "bundle_state": "promoted",
  "assets": [
    {
      "original_relative_path": "sprites/hero.svg",
      "promoted_relative_path": "prototype_sprites/hero.svg",
      "category": "prototype_sprite",
      "status": "promoted"
    }
  ]
}
)");
    WriteFile(normalizedRoot / "prototype_sprites" / "hero.svg", "<svg></svg>");

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = (base / "out").string();
    config.assetBundleManifestRootOverride = manifestRoot.string();
    config.normalizedAssetRootOverride = normalizedRoot.string();

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.log.find("Disallowed asset source legal disposition") != std::string::npos);
    REQUIRE_FALSE(std::filesystem::exists(base / "out" / "data.pck"));

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager stages canonical release-required visual intake lane", "[export][packager][assets]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_canonical_promoted_assets";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = (base / "out").string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);

    const auto bundlePath = base / "out" / "data.pck";
    const auto manifest = ReadBundleManifest(bundlePath);

    const auto findEntry = [&](const std::string& path) -> nlohmann::json {
        for (const auto& entry : manifest["entries"]) {
            if (entry["path"] == path) {
                return entry;
            }
        }
        INFO(path);
        REQUIRE(false);
        return nlohmann::json::object();
    };

    const auto visualManifestEntry = findEntry("imports/manifests/asset_bundles/BND-001.json");
    const auto visualAssetEntry = findEntry("imports/normalized/prototype_sprites/gdquest_blue_actor.svg");

    const auto visualBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, visualAssetEntry);
    const std::string visualText(visualBytes.begin(), visualBytes.end());

    REQUIRE(visualManifestEntry["kind"] == "asset_bundle_manifest");
    REQUIRE(visualAssetEntry["kind"] == "promoted_asset");
    REQUIRE(visualText.find("<svg") != std::string::npos);
    for (const auto& entry : manifest["entries"]) {
        const auto path = entry["path"].get<std::string>();
        REQUIRE(path != "imports/normalized/ui_sfx/kenney_click_001.wav");
        REQUIRE(path.rfind("imports/raw/", 0) != 0);
        REQUIRE(path.rfind("third_party/", 0) != 0);
        REQUIRE(path.rfind("itch/", 0) != 0);
        REQUIRE(path.rfind("project_assets/imports_normalized/", 0) != 0);
    }

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager auto-discovers configured project asset roots and writes a discovery manifest",
          "[export][packager][discovery]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_auto_discovery";
    const auto projectRoot = base / "sample_project";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(projectRoot / "assets" / "ui");
    std::filesystem::create_directories(projectRoot / "audio");

    WriteFile(projectRoot / "assets" / "ui" / "hud.png", "fake_png_payload");
    WriteFile(projectRoot / "audio" / "battle_theme.ogg", "fake_ogg_payload");
    WriteAssetLicenseManifest(projectRoot / "assets", {"ui/hud.png"});
    WriteAssetLicenseManifest(projectRoot / "audio", {"battle_theme.ogg"});

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = (base / "out").string();
    config.compressAssets = true;
    config.assetDiscoveryRoots = {
        (projectRoot / "assets").string(),
        (projectRoot / "audio").string(),
    };

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);

    const auto bundlePath = base / "out" / "data.pck";
    const auto manifest = ReadBundleManifest(bundlePath);

    const auto findEntry = [&](const std::string& path) -> nlohmann::json {
        for (const auto& entry : manifest["entries"]) {
            if (entry["path"] == path) {
                return entry;
            }
        }
        INFO(path);
        REQUIRE(false);
        return nlohmann::json::object();
    };

    const auto hudEntry = findEntry("project_assets/root_01/ui/hud.png");
    const auto audioEntry = findEntry("project_assets/root_02/battle_theme.ogg");
    const auto discoveryEntry = findEntry(kAssetDiscoveryManifestPath);

    const auto hudBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, hudEntry);
    const auto audioBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, audioEntry);
    const auto discoveryBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, discoveryEntry);
    const auto discoveryManifest = nlohmann::json::parse(std::string(discoveryBytes.begin(), discoveryBytes.end()));

    REQUIRE(hudEntry["kind"] == "auto_discovered_asset");
    REQUIRE(audioEntry["kind"] == "auto_discovered_asset");
    REQUIRE(std::string(hudBytes.begin(), hudBytes.end()) == "fake_png_payload");
    REQUIRE(std::string(audioBytes.begin(), audioBytes.end()) == "fake_ogg_payload");
    REQUIRE(discoveryManifest["format"] == "URPG_PROJECT_ASSET_DISCOVERY_V1");
    REQUIRE(discoveryManifest["discoveredAssetCount"] == 2);
    REQUIRE(discoveryManifest["assets"].size() == 2);
    REQUIRE(discoveryManifest["assets"][0]["path"] == "project_assets/root_01/ui/hud.png");
    REQUIRE(discoveryManifest["assets"][1]["path"] == "project_assets/root_02/battle_theme.ogg");

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager rejects relative auto-discovery roots that escape the repository",
          "[export][packager][discovery][security]") {
    const auto escapeRoot = std::filesystem::path(URPG_SOURCE_DIR).parent_path() / "urpg_asset_escape_fixture";
    std::filesystem::remove_all(escapeRoot);
    WriteFile(escapeRoot / "assets" / "escape.txt", "should_not_bundle");

    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.compressAssets = false;
    config.assetDiscoveryRoots = {"../urpg_asset_escape_fixture/assets"};

    const auto result = urpg::tools::export_packager_detail::buildBundlePayloads(config);

    REQUIRE_FALSE(result.errors.empty());
    REQUIRE(std::find(result.errors.begin(), result.errors.end(),
                      "Asset discovery root escapes repository: ../urpg_asset_escape_fixture/assets") !=
            result.errors.end());
    for (const auto& payload : result.payloads) {
        REQUIRE(payload.path.find("escape.txt") == std::string::npos);
    }

    std::filesystem::remove_all(escapeRoot);
}

TEST_CASE("ExportPackager fails closed when discovered assets have malformed license evidence",
          "[export][packager][discovery][security]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_bad_discovery_license";
    const auto projectRoot = base / "sample_project";
    std::filesystem::remove_all(base);

    WriteFile(projectRoot / "assets" / "ui" / "hud.png", "fake_png_payload");
    WriteFile(projectRoot / "assets" / "asset_licenses.json", R"({"format":"wrong","assets":[]})");

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = (base / "out").string();
    config.assetDiscoveryRoots = {
        (projectRoot / "assets").string(),
    };

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.log.find("Malformed asset license manifest") != std::string::npos);
    REQUIRE_FALSE(std::filesystem::exists(base / "out" / "data.pck"));

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager rejects discovered payloads that exceed uint32 bundle limits",
          "[export][packager][security]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_oversized_payload";
    const auto projectRoot = base / "sample_project";
    const auto largeAsset = projectRoot / "assets" / "large.bin";
    std::filesystem::remove_all(base);

    WriteAssetLicenseManifest(projectRoot / "assets", {"large.bin"});
    std::filesystem::create_directories(largeAsset.parent_path());
    std::error_code resizeError;
    std::filesystem::resize_file(
        largeAsset, static_cast<std::uintmax_t>((std::numeric_limits<std::uint32_t>::max)()) + 1u, resizeError);
    if (resizeError) {
        std::filesystem::remove_all(base);
        SUCCEED("Filesystem could not create sparse oversized fixture: " << resizeError.message());
        return;
    }

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = (base / "out").string();
    config.compressAssets = false;
    config.assetDiscoveryRoots = {
        (projectRoot / "assets").string(),
    };

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.log.find("uint32 bundle size limit") != std::string::npos);
    REQUIRE_FALSE(std::filesystem::exists(base / "out" / "data.pck"));

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager stores bundle payloads as reversible RLE+XOR entries", "[export][packager][security]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_bundle_protection";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);

    const auto bundlePath = base / "data.pck";
    const auto manifest = ReadBundleManifest(bundlePath);
    REQUIRE(manifest["protectionMode"] == "rle_xor");
    REQUIRE(manifest["integrityMode"] == "fnv1a64_keyed");
    REQUIRE(manifest["signatureMode"] == "sha256_keyed_bundle_v1");
    REQUIRE(manifest["bundleSignature"] == ComputeBundleSignature(bundlePath, manifest, ExportTarget::Windows_x64));

    const auto& firstEntry = manifest["entries"][0];
    const auto storedBytes = ReadBundleEntryBytes(bundlePath, firstEntry);
    const auto decodedBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, firstEntry);
    const std::string decodedText(decodedBytes.begin(), decodedBytes.end());

    REQUIRE(firstEntry["compressed"] == true);
    REQUIRE(firstEntry["obfuscated"] == true);
    REQUIRE(firstEntry["integrityTag"] ==
            ComputeBundleIntegrityTag(firstEntry, storedBytes, ExportTarget::Windows_x64));
    REQUIRE_FALSE(storedBytes.empty());
    REQUIRE(decodedText.find("\"format\": \"URPG_PROJECT_EXPORT_METADATA_V1\"") != std::string::npos);
    REQUIRE(decodedText.find("\"bundleMode\": \"project_content_bundle_v1\"") != std::string::npos);
    REQUIRE(decodedText.find("\"target\": \"Windows (x64)\"") != std::string::npos);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager keyed integrity tags detect stored bundle tampering", "[export][packager][security]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_bundle_integrity_tamper";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);

    const auto bundlePath = base / "data.pck";
    const auto manifest = ReadBundleManifest(bundlePath);
    const auto& firstEntry = manifest["entries"][0];
    auto storedBytes = ReadBundleEntryBytes(bundlePath, firstEntry);

    REQUIRE_FALSE(storedBytes.empty());
    const auto originalTag = firstEntry["integrityTag"].get<std::string>();
    REQUIRE(originalTag == ComputeBundleIntegrityTag(firstEntry, storedBytes, ExportTarget::Windows_x64));

    storedBytes[0] ^= 0x1;
    REQUIRE(originalTag != ComputeBundleIntegrityTag(firstEntry, storedBytes, ExportTarget::Windows_x64));

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager keyed SHA-256 bundle signature detects manifest tampering", "[export][packager][security]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_bundle_signature_tamper";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);

    const auto bundlePath = base / "data.pck";
    auto manifest = ReadBundleManifest(bundlePath);
    const auto originalSignature = manifest["bundleSignature"].get<std::string>();

    manifest["assetDiscoveryMode"] = "tampered_mode";
    REQUIRE(originalSignature != ComputeBundleSignature(bundlePath, manifest, ExportTarget::Windows_x64));

    std::filesystem::remove_all(base);
}
