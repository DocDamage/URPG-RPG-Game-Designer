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

TEST_CASE("ExportPackager synthesizes bounded real Web bootstrap artifacts", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_web_bootstrap";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Web_WASM;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);
    REQUIRE(std::filesystem::exists(base / "index.html"));
    REQUIRE(std::filesystem::exists(base / "game.js"));
    REQUIRE(std::filesystem::exists(base / "game.wasm"));
    REQUIRE(std::filesystem::exists(base / kDevBootstrapMetadataPath));

    const auto html = ReadFileText(base / "index.html");
    const auto js = ReadFileText(base / "game.js");
    const auto wasm = ReadFileBytes(base / "game.wasm");
    const auto metadata = nlohmann::json::parse(ReadFileText(base / kDevBootstrapMetadataPath));

    REQUIRE(html.find("<script type=\"module\" src=\"./game.js\"></script>") != std::string::npos);
    REQUIRE(js.find("fetch(bundlePath)") != std::string::npos);
    REQUIRE(js.find("WebAssembly.instantiate") != std::string::npos);
    REQUIRE(metadata["mode"] == "DevBootstrap");
    REQUIRE(metadata["target"] == "Web_WASM");
    REQUIRE(metadata["releaseEligible"] == false);
    REQUIRE(wasm.size() == 8);
    REQUIRE(wasm[0] == 0x00);
    REQUIRE(wasm[1] == 0x61);
    REQUIRE(wasm[2] == 0x73);
    REQUIRE(wasm[3] == 0x6D);
    REQUIRE(wasm[4] == 0x01);
    REQUIRE(wasm[5] == 0x00);
    REQUIRE(wasm[6] == 0x00);
    REQUIRE(wasm[7] == 0x00);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager synthesizes bounded Linux bootstrap launcher", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_linux_bootstrap";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Linux_x64;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);
    REQUIRE(std::filesystem::exists(base / "game"));
    REQUIRE(std::filesystem::exists(base / "data.pck"));
    REQUIRE(std::filesystem::exists(base / kDevBootstrapMetadataPath));

    const auto script = ReadFileText(base / "game");
    const auto metadata = nlohmann::json::parse(ReadFileText(base / kDevBootstrapMetadataPath));
    REQUIRE(script.find("#!/bin/sh") == 0);
    REQUIRE(script.find("data.pck") != std::string::npos);
    REQUIRE(script.find("URPG bounded Linux export bootstrap ready") != std::string::npos);
    REQUIRE(metadata["mode"] == "DevBootstrap");
    REQUIRE(metadata["target"] == "Linux_x64");
    REQUIRE(metadata["releaseEligible"] == false);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager synthesizes bounded macOS app bootstrap", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_macos_bootstrap";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::macOS_Universal;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);
    REQUIRE(std::filesystem::exists(base / "MyGame.app"));
    REQUIRE(std::filesystem::exists(base / "data.pck"));
    REQUIRE(std::filesystem::exists(base / "MyGame.app" / "Contents" / "Info.plist"));
    REQUIRE(std::filesystem::exists(base / "MyGame.app" / "Contents" / "MacOS" / "MyGame"));
    REQUIRE(std::filesystem::exists(base / kDevBootstrapMetadataPath));

    const auto plist = ReadFileText(base / "MyGame.app" / "Contents" / "Info.plist");
    const auto script = ReadFileText(base / "MyGame.app" / "Contents" / "MacOS" / "MyGame");
    const auto metadata = nlohmann::json::parse(ReadFileText(base / kDevBootstrapMetadataPath));

    REQUIRE(plist.find("<key>CFBundleExecutable</key>") != std::string::npos);
    REQUIRE(plist.find("<string>MyGame</string>") != std::string::npos);
    REQUIRE(script.find("#!/bin/sh") == 0);
    REQUIRE(script.find("data.pck") != std::string::npos);
    REQUIRE(script.find("URPG bounded macOS export bootstrap ready") != std::string::npos);
    REQUIRE(metadata["mode"] == "DevBootstrap");
    REQUIRE(metadata["target"] == "macOS_Universal");
    REQUIRE(metadata["releaseEligible"] == false);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager::validateBeforeExport stays independent from post-export artifact validation",
          "[export][packager]") {
    const std::vector<ExportTarget> targets = {
        ExportTarget::Windows_x64,
        ExportTarget::Linux_x64,
        ExportTarget::macOS_Universal,
        ExportTarget::Web_WASM,
    };

    ExportPackager packager;
    ExportValidator validator;

    for (const auto target : targets) {
        const auto base = std::filesystem::temp_directory_path() /
                          ("urpg_export_packager_target_" + std::to_string(static_cast<int>(target)));
        std::filesystem::remove_all(base);
        std::filesystem::create_directories(base);

        ExportConfig config{};
        config.target = target;
        config.outputDir = base.string();

        const auto validation = packager.validateBeforeExport(config);
        const auto validatorErrors = validator.validateExportDirectory(base.string(), target);

        REQUIRE(validation.passed);
        REQUIRE(validation.errors.empty());
        REQUIRE_FALSE(validatorErrors.empty());

        std::filesystem::remove_all(base);
    }
}

TEST_CASE("ExportPackager emits a valid export tree that passes post-export validation", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_emit_valid";
    std::filesystem::remove_all(base);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.compressAssets = true;

    auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);
    REQUIRE_FALSE(result.generatedFiles.empty());

    // Verify emitted files exist on disk
    REQUIRE(std::filesystem::exists(base / "data.pck"));
    REQUIRE(std::filesystem::exists(base / "game.exe"));

    // Post-export validation must pass
    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager emits valid export trees for all supported targets", "[export][packager]") {
    const std::vector<ExportTarget> targets = {
        ExportTarget::Windows_x64,
        ExportTarget::Linux_x64,
        ExportTarget::macOS_Universal,
        ExportTarget::Web_WASM,
    };

    ExportPackager packager;
    ExportValidator validator;

    for (const auto target : targets) {
        const auto base = std::filesystem::temp_directory_path() /
                          ("urpg_export_packager_emit_target_" + std::to_string(static_cast<int>(target)));
        std::filesystem::remove_all(base);

        ExportConfig config{};
        config.target = target;
        config.outputDir = base.string();

        auto result = packager.runExport(config);
        INFO(result.log);
        REQUIRE(result.success);

        auto errors = validator.validateExportDirectory(base.string(), target);
        REQUIRE(errors.empty());

        std::filesystem::remove_all(base);
    }
}

TEST_CASE("ExportPackager::runExport fails when pre-export validation fails", "[export][packager]") {
    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = "";

    const auto result = packager.runExport(config);

    REQUIRE_FALSE(result.success);
    REQUIRE(result.generatedFiles.empty());
    REQUIRE(result.log.find("Pre-export validation failed.") != std::string::npos);
    REQUIRE(result.log.find("Output directory is required") != std::string::npos);
}

TEST_CASE("urpg_pack_cli runs preflight, export, and post-export validation as JSON", "[export][packager][cli]") {
#ifdef URPG_PACK_CLI_PATH
    const auto outputDir =
        std::filesystem::temp_directory_path() /
        ("urpg_pack_cli_success_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::remove_all(outputDir);

    const auto result = RunPackCli({
        "--json",
        "--target",
        "Web_WASM",
        "--output",
        outputDir.string(),
    });

    INFO(result.stderrText);
    INFO(result.stdoutText);
    REQUIRE(result.exitCode == 0);
    REQUIRE_FALSE(result.stdoutText.empty());

    const auto report = nlohmann::json::parse(result.stdoutText);
    REQUIRE(report["tool"] == "urpg_pack_cli");
    REQUIRE(report["phase"] == "export");
    REQUIRE(report["target"] == "Web_WASM");
    REQUIRE(report["success"] == true);
    REQUIRE(report["preflight"]["passed"] == true);
    REQUIRE(report["preflight"]["errors"].empty());
    REQUIRE(report["export"]["success"] == true);
    REQUIRE(report["postExportValidation"]["passed"] == true);
    REQUIRE(report["postExportValidation"]["errors"].empty());
    REQUIRE(report["postExportValidation"]["bundleSummary"]["signatureMode"] == "sha256_keyed_bundle_v1");
    REQUIRE(report["postExportValidation"]["bundleSummary"]["bundleSignaturePresent"] == true);

    REQUIRE(std::filesystem::exists(outputDir / "data.pck"));
    REQUIRE(std::filesystem::exists(outputDir / "index.html"));
    REQUIRE(std::filesystem::exists(outputDir / "game.js"));
    REQUIRE(std::filesystem::exists(outputDir / "game.wasm"));

    const auto manifest = ReadBundleManifest(outputDir / "data.pck");
    REQUIRE(manifest["signatureMode"] == "sha256_keyed_bundle_v1");
    REQUIRE(manifest["bundleSignature"] ==
            ComputeBundleSignature(outputDir / "data.pck", manifest, ExportTarget::Web_WASM));

    std::filesystem::remove_all(outputDir);
#else
    SUCCEED("URPG pack CLI path is not available in this build.");
#endif
}

TEST_CASE("urpg_pack_cli reports preflight failures without exporting", "[export][packager][cli]") {
#ifdef URPG_PACK_CLI_PATH
    const auto outputFile = std::filesystem::temp_directory_path() /
                            ("urpg_pack_cli_failure_" +
                             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".txt");
    std::filesystem::remove(outputFile);
    WriteFile(outputFile, "not_a_directory");

    const auto result = RunPackCli({
        "--json",
        "--target",
        "Windows_x64",
        "--output",
        outputFile.string(),
    });

    INFO(result.stderrText);
    INFO(result.stdoutText);
    REQUIRE(result.exitCode != 0);
    REQUIRE_FALSE(result.stdoutText.empty());

    const auto report = nlohmann::json::parse(result.stdoutText);
    REQUIRE(report["tool"] == "urpg_pack_cli");
    REQUIRE(report["success"] == false);
    REQUIRE(report["preflight"]["passed"] == false);
    REQUIRE_FALSE(report["preflight"]["errors"].empty());
    REQUIRE(report["preflight"]["errors"][0].get<std::string>().find("not a directory") != std::string::npos);
    REQUIRE(report["export"]["success"] == false);
    REQUIRE(std::filesystem::is_regular_file(outputFile));

    std::filesystem::remove(outputFile);
#else
    SUCCEED("URPG pack CLI path is not available in this build.");
#endif
}

TEST_CASE("urpg_pack_cli release mode rejects bootstrap-only native exports", "[export][packager][cli][release]") {
#ifdef URPG_PACK_CLI_PATH
    const auto outputDir =
        std::filesystem::temp_directory_path() /
        ("urpg_pack_cli_release_missing_runtime_" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::remove_all(outputDir);

    const auto result = RunPackCli({
        "--json",
        "--release",
        "--target",
        "Windows_x64",
        "--output",
        outputDir.string(),
    });

    INFO(result.stderrText);
    INFO(result.stdoutText);
    REQUIRE(result.exitCode != 0);
    REQUIRE_FALSE(result.stdoutText.empty());

    const auto report = nlohmann::json::parse(result.stdoutText);
    REQUIRE(report["tool"] == "urpg_pack_cli");
    REQUIRE(report["mode"] == "release");
    REQUIRE(report["success"] == false);
    REQUIRE(report["preflight"]["passed"] == false);
    REQUIRE(report["preflight"]["errors"][0].get<std::string>().find("Release native export requires runtimeBinaryPath") !=
            std::string::npos);
    REQUIRE_FALSE(std::filesystem::exists(outputDir / "game.exe"));

    std::filesystem::remove_all(outputDir);
#else
    SUCCEED("URPG pack CLI path is not available in this build.");
#endif
}

#ifdef _WIN32
TEST_CASE("ExportPackager stages a real Windows smoke export that launches successfully",
          "[export][packager][real_smoke]") {
#ifdef URPG_EXPORT_SMOKE_APP_PATH
    const auto outputDir = std::filesystem::temp_directory_path() /
                           ("urpg_export_packager_real_smoke_" +
                            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto markerPath = outputDir / "smoke_marker.txt";
    const auto logPath = outputDir / "smoke_stdout.txt";
    const auto smokeAppPath = GetExportSmokeAppPath();

    REQUIRE(std::filesystem::exists(smokeAppPath));

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = outputDir.string();
    config.runtimeBinaryPath = smokeAppPath.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);
    REQUIRE(std::filesystem::exists(outputDir / "game.exe"));
    REQUIRE(std::filesystem::exists(outputDir / "data.pck"));

    REQUIRE(LaunchProcessAndCaptureOutput(outputDir / "game.exe", markerPath, logPath));

    REQUIRE(std::filesystem::exists(markerPath));

    std::ifstream marker(markerPath);
    REQUIRE(marker.good());
    std::string markerContents;
    std::getline(marker, markerContents);
    REQUIRE(markerContents == "URPG_EXPORT_SMOKE_OK");

    std::error_code cleanupError;
    std::filesystem::remove_all(outputDir, cleanupError);
#else
    SUCCEED("URPG export smoke app path is not available in this build.");
#endif
}
#endif

TEST_CASE("ResourceProtector::compress performs reversible lightweight compression", "[export][security]") {
    urpg::security::ResourceProtector protector;
    const std::vector<uint8_t> rawData = {0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0xFF, 0xFF, 0x7A};

    REQUIRE(urpg::security::ResourceProtector::compressionImplemented());

    const auto compressed = protector.compress(rawData);
    REQUIRE_FALSE(compressed.empty());
    REQUIRE(compressed != rawData);
    REQUIRE(compressed.size() < rawData.size());
    REQUIRE(urpg::core::AssetCompressor::instance().decompress(compressed) == rawData);
}

TEST_CASE("ResourceProtector obfuscation remains reversible XOR over compressed bytes", "[export][security]") {
    urpg::security::ResourceProtector protector;
    const std::vector<uint8_t> rawData = {0x12, 0x12, 0x12, 0x34, 0x34, 0x56, 0x56, 0x56, 0x56};
    auto protectedData = protector.compress(rawData);

    protector.obfuscate(protectedData, "urpg");
    REQUIRE(protectedData != rawData);

    protector.obfuscate(protectedData, "urpg");
    REQUIRE(urpg::core::AssetCompressor::instance().decompress(protectedData) == rawData);
}
