#include "engine/core/assets/asset_action_view.h"
#include "engine/core/assets/asset_import_session.h"
#include "engine/core/assets/asset_library.h"
#include "engine/core/assets/asset_promotion_manifest.h"
#include "engine/core/assets/asset_transform_revision_service.h"
#include "engine/core/assets/global_asset_library_store.h"
#include "engine/core/assets/global_asset_promotion_service.h"
#include "engine/core/assets/project_asset_attachment_service.h"
#include "editor/assets/asset_library_model.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <stb_image.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace {

std::filesystem::path uniqueAssetTempRoot(const std::string& prefix) {
    const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (prefix + "_" + std::to_string(tick));
}

void writeBinaryFile(const std::filesystem::path& path, std::string_view payload) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << payload;
}

std::string readBinaryFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

void writePcm16Wav(const std::filesystem::path& path, const std::vector<int16_t>& samples, uint16_t channels = 1,
                   uint32_t sampleRate = 1000) {
    const auto append16 = [](std::string* bytes, const uint16_t value) {
        bytes->push_back(static_cast<char>(value & 0xFFU));
        bytes->push_back(static_cast<char>((value >> 8U) & 0xFFU));
    };
    const auto append32 = [](std::string* bytes, const uint32_t value) {
        for (uint32_t shift = 0; shift < 32; shift += 8) bytes->push_back(static_cast<char>((value >> shift) & 0xFFU));
    };
    std::string bytes = "RIFF";
    append32(&bytes, 36U + static_cast<uint32_t>(samples.size() * sizeof(int16_t)));
    bytes += "WAVEfmt ";
    append32(&bytes, 16U);
    append16(&bytes, 1U);
    append16(&bytes, channels);
    append32(&bytes, sampleRate);
    append32(&bytes, sampleRate * channels * 2U);
    append16(&bytes, static_cast<uint16_t>(channels * 2U));
    append16(&bytes, 16U);
    bytes += "data";
    append32(&bytes, static_cast<uint32_t>(samples.size() * sizeof(int16_t)));
    for (const auto sample : samples) append16(&bytes, static_cast<uint16_t>(sample));
    writeBinaryFile(path, bytes);
}

} // namespace

TEST_CASE("AssetImportSession round-trips and builds review rows", "[assets][asset_library][asset_import]") {
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_20260430_001";
    session.sourceKind = urpg::assets::AssetImportSourceKind::Zip;
    session.sourcePath = "C:/Users/Creator/Downloads/fantasy_pack.zip";
    session.managedSourceRoot = ".urpg/asset-library/sources/import_20260430_001/extracted";
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.createdAt = "2026-04-30T00:00:00Z";
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_20260430_001/characters/hero.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            1024,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
            "",
            {},
            false,
            "",
            -1,
            0,
            true,
            "image",
            "",
        },
        {
            "asset.music",
            "audio/theme.mp3",
            "asset://import_20260430_001/audio/theme.mp3",
            ".mp3",
            "audio",
            "audio/bgm",
            "Fantasy Pack",
            "bbb",
            2048,
            0,
            0,
            93000,
            false,
            "",
            false,
            false,
            false,
            false,
            {"conversion_required"},
            "converted/audio/theme.wav",
            {"ffmpeg", "-y", "-i", "audio/theme.mp3", "converted/audio/theme.wav"},
            true,
            "import_20260430_001:audio-theme",
            0,
            1,
            true,
            "audio",
            "",
        },
        {
            "asset.copy",
            "characters/hero-copy.png",
            "asset://import_20260430_001/characters/hero-copy.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            1024,
            48,
            48,
            0,
            true,
            "asset.hero",
            false,
            false,
            true,
            false,
            {},
            "",
            {},
            false,
            "",
            -1,
            0,
            true,
            "image",
            "",
        },
        {
            "asset.psd",
            "source/hero.psd",
            "asset://import_20260430_001/source/hero.psd",
            ".psd",
            "source",
            "source/art",
            "Fantasy Pack",
            "ccc",
            4096,
            0,
            0,
            0,
            false,
            "",
            true,
            false,
            false,
            false,
            {},
            "",
            {},
            false,
            "",
            -1,
            0,
            false,
            "none",
            "no_preview_source_only",
        },
        {
            "asset.exe",
            "tools/setup.exe",
            "",
            ".exe",
            "tool",
            "tooling",
            "Fantasy Pack",
            "ddd",
            8192,
            0,
            0,
            0,
            false,
            "",
            false,
            true,
            false,
            false,
            {"unsupported_format"},
            "",
            {},
            false,
            "",
            -1,
            0,
            false,
            "none",
            "no_preview_tooling_only",
        },
        {
            "asset.unlicensed",
            "ui/button.png",
            "asset://import_20260430_001/ui/button.png",
            ".png",
            "image",
            "ui",
            "Fantasy Pack",
            "eee",
            512,
            32,
            16,
            0,
            false,
            "",
            false,
            false,
            true,
            true,
            {},
            "",
            {},
            false,
            "",
            -1,
            0,
            true,
            "image",
            "",
        },
    };
    session.summary = urpg::assets::summarizeAssetImportSession(session);

    const auto json = urpg::assets::serializeAssetImportSession(session);
    const auto loaded = urpg::assets::deserializeAssetImportSession(json);

    REQUIRE(loaded.sessionId == session.sessionId);
    REQUIRE(loaded.sourceKind == urpg::assets::AssetImportSourceKind::Zip);
    REQUIRE(loaded.status == urpg::assets::AssetImportStatus::ReviewReady);
    REQUIRE(loaded.records.size() == 6);
    REQUIRE(loaded.summary.readyCount == 1);
    REQUIRE(loaded.summary.needsConversionCount == 1);
    REQUIRE(loaded.summary.duplicateCount == 1);
    REQUIRE(loaded.summary.missingLicenseCount == 1);
    REQUIRE(loaded.summary.sourceOnlyCount == 2);
    REQUIRE(loaded.records[1].conversionRequired);
    REQUIRE(loaded.records[1].conversionTargetPath == "converted/audio/theme.wav");
    REQUIRE(loaded.records[1].conversionCommand.size() == 5);
    REQUIRE(loaded.records[1].sequenceId == "import_20260430_001:audio-theme");
    REQUIRE(loaded.records[1].sequenceFrameIndex == 0);
    REQUIRE(loaded.records[1].sequenceFrameCount == 1);

    const auto rows = urpg::assets::buildAssetImportReviewRows({loaded});
    REQUIRE(rows.size() == 6);
    const auto ready = std::find_if(rows.begin(), rows.end(),
                                    [](const auto& row) { return row["relative_path"] == "characters/hero.png"; });
    REQUIRE(ready != rows.end());
    REQUIRE((*ready)["review_state"] == "ready_to_promote");
    REQUIRE((*ready)["recommended_action"] == "promote");
    REQUIRE((*ready)["promotable"] == true);
    REQUIRE((*ready)["preview_available"] == true);
    REQUIRE((*ready)["preview_kind"] == "image");

    const auto missingLicense =
        std::find_if(rows.begin(), rows.end(), [](const auto& row) { return row["relative_path"] == "ui/button.png"; });
    REQUIRE(missingLicense != rows.end());
    REQUIRE((*missingLicense)["review_state"] == "missing_license");
    REQUIRE((*missingLicense)["recommended_action"] == "add_license_attribution");

    const auto conversion = std::find_if(rows.begin(), rows.end(),
                                         [](const auto& row) { return row["relative_path"] == "audio/theme.mp3"; });
    REQUIRE(conversion != rows.end());
    REQUIRE((*conversion)["conversion_required"] == true);
    REQUIRE((*conversion)["conversion_target_path"] == "converted/audio/theme.wav");
    REQUIRE((*conversion)["sequence_id"] == "import_20260430_001:audio-theme");
    REQUIRE((*conversion)["preview_available"] == true);
    REQUIRE((*conversion)["preview_kind"] == "audio");

    const auto tooling = std::find_if(rows.begin(), rows.end(),
                                      [](const auto& row) { return row["relative_path"] == "tools/setup.exe"; });
    REQUIRE(tooling != rows.end());
    REQUIRE((*tooling)["preview_available"] == false);
    REQUIRE((*tooling)["no_preview_diagnostic"] == "no_preview_tooling_only");
}

TEST_CASE("AssetImportSession plans governed promotion manifests", "[assets][asset_library][asset_import][promotion]") {
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_20260430_001";
    session.managedSourceRoot = ".urpg/asset-library/sources/import_20260430_001/extracted";
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_20260430_001/sprite/hero.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            1024,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
            "",
            {},
            false,
            "",
            -1,
            0,
        },
        {
            "asset.theme",
            "audio/theme.mp3",
            "asset://import_20260430_001/audio/bgm/theme.mp3",
            ".mp3",
            "audio",
            "audio/bgm",
            "Fantasy Pack",
            "bbb",
            2048,
            0,
            0,
            93000,
            false,
            "",
            false,
            false,
            false,
            false,
            {"conversion_required"},
            "converted/audio/theme.wav",
            {"ffmpeg", "-y", "-i", "audio/theme.mp3", "converted/audio/theme.wav"},
            true,
            "",
            -1,
            0,
        },
        {
            "asset.copy",
            "characters/hero-copy.png",
            "asset://import_20260430_001/sprite/hero-copy.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            1024,
            48,
            48,
            0,
            true,
            "asset.hero",
            false,
            false,
            true,
            false,
            {},
            "",
            {},
            false,
            "",
            -1,
            0,
        },
    };
    session.records.front().authoredMetadata = {
        {"sprite_sheet_slice", {{"schema", "urpg.sprite_sheet_slice.v1"}, {"frame_width", 16}}},
    };

    const auto ready = urpg::assets::planAssetPromotionManifest(session, session.records[0], "user_license_note",
                                                                ".urpg/asset-library/promoted", true);
    REQUIRE(ready.status == urpg::assets::AssetPromotionStatus::RuntimeReady);
    REQUIRE(ready.assetId == "asset.hero");
    REQUIRE(ready.sourcePath == ".urpg/asset-library/sources/import_20260430_001/extracted/characters/hero.png");
    REQUIRE(ready.promotedPath == ".urpg/asset-library/promoted/asset.hero/payloads/hero.png");
    REQUIRE(ready.licenseId == "user_license_note");
    REQUIRE(ready.package.includeInRuntime);
    REQUIRE(ready.preview.kind == "image");
    REQUIRE(ready.preview.width == 48);
    REQUIRE(ready.authoredMetadata["sprite_sheet_slice"]["frame_width"] == 16);
    REQUIRE(ready.diagnostics.empty());

    const auto conversionNeeded = urpg::assets::planAssetPromotionManifest(
        session, session.records[1], "user_license_note", ".urpg/asset-library/promoted", true);
    REQUIRE(conversionNeeded.status == urpg::assets::AssetPromotionStatus::Blocked);
    REQUIRE_FALSE(conversionNeeded.package.includeInRuntime);
    REQUIRE(conversionNeeded.promotedPath.empty());
    REQUIRE(std::find(conversionNeeded.diagnostics.begin(), conversionNeeded.diagnostics.end(),
                      "source_record_requires_conversion") != conversionNeeded.diagnostics.end());

    const auto duplicate = urpg::assets::planAssetPromotionManifest(session, session.records[2], "user_license_note",
                                                                    ".urpg/asset-library/promoted", true);
    REQUIRE(duplicate.status == urpg::assets::AssetPromotionStatus::Blocked);
    REQUIRE(std::find(duplicate.diagnostics.begin(), duplicate.diagnostics.end(), "source_record_duplicate") !=
            duplicate.diagnostics.end());

    const auto missingLicense =
        urpg::assets::planAssetPromotionManifest(session, session.records[0], "", ".urpg/asset-library/promoted", true);
    REQUIRE(missingLicense.status == urpg::assets::AssetPromotionStatus::Blocked);
    REQUIRE(std::find(missingLicense.diagnostics.begin(), missingLicense.diagnostics.end(),
                      "license_evidence_missing") != missingLicense.diagnostics.end());
}

TEST_CASE("AssetImportSession builds character appearance import rows",
          "[assets][asset_library][asset_import][character]") {
    urpg::assets::AssetImportSession session;
    session.sessionId = "import_character_parts_001";
    session.sourceKind = urpg::assets::AssetImportSourceKind::Folder;
    session.sourcePath = "C:/Users/Creator/Downloads/actor_pack";
    session.managedSourceRoot = ".urpg/asset-library/sources/import_character_parts_001/original";
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.createdAt = "2026-05-01T01:00:00Z";
    session.records = {
        {
            "asset.hero.face",
            "img/faces/Actor1.png",
            "asset://import_character_parts_001/portrait/actor1.png",
            ".png",
            "image",
            "portrait",
            "Actor Pack",
            "facehash",
            1024,
            144,
            144,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
        },
        {
            "asset.hero.field",
            "img/characters/Actor1.png",
            "asset://import_character_parts_001/character/field/actor1.png",
            ".png",
            "image",
            "character/field",
            "Actor Pack",
            "fieldhash",
            2048,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
        },
        {
            "asset.hero.battle",
            "img/sv_actors/Actor1.png",
            "asset://import_character_parts_001/character/battle/actor1.png",
            ".png",
            "image",
            "character/battle",
            "Actor Pack",
            "battlehash",
            4096,
            576,
            384,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
        },
        {
            "asset.hero.missing_license",
            "portrait/missing-license.png",
            "asset://import_character_parts_001/portrait/missing-license.png",
            ".png",
            "image",
            "portrait",
            "Actor Pack",
            "missinghash",
            512,
            96,
            96,
            0,
            false,
            "",
            false,
            false,
            true,
            true,
            {},
        },
    };

    const auto rows = urpg::assets::buildAppearancePartImportRows({session});
    REQUIRE(rows.size() == 4);

    const auto portrait =
        std::find_if(rows.begin(), rows.end(), [](const auto& row) { return row["asset_id"] == "asset.hero.face"; });
    REQUIRE(portrait != rows.end());
    REQUIRE((*portrait)["slot"] == "portrait");
    REQUIRE((*portrait)["source_path"] ==
            ".urpg/asset-library/sources/import_character_parts_001/original/img/faces/Actor1.png");
    REQUIRE((*portrait)["normalized_asset_id"] == "asset.hero.face");
    REQUIRE((*portrait)["category"] == "portrait");
    REQUIRE((*portrait)["dimensions"]["width"] == 144);
    REQUIRE((*portrait)["dimensions"]["height"] == 144);
    REQUIRE((*portrait)["runtime_ready"] == true);
    REQUIRE((*portrait)["attribution_state"] == "complete");
    REQUIRE((*portrait)["blocked_reason"].is_null());
    REQUIRE((*portrait)["management_actions"]["accept"]["enabled"] == true);
    REQUIRE((*portrait)["management_actions"]["reject"]["enabled"] == true);
    REQUIRE((*portrait)["management_actions"]["archive"]["enabled"] == false);
    REQUIRE((*portrait)["management_actions"]["assign"]["target_slot"] == "portrait");

    const auto field =
        std::find_if(rows.begin(), rows.end(), [](const auto& row) { return row["asset_id"] == "asset.hero.field"; });
    REQUIRE(field != rows.end());
    REQUIRE((*field)["slot"] == "field");
    REQUIRE((*field)["category"] == "character/field");

    const auto battle =
        std::find_if(rows.begin(), rows.end(), [](const auto& row) { return row["asset_id"] == "asset.hero.battle"; });
    REQUIRE(battle != rows.end());
    REQUIRE((*battle)["slot"] == "battle");
    REQUIRE((*battle)["category"] == "character/battle");

    const auto blocked = std::find_if(rows.begin(), rows.end(),
                                      [](const auto& row) { return row["asset_id"] == "asset.hero.missing_license"; });
    REQUIRE(blocked != rows.end());
    REQUIRE((*blocked)["runtime_ready"] == false);
    REQUIRE((*blocked)["attribution_state"] == "missing_license");
    REQUIRE((*blocked)["blocked_reason"] == "license_evidence_missing");
    REQUIRE((*blocked)["management_actions"]["accept"]["enabled"] == false);
    REQUIRE((*blocked)["management_actions"]["assign"]["enabled"] == false);
}

TEST_CASE("GlobalAssetLibraryStore persists import sessions and promoted manifests",
          "[assets][asset_library][asset_import][promotion]") {
    const auto root = uniqueAssetTempRoot("urpg_global_asset_library_store");
    std::filesystem::remove_all(root);
    const auto libraryRoot = root / ".urpg" / "asset-library";
    urpg::assets::GlobalAssetLibraryStore store(libraryRoot);

    REQUIRE(store.catalogDatabasePath() == root / ".urpg" / "asset-index" / "asset_catalog.db");

    urpg::assets::AssetImportSession session;
    session.sessionId = "import:20260430";
    session.sourceKind = urpg::assets::AssetImportSourceKind::Zip;
    session.sourcePath = "C:/Users/Creator/Downloads/fantasy_pack.zip";
    session.managedSourceRoot = (libraryRoot / "sources" / "import-20260430" / "extracted").generic_string();
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.createdAt = "2026-04-30T00:00:00Z";
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import-20260430/characters/hero.png",
            ".png",
            "image",
            "characters",
            "Fantasy Pack",
            "abc",
            1024,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            true,
            {},
            "",
            {},
            false,
            "",
            -1,
            0,
        },
    };
    session.summary = urpg::assets::summarizeAssetImportSession(session);

    const auto importWrite = store.writeImportSession(session);
    REQUIRE(importWrite.success);
    REQUIRE(importWrite.code == "import_session_written");
    REQUIRE(std::filesystem::is_regular_file(store.importSessionManifestPath("import:20260430")));
    REQUIRE(std::filesystem::is_regular_file(store.sourceManifestPath("import:20260430")));

    const auto loadedSessions = store.loadImportSessions();
    REQUIRE(loadedSessions.size() == 1);
    REQUIRE(loadedSessions.front().sessionId == "import:20260430");
    REQUIRE(loadedSessions.front().records.size() == 1);

    urpg::assets::AssetPromotionManifest manifest;
    manifest.assetId = "asset.hero";
    manifest.sourcePath = "imports/raw/example/hero.png";
    manifest.promotedPath = (libraryRoot / "promoted" / "asset.hero" / "payloads" / "hero.png").generic_string();
    manifest.licenseId = "user_license_note";
    manifest.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    manifest.preview.kind = "image";
    manifest.preview.thumbnailPath = manifest.promotedPath;
    manifest.preview.width = 48;
    manifest.preview.height = 48;
    manifest.package.includeInRuntime = true;

    const auto promotedWrite = store.writePromotedAssetManifest(manifest);
    REQUIRE(promotedWrite.success);
    REQUIRE(promotedWrite.code == "promoted_asset_manifest_written");
    REQUIRE(std::filesystem::is_regular_file(store.promotedAssetManifestPath("asset.hero")));

    const auto loadedPromoted = store.loadPromotedAssetManifests();
    REQUIRE(loadedPromoted.size() == 1);
    REQUIRE(loadedPromoted.front().assetId == "asset.hero");
    REQUIRE(loadedPromoted.front().status == urpg::assets::AssetPromotionStatus::RuntimeReady);

    std::filesystem::remove_all(root);
}

TEST_CASE("GlobalAssetPromotionService copies quarantined payloads into promoted library",
          "[assets][asset_library][asset_import][promotion]") {
    const auto root = uniqueAssetTempRoot("urpg_global_asset_promotion");
    std::filesystem::remove_all(root);
    const auto quarantineRoot = root / ".urpg" / "asset-library" / "sources" / "import_001" / "extracted";
    const auto sourcePayload = quarantineRoot / "characters" / "hero.png";
    writeBinaryFile(sourcePayload, "hero-payload");

    urpg::assets::AssetImportSession session;
    session.sessionId = "import_001";
    session.managedSourceRoot = quarantineRoot.generic_string();
    session.records = {
        {
            "asset.hero",
            "characters/hero.png",
            "asset://import_001/sprite/hero.png",
            ".png",
            "image",
            "sprite",
            "Fantasy Pack",
            "aaa",
            12,
            48,
            48,
            0,
            false,
            "",
            false,
            false,
            true,
            false,
            {},
            "",
            {},
            false,
            "",
            -1,
            0,
        },
    };

    urpg::assets::GlobalAssetPromotionService service;
    const auto result = service.promoteImportRecord(session, session.records.front(), "user_license_note",
                                                    root / ".urpg" / "asset-library" / "promoted");

    REQUIRE(result.success);
    REQUIRE(result.code == "global_asset_promoted");
    REQUIRE(result.manifest.status == urpg::assets::AssetPromotionStatus::RuntimeReady);
    REQUIRE(std::filesystem::is_regular_file(result.payloadPath));
    REQUIRE(std::filesystem::is_regular_file(result.manifestPath));
    REQUIRE(result.payloadPath ==
            root / ".urpg" / "asset-library" / "promoted" / "asset.hero" / "payloads" / "hero.png");
    REQUIRE(result.manifestPath ==
            root / ".urpg" / "asset-library" / "promoted" / "asset.hero" / "asset_promotion_manifest.json");

    std::ifstream manifestStream(result.manifestPath);
    const auto manifest = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json::parse(manifestStream));
    manifestStream.close();
    REQUIRE(manifest.assetId == "asset.hero");
    REQUIRE(manifest.promotedPath == result.payloadPath.generic_string());
    REQUIRE(manifest.sourceSha256 == "aaa");
    REQUIRE(manifest.diagnostics.empty());

    const auto duplicateSource = quarantineRoot / "characters" / "hero-copy.png";
    writeBinaryFile(duplicateSource, "hero-payload");
    auto duplicateRecord = session.records.front();
    duplicateRecord.assetId = "asset.hero.copy";
    duplicateRecord.relativePath = "characters/hero-copy.png";
    const auto reused = service.promoteImportRecord(session, duplicateRecord, "user_license_note",
                                                    root / ".urpg" / "asset-library" / "promoted");
    REQUIRE(reused.success);
    REQUIRE(reused.code == "global_asset_reused_by_hash");
    REQUIRE(reused.payloadPath == result.payloadPath);
    REQUIRE(reused.manifestPath == result.manifestPath);

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetPromotionManifest round-trips runtime-ready promoted asset", "[assets][promotion]") {
    urpg::assets::AssetPromotionManifest manifest;
    manifest.schemaVersion = "1.0.0";
    manifest.assetId = "asset.hero.walk";
    manifest.sourcePath = "imports/raw/example/hero.png";
    manifest.promotedPath = "resources/assets/characters/hero.png";
    manifest.licenseId = "BND-001";
    manifest.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    manifest.preview.kind = "image";
    manifest.preview.thumbnailPath = "resources/previews/hero.thumb.png";
    manifest.preview.width = 48;
    manifest.preview.height = 48;
    manifest.package.includeInRuntime = true;

    const auto json = urpg::assets::serializeAssetPromotionManifest(manifest);
    const auto loaded = urpg::assets::deserializeAssetPromotionManifest(json);

    REQUIRE(loaded.assetId == manifest.assetId);
    REQUIRE(loaded.status == urpg::assets::AssetPromotionStatus::RuntimeReady);
    REQUIRE(loaded.package.includeInRuntime);
    REQUIRE(loaded.diagnostics.empty());
}

TEST_CASE("AssetPromotionManifest validates package and readiness blockers", "[assets][promotion]") {
    auto missingLicense = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.hero.walk"},
        {"sourcePath", "imports/raw/example/hero.png"},
        {"promotedPath", "resources/assets/characters/hero.png"},
        {"status", "runtime_ready"},
        {"preview", {{"kind", "image"}, {"thumbnailPath", "resources/previews/hero.thumb.png"}}},
        {"package", {{"includeInRuntime", true}}},
        {"diagnostics", nlohmann::json::array()},
    });
    REQUIRE(std::find(missingLicense.diagnostics.begin(), missingLicense.diagnostics.end(),
                      "license_evidence_missing") != missingLicense.diagnostics.end());

    auto missingPromotedPath = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.hero.walk"},
        {"sourcePath", "imports/raw/example/hero.png"},
        {"licenseId", "BND-001"},
        {"status", "runtime_ready"},
        {"preview", {{"kind", "pending"}}},
        {"package", {{"includeInRuntime", true}}},
        {"diagnostics", nlohmann::json::array()},
    });
    REQUIRE(std::find(missingPromotedPath.diagnostics.begin(), missingPromotedPath.diagnostics.end(),
                      "promoted_path_missing") != missingPromotedPath.diagnostics.end());

    auto archivedPackaged = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.hero.walk.copy"},
        {"sourcePath", "imports/raw/example/hero-copy.png"},
        {"promotedPath", "resources/assets/characters/hero-copy.png"},
        {"licenseId", "BND-001"},
        {"status", "archived"},
        {"preview", {{"kind", "none"}}},
        {"package", {{"includeInRuntime", true}, {"requiredForRelease", true}}},
        {"diagnostics", nlohmann::json::array()},
    });
    REQUIRE(std::find(archivedPackaged.diagnostics.begin(), archivedPackaged.diagnostics.end(),
                      "archived_asset_packaged") != archivedPackaged.diagnostics.end());
}

TEST_CASE("AssetTransformRevisionService creates deterministic atlas metadata revisions",
          "[assets][asset_library][asset_transform]") {
    const auto root = uniqueAssetTempRoot("urpg_asset_transform_revision");
    std::filesystem::remove_all(root);
    const auto sourcePayload = root / ".urpg" / "asset-library" / "promoted" / "asset.atlas" / "payloads" / "atlas.png";
    writeBinaryFile(sourcePayload, "atlas-source-payload");

    urpg::assets::AssetPromotionManifest source;
    source.assetId = "asset.atlas";
    source.sourcePath = "imports/raw/atlas.png";
    source.promotedPath = sourcePayload.generic_string();
    source.licenseId = "project_private";
    source.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    source.preview.kind = "image";
    source.preview.thumbnailPath = sourcePayload.generic_string();
    source.preview.width = 64;
    source.preview.height = 32;
    source.package.includeInRuntime = true;

    urpg::assets::AssetAtlasMetadataPlan plan;
    plan.operationId = "atlas-grid-8";
    plan.source = source;
    plan.derivedRoot = root / ".urpg" / "asset-library" / "derived";
    plan.atlasWidth = 64;
    plan.atlasHeight = 32;
    plan.frameWidth = 8;
    plan.frameHeight = 8;

    urpg::assets::AssetTransformRevisionService service;
    const auto created = service.createAtlasMetadataRevision(plan);
    REQUIRE(created.success);
    REQUIRE(created.code == "asset_transform_revision_created");
    REQUIRE(std::filesystem::is_regular_file(created.manifestPath));
    const auto reused = service.createAtlasMetadataRevision(plan);
    REQUIRE(reused.success);
    REQUIRE(reused.code == "asset_transform_revision_reused");
    REQUIRE(reused.derivedRevision == created.derivedRevision);
    REQUIRE(readBinaryFile(sourcePayload) == "atlas-source-payload");

    const urpg::assets::AssetTransformRevisionRemovalRequest removal{
        plan.derivedRoot, source.assetId, created.derivedRevision};
    const auto removed = service.removeDerivedRevision(removal);
    REQUIRE(removed.success);
    REQUIRE(removed.code == "asset_transform_revision_removed");
    REQUIRE_FALSE(std::filesystem::exists(created.manifestPath));
    REQUIRE(readBinaryFile(sourcePayload) == "atlas-source-payload");
    const auto missing = service.removeDerivedRevision(removal);
    REQUIRE_FALSE(missing.success);
    REQUIRE(missing.code == "asset_transform_revision_missing");

    plan.frameWidth = 7;
    const auto invalid = service.createAtlasMetadataRevision(plan);
    REQUIRE_FALSE(invalid.success);
    REQUIRE(invalid.code == "asset_transform_atlas_plan_invalid");

    const auto cropPayload = root / ".urpg" / "asset-library" / "promoted" / "asset.crop" / "payloads" / "crop.ppm";
    std::string ppm = "P6\n3 1\n255\n";
    ppm.append({static_cast<char>(0xFF), 0, 0, 0, static_cast<char>(0xFF), 0, 0, 0, static_cast<char>(0xFF)});
    writeBinaryFile(cropPayload, ppm);
    auto cropSource = source;
    cropSource.assetId = "asset.crop";
    cropSource.sourcePath = "imports/raw/crop.ppm";
    cropSource.promotedPath = cropPayload.generic_string();
    cropSource.preview.thumbnailPath = cropPayload.generic_string();
    cropSource.preview.width = 3;
    cropSource.preview.height = 1;
    urpg::assets::AssetImageCropScalePlan cropPlan;
    cropPlan.operationId = "crop-green-blue";
    cropPlan.source = cropSource;
    cropPlan.derivedRoot = plan.derivedRoot;
    cropPlan.cropX = 1;
    cropPlan.cropY = 0;
    cropPlan.cropWidth = 2;
    cropPlan.cropHeight = 1;
    cropPlan.outputWidth = 4;
    cropPlan.outputHeight = 2;
    const auto crop = service.createImageCropScaleRevision(cropPlan);
    REQUIRE(crop.success);
    REQUIRE(crop.code == "asset_transform_revision_created");
    REQUIRE(std::filesystem::is_regular_file(crop.outputPath));
    REQUIRE(readBinaryFile(cropPayload) == ppm);
    const auto reusedCrop = service.createImageCropScaleRevision(cropPlan);
    REQUIRE(reusedCrop.success);
    REQUIRE(reusedCrop.code == "asset_transform_revision_reused");
    cropPlan.cropX = 2;
    cropPlan.cropWidth = 2;
    const auto outOfBounds = service.createImageCropScaleRevision(cropPlan);
    REQUIRE_FALSE(outOfBounds.success);
    REQUIRE(outOfBounds.code == "asset_transform_crop_out_of_bounds");

    urpg::assets::AssetImagePalettePlan palettePlan;
    palettePlan.operationId = "two-color-palette";
    palettePlan.source = cropSource;
    palettePlan.derivedRoot = plan.derivedRoot;
    palettePlan.colorsRgba = {0xFF0000FFU, 0x0000FFFFU};
    const auto palette = service.createImagePaletteRevision(palettePlan);
    REQUIRE(palette.success);
    REQUIRE(std::filesystem::is_regular_file(palette.outputPath));
    int paletteWidth = 0;
    int paletteHeight = 0;
    int paletteChannels = 0;
    stbi_uc* palettePixels = stbi_load(palette.outputPath.string().c_str(), &paletteWidth, &paletteHeight,
                                      &paletteChannels, STBI_rgb_alpha);
    REQUIRE(palettePixels != nullptr);
    REQUIRE(paletteWidth == 3);
    REQUIRE(paletteHeight == 1);
    REQUIRE(palettePixels[0] == 0xFF);
    REQUIRE(palettePixels[4] == 0xFF);
    REQUIRE(palettePixels[8] == 0x00);
    REQUIRE(palettePixels[10] == 0xFF);
    stbi_image_free(palettePixels);
    REQUIRE(service.createImagePaletteRevision(palettePlan).code == "asset_transform_revision_reused");
    palettePlan.dither = true;
    const auto ditheredManualPalette = service.createImagePaletteRevision(palettePlan);
    REQUIRE(ditheredManualPalette.success);
    REQUIRE(ditheredManualPalette.derivedRevision != palette.derivedRevision);
    std::ifstream ditheredManualManifestStream(ditheredManualPalette.manifestPath);
    const auto ditheredManualManifest = nlohmann::json::parse(ditheredManualManifestStream);
    REQUIRE(ditheredManualManifest["dither"] == "floyd_steinberg_rgba_fixed16");
    REQUIRE(service.createImagePaletteRevision(palettePlan).code == "asset_transform_revision_reused");
    palettePlan.dither = false;
    palettePlan.colorsRgba = {0xFF0000FFU, 0xFF0000FFU};
    const auto invalidPalette = service.createImagePaletteRevision(palettePlan);
    REQUIRE_FALSE(invalidPalette.success);
    REQUIRE(invalidPalette.code == "asset_transform_palette_plan_invalid");

    urpg::assets::AssetImagePaletteExtractPlan paletteExtractPlan;
    paletteExtractPlan.operationId = "extract-two-colors";
    paletteExtractPlan.source = cropSource;
    paletteExtractPlan.derivedRoot = plan.derivedRoot;
    paletteExtractPlan.maxColors = 2;
    const auto extractedPalette = service.createImagePaletteExtractRevision(paletteExtractPlan);
    REQUIRE(extractedPalette.success);
    REQUIRE(std::filesystem::is_regular_file(extractedPalette.outputPath));
    std::ifstream extractedManifestStream(extractedPalette.manifestPath);
    const auto extractedManifest = nlohmann::json::parse(extractedManifestStream);
    REQUIRE(extractedManifest["operation"] == "image_palette_extract");
    REQUIRE(extractedManifest["palette_rgba"] == nlohmann::json::array({0x0000FFFFU, 0x00FF00FFU}));
    REQUIRE(service.createImagePaletteExtractRevision(paletteExtractPlan).code == "asset_transform_revision_reused");
    paletteExtractPlan.dither = true;
    const auto ditheredPalette = service.createImagePaletteExtractRevision(paletteExtractPlan);
    REQUIRE(ditheredPalette.success);
    REQUIRE(ditheredPalette.derivedRevision != extractedPalette.derivedRevision);
    std::ifstream ditheredManifestStream(ditheredPalette.manifestPath);
    const auto ditheredManifest = nlohmann::json::parse(ditheredManifestStream);
    REQUIRE(ditheredManifest["dither"] == "floyd_steinberg_rgba_fixed16");
    REQUIRE(service.createImagePaletteExtractRevision(paletteExtractPlan).code == "asset_transform_revision_reused");
    paletteExtractPlan.dither = false;
    paletteExtractPlan.maxColors = 1;
    const auto invalidExtractedPalette = service.createImagePaletteExtractRevision(paletteExtractPlan);
    REQUIRE_FALSE(invalidExtractedPalette.success);
    REQUIRE(invalidExtractedPalette.code == "asset_transform_palette_extract_plan_invalid");

    urpg::assets::AssetTilesetSlicePlan tilesetPlan;
    tilesetPlan.operationId = "slice-one-pixel-tiles";
    tilesetPlan.source = cropSource;
    tilesetPlan.derivedRoot = plan.derivedRoot;
    tilesetPlan.tileWidth = 1;
    tilesetPlan.tileHeight = 1;
    const auto tileset = service.createTilesetSliceRevision(tilesetPlan);
    REQUIRE(tileset.success);
    REQUIRE(std::filesystem::is_directory(tileset.outputPath));
    std::ifstream tilesetManifestStream(tileset.manifestPath);
    const auto tilesetManifest = nlohmann::json::parse(tilesetManifestStream);
    REQUIRE(tilesetManifest["grid"]["tile_count"] == 3);
    REQUIRE(tilesetManifest["output_paths"].size() == 3);
    tilesetManifestStream.close();
    REQUIRE(service.createTilesetSliceRevision(tilesetPlan).code == "asset_transform_revision_reused");
    const urpg::assets::AssetTransformRevisionRemovalRequest tilesetRemoval{
        tilesetPlan.derivedRoot, cropSource.assetId, tileset.derivedRevision};
    REQUIRE(service.removeDerivedRevision(tilesetRemoval).success);
    REQUIRE_FALSE(std::filesystem::exists(tileset.manifestPath));
    REQUIRE_FALSE(std::filesystem::exists(tileset.outputPath));
    tilesetPlan.tileWidth = 2;
    const auto invalidTileset = service.createTilesetSliceRevision(tilesetPlan);
    REQUIRE_FALSE(invalidTileset.success);
    REQUIRE(invalidTileset.code == "asset_transform_tileset_grid_invalid");

    const auto audioPayload = root / ".urpg" / "asset-library" / "promoted" / "asset.audio" / "payloads" / "tone.wav";
    const std::vector<int16_t> originalAudio = {10000, 10000, 10000, 10000, 10000, 10000};
    writePcm16Wav(audioPayload, originalAudio);
    const auto originalAudioBytes = readBinaryFile(audioPayload);
    auto audioSource = source;
    audioSource.assetId = "asset.audio";
    audioSource.sourcePath = "imports/raw/tone.wav";
    audioSource.promotedPath = audioPayload.generic_string();
    audioSource.preview.kind = "audio";
    audioSource.preview.thumbnailPath = audioPayload.generic_string();
    urpg::assets::AssetAudioTrimFadeGainPlan audioPlan;
    audioPlan.operationId = "trim-tone";
    audioPlan.source = audioSource;
    audioPlan.derivedRoot = plan.derivedRoot;
    audioPlan.startFrame = 1;
    audioPlan.endFrame = 5;
    audioPlan.fadeInFrames = 1;
    audioPlan.fadeOutFrames = 1;
    audioPlan.gainMilliDb = 0;
    audioPlan.loopStartFrame = 1;
    audioPlan.loopEndFrame = 3;
    const auto audio = service.createAudioTrimFadeGainRevision(audioPlan);
    REQUIRE(audio.success);
    REQUIRE(audio.code == "asset_transform_revision_created");
    REQUIRE(std::filesystem::is_regular_file(audio.outputPath));
    REQUIRE(readBinaryFile(audioPayload) == originalAudioBytes);
    std::ifstream audioManifestStream(audio.manifestPath);
    const auto audioManifest = nlohmann::json::parse(audioManifestStream);
    REQUIRE(audioManifest["codec"] == "pcm_s16le_wav");
    REQUIRE(audioManifest["frame_count"] == 4);
    REQUIRE(audioManifest["waveform_peaks"].size() == 64);
    REQUIRE(audioManifest["loop"]["start_frame"] == 1);
    audioManifestStream.close();
    REQUIRE(service.createAudioTrimFadeGainRevision(audioPlan).code == "asset_transform_revision_reused");
    const urpg::assets::AssetTransformRevisionRemovalRequest audioRemoval{
        audioPlan.derivedRoot, audioSource.assetId, audio.derivedRevision};
    REQUIRE(service.removeDerivedRevision(audioRemoval).success);
    REQUIRE_FALSE(std::filesystem::exists(audio.manifestPath));
    REQUIRE_FALSE(std::filesystem::exists(audio.outputPath));
    audioPlan.endFrame = 7;
    const auto invalidAudio = service.createAudioTrimFadeGainRevision(audioPlan);
    REQUIRE_FALSE(invalidAudio.success);
    REQUIRE(invalidAudio.code == "asset_transform_audio_range_invalid");

    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectAssetAttachmentService attaches validated single-output derived revisions",
          "[assets][asset_library][asset_attachment][asset_transform]") {
    const auto root = uniqueAssetTempRoot("urpg_derived_revision_attachment");
    std::filesystem::remove_all(root);
    const auto sourcePayload = root / ".urpg" / "asset-library" / "promoted" / "asset.hero" / "payloads" / "hero.ppm";
    const auto derivedRoot = root / ".urpg" / "asset-library" / "derived";
    const auto projectRoot = root / "project";
    std::string ppm = "P6\n2 1\n255\n";
    ppm.append({static_cast<char>(0xFF), 0, 0, 0, static_cast<char>(0xFF), 0});
    writeBinaryFile(sourcePayload, ppm);

    urpg::assets::AssetPromotionManifest source;
    source.assetId = "asset.hero";
    source.sourcePath = "imports/raw/characters/hero.ppm";
    source.promotedPath = sourcePayload.generic_string();
    source.licenseId = "project_private";
    source.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    source.preview.kind = "image";
    source.preview.thumbnailPath = source.promotedPath;
    source.preview.width = 2;
    source.preview.height = 1;
    source.package.includeInRuntime = true;

    urpg::assets::AssetTransformRevisionService transforms;
    urpg::assets::AssetImageCropScalePlan crop;
    crop.operationId = "crop-stale-check";
    crop.source = source;
    crop.derivedRoot = derivedRoot;
    crop.cropWidth = 2;
    crop.cropHeight = 1;
    crop.outputWidth = 2;
    crop.outputHeight = 1;
    const auto staleRevision = transforms.createImageCropScaleRevision(crop);
    REQUIRE(staleRevision.success);

    urpg::assets::ProjectAssetAttachmentService attachments;
    const auto stalePlan = attachments.planDerivedRevisionAttachment(source, staleRevision.manifestPath, projectRoot);
    REQUIRE(stalePlan.valid);
    writeBinaryFile(staleRevision.outputPath, "changed-derived-output");
    urpg::assets::ProjectDerivedAssetAttachmentRequest staleRequest;
    staleRequest.source = source;
    staleRequest.derivedManifestPath = staleRevision.manifestPath;
    staleRequest.projectRoot = projectRoot;
    staleRequest.operationId = "derived-attach-stale";
    staleRequest.expectedSourceRevision = stalePlan.sourceRevision;
    const auto stale = attachments.attachDerivedRevision(staleRequest);
    REQUIRE_FALSE(stale.success);
    REQUIRE(stale.code == "asset_attachment_source_revision_mismatch");

    urpg::assets::AssetImagePalettePlan palette;
    palette.operationId = "palette-attach";
    palette.source = source;
    palette.derivedRoot = derivedRoot;
    palette.colorsRgba = {0xFF0000FFU, 0x00FF00FFU};
    const auto revision = transforms.createImagePaletteRevision(palette);
    REQUIRE(revision.success);
    const auto plan = attachments.planDerivedRevisionAttachment(source, revision.manifestPath, projectRoot);
    REQUIRE(plan.valid);
    REQUIRE(plan.assetId == "asset.hero.revision." + revision.derivedRevision.substr(0, 16));
    REQUIRE(plan.sourceRevision.size() == 64);

    urpg::assets::AssetImagePaletteExtractPlan extractedPalette;
    extractedPalette.operationId = "palette-extract-attach";
    extractedPalette.source = source;
    extractedPalette.derivedRoot = derivedRoot;
    extractedPalette.maxColors = 2;
    const auto extractedRevision = transforms.createImagePaletteExtractRevision(extractedPalette);
    REQUIRE(extractedRevision.success);
    REQUIRE(attachments.planDerivedRevisionAttachment(source, extractedRevision.manifestPath, projectRoot).valid);

    urpg::assets::ProjectDerivedAssetAttachmentRequest request;
    request.source = source;
    request.derivedManifestPath = revision.manifestPath;
    request.projectRoot = projectRoot;
    request.operationId = "derived-attach-palette";
    request.expectedSourceRevision = plan.sourceRevision;
    const auto attached = attachments.attachDerivedRevision(request);
    REQUIRE(attached.success);
    REQUIRE(attached.code == "project_asset_attached");
    REQUIRE(std::filesystem::is_regular_file(attached.payloadPath));
    REQUIRE(readBinaryFile(attached.payloadPath) == readBinaryFile(revision.outputPath));
    REQUIRE(std::filesystem::is_regular_file(attached.manifestPath));
    std::ifstream projectManifestStream(attached.manifestPath);
    const auto projectManifest = nlohmann::json::parse(projectManifestStream);
    REQUIRE(projectManifest["authoredMetadata"]["derived_revision"]["schema"] ==
            "urpg.project_asset_derived_revision.v1");
    REQUIRE(projectManifest["authoredMetadata"]["derived_revision"]["derived_revision"] == revision.derivedRevision);
    REQUIRE(projectManifest["authoredMetadata"]["derived_revision"]["source_asset_id"] == source.assetId);

    const urpg::assets::AssetTransformRevisionRemovalRequest attachedRemoval{
        derivedRoot, source.assetId, revision.derivedRevision};
    const auto removalBlocked = transforms.removeDerivedRevision(attachedRemoval);
    REQUIRE_FALSE(removalBlocked.success);
    REQUIRE(removalBlocked.code == "asset_transform_revision_attached");
    REQUIRE(std::filesystem::is_regular_file(revision.manifestPath));
    REQUIRE(std::filesystem::is_regular_file(revision.outputPath));

    const auto replayed = attachments.attachDerivedRevision(request);
    REQUIRE(replayed.success);
    REQUIRE(replayed.code == "project_asset_attached");
    REQUIRE(replayed.message.find("without reapplying") != std::string::npos);

    urpg::editor::AssetLibraryModel model;
    std::string loadError;
    REQUIRE(model.loadProjectAssetAttachments(projectRoot, &loadError));
    REQUIRE(loadError.empty());
    const auto picker = std::find_if(model.snapshot().project_asset_picker_rows.begin(),
                                     model.snapshot().project_asset_picker_rows.end(), [&](const auto& row) {
                                         return row["asset_id"] == plan.assetId;
                                     });
    REQUIRE(picker != model.snapshot().project_asset_picker_rows.end());
    REQUIRE((*picker)["project_path"] == attached.payloadPath.generic_string());
    REQUIRE((*picker)["picker_kind"] == "portrait");
    REQUIRE((*picker)["picker_targets"][0] == "sprite_selector");

    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectAssetAttachmentService assigns validated derived tileset bundles",
          "[assets][asset_library][asset_attachment][tileset]") {
    const auto root = uniqueAssetTempRoot("urpg_derived_tileset_assignment");
    std::filesystem::remove_all(root);
    const auto sourcePayload = root / ".urpg" / "asset-library" / "promoted" / "asset.tiles" / "payloads" /
                               "tiles.ppm";
    const auto derivedRoot = root / ".urpg" / "asset-library" / "derived";
    const auto projectRoot = root / "project";
    std::string ppm = "P6\n3 1\n255\n";
    ppm.append({static_cast<char>(0xFF), 0, 0, 0, static_cast<char>(0xFF), 0, 0, 0, static_cast<char>(0xFF)});
    writeBinaryFile(sourcePayload, ppm);

    urpg::assets::AssetPromotionManifest source;
    source.assetId = "asset.tiles";
    source.sourcePath = "imports/raw/tiles.ppm";
    source.promotedPath = sourcePayload.generic_string();
    source.licenseId = "project_private";
    source.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    source.preview.kind = "image";
    source.preview.thumbnailPath = source.promotedPath;
    source.preview.width = 3;
    source.preview.height = 1;
    source.package.includeInRuntime = true;

    urpg::assets::AssetTransformRevisionService transforms;
    urpg::assets::AssetTilesetSlicePlan slice;
    slice.operationId = "three-single-tile-slice";
    slice.source = source;
    slice.derivedRoot = derivedRoot;
    slice.tileWidth = 1;
    slice.tileHeight = 1;
    const auto revision = transforms.createTilesetSliceRevision(slice);
    REQUIRE(revision.success);

    urpg::assets::ProjectAssetAttachmentService attachments;
    const auto plan = attachments.planDerivedTilesetAssignment(source, revision.manifestPath, projectRoot);
    REQUIRE(plan.valid);
    REQUIRE(plan.tilesetId == "asset.tiles.tileset." + revision.derivedRevision.substr(0, 16));
    REQUIRE(plan.columns == 3);
    REQUIRE(plan.rows == 1);
    REQUIRE(plan.sourceRevision.size() == 64);

    urpg::assets::ProjectDerivedTilesetAssignmentRequest request;
    request.source = source;
    request.derivedManifestPath = revision.manifestPath;
    request.projectRoot = projectRoot;
    request.operationId = "assign-derived-tiles";
    request.expectedSourceRevision = plan.sourceRevision;
    const auto assigned = attachments.assignDerivedTileset(request);
    REQUIRE(assigned.success);
    REQUIRE(assigned.code == "project_derived_tileset_assigned");
    REQUIRE(std::filesystem::is_directory(assigned.payloadPath));
    REQUIRE(std::filesystem::is_regular_file(assigned.manifestPath));
    REQUIRE(readBinaryFile(assigned.payloadPath / "000000.png") == readBinaryFile(revision.outputPath / "000000.png"));
    REQUIRE(readBinaryFile(assigned.payloadPath / "000002.png") == readBinaryFile(revision.outputPath / "000002.png"));
    std::ifstream assignmentManifestStream(assigned.manifestPath);
    const auto assignmentManifest = nlohmann::json::parse(assignmentManifestStream);
    REQUIRE(assignmentManifest["schema"] == "urpg.project_derived_tileset_assignment.v1");
    REQUIRE(assignmentManifest["grid"]["tile_count"] == 3);
    REQUIRE(assignmentManifest["tile_paths"].size() == 3);
    REQUIRE(assignmentManifest["atlas"]["path"] ==
            "content/tilesets/" + plan.tilesetId + "/atlas.png");
    REQUIRE(assignmentManifest["atlas"]["width"] == 3);
    REQUIRE(assignmentManifest["atlas"]["height"] == 1);
    REQUIRE(assignmentManifest["atlas"]["sha256"].get<std::string>().size() == 64);
    REQUIRE(std::filesystem::is_regular_file(assigned.payloadPath.parent_path() / "atlas.png"));

    const urpg::assets::AssetTransformRevisionRemovalRequest removal{
        derivedRoot, source.assetId, revision.derivedRevision};
    const auto removalBlocked = transforms.removeDerivedRevision(removal);
    REQUIRE_FALSE(removalBlocked.success);
    REQUIRE(removalBlocked.code == "asset_transform_revision_attached");

    const auto replayed = attachments.assignDerivedTileset(request);
    REQUIRE(replayed.success);
    REQUIRE(replayed.code == "project_derived_tileset_assigned");
    REQUIRE(replayed.message.find("without reapplying") != std::string::npos);

    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectAssetAttachmentService copies promoted payloads and writes project manifests",
          "[assets][asset_library][asset_attachment]") {
    const auto root = uniqueAssetTempRoot("urpg_project_asset_attachment");
    std::filesystem::remove_all(root);
    const auto globalPayload = root / ".urpg" / "asset-library" / "promoted" / "asset.hero" / "payloads" / "hero.png";
    const auto projectRoot = root / "project";
    writeBinaryFile(globalPayload, "hero-payload");

    urpg::assets::AssetPromotionManifest manifest;
    manifest.assetId = "asset.hero";
    manifest.sourcePath = ".urpg/asset-library/sources/import/extracted/characters/hero.png";
    manifest.promotedPath = globalPayload.string();
    manifest.licenseId = "user_license_note";
    manifest.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    manifest.preview.kind = "image";
    manifest.preview.thumbnailPath = globalPayload.string();
    manifest.preview.width = 48;
    manifest.preview.height = 48;
    manifest.package.includeInRuntime = true;

    urpg::assets::ProjectAssetAttachmentService service;
    const auto result = service.attachPromotedAsset(manifest, projectRoot);

    REQUIRE(result.success);
    REQUIRE(result.code == "project_asset_attached");
    REQUIRE(std::filesystem::is_regular_file(result.payloadPath));
    REQUIRE(std::filesystem::is_regular_file(result.manifestPath));
    REQUIRE(result.payloadPath == projectRoot / "content" / "assets" / "imported" / "asset.hero" / "hero.png");
    REQUIRE(result.manifestPath == projectRoot / "content" / "assets" / "manifests" / "asset.hero.json");

    const auto cancelled = service.attachPromotedAsset(manifest, projectRoot);
    REQUIRE_FALSE(cancelled.success);
    REQUIRE(cancelled.code == "project_attachment_conflict_requires_resolution");

    const auto keptBoth = service.attachPromotedAsset(
        manifest, projectRoot, urpg::assets::ProjectAssetAttachmentConflictPolicy::KeepBoth);
    REQUIRE(keptBoth.success);
    REQUIRE(keptBoth.payloadPath.filename() == "hero.png");
    REQUIRE(keptBoth.payloadPath.parent_path().filename() == "asset.hero-2");

    const auto relinked = service.attachPromotedAsset(
        manifest, projectRoot, urpg::assets::ProjectAssetAttachmentConflictPolicy::RelinkExisting);
    REQUIRE(relinked.success);
    REQUIRE(relinked.code == "project_asset_relinked_existing");
    REQUIRE(relinked.payloadPath == result.payloadPath);

    writeBinaryFile(globalPayload, "updated-hero-payload");
    const auto replaced = service.attachPromotedAsset(
        manifest, projectRoot, urpg::assets::ProjectAssetAttachmentConflictPolicy::Replace);
    REQUIRE(replaced.success);
    REQUIRE(readBinaryFile(replaced.payloadPath) == "updated-hero-payload");
    for (const auto& entry : std::filesystem::recursive_directory_iterator(projectRoot)) {
        const auto filename = entry.path().filename().string();
        REQUIRE(filename.find(".urpg-stage-") == std::string::npos);
        REQUIRE(filename.find(".urpg-backup-") == std::string::npos);
    }

    {
        std::ifstream manifestStream(result.manifestPath);
        const auto projectManifest =
            urpg::assets::deserializeAssetPromotionManifest(nlohmann::json::parse(manifestStream));
        REQUIRE(projectManifest.assetId == "asset.hero");
        REQUIRE(projectManifest.sourcePath == globalPayload.string());
        REQUIRE(projectManifest.promotedPath == result.payloadPath.generic_string());
        REQUIRE(projectManifest.status == urpg::assets::AssetPromotionStatus::RuntimeReady);
        REQUIRE(projectManifest.package.includeInRuntime);
        REQUIRE(projectManifest.diagnostics.empty());
    }

    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectAssetAttachmentService rejects blocked or missing promoted payloads",
          "[assets][asset_library][asset_attachment]") {
    const auto root = uniqueAssetTempRoot("urpg_project_asset_attachment_blocked");
    std::filesystem::remove_all(root);

    urpg::assets::AssetPromotionManifest blockedManifest;
    blockedManifest.assetId = "asset.theme";
    blockedManifest.promotedPath = (root / ".urpg" / "asset-library" / "promoted" / "theme.mp3").string();
    blockedManifest.licenseId = "user_license_note";
    blockedManifest.status = urpg::assets::AssetPromotionStatus::Blocked;
    blockedManifest.package.includeInRuntime = false;
    blockedManifest.preview.kind = "pending";
    blockedManifest.diagnostics = {"source_record_requires_conversion"};

    urpg::assets::ProjectAssetAttachmentService service;
    const auto blocked = service.attachPromotedAsset(blockedManifest, root / "project");
    REQUIRE_FALSE(blocked.success);
    REQUIRE(blocked.code == "asset_promotion_invalid");
    REQUIRE_FALSE(blocked.diagnostics.empty());

    urpg::assets::AssetPromotionManifest missingPayload;
    missingPayload.assetId = "asset.missing";
    missingPayload.promotedPath = (root / ".urpg" / "asset-library" / "promoted" / "missing.png").string();
    missingPayload.licenseId = "user_license_note";
    missingPayload.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    missingPayload.package.includeInRuntime = true;
    missingPayload.preview.kind = "none";

    const auto missing = service.attachPromotedAsset(missingPayload, root / "project");
    REQUIRE_FALSE(missing.success);
    REQUIRE(missing.code == "promoted_payload_missing");

    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectAssetAttachmentService restores interrupted attachment transactions",
          "[assets][asset_library][asset_attachment][recovery]") {
    const auto root = uniqueAssetTempRoot("urpg_project_asset_attachment_recovery");
    std::filesystem::remove_all(root);
    const auto projectRoot = root / "project";
    const auto sourcePayload = root / ".urpg" / "asset-library" / "promoted" / "asset.recovery" / "payloads" /
                               "hero.png";
    const auto payload = projectRoot / "content" / "assets" / "imported" / "asset.recovery" / "hero.png";
    const auto manifestPath = projectRoot / "content" / "assets" / "manifests" / "asset.recovery.json";
    const auto stagedPayload = payload.parent_path() / "hero.png.urpg-stage-payload-crash";
    const auto stagedManifest = manifestPath.parent_path() / "asset.recovery.json.urpg-stage-manifest-crash";
    const auto payloadBackup = payload.parent_path() / "hero.png.urpg-backup-payload-crash";
    const auto manifestBackup = manifestPath.parent_path() / "asset.recovery.json.urpg-backup-manifest-crash";
    writeBinaryFile(sourcePayload, "source-new");
    writeBinaryFile(payload, "interrupted-new");
    writeBinaryFile(payloadBackup, "prior-payload");
    writeBinaryFile(manifestBackup, "prior-manifest");
    writeBinaryFile(stagedPayload, "staged-payload");
    writeBinaryFile(stagedManifest, "staged-manifest");

    const auto journalPath = projectRoot / ".urpg" / "asset-attachment-transactions" / "attachment-crash.json";
    std::filesystem::create_directories(journalPath.parent_path());
    {
        std::ofstream journal(journalPath, std::ios::binary | std::ios::trunc);
        journal << nlohmann::json({
                       {"schema", "urpg.asset_attachment_transaction.v1"},
                       {"state", "payload_published"},
                       {"payload_path", "content/assets/imported/asset.recovery/hero.png"},
                       {"manifest_path", "content/assets/manifests/asset.recovery.json"},
                       {"staged_payload_path", "content/assets/imported/asset.recovery/hero.png.urpg-stage-payload-crash"},
                       {"staged_manifest_path", "content/assets/manifests/asset.recovery.json.urpg-stage-manifest-crash"},
                       {"payload_backup_path", "content/assets/imported/asset.recovery/hero.png.urpg-backup-payload-crash"},
                       {"manifest_backup_path", "content/assets/manifests/asset.recovery.json.urpg-backup-manifest-crash"},
                       {"had_payload", true},
                       {"had_manifest", true},
                   }).dump(2)
                << '\n';
    }

    urpg::assets::AssetPromotionManifest manifest;
    manifest.schemaVersion = "1.0.0";
    manifest.assetId = "asset.recovery";
    manifest.sourcePath = "imports/raw/hero.png";
    manifest.promotedPath = sourcePayload.generic_string();
    manifest.licenseId = "project_private";
    manifest.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    manifest.preview.kind = "image";
    manifest.preview.thumbnailPath = sourcePayload.generic_string();
    manifest.preview.width = 48;
    manifest.preview.height = 48;
    manifest.package.includeInRuntime = true;

    urpg::assets::ProjectAssetAttachmentService service;
    const auto attached = service.attachPromotedAsset(
        manifest, projectRoot, urpg::assets::ProjectAssetAttachmentConflictPolicy::KeepBoth);
    REQUIRE(attached.success);
    REQUIRE(readBinaryFile(payload) == "prior-payload");
    REQUIRE(readBinaryFile(manifestPath) == "prior-manifest");
    REQUIRE_FALSE(std::filesystem::exists(journalPath));
    REQUIRE_FALSE(std::filesystem::exists(stagedPayload));
    REQUIRE_FALSE(std::filesystem::exists(stagedManifest));
    REQUIRE_FALSE(std::filesystem::exists(payloadBackup));
    REQUIRE_FALSE(std::filesystem::exists(manifestBackup));

    std::filesystem::remove_all(root);
}

TEST_CASE("ProjectAssetAttachmentService applies checked revisions and operation receipts",
          "[assets][asset_library][asset_attachment][revision]") {
    const auto root = uniqueAssetTempRoot("urpg_project_asset_attachment_revision");
    std::filesystem::remove_all(root);
    const auto projectRoot = root / "project";
    const auto globalPayload = root / ".urpg" / "asset-library" / "promoted" / "asset.revision" / "payloads" /
                               "hero.png";
    std::filesystem::create_directories(projectRoot / "content");
    writeBinaryFile(globalPayload, "revision-one");

    urpg::assets::AssetPromotionManifest manifest;
    manifest.assetId = "asset.revision";
    manifest.sourcePath = "imports/raw/hero.png";
    manifest.promotedPath = globalPayload.string();
    manifest.licenseId = "user_license_note";
    manifest.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    manifest.preview.kind = "image";
    manifest.preview.thumbnailPath = globalPayload.string();
    manifest.preview.width = 48;
    manifest.preview.height = 48;
    manifest.package.includeInRuntime = true;

    urpg::assets::ProjectAssetAttachmentService service;
    const auto plan = service.planPromotedAssetAttachment(manifest, projectRoot);
    REQUIRE(plan.valid);
    REQUIRE_FALSE(plan.sourceRevision.empty());

    urpg::assets::ProjectAssetAttachmentRequest request;
    request.manifest = manifest;
    request.projectRoot = projectRoot;
    request.operationId = "attach-revision-1";
    request.expectedSourceRevision = plan.sourceRevision;
    const auto applied = service.attachPromotedAsset(request);
    REQUIRE(applied.success);
    REQUIRE(applied.code == "project_asset_attached");
    REQUIRE(applied.sourceRevision == plan.sourceRevision);
    REQUIRE(std::filesystem::is_regular_file(projectRoot / ".urpg" / "asset-attachment-operations" /
                                              "attach-revision-1.json"));

    const auto replayed = service.attachPromotedAsset(request);
    REQUIRE(replayed.success);
    REQUIRE(replayed.code == "project_asset_attached");
    REQUIRE(replayed.message.find("without reapplying") != std::string::npos);

    auto conflictingRequest = request;
    conflictingRequest.manifest.licenseId = "different_license";
    const auto conflicting = service.attachPromotedAsset(conflictingRequest);
    REQUIRE_FALSE(conflicting.success);
    REQUIRE(conflicting.code == "asset_attachment_operation_mismatch");

    writeBinaryFile(globalPayload, "revision-two");
    urpg::assets::ProjectAssetAttachmentRequest staleRequest;
    staleRequest.manifest = manifest;
    staleRequest.projectRoot = projectRoot;
    staleRequest.operationId = "attach-revision-2";
    staleRequest.expectedSourceRevision = plan.sourceRevision;
    const auto stale = service.attachPromotedAsset(staleRequest);
    REQUIRE_FALSE(stale.success);
    REQUIRE(stale.code == "asset_attachment_source_revision_mismatch");
    REQUIRE(readBinaryFile(applied.payloadPath) == "revision-one");

    std::filesystem::remove_all(root);
}

TEST_CASE("AssetLibrary ingests duplicate groups deterministically", "[assets][asset_library]") {
    urpg::assets::AssetLibrary library;

    library.ingestDuplicateCsv("sha256,size_bytes,path_rel,recommended_keep,recommended_remove\n"
                               "bbb,20,z/path.png,z/path.png,no\n"
                               "aaa,10,b/path.png,a/path.png,yes\n"
                               "aaa,10,a/path.png,a/path.png,no\n");

    const auto& snapshot = library.snapshot();
    REQUIRE(snapshot.duplicate_groups.size() == 2);
    REQUIRE(snapshot.duplicate_groups[0].sha256 == "aaa");
    REQUIRE(snapshot.duplicate_groups[0].entries[0].path == "a/path.png");
    REQUIRE(snapshot.duplicate_groups[0].entries[1].path == "b/path.png");
    REQUIRE(snapshot.assets[0].path == "a/path.png");
    REQUIRE(snapshot.assets[1].path == "b/path.png");
}

TEST_CASE("AssetLibrary provenance packet round-trips JSON", "[assets][asset_library]") {
    urpg::assets::AssetLibrary library;
    library.ingestIntakeReport(
        nlohmann::json{{"sources",
                        {{{"source_id", "SRC-001"},
                          {"repo_name", "GDQuest/game-sprites"},
                          {"legal_disposition", "cc0_candidate_recorded_for_private_use_intake"},
                          {"promotion_status", "promoted"},
                          {"normalized_assets", {"imports/normalized/prototype_sprites/gdquest_blue_actor.svg"}}}}}});

    const auto asset = library.findAsset("imports/normalized/prototype_sprites/gdquest_blue_actor.svg");
    REQUIRE(asset.has_value());

    const auto packet = nlohmann::json::parse(urpg::assets::exportProvenancePacket(*asset));
    const auto provenance = urpg::assets::assetProvenanceFromJson(packet["provenance"]);
    REQUIRE(provenance.original_source == "GDQuest/game-sprites");
    REQUIRE(provenance.export_eligible);
}

TEST_CASE("AssetLibrary ingests local promotion catalog records", "[assets][asset_library][asset_intake]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-007"},
        {"source_root", "imports/raw/urpg_stuff"},
        {"export_eligible", false},
        {"assets",
         {
             {
                 {"source_path", "imports/raw/urpg_stuff/audio/UI/click.ogg"},
                 {"normalized_path", "asset://src-007/audio/ui/click-abc123.ogg"},
                 {"preview_path", "imports/raw/urpg_stuff/audio/UI/click.ogg"},
                 {"preview_kind", "audio"},
                 {"duration_ms", 880},
                 {"waveform_peaks", {0.1, 0.5, 1.0}},
                 {"media_kind", "audio"},
                 {"category", "audio/ui"},
                 {"pack", "UI Soundpack"},
                 {"size_bytes", 1234},
                 {"sha256", "abc123"},
                 {"tags", {"kind:audio", "category:audio-ui", "ui"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
             {
                 {"source_path", "imports/raw/urpg_stuff/audio/UI/click-copy.ogg"},
                 {"normalized_path", "asset://src-007/audio/ui/click-copy-abc123.ogg"},
                 {"media_kind", "audio"},
                 {"category", "audio/ui"},
                 {"pack", "UI Soundpack"},
                 {"size_bytes", 1234},
                 {"sha256", "abc123"},
                 {"duplicate_of", "src007:abc123"},
                 {"status", "duplicate"},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
         }},
    });

    const auto asset = library.findAsset("imports/raw/urpg_stuff/audio/UI/click.ogg");
    REQUIRE(asset.has_value());
    REQUIRE(asset->normalized_path == "asset://src-007/audio/ui/click-abc123.ogg");
    REQUIRE(asset->preview_kind == "audio");
    REQUIRE(asset->duration_ms == 880);
    REQUIRE(asset->waveform_peaks.size() == 3);
    REQUIRE(asset->media_kind == "audio");
    REQUIRE(asset->category == "audio/ui");
    REQUIRE(asset->tags.size() == 3);

    const auto duplicate = library.findAsset("imports/raw/urpg_stuff/audio/UI/click-copy.ogg");
    REQUIRE(duplicate.has_value());
    REQUIRE(duplicate->statuses.contains(urpg::assets::AssetStatus::Duplicate));
}

TEST_CASE("AssetLibrary ingests promotion catalog summary", "[assets][asset_library][asset_intake]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-007"},
        {"source_root", "imports/raw/urpg_stuff"},
        {"promotion_status", "cataloged_local"},
        {"export_eligible", false},
        {"summary",
         {
             {"asset_count", 55192},
             {"canonical_asset_count", 53126},
             {"duplicate_group_count", 2066},
             {"duplicate_asset_count", 2066},
             {"unsupported_count", 0},
             {"category_counts", {{"audio/ui", 240}, {"characters", 1438}}},
             {"kind_counts", {{"audio", 1303}, {"image", 53881}}},
         }},
        {"shards",
         {
             {{"category", "audio/ui"},
              {"path", "imports/reports/asset_intake/urpg_stuff_promotion_catalog/audio-ui.json"}},
             {{"category", "characters"},
              {"path", "imports/reports/asset_intake/urpg_stuff_promotion_catalog/characters.json"}},
         }},
    });

    const auto& snapshot = library.snapshot();
    REQUIRE(snapshot.catalog_asset_count == 55192);
    REQUIRE(snapshot.canonical_asset_count == 53126);
    REQUIRE(snapshot.duplicate_group_count == 2066);
    REQUIRE(snapshot.duplicate_asset_count == 2066);
    REQUIRE(snapshot.unsupported_count == 0);
    REQUIRE(snapshot.catalog_shard_count == 2);
    REQUIRE_FALSE(snapshot.export_eligible);
    REQUIRE(snapshot.promotion_status == "cataloged_local");
    REQUIRE(snapshot.category_counts.at("audio/ui") == 240);
    REQUIRE(snapshot.kind_counts.at("image") == 53881);
}

TEST_CASE("AssetLibrary detects case collisions and unsupported paths", "[assets][asset_library]") {
    urpg::assets::AssetLibrary library;
    library.markUnsupportedFormat("Assets/Hero.PNG");
    library.markMissingFile("assets/hero.png");

    library.detectCaseCollisions();

    const auto upper = library.findAsset("Assets/Hero.PNG");
    const auto lower = library.findAsset("assets/hero.png");
    REQUIRE(upper.has_value());
    REQUIRE(lower.has_value());
    REQUIRE(upper->statuses.contains(urpg::assets::AssetStatus::UnsupportedFormat));
    REQUIRE(upper->statuses.contains(urpg::assets::AssetStatus::CaseCollision));
    REQUIRE(lower->statuses.contains(urpg::assets::AssetStatus::MissingFile));
    REQUIRE(lower->statuses.contains(urpg::assets::AssetStatus::CaseCollision));
    REQUIRE(library.snapshot().case_collision_count == 2);
}

TEST_CASE("AssetLibrary filters by tags status references and runtime readiness", "[assets][asset_library][browser]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(
        nlohmann::json{{"source_id", "SRC-007"},
                       {"source_root", "imports/raw/urpg_stuff"},
                       {"export_eligible", false},
                       {"assets",
                        {
                            {
                                {"source_path", "imports/raw/urpg_stuff/characters/hero.png"},
                                {"normalized_path", "asset://src-007/characters/hero.png"},
                                {"preview_path", "imports/raw/urpg_stuff/characters/hero.png"},
                                {"preview_kind", "image"},
                                {"preview_width", 64},
                                {"preview_height", 48},
                                {"media_kind", "image"},
                                {"category", "characters"},
                                {"pack", "Hero Pack"},
                                {"tags", {"kind:image", "character", "hero"}},
                                {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
                            },
                            {
                                {"source_path", "imports/raw/urpg_stuff/audio/click.ogg"},
                                {"normalized_path", "asset://src-007/audio/click.ogg"},
                                {"preview_path", "imports/raw/urpg_stuff/audio/click.ogg"},
                                {"preview_kind", "audio"},
                                {"duration_ms", 420},
                                {"waveform_peaks", {0.2, 0.4, 0.8, 0.4}},
                                {"media_kind", "audio"},
                                {"category", "audio/ui"},
                                {"pack", "UI Pack"},
                                {"tags", {"kind:audio", "ui"}},
                                {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
                            },
                            {
                                {"source_path", "imports/raw/urpg_stuff/audio/click-copy.ogg"},
                                {"normalized_path", "asset://src-007/audio/click-copy.ogg"},
                                {"preview_path", "imports/raw/urpg_stuff/audio/click-copy.ogg"},
                                {"preview_kind", "audio"},
                                {"media_kind", "audio"},
                                {"category", "audio/ui"},
                                {"pack", "UI Pack"},
                                {"tags", {"kind:audio", "ui"}},
                                {"duplicate_of", "click"},
                                {"status", "duplicate"},
                                {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
                            },
                        }}});

    library.addUsageReference("imports/raw/urpg_stuff/characters/hero.png", "project.map_001");
    library.addUsageReference("imports/raw/urpg_stuff/characters/hero.png", "project.actor_hero");

    urpg::assets::AssetLibraryFilter image_filter;
    image_filter.media_kind = "image";
    image_filter.required_tag = "hero";
    image_filter.referenced_only = true;
    image_filter.runtime_ready_only = true;
    image_filter.previewable_only = true;
    const auto images = library.filterAssets(image_filter);

    REQUIRE(images.size() == 1);
    REQUIRE(images.front().used_by.size() == 2);
    REQUIRE(library.snapshot().referenced_asset_count == 1);
    REQUIRE(library.snapshot().runtime_ready_count == 2);
    REQUIRE(library.snapshot().previewable_count == 3);

    urpg::assets::AssetLibraryFilter duplicate_filter;
    duplicate_filter.required_status = urpg::assets::AssetStatus::Duplicate;
    REQUIRE(library.filterAssets(duplicate_filter).size() == 1);
}

TEST_CASE("AssetLibrary promotes and archives curated assets", "[assets][asset_library][browser][actions]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(
        nlohmann::json{{"source_id", "SRC-007"},
                       {"source_root", "imports/raw/urpg_stuff"},
                       {"assets",
                        {
                            {
                                {"source_path", "imports/raw/urpg_stuff/characters/hero.png"},
                                {"normalized_path", "asset://src-007/characters/hero.png"},
                                {"preview_path", "imports/raw/urpg_stuff/characters/hero.png"},
                                {"preview_kind", "image"},
                                {"preview_width", 64},
                                {"preview_height", 48},
                                {"media_kind", "image"},
                                {"category", "characters"},
                                {"tags", {"hero", "kind:image"}},
                                {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
                            },
                            {
                                {"source_path", "imports/raw/urpg_stuff/characters/hero-copy.png"},
                                {"normalized_path", "asset://src-007/characters/hero-copy.png"},
                                {"media_kind", "image"},
                                {"category", "characters"},
                                {"duplicate_of", "hero"},
                                {"status", "duplicate"},
                                {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
                            },
                        }}});

    const auto promoted = library.promoteAsset("imports/raw/urpg_stuff/characters/hero.png");
    REQUIRE(promoted.success);
    REQUIRE(promoted.code == "asset_promoted");
    auto hero = library.findAsset("imports/raw/urpg_stuff/characters/hero.png");
    REQUIRE(hero.has_value());
    REQUIRE(hero->statuses.contains(urpg::assets::AssetStatus::Promoted));
    REQUIRE(hero->provenance.export_eligible);
    REQUIRE(library.snapshot().promoted_count == 1);

    const auto duplicate = library.promoteAsset("imports/raw/urpg_stuff/characters/hero-copy.png");
    REQUIRE_FALSE(duplicate.success);
    REQUIRE(duplicate.code == "asset_duplicate");

    const auto archived = library.archiveAsset("imports/raw/urpg_stuff/characters/hero.png", "not needed");
    REQUIRE(archived.success);
    hero = library.findAsset("imports/raw/urpg_stuff/characters/hero.png");
    REQUIRE(hero.has_value());
    REQUIRE(hero->statuses.contains(urpg::assets::AssetStatus::Archived));
    REQUIRE_FALSE(hero->statuses.contains(urpg::assets::AssetStatus::Promoted));
    REQUIRE_FALSE(hero->provenance.export_eligible);
    REQUIRE(library.snapshot().promoted_count == 0);
    REQUIRE(library.snapshot().archived_count == 1);
    REQUIRE(library.snapshot().runtime_ready_count == 0);
}

TEST_CASE("Asset action view recommends promote archive and blocked states",
          "[assets][asset_library][browser][actions]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-007"},
        {"source_root", "imports/raw/urpg_stuff"},
        {"assets",
         {
             {
                 {"source_path", "imports/raw/urpg_stuff/characters/hero.png"},
                 {"normalized_path", "asset://src-007/characters/hero.png"},
                 {"preview_path", "imports/raw/urpg_stuff/characters/hero.png"},
                 {"preview_kind", "image"},
                 {"preview_width", 64},
                 {"preview_height", 48},
                 {"media_kind", "image"},
                 {"category", "characters"},
                 {"tags", {"hero", "kind:image"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
             {
                 {"source_path", "imports/raw/urpg_stuff/characters/hero-copy.png"},
                 {"normalized_path", "asset://src-007/characters/hero-copy.png"},
                 {"media_kind", "image"},
                 {"category", "characters"},
                 {"duplicate_of", "hero"},
                 {"status", "duplicate"},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
             {
                 {"source_path", "imports/raw/urpg_stuff/audio/click.ogg"},
                 {"normalized_path", "asset://src-007/audio/click.ogg"},
                 {"preview_path", "imports/raw/urpg_stuff/audio/click.ogg"},
                 {"preview_kind", "audio"},
                 {"duration_ms", 420},
                 {"waveform_peaks", {0.2, 0.4, 0.8, 0.4}},
                 {"media_kind", "audio"},
                 {"category", "audio/ui"},
                 {"tags", {"ui", "kind:audio"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
             {
                 {"source_path", "imports/raw/urpg_stuff/characters/unlicensed.png"},
                 {"normalized_path", "asset://src-007/characters/unlicensed.png"},
                 {"media_kind", "image"},
                 {"category", "characters"},
             },
             {
                 {"source_path", "imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon"},
                 {"normalized_path", "asset://src-008/characters/isometric/animated-demon"},
                 {"preview_path", "imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon/Idle/0001.png"},
                 {"preview_kind", "image"},
                 {"preview_width", 512},
                 {"preview_height", 512},
                 {"media_kind", "image_sequence_collection"},
                 {"category", "characters/isometric"},
                 {"pack", "Animated Demon"},
                 {"frame_count", 1200},
                 {"sequence_count", 18},
                 {"representative_sequences",
                  {{{"path", "imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon/Idle"},
                    {"frame_count", 64},
                    {"representative_files",
                     {"imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon/Idle/0001.png"}}}}},
                 {"tags", {"kind:image_sequence_collection", "category:characters-isometric"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
         }}});
    library.addUsageReference("imports/raw/urpg_stuff/characters/hero.png", "actor.hero");

    REQUIRE(library.snapshot().sequence_asset_count == 1);
    REQUIRE(library.snapshot().sequence_frame_count == 1200);
    REQUIRE(library.snapshot().sequence_clip_count == 18);

    const auto rows = urpg::assets::buildAssetActionRows(library.snapshot());
    REQUIRE(rows.size() == 5);

    const auto hero = std::find_if(rows.begin(), rows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/urpg_stuff/characters/hero.png";
    });
    REQUIRE(hero != rows.end());
    REQUIRE((*hero)["recommended_action"] == "promote");
    REQUIRE((*hero)["promote_button"]["enabled"] == true);
    REQUIRE((*hero)["attach_button"]["enabled"] == false);
    REQUIRE((*hero)["attach_button"]["disabled_reason"] == "asset_not_promoted");
    REQUIRE((*hero)["archive_button"]["enabled"] == false);
    REQUIRE((*hero)["archive_button"]["disabled_reason"] == "asset_in_use");

    const auto duplicate = std::find_if(rows.begin(), rows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/urpg_stuff/characters/hero-copy.png";
    });
    REQUIRE(duplicate != rows.end());
    REQUIRE((*duplicate)["recommended_action"] == "archive_duplicate");
    REQUIRE((*duplicate)["promote_button"]["enabled"] == false);
    REQUIRE((*duplicate)["promote_button"]["disabled_reason"] == "asset_duplicate");
    REQUIRE((*duplicate)["attach_button"]["enabled"] == false);
    REQUIRE((*duplicate)["archive_button"]["enabled"] == true);

    const auto unlicensed = std::find_if(rows.begin(), rows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/urpg_stuff/characters/unlicensed.png";
    });
    REQUIRE(unlicensed != rows.end());
    REQUIRE((*unlicensed)["recommended_action"] == "add_license_evidence");
    REQUIRE((*unlicensed)["promote_button"]["enabled"] == false);
    REQUIRE((*unlicensed)["promote_button"]["disabled_reason"] == "asset_missing_license");
    REQUIRE((*unlicensed)["attach_button"]["disabled_reason"] == "asset_not_promoted");
    REQUIRE((*unlicensed)["readiness"]["warning_count"].get<std::size_t>() >= 1);
    REQUIRE((*unlicensed)["readiness"]["diagnostics"][0]["code"] == "license_evidence_missing");
    REQUIRE((*unlicensed)["readiness"]["render_contract"]["component"] == "asset_readiness_badge_stack");

    const auto sequence = std::find_if(rows.begin(), rows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon";
    });
    REQUIRE(sequence != rows.end());
    REQUIRE((*sequence)["sequence"]["visible"] == true);
    REQUIRE((*sequence)["sequence"]["frame_count"] == 1200);
    REQUIRE((*sequence)["sequence"]["sequence_count"] == 18);
    REQUIRE((*sequence)["sequence"]["representative_sequences"].size() == 1);

    const auto previewRows = urpg::assets::buildAssetPreviewRows(library.snapshot());
    REQUIRE(previewRows.size() == 5);
    const auto heroPreview = std::find_if(previewRows.begin(), previewRows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/urpg_stuff/characters/hero.png";
    });
    REQUIRE(heroPreview != previewRows.end());
    REQUIRE((*heroPreview)["status"] == "ready");
    REQUIRE((*heroPreview)["thumbnail"]["ready"] == true);
    REQUIRE((*heroPreview)["thumbnail"]["width"] == 64);
    REQUIRE((*heroPreview)["thumbnail"]["height"] == 48);

    const auto audioPreview = std::find_if(previewRows.begin(), previewRows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/urpg_stuff/audio/click.ogg";
    });
    REQUIRE(audioPreview != previewRows.end());
    REQUIRE((*audioPreview)["status"] == "ready");
    REQUIRE((*audioPreview)["waveform"]["ready"] == true);
    REQUIRE((*audioPreview)["waveform"]["duration_ms"] == 420);
    REQUIRE((*audioPreview)["waveform"]["peak_count"] == 4);
    REQUIRE((*audioPreview)["readiness"]["preview_ready"] == true);
    REQUIRE((*audioPreview)["readiness"]["render_contract"]["diagnostic_renderer"] ==
            "asset_readiness_diagnostic_rows");

    const auto sequencePreview = std::find_if(previewRows.begin(), previewRows.end(), [](const auto& row) {
        return row["path"] == "imports/raw/urpg_stuff/assets_to_ingest_20260429/Animated Demon";
    });
    REQUIRE(sequencePreview != previewRows.end());
    REQUIRE((*sequencePreview)["status"] == "ready");
    REQUIRE((*sequencePreview)["sequence"]["visible"] == true);
    REQUIRE((*sequencePreview)["sequence"]["frame_count"] == 1200);
    REQUIRE((*sequencePreview)["sequence"]["sequence_count"] == 18);
    REQUIRE((*sequencePreview)["thumbnail"]["ready"] == true);
    REQUIRE((*sequencePreview)["readiness"]["preview_ready"] == true);
}

TEST_CASE("AssetLibrary action rows expose governed promotion manifests", "[assets][promotion][asset_library]") {
    urpg::assets::AssetLibrary library;

    auto ready = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.hero.walk"},
        {"sourcePath", "imports/raw/example/hero.png"},
        {"promotedPath", "resources/assets/characters/hero.png"},
        {"licenseId", "BND-001"},
        {"status", "runtime_ready"},
        {"preview",
         {{"kind", "image"}, {"thumbnailPath", "resources/previews/hero.thumb.png"}, {"width", 48}, {"height", 48}}},
        {"package", {{"includeInRuntime", true}, {"requiredForRelease", false}}},
        {"diagnostics", nlohmann::json::array()},
    });
    library.ingestPromotionManifest(ready);

    auto missingLicense = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.unlicensed"},
        {"sourcePath", "imports/raw/example/unlicensed.png"},
        {"promotedPath", "resources/assets/characters/unlicensed.png"},
        {"status", "runtime_ready"},
        {"preview",
         {{"kind", "image"},
          {"thumbnailPath", "resources/previews/unlicensed.thumb.png"},
          {"width", 48},
          {"height", 48}}},
        {"package", {{"includeInRuntime", true}}},
        {"diagnostics", nlohmann::json::array()},
    });
    library.ingestPromotionManifest(missingLicense);

    auto missingFile = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.missing"},
        {"sourcePath", "imports/raw/example/missing.png"},
        {"licenseId", "BND-001"},
        {"status", "runtime_ready"},
        {"preview", {{"kind", "pending"}}},
        {"package", {{"includeInRuntime", true}}},
        {"diagnostics", nlohmann::json::array()},
    });
    library.ingestPromotionManifest(missingFile);

    auto archivedDuplicate = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", "asset.hero.copy"},
        {"sourcePath", "imports/raw/example/hero-copy.png"},
        {"promotedPath", "resources/assets/characters/hero-copy.png"},
        {"licenseId", "BND-001"},
        {"status", "archived"},
        {"preview", {{"kind", "none"}}},
        {"package", {{"includeInRuntime", true}, {"requiredForRelease", true}}},
        {"diagnostics", nlohmann::json::array()},
    });
    library.ingestPromotionManifest(archivedDuplicate);

    const auto rows = urpg::assets::buildAssetActionRows(library.snapshot());
    REQUIRE(rows.size() == 4);

    const auto promoted = std::find_if(rows.begin(), rows.end(),
                                       [](const auto& row) { return row["path"] == "imports/raw/example/hero.png"; });
    REQUIRE(promoted != rows.end());
    REQUIRE((*promoted)["recommended_action"] == "attach_to_project");
    REQUIRE((*promoted)["attach_button"]["enabled"] == true);
    REQUIRE((*promoted)["attach_button"]["disabled_reason"].is_null());
    REQUIRE((*promoted)["project_attached"] == false);
    REQUIRE((*promoted)["promotion_status"] == "runtime_ready");
    REQUIRE((*promoted)["promoted_path"] == "resources/assets/characters/hero.png");
    REQUIRE((*promoted)["license_id"] == "BND-001");
    REQUIRE((*promoted)["include_in_runtime"] == true);
    REQUIRE((*promoted)["required_for_release"] == false);
    REQUIRE((*promoted)["promotion_diagnostics"].empty());

    const auto unlicensed = std::find_if(
        rows.begin(), rows.end(), [](const auto& row) { return row["path"] == "imports/raw/example/unlicensed.png"; });
    REQUIRE(unlicensed != rows.end());
    REQUIRE((*unlicensed)["recommended_action"] == "add_license_evidence");
    REQUIRE((*unlicensed)["include_in_runtime"] == false);
    REQUIRE((*unlicensed)["attach_button"]["enabled"] == false);
    REQUIRE((*unlicensed)["attach_button"]["disabled_reason"] == "asset_not_promoted");
    REQUIRE((*unlicensed)["promotion_diagnostics"][0] == "license_evidence_missing");

    const auto missing = std::find_if(rows.begin(), rows.end(),
                                      [](const auto& row) { return row["path"] == "imports/raw/example/missing.png"; });
    REQUIRE(missing != rows.end());
    REQUIRE((*missing)["recommended_action"] == "fix_missing_file");
    REQUIRE((*missing)["include_in_runtime"] == false);
    REQUIRE((*missing)["attach_button"]["disabled_reason"] == "asset_not_promoted");

    const auto archived = std::find_if(
        rows.begin(), rows.end(), [](const auto& row) { return row["path"] == "imports/raw/example/hero-copy.png"; });
    REQUIRE(archived != rows.end());
    REQUIRE((*archived)["recommended_action"] == "archived");
    REQUIRE((*archived)["include_in_runtime"] == false);
    REQUIRE((*archived)["attach_button"]["disabled_reason"] == "asset_not_promoted");
    REQUIRE((*archived)["required_for_release"] == false);
    REQUIRE((*archived)["promotion_diagnostics"][0] == "archived_asset_packaged");

    urpg::assets::AssetLibraryFilter attachableFilter;
    attachableFilter.attachable_only = true;
    const auto attachable = library.filterAssets(attachableFilter);
    REQUIRE(attachable.size() == 1);
    REQUIRE(attachable.front().path == "imports/raw/example/hero.png");

    library.addUsageReference("imports/raw/example/hero.png", "project_asset_attachment:asset.hero");
    urpg::assets::AssetLibraryFilter attachedFilter;
    attachedFilter.project_attached_only = true;
    const auto attached = library.filterAssets(attachedFilter);
    REQUIRE(attached.size() == 1);
    REQUIRE(attached.front().path == "imports/raw/example/hero.png");
    REQUIRE(library.filterAssets(attachableFilter).empty());
}

TEST_CASE("AssetLibraryModel executes a requested import and loads its review manifest", "[assets][asset_library][import]") {
    const auto root = uniqueAssetTempRoot("urpg_asset_library_execute_import");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "catalog" / "import_sessions");
    const auto source = root / "hero.png";
    std::ofstream(source) << "fixture";

    urpg::editor::AssetLibraryModel model;
    const auto requested = model.requestImportSource(source, root, "review-hero", "private-project-only");
    REQUIRE(requested["success"] == true);
    REQUIRE(model.snapshot().import_wizard["pending_request"].is_object());

    const auto result = model.executePendingImportRequest([&](const auto& command) {
        REQUIRE(command.arguments.size() >= 2);
        urpg::assets::AssetImportSession session;
        session.sessionId = "review-hero";
        session.sourceKind = urpg::assets::AssetImportSourceKind::File;
        session.sourcePath = source.generic_string();
        session.managedSourceRoot = root.generic_string();
        session.status = urpg::assets::AssetImportStatus::ReviewReady;
        session.records.push_back({"asset.hero", "hero.png", "normalized/hero.png", "png", "image", "characters",
                                   "fixture", "abc", 7, 16, 16, 0, false, "", false, false, true, false, {}});
        std::ofstream(root / "catalog" / "import_sessions" / "review-hero.json")
            << urpg::assets::serializeAssetImportSession(session).dump(2);
        return urpg::editor::AssetLibraryModel::ConversionCommandResult{0, "imported", ""};
    });

    REQUIRE(result["success"] == true);
    REQUIRE(result["code"] == "import_source_loaded_for_review");
    REQUIRE(model.snapshot().import_session_count == 1);
    REQUIRE(model.snapshot().import_review_row_count == 1);
    REQUIRE(model.snapshot().import_wizard["pending_request"].is_null());

    std::filesystem::remove_all(root);
}
