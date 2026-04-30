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

TEST_CASE("ExportPackager::validateBeforeExport passes for a fresh output directory", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_valid";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();

    auto result = packager.validateBeforeExport(config);

    REQUIRE(result.passed);
    REQUIRE(result.errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager::validateBeforeExport fails when output directory is missing", "[export][packager]") {
    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = "";

    auto result = packager.validateBeforeExport(config);

    REQUIRE_FALSE(result.passed);
    REQUIRE_FALSE(result.errors.empty());
    REQUIRE(result.errors[0].find("Output directory") != std::string::npos);
}

TEST_CASE("ExportPackager::validateBeforeExport fails when output path is a file", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_output_file";
    std::filesystem::remove_all(base);
    WriteFile(base, "not_a_directory");

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();

    auto result = packager.validateBeforeExport(config);

    REQUIRE_FALSE(result.passed);
    REQUIRE_FALSE(result.errors.empty());
    REQUIRE(result.errors[0].find("not a directory") != std::string::npos);

    std::filesystem::remove(base);
}

TEST_CASE("ExportPackager::validateBeforeExport fails when real Windows smoke runtime is missing",
          "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_missing_runtime";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.runtimeBinaryPath = (base / "missing_runtime.exe").string();

    const auto result = packager.validateBeforeExport(config);

    REQUIRE_FALSE(result.passed);
    REQUIRE_FALSE(result.errors.empty());
    REQUIRE(result.errors[0].find("Missing runtime binary") != std::string::npos);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager::validateBeforeExport rejects release native exports without a real runtime",
          "[export][packager][release]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_release_missing_runtime";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.mode = ExportMode::Release;
    config.outputDir = base.string();

    const auto result = packager.validateBeforeExport(config);

    REQUIRE_FALSE(result.passed);
    REQUIRE_FALSE(result.errors.empty());
    REQUIRE(result.errors[0].find("Release native export requires runtimeBinaryPath") != std::string::npos);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager::validateBeforeExport rejects release Web bootstrap exports",
          "[export][packager][release]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_release_web_bootstrap";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Web_WASM;
    config.mode = ExportMode::Release;
    config.outputDir = base.string();

    const auto result = packager.validateBeforeExport(config);

    REQUIRE_FALSE(result.passed);
    REQUIRE_FALSE(result.errors.empty());
    REQUIRE(result.errors[0].find("Release Web export requires a real WebAssembly/runtime artifact") !=
            std::string::npos);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager::runExport result contains correct file list from a fresh directory", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_run_valid";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.compressAssets = true;

    auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);
    REQUIRE(result.log.find("Export mode: dev_bootstrap") != std::string::npos);
    REQUIRE(result.log.find("bootstrap-only and not a playable release package") != std::string::npos);
    REQUIRE_FALSE(result.generatedFiles.empty());
    REQUIRE(result.generatedFiles[0] == "data.pck");
    REQUIRE(std::filesystem::exists(base / "data.pck"));
    REQUIRE(std::filesystem::exists(base / kDevBootstrapMetadataPath));
    REQUIRE_FALSE(std::filesystem::exists(base / "data.pck.tmp"));

    const auto bootstrapMetadata = nlohmann::json::parse(ReadFileText(base / kDevBootstrapMetadataPath));
    REQUIRE(bootstrapMetadata["schema"] == "urpg.export.dev_bootstrap.v1");
    REQUIRE(bootstrapMetadata["mode"] == "DevBootstrap");
    REQUIRE(bootstrapMetadata["target"] == "Windows_x64");
    REQUIRE(bootstrapMetadata["productionPlayable"] == false);
    REQUIRE(bootstrapMetadata["releaseEligible"] == false);

    const auto manifest = ReadBundleManifest(base / "data.pck");
    REQUIRE(manifest["format"] == "URPG_BOUNDED_EXPORT_BUNDLE_V1");
    REQUIRE(manifest["bundleMode"] == "project_content_bundle_v1");
    REQUIRE(manifest["target"] == "Windows (x64)");
    REQUIRE(manifest["assetDiscoveryMode"] == "project_root_scan_v1");
    REQUIRE(manifest["protectionMode"] == "rle_xor");
    REQUIRE(manifest["integrityMode"] == "fnv1a64_keyed");
    REQUIRE(manifest["signatureMode"] == "sha256_keyed_bundle_v1");
    REQUIRE(manifest["bundleSignature"] ==
            ComputeBundleSignature(base / "data.pck", manifest, ExportTarget::Windows_x64));
    REQUIRE(manifest["entries"].is_array());
    REQUIRE(manifest["entries"].size() > 3);

    const std::vector<std::string> expectedPaths = {
        "export/export_metadata.json",
        kAssetDiscoveryManifestPath,
        "runtime/project_entry.json",
        "runtime/script_pack_policy.json",
        "content/fixtures/export_packaging_fixture.json",
        "content/fixtures/map_worldbuilding_fixture.json",
        "content/fixtures/project_governance_fixture.json",
        "content/readiness/readiness_status.json",
        "content/schemas/readiness_status.schema.json",
        "content/level_libraries/starter_dungeon.json",
    };
    for (const auto& expectedPath : expectedPaths) {
        bool found = false;
        for (const auto& entry : manifest["entries"]) {
            if (entry["path"] == expectedPath) {
                found = true;
                REQUIRE(entry["compressed"] == true);
                REQUIRE(entry["obfuscated"] == true);
                REQUIRE(entry.contains("integrityTag"));
                REQUIRE(entry["integrityTag"].is_string());
                REQUIRE_FALSE(entry["integrityTag"].get<std::string>().empty());
                break;
            }
        }
        REQUIRE(found);
    }

    for (const auto& entry : manifest["entries"]) {
        if (entry["path"] == "runtime/project_entry.json") {
            const auto bytes = DecodeBundleEntryBytes(base / "data.pck", ExportTarget::Windows_x64, entry);
            const auto payload = nlohmann::json::parse(bytes.begin(), bytes.end());
            REQUIRE(payload["bundleMode"] == "project_content_bundle_v1");
            REQUIRE(payload["entryScene"] == "content/fixtures/map_worldbuilding_fixture.json");
            REQUIRE(payload["projectConfig"] == "content/fixtures/project_governance_fixture.json");
        }
        if (entry["path"] == "runtime/script_pack_policy.json") {
            const auto bytes = DecodeBundleEntryBytes(base / "data.pck", ExportTarget::Windows_x64, entry);
            const std::string text(bytes.begin(), bytes.end());
            REQUIRE(text.find("placeholder") == std::string::npos);
            REQUIRE(text.find("bounded_obfuscation_header") == std::string::npos);

            const auto payload = nlohmann::json::parse(text);
            REQUIRE(payload["scriptExportMode"] == "verbatim");
            REQUIRE(payload["failClosedForUnsupportedModes"] == true);
        }
    }

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager::runExport fails release native export before bootstrap artifact synthesis",
          "[export][packager][release]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_release_run_missing_runtime";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.mode = ExportMode::Release;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.log.find("Pre-export validation failed.") != std::string::npos);
    REQUIRE(result.log.find("Release native export requires runtimeBinaryPath") != std::string::npos);
    REQUIRE_FALSE(std::filesystem::exists(base / "game.exe"));

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager fails closed for unsupported script export modes", "[export][packager][security]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_unsupported_scripts";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.obfuscateScripts = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.log.find("Unsupported script export mode") != std::string::npos);
    REQUIRE_FALSE(std::filesystem::exists(base / "data.pck"));

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager bundle writer preserves existing bundle when temp validation fails",
          "[export][packager][security]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_atomic_validation_failure";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);
    WriteFile(base / "data.pck", "previous-bundle");

    urpg::tools::export_packager_detail::BundlePayload payload;
    payload.path = "data/Actors.json";
    payload.kind = "database";
    payload.bytes = {1, 2, 3, 4};
    payload.rawSize = payload.bytes.size();
    payload.integrityTag = "not-a-valid-integrity-tag";

    auto result = urpg::tools::export_packager_detail::writeBundleFile(base, ExportTarget::Windows_x64, false,
                                                                       "test_fixture", {payload});

    INFO(result.log);
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.errors.empty());
    const auto joinedErrors = [&result]() {
        std::string joined;
        for (const auto& error : result.errors) {
            joined += error;
            joined += "\n";
        }
        return joined;
    }();
    REQUIRE(joinedErrors.find("bundle_publish.temp_validation_failed") != std::string::npos);
    REQUIRE(joinedErrors.find("bundle_publish.validation.entry_integrity_mismatch:data/Actors.json") !=
            std::string::npos);
    REQUIRE(ReadFileText(base / "data.pck") == "previous-bundle");
    REQUIRE_FALSE(std::filesystem::exists(base / "data.pck.tmp"));

    std::filesystem::remove_all(base);
}
