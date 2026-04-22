#include <catch2/catch_test_macros.hpp>
#include "engine/core/security/resource_protector.h"
#include "engine/core/tools/export_packager.h"
#include "engine/core/export/export_validator.h"
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace urpg::tools;
using urpg::exporting::ExportValidator;

namespace {

constexpr char kBundleMagic[] = "URPGPCK1";

void WriteFile(const std::filesystem::path& path, const std::string& content = "") {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << content;
}

std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::string ReadFileText(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::uint32_t ReadUint32LE(const std::vector<uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::string BundleObfuscationKeyForTarget(ExportTarget target) {
    switch (target) {
        case ExportTarget::Windows_x64: return "urpg-export-bundle-win";
        case ExportTarget::Linux_x64: return "urpg-export-bundle-linux";
        case ExportTarget::macOS_Universal: return "urpg-export-bundle-macos";
        case ExportTarget::Web_WASM: return "urpg-export-bundle-web";
        default: return "urpg-export-bundle";
    }
}

std::string BundleIntegrityScope(const nlohmann::json& entry) {
    return entry.value("path", "") + "|" + entry.value("kind", "") + "|" +
           (entry.value("compressed", false) ? "1" : "0") + "|" +
           (entry.value("obfuscated", false) ? "1" : "0") + "|" +
           std::to_string(entry.value("rawSize", 0u));
}

std::string ComputeBundleIntegrityTag(const nlohmann::json& entry,
                                      const std::vector<uint8_t>& storedBytes,
                                      ExportTarget target) {
    urpg::security::ResourceProtector protector;
    return protector.computeIntegrityTag(
        BundleIntegrityScope(entry),
        storedBytes,
        BundleObfuscationKeyForTarget(target));
}

nlohmann::json ReadBundleManifest(const std::filesystem::path& path) {
    const auto bytes = ReadFileBytes(path);
    REQUIRE(bytes.size() > sizeof(kBundleMagic) - 1 + sizeof(std::uint32_t));
    REQUIRE(std::string(bytes.begin(), bytes.begin() + (sizeof(kBundleMagic) - 1)) == kBundleMagic);

    const auto manifestSize = ReadUint32LE(bytes, sizeof(kBundleMagic) - 1);
    const auto manifestOffset = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t);
    REQUIRE(bytes.size() >= manifestOffset + manifestSize);

    const std::string manifestText(bytes.begin() + manifestOffset, bytes.begin() + manifestOffset + manifestSize);
    return nlohmann::json::parse(manifestText);
}

std::vector<uint8_t> ReadBundleEntryBytes(const std::filesystem::path& path, const nlohmann::json& entry) {
    const auto bytes = ReadFileBytes(path);
    const auto manifest = ReadBundleManifest(path);
    const auto payloadOffset = manifest["payloadOffset"].get<std::size_t>();
    const auto offset = entry["offset"].get<std::size_t>();
    const auto storedSize = entry["storedSize"].get<std::size_t>();

    REQUIRE(bytes.size() >= payloadOffset + offset + storedSize);
    return std::vector<uint8_t>(
        bytes.begin() + static_cast<std::ptrdiff_t>(payloadOffset + offset),
        bytes.begin() + static_cast<std::ptrdiff_t>(payloadOffset + offset + storedSize));
}

std::vector<uint8_t> DecodeBundleEntryBytes(const std::filesystem::path& path,
                                            ExportTarget target,
                                            const nlohmann::json& entry) {
    auto payload = ReadBundleEntryBytes(path, entry);
    if (entry.value("obfuscated", false)) {
        urpg::security::ResourceProtector protector;
        protector.obfuscate(payload, BundleObfuscationKeyForTarget(target));
    }
    if (entry.value("compressed", false)) {
        payload = urpg::core::AssetCompressor::instance().decompress(payload);
    }
    return payload;
}

#ifdef URPG_EXPORT_SMOKE_APP_PATH
std::filesystem::path GetExportSmokeAppPath() {
    return std::filesystem::path(URPG_EXPORT_SMOKE_APP_PATH);
}
#endif

#ifdef _WIN32
bool LaunchProcessAndCaptureOutput(const std::filesystem::path& executable,
                                   const std::filesystem::path& markerPath,
                                   const std::filesystem::path& logPath) {
    SECURITY_ATTRIBUTES securityAttributes{};
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;

    HANDLE logHandle = CreateFileW(
        logPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &securityAttributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (logHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = logHandle;
    startupInfo.hStdError = logHandle;

    PROCESS_INFORMATION processInfo{};
    std::wstring commandLine =
        L"\"" + executable.wstring() + L"\" --write-marker \"" + markerPath.wstring() + L"\"";

    const BOOL launched = CreateProcessW(
        nullptr,
        commandLine.data(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo);

    CloseHandle(logHandle);

    if (!launched) {
        return false;
    }

    WaitForSingleObject(processInfo.hProcess, 30000);

    DWORD exitCode = 1;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    return exitCode == 0;
}
#endif

} // namespace

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
    REQUIRE_FALSE(result.generatedFiles.empty());
    REQUIRE(result.generatedFiles[0] == "data.pck");
    REQUIRE(std::filesystem::exists(base / "data.pck"));

    const auto manifest = ReadBundleManifest(base / "data.pck");
    REQUIRE(manifest["format"] == "URPG_BOUNDED_EXPORT_BUNDLE_V1");
    REQUIRE(manifest["bundleMode"] == "bounded_export_smoke");
    REQUIRE(manifest["target"] == "Windows (x64)");
    REQUIRE(manifest["protectionMode"] == "rle_xor");
    REQUIRE(manifest["integrityMode"] == "fnv1a64_keyed");
    REQUIRE(manifest["entries"].is_array());
    REQUIRE(manifest["entries"].size() > 3);

    const std::vector<std::string> expectedPaths = {
        "export/export_metadata.json",
        "runtime/bootstrap_scene.json",
        "runtime/script_pack_policy.json",
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

    std::filesystem::remove_all(base);
}

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

    const auto readinessBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, readinessEntry);
    const auto readinessSchemaBytes =
        DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, readinessSchemaEntry);
    const auto starterDungeonBytes =
        DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, starterDungeonEntry);

    const std::string readinessText(readinessBytes.begin(), readinessBytes.end());
    const std::string readinessSchemaText(readinessSchemaBytes.begin(), readinessSchemaBytes.end());
    const std::string starterDungeonText(starterDungeonBytes.begin(), starterDungeonBytes.end());

    REQUIRE(readinessEntry["kind"] == "readiness");
    REQUIRE(readinessSchemaEntry["kind"] == "schema");
    REQUIRE(starterDungeonEntry["kind"] == "level_library");
    REQUIRE(readinessText.find("\"schemaVersion\": \"1.0.0\"") != std::string::npos);
    REQUIRE(readinessSchemaText.find("\"title\"") != std::string::npos);
    REQUIRE(starterDungeonText.find("\"libraryName\": \"Starter Dungeon Kit\"") != std::string::npos);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager stages promoted asset bundles from governed manifests", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_promoted_asset_bundles";
    const auto manifestRoot = base / "asset_bundles";
    const auto normalizedRoot = base / "normalized";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(manifestRoot);
    std::filesystem::create_directories(normalizedRoot);

    WriteFile(
        manifestRoot / "BND-900.json",
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
    WriteFile(
        manifestRoot / "BND-901.json",
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
    WriteFile(
        normalizedRoot / "prototype_sprites" / "hero_placeholder.svg",
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\"><rect width=\"16\" height=\"16\" fill=\"#3a7\"/></svg>");
    WriteFile(
        normalizedRoot / "prototype_sprites" / "planned.svg",
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\"><circle cx=\"8\" cy=\"8\" r=\"6\" fill=\"#a73\"/></svg>");

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
    const auto bundleManifestBytes =
        DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, bundleManifestEntry);
    const auto promotedAssetBytes =
        DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, promotedAssetEntry);

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

    const auto& firstEntry = manifest["entries"][0];
    const auto storedBytes = ReadBundleEntryBytes(bundlePath, firstEntry);
    const auto decodedBytes = DecodeBundleEntryBytes(bundlePath, ExportTarget::Windows_x64, firstEntry);
    const std::string decodedText(decodedBytes.begin(), decodedBytes.end());

    REQUIRE(firstEntry["compressed"] == true);
    REQUIRE(firstEntry["obfuscated"] == true);
    REQUIRE(firstEntry["integrityTag"] == ComputeBundleIntegrityTag(firstEntry, storedBytes, ExportTarget::Windows_x64));
    REQUIRE_FALSE(storedBytes.empty());
    REQUIRE(decodedText.find("\"format\": \"URPG_BOUNDED_EXPORT_BUNDLE_V1\"") != std::string::npos);
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

    const auto html = ReadFileText(base / "index.html");
    const auto js = ReadFileText(base / "game.js");
    const auto wasm = ReadFileBytes(base / "game.wasm");

    REQUIRE(html.find("<script type=\"module\" src=\"./game.js\"></script>") != std::string::npos);
    REQUIRE(js.find("fetch(bundlePath)") != std::string::npos);
    REQUIRE(js.find("WebAssembly.instantiate") != std::string::npos);
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

    const auto script = ReadFileText(base / "game");
    REQUIRE(script.find("#!/bin/sh") == 0);
    REQUIRE(script.find("data.pck") != std::string::npos);
    REQUIRE(script.find("URPG bounded Linux export bootstrap ready") != std::string::npos);

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

    const auto plist = ReadFileText(base / "MyGame.app" / "Contents" / "Info.plist");
    const auto script = ReadFileText(base / "MyGame.app" / "Contents" / "MacOS" / "MyGame");

    REQUIRE(plist.find("<key>CFBundleExecutable</key>") != std::string::npos);
    REQUIRE(plist.find("<string>MyGame</string>") != std::string::npos);
    REQUIRE(script.find("#!/bin/sh") == 0);
    REQUIRE(script.find("data.pck") != std::string::npos);
    REQUIRE(script.find("URPG bounded macOS export bootstrap ready") != std::string::npos);

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

TEST_CASE("ResourceProtector::compress performs reversible lightweight compression",
          "[export][security]") {
    urpg::security::ResourceProtector protector;
    const std::vector<uint8_t> rawData = {
        0x10, 0x10, 0x10, 0x10,
        0x20, 0x20, 0x20,
        0xFF, 0xFF,
        0x7A
    };

    REQUIRE(urpg::security::ResourceProtector::compressionImplemented());

    const auto compressed = protector.compress(rawData);
    REQUIRE_FALSE(compressed.empty());
    REQUIRE(compressed != rawData);
    REQUIRE(compressed.size() < rawData.size());
    REQUIRE(urpg::core::AssetCompressor::instance().decompress(compressed) == rawData);
}

TEST_CASE("ResourceProtector obfuscation remains reversible XOR over compressed bytes",
          "[export][security]") {
    urpg::security::ResourceProtector protector;
    const std::vector<uint8_t> rawData = {0x12, 0x12, 0x12, 0x34, 0x34, 0x56, 0x56, 0x56, 0x56};
    auto protectedData = protector.compress(rawData);

    protector.obfuscate(protectedData, "urpg");
    REQUIRE(protectedData != rawData);

    protector.obfuscate(protectedData, "urpg");
    REQUIRE(urpg::core::AssetCompressor::instance().decompress(protectedData) == rawData);
}
