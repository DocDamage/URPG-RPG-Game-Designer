#include "engine/core/export/export_validator.h"
#include "engine/core/security/resource_protector.h"
#include "engine/core/tools/export_packager.h"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <vector>

using namespace urpg::exporting;
using urpg::tools::ExportTarget;

namespace {

constexpr char kBundleMagic[] = "URPGPCK1";

std::string BundleSignatureKeyForTarget(ExportTarget target) {
    switch (target) {
    case ExportTarget::Windows_x64:
        return "urpg-export-signature-win-v1";
    case ExportTarget::Linux_x64:
        return "urpg-export-signature-linux-v1";
    case ExportTarget::macOS_Universal:
        return "urpg-export-signature-macos-v1";
    case ExportTarget::Web_WASM:
        return "urpg-export-signature-web-v1";
    default:
        return "urpg-export-signature-v1";
    }
}

std::string BundleObfuscationKeyForTarget(ExportTarget target) {
    switch (target) {
    case ExportTarget::Windows_x64:
        return "urpg-export-bundle-win";
    case ExportTarget::Linux_x64:
        return "urpg-export-bundle-linux";
    case ExportTarget::macOS_Universal:
        return "urpg-export-bundle-macos";
    case ExportTarget::Web_WASM:
        return "urpg-export-bundle-web";
    default:
        return "urpg-export-bundle";
    }
}

nlohmann::json BuildBundleSignatureView(const nlohmann::json& manifest) {
    auto signatureView = manifest;
    signatureView.erase("payloadOffset");
    signatureView.erase("bundleSignature");
    return signatureView;
}

std::string BundleIntegrityScope(const nlohmann::json& entry) {
    return entry.value("path", "") + "|" + entry.value("kind", "") + "|" +
           (entry.value("compressed", false) ? "1" : "0") + "|" + (entry.value("obfuscated", false) ? "1" : "0") + "|" +
           std::to_string(entry.value("rawSize", 0u));
}

void CreateRealExportFixture(const std::filesystem::path& base, ExportTarget target) {
    std::filesystem::remove_all(base);
    urpg::tools::ExportPackager packager;
    urpg::tools::ExportConfig config{};
    config.target = target;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);
    REQUIRE(result.success);
}

std::vector<std::uint8_t> ReadFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

} // namespace

TEST_CASE("ExportValidator: Valid Windows export directory passes", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_win";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Missing required file produces error", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_missing";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);
    std::filesystem::remove(base / "game.exe");

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE_FALSE(errors.empty());
    REQUIRE(errors[0].find("*.exe") != std::string::npos);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Missing optional file does not produce error", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_optional";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: macOS check detects .app directory", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_macos";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);
    CreateRealExportFixture(base, ExportTarget::macOS_Universal);

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::macOS_Universal);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Web check requires all file types", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_web";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Web_WASM);

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Web_WASM);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Report JSON contains errors array and target name", "[export][validation]") {
    ExportValidator validator;
    std::vector<std::string> errors = {"Missing required file: index.html"};
    auto report = validator.buildReportJson(errors, ExportTarget::Web_WASM);

    REQUIRE(report["target"] == "Web_WASM");
    REQUIRE(report["passed"] == false);
    REQUIRE(report["errors"].is_array());
    REQUIRE(report["errors"].size() == 1);
    REQUIRE(report["errors"][0] == "Missing required file: index.html");
}

TEST_CASE("ExportValidator: directory report JSON surfaces bundle discovery summary", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_report_summary";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    ExportValidator validator;
    const auto report = validator.buildReportJson(base.string(), ExportTarget::Windows_x64);

    REQUIRE(report["target"] == "Windows_x64");
    REQUIRE(report["passed"] == true);
    REQUIRE(report.contains("bundleSummary"));
    REQUIRE(report["bundleSummary"]["format"] == "URPG_BOUNDED_EXPORT_BUNDLE_V1");
    REQUIRE(report["bundleSummary"]["signatureMode"] == "sha256_keyed_bundle_v1");
    REQUIRE(report["bundleSummary"]["assetDiscoveryManifest"]["format"] == "URPG_PROJECT_ASSET_DISCOVERY_V1");

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Linux check detects executable without extension", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_linux";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Linux_x64);

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Linux_x64);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: supported target fixtures satisfy all required artifacts", "[export][validation]") {
    const std::vector<ExportTarget> targets = {
        ExportTarget::Windows_x64,
        ExportTarget::Linux_x64,
        ExportTarget::macOS_Universal,
        ExportTarget::Web_WASM,
    };

    ExportValidator validator;

    for (const auto target : targets) {
        const auto base = std::filesystem::temp_directory_path() /
                          ("urpg_export_validator_target_" + std::to_string(static_cast<int>(target)));
        CreateRealExportFixture(base, target);

        const auto errors = validator.validateExportDirectory(base.string(), target);
        REQUIRE(errors.empty());

        std::filesystem::remove_all(base);
    }
}

TEST_CASE("check_platform_exports.ps1 -Json emits matching validator result shape for valid export",
          "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_ps1_json_valid";
    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    const std::filesystem::path scriptPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "tools" / "ci" / "check_platform_exports.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_export_ps1_json_valid_out.json";

    const std::string command = "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
                                "\" -ExportDir \"" + base.string() + "\" -Target Windows_x64 -Json > \"" +
                                outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.good());
    nlohmann::json result;
    resultFile >> result;
    resultFile.close();

    REQUIRE(result["target"] == "Windows_x64");
    REQUIRE(result["passed"] == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());

    std::filesystem::remove(outputPath);
    std::filesystem::remove_all(base);
}

TEST_CASE("check_platform_exports.ps1 -Json emits matching validator result shape for invalid export",
          "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_ps1_json_invalid";
    CreateRealExportFixture(base, ExportTarget::Windows_x64);
    std::filesystem::remove(base / "game.exe");

    const std::filesystem::path scriptPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "tools" / "ci" / "check_platform_exports.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_export_ps1_json_invalid_out.json";

    const std::string command = "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
                                "\" -ExportDir \"" + base.string() + "\" -Target Windows_x64 -Json > \"" +
                                outputPath.string() + "\" 2>nul";

    const int exitCode = std::system(command.c_str());
    // The script exits with 1 on failure when -Json is present, but std::system
    // on Windows returns the raw exit code. We accept non-zero.
    (void)exitCode;

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.good());
    nlohmann::json result;
    resultFile >> result;
    resultFile.close();

    REQUIRE(result["target"] == "Windows_x64");
    REQUIRE(result["passed"] == false);
    REQUIRE(result["errors"].is_array());
    REQUIRE_FALSE(result["errors"].empty());

    std::filesystem::remove(outputPath);
    std::filesystem::remove_all(base);
}

TEST_CASE("check_platform_exports.ps1 emits a JSON platform export matrix", "[export][validation]") {
#ifdef URPG_PACK_CLI_PATH
    const auto exportRoot = std::filesystem::temp_directory_path() / "urpg_export_ps1_matrix";
    std::filesystem::remove_all(exportRoot);

    const std::filesystem::path scriptPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "tools" / "ci" / "check_platform_exports.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_export_ps1_matrix_out.json";
    const auto packCliPath = std::filesystem::path(URPG_PACK_CLI_PATH);
    const auto buildDirectory = packCliPath.parent_path();

    const std::string command = "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
                                "\" -BuildDirectory \"" + buildDirectory.string() + "\" -ExportDir \"" +
                                exportRoot.string() + "\" -Json > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.good());
    nlohmann::json result;
    resultFile >> result;
    resultFile.close();

    REQUIRE(result["tool"] == "check_platform_exports");
    REQUIRE(result["phase"] == "platform_export_matrix");
    REQUIRE(result["success"] == true);
    REQUIRE(result["results"].is_array());
    REQUIRE(result["results"].size() == 4);

    for (const auto& targetResult : result["results"]) {
        REQUIRE(targetResult["target"].is_string());
        REQUIRE(targetResult["status"].is_string());
        REQUIRE(targetResult["status"] != "failed");
    }

    std::filesystem::remove(outputPath);
    std::filesystem::remove_all(exportRoot);
#else
    SUCCEED("URPG pack CLI path is not available in this build.");
#endif
}

TEST_CASE("package_release_artifacts.ps1 reports explicit unsigned development dry run", "[export][validation]") {
    const std::filesystem::path scriptPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "tools" / "ci" / "package_release_artifacts.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_release_package_dev_unsigned.json";

    const std::string command = "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
                                "\" -Mode DevUnsigned -DryRun -ReportPath \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.good());
    nlohmann::json result;
    resultFile >> result;
    resultFile.close();

    REQUIRE(result["tool"] == "package_release_artifacts");
    REQUIRE(result["mode"] == "DevUnsigned");
    REQUIRE(result["dryRun"] == true);
    REQUIRE(result["success"] == true);
    REQUIRE(result["unsignedArtifactsAllowed"] == true);
    REQUIRE(result["pluginDropinsManifest"].is_string());
    REQUIRE(result["pluginDropinsReleaseRoot"].is_string());
    REQUIRE(result["pluginDropinsManifestPresent"] == true);
    REQUIRE(result["artifacts"].is_array());
    REQUIRE(result["artifacts"].size() == 4);
    for (const auto& artifact : result["artifacts"]) {
        REQUIRE(artifact["signed"] == false);
        REQUIRE(artifact["unsignedReason"] == "explicit_dev_unsigned_dry_run");
    }

    std::filesystem::remove(outputPath);
}

TEST_CASE("package_release_artifacts.ps1 release mode fails without signing inputs", "[export][validation]") {
    const std::filesystem::path scriptPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "tools" / "ci" / "package_release_artifacts.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_release_package_missing_signing.json";
    const auto wrapperPath = std::filesystem::temp_directory_path() / "urpg_release_package_missing_signing.ps1";

    {
        std::ofstream wrapper(wrapperPath, std::ios::binary | std::ios::trunc);
        wrapper << "$env:URPG_WINDOWS_SIGN_CERT_PATH=''\n";
        wrapper << "$env:URPG_WINDOWS_SIGN_CERT_PASSWORD=''\n";
        wrapper << "$env:URPG_LINUX_SIGNING_KEY_PATH=''\n";
        wrapper << "$env:URPG_MACOS_DEVELOPER_ID_APPLICATION=''\n";
        wrapper << "$env:URPG_MACOS_NOTARY_PROFILE=''\n";
        wrapper << "& '" << scriptPath.string() << "' -Mode Release -DryRun -ReportPath '" << outputPath.string()
                << "'\n";
        wrapper << "exit $LASTEXITCODE\n";
    }

    const std::string command = "powershell -ExecutionPolicy Bypass -File \"" + wrapperPath.string() + "\"";

    const int exitCode = std::system(command.c_str());
    REQUIRE(exitCode != 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.good());
    nlohmann::json result;
    resultFile >> result;
    resultFile.close();

    REQUIRE(result["tool"] == "package_release_artifacts");
    REQUIRE(result["mode"] == "Release");
    REQUIRE(result["dryRun"] == true);
    REQUIRE(result["success"] == false);
    REQUIRE(result["unsignedArtifactsAllowed"] == false);
    REQUIRE(result["missingSigningInputs"].is_array());
    REQUIRE_FALSE(result["missingSigningInputs"].empty());

    std::filesystem::remove(outputPath);
    std::filesystem::remove(wrapperPath);
}

TEST_CASE("ExportValidator: corrupted bundle integrity produces an error", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_corrupt_bundle";
    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    auto bytes = ReadFileBytes(base / "data.pck");
    REQUIRE(bytes.size() > 32);
    bytes.back() ^= 0x1;

    std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    out.close();

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE_FALSE(errors.empty());
    bool foundIntegrityMismatch = false;
    for (const auto& error : errors) {
        if (error.find("integrity mismatch") != std::string::npos) {
            foundIntegrityMismatch = true;
            break;
        }
    }
    REQUIRE(foundIntegrityMismatch);

    std::filesystem::remove_all(base);
}

// ─── S29-T05/T06: Signing/enforcement seam + edge-case tamper coverage ───────

TEST_CASE("ExportValidator: bundle with only an empty asset discovery manifest passes integrity check",
          "[export][validation][s29]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_val_empty_discovery_manifest";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    nlohmann::json discoveryManifest;
    discoveryManifest["format"] = "URPG_PROJECT_ASSET_DISCOVERY_V1";
    discoveryManifest["mode"] = "enabled";
    discoveryManifest["roots"] = nlohmann::json::array();
    discoveryManifest["assets"] = nlohmann::json::array();
    discoveryManifest["discoveredAssetCount"] = 0u;
    const std::string discoveryText = discoveryManifest.dump();
    const std::vector<std::uint8_t> discoveryBytes(discoveryText.begin(), discoveryText.end());

    urpg::security::ResourceProtector protector;
    nlohmann::json entry = {
        {"path", "export/asset_discovery_manifest.json"},
        {"kind", "asset_discovery_manifest"},
        {"compressed", false},
        {"obfuscated", false},
        {"offset", 0u},
        {"storedSize", discoveryBytes.size()},
        {"rawSize", discoveryBytes.size()},
    };
    entry["integrityTag"] = protector.computeIntegrityTag(BundleIntegrityScope(entry), discoveryBytes,
                                                          BundleObfuscationKeyForTarget(ExportTarget::Windows_x64));

    nlohmann::json manifest;
    manifest["entries"] = nlohmann::json::array({entry});
    manifest["payloadOffset"] = 0u;
    manifest["integrityMode"] = "fnv1a64_keyed";
    manifest["signatureMode"] = "sha256_keyed_bundle_v1";
    manifest["bundleSignature"] = std::string(64, '0');
    std::string manifestText = manifest.dump();
    for (int i = 0; i < 2; ++i) {
        manifest["payloadOffset"] = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t) + manifestText.size();
        manifestText = manifest.dump();
    }
    manifest["bundleSignature"] =
        protector.computeCryptographicSignature(BuildBundleSignatureView(manifest).dump(), discoveryBytes,
                                                BundleSignatureKeyForTarget(ExportTarget::Windows_x64));
    manifestText = manifest.dump();
    const auto manifestSize = static_cast<std::uint32_t>(manifestText.size());

    std::vector<std::uint8_t> bundle;
    for (char c : std::string(kBundleMagic, sizeof(kBundleMagic) - 1)) {
        bundle.push_back(static_cast<std::uint8_t>(c));
    }
    bundle.push_back(static_cast<std::uint8_t>(manifestSize & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 8) & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 16) & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 24) & 0xFF));
    for (char c : manifestText) {
        bundle.push_back(static_cast<std::uint8_t>(c));
    }
    bundle.insert(bundle.end(), discoveryBytes.begin(), discoveryBytes.end());

    {
        std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(bundle.data()), static_cast<std::streamsize>(bundle.size()));
    }

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: truncated bundle header reports structured error", "[export][validation][s29]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_val_truncated";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    // Truncate data.pck to only 4 bytes — less than the magic + manifest-size header
    {
        std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
        const char tiny[] = {0x55, 0x52, 0x50, 0x47};
        out.write(tiny, sizeof(tiny));
    }

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE_FALSE(errors.empty());
    bool foundHeaderError = false;
    for (const auto& e : errors) {
        if (e.find("truncated") != std::string::npos || e.find("header") != std::string::npos ||
            e.find("Invalid asset package") != std::string::npos) {
            foundHeaderError = true;
            break;
        }
    }
    REQUIRE(foundHeaderError);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: wrong magic bytes reports structured error", "[export][validation][s29]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_val_badmagic";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    auto bytes = ReadFileBytes(base / "data.pck");
    REQUIRE(bytes.size() >= 8);
    // Corrupt first byte of magic
    bytes[0] ^= 0xFF;

    {
        std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE_FALSE(errors.empty());
    bool foundMagicError = false;
    for (const auto& e : errors) {
        if (e.find("URPGPCK1") != std::string::npos || e.find("header") != std::string::npos ||
            e.find("Invalid asset package") != std::string::npos) {
            foundMagicError = true;
            break;
        }
    }
    REQUIRE(foundMagicError);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: manifest with missing integrityMode reports enforcement error",
          "[export][validation][s29]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_val_no_integrity_mode";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    // Re-write data.pck with a valid header but manifest lacking integrityMode
    constexpr char kMagic[] = "URPGPCK1";
    nlohmann::json manifest;
    manifest["entries"] = nlohmann::json::array();
    manifest["payloadOffset"] = 0u;
    // integrityMode deliberately omitted

    const std::string manifestText = manifest.dump();
    const auto manifestSize = static_cast<std::uint32_t>(manifestText.size());

    std::vector<std::uint8_t> bundle;
    for (char c : std::string(kMagic, sizeof(kMagic) - 1)) {
        bundle.push_back(static_cast<std::uint8_t>(c));
    }
    bundle.push_back(static_cast<std::uint8_t>(manifestSize & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 8) & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 16) & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 24) & 0xFF));
    for (char c : manifestText) {
        bundle.push_back(static_cast<std::uint8_t>(c));
    }

    {
        std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(bundle.data()), static_cast<std::streamsize>(bundle.size()));
    }

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE_FALSE(errors.empty());
    bool foundIntegrityModeError = false;
    for (const auto& e : errors) {
        if (e.find("integrity") != std::string::npos) {
            foundIntegrityModeError = true;
            break;
        }
    }
    REQUIRE(foundIntegrityModeError);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: manifest with missing bundle signature reports enforcement error",
          "[export][validation][s29]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_val_no_bundle_signature";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    auto bytes = ReadFileBytes(base / "data.pck");
    REQUIRE(bytes.size() > sizeof(kBundleMagic) - 1 + sizeof(std::uint32_t));

    const auto manifestSize =
        static_cast<std::uint32_t>(bytes[sizeof(kBundleMagic) - 1] | (bytes[sizeof(kBundleMagic)] << 8) |
                                   (bytes[sizeof(kBundleMagic) + 1] << 16) | (bytes[sizeof(kBundleMagic) + 2] << 24));
    const auto manifestOffset = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t);
    std::string manifestText(bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset),
                             bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset + manifestSize));
    auto manifest = nlohmann::json::parse(manifestText);
    manifest.erase("bundleSignature");
    manifest["payloadOffset"] = 0u;
    manifestText = manifest.dump();
    for (int i = 0; i < 2; ++i) {
        manifest["payloadOffset"] = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t) + manifestText.size();
        manifestText = manifest.dump();
    }
    std::vector<std::uint8_t> rewritten;
    rewritten.insert(rewritten.end(), bytes.begin(), bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset));
    rewritten[sizeof(kBundleMagic) - 1] = static_cast<std::uint8_t>(manifestText.size() & 0xFFu);
    rewritten[sizeof(kBundleMagic)] = static_cast<std::uint8_t>((manifestText.size() >> 8u) & 0xFFu);
    rewritten[sizeof(kBundleMagic) + 1] = static_cast<std::uint8_t>((manifestText.size() >> 16u) & 0xFFu);
    rewritten[sizeof(kBundleMagic) + 2] = static_cast<std::uint8_t>((manifestText.size() >> 24u) & 0xFFu);
    rewritten.insert(rewritten.end(), manifestText.begin(), manifestText.end());
    rewritten.insert(rewritten.end(), bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset + manifestSize),
                     bytes.end());

    {
        std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(rewritten.data()), static_cast<std::streamsize>(rewritten.size()));
    }

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE_FALSE(errors.empty());
    bool foundSignatureError = false;
    for (const auto& e : errors) {
        if (e.find("signature") != std::string::npos) {
            foundSignatureError = true;
            break;
        }
    }
    REQUIRE(foundSignatureError);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: path-not-found returns a structured error", "[export][validation][s29]") {
    ExportValidator validator;
    const auto errors =
        validator.validateExportDirectory("/nonexistent/urpg_export_does_not_exist_xyz", ExportTarget::Windows_x64);
    REQUIRE_FALSE(errors.empty());
    const bool hasStructuredPathError =
        errors[0].find("not a directory") != std::string::npos || errors[0].find("does not exist") != std::string::npos;
    REQUIRE(hasStructuredPathError);
}

TEST_CASE("ExportValidator: report JSON for all-passing export is stable across calls", "[export][validation][s29]") {
    ExportValidator validator;
    const std::vector<std::string> noErrors;
    const auto report1 = validator.buildReportJson(noErrors, ExportTarget::Windows_x64);
    const auto report2 = validator.buildReportJson(noErrors, ExportTarget::Windows_x64);
    REQUIRE(report1.dump() == report2.dump());
    REQUIRE(report1["passed"] == true);
    REQUIRE(report1["errors"].empty());
}
