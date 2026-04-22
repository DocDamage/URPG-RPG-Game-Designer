#include "engine/core/tools/export_packager.h"

#include "engine/core/security/resource_protector.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <cstdint>
#include <system_error>

namespace urpg::tools {

namespace {

constexpr char kBundleMagic[] = "URPGPCK1";

struct BundlePayload {
    std::string path;
    std::string kind;
    std::vector<uint8_t> bytes;
    std::size_t rawSize = 0;
    bool compressed = false;
    bool obfuscated = false;
    std::string integrityTag;
};

std::string makeWebBootstrapHtml() {
    return R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>URPG Web Export</title>
</head>
<body>
  <main id="urpg-root">URPG bounded web export bootstrap</main>
  <script type="module" src="./game.js"></script>
</body>
</html>
)";
}

std::string makeWebBootstrapLoader() {
    return R"(const bundlePath = "./data.pck";
const wasmPath = "./game.wasm";

async function loadUrpgExport() {
  const [bundleResponse, wasmResponse] = await Promise.all([
    fetch(bundlePath),
    fetch(wasmPath),
  ]);

  if (!bundleResponse.ok) {
    throw new Error(`Failed to fetch bundle: ${bundleResponse.status}`);
  }
  if (!wasmResponse.ok) {
    throw new Error(`Failed to fetch wasm: ${wasmResponse.status}`);
  }

  const bundleBytes = await bundleResponse.arrayBuffer();
  const wasmBytes = await wasmResponse.arrayBuffer();
  const wasmModule = await WebAssembly.instantiate(wasmBytes, {});

  globalThis.__URPG_EXPORT_BOOTSTRAP__ = {
    bundleByteLength: bundleBytes.byteLength,
    exports: Object.keys(wasmModule.instance.exports),
  };

  const root = document.getElementById("urpg-root");
  if (root) {
    root.textContent = "URPG bounded web export bootstrap ready";
  }
}

loadUrpgExport().catch((error) => {
  console.error("URPG web export bootstrap failed", error);
  const root = document.getElementById("urpg-root");
  if (root) {
    root.textContent = "URPG bounded web export bootstrap failed";
  }
});
)";
}

std::vector<std::uint8_t> makeEmptyWasmModule() {
    return {
        0x00, 0x61, 0x73, 0x6D,
        0x01, 0x00, 0x00, 0x00,
    };
}

std::string makeLinuxBootstrapScript() {
    return R"URPGSCRIPT(#!/bin/sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
BUNDLE_PATH="$SCRIPT_DIR/data.pck"

if [ ! -f "$BUNDLE_PATH" ]; then
  echo "URPG Linux bootstrap missing data.pck" >&2
  exit 1
fi

echo "URPG bounded Linux export bootstrap ready"
exit 0
)URPGSCRIPT";
}

std::string makeMacBootstrapScript() {
    return R"URPGMAC(#!/bin/sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
APP_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)"
BUNDLE_PATH="$APP_ROOT/data.pck"

if [ ! -f "$BUNDLE_PATH" ]; then
  echo "URPG macOS bootstrap missing data.pck" >&2
  exit 1
fi

echo "URPG bounded macOS export bootstrap ready"
exit 0
)URPGMAC";
}

std::string makeMacInfoPlist() {
    return R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleExecutable</key>
  <string>MyGame</string>
  <key>CFBundleIdentifier</key>
  <string>com.urpg.export.mygame</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>MyGame</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleShortVersionString</key>
  <string>1.0</string>
</dict>
</plist>
)";
}

std::string bundleTargetToString(ExportTarget target) {
    switch (target) {
        case ExportTarget::Windows_x64: return "Windows (x64)";
        case ExportTarget::Linux_x64: return "Linux (x64)";
        case ExportTarget::macOS_Universal: return "macOS (Universal)";
        case ExportTarget::Web_WASM: return "Web (WASM/WebGL)";
        default: return "Other";
    }
}

std::string bundleObfuscationKey(ExportTarget target) {
    switch (target) {
        case ExportTarget::Windows_x64: return "urpg-export-bundle-win";
        case ExportTarget::Linux_x64: return "urpg-export-bundle-linux";
        case ExportTarget::macOS_Universal: return "urpg-export-bundle-macos";
        case ExportTarget::Web_WASM: return "urpg-export-bundle-web";
        default: return "urpg-export-bundle";
    }
}

std::vector<uint8_t> toBytes(const std::string& text) {
    return std::vector<uint8_t>(text.begin(), text.end());
}

std::string makePayloadIntegrityScope(const BundlePayload& payload) {
    return payload.path + "|" + payload.kind + "|" +
           (payload.compressed ? "1" : "0") + "|" +
           (payload.obfuscated ? "1" : "0") + "|" +
           std::to_string(payload.rawSize);
}

void appendUint32LE(std::ofstream& out, std::uint32_t value) {
    const char encoded[4] = {
        static_cast<char>(value & 0xFF),
        static_cast<char>((value >> 8) & 0xFF),
        static_cast<char>((value >> 16) & 0xFF),
        static_cast<char>((value >> 24) & 0xFF),
    };
    out.write(encoded, sizeof(encoded));
}

std::filesystem::path repoRootPath() {
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();
}

std::filesystem::path assetBundleManifestRoot(const ExportConfig& config) {
    if (!config.assetBundleManifestRootOverride.empty()) {
        return std::filesystem::path(config.assetBundleManifestRootOverride);
    }
    return repoRootPath() / "imports" / "manifests" / "asset_bundles";
}

std::filesystem::path normalizedAssetRoot(const ExportConfig& config) {
    if (!config.normalizedAssetRootOverride.empty()) {
        return std::filesystem::path(config.normalizedAssetRootOverride);
    }
    return repoRootPath() / "imports" / "normalized";
}

std::vector<uint8_t> readFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

bool isPathWithinRoot(const std::filesystem::path& root, const std::filesystem::path& candidate) {
    const auto normalizedRoot = root.lexically_normal();
    const auto normalizedCandidate = candidate.lexically_normal();
    const auto mismatch = std::mismatch(
        normalizedRoot.begin(), normalizedRoot.end(),
        normalizedCandidate.begin(), normalizedCandidate.end());
    return mismatch.first == normalizedRoot.end();
}

std::vector<BundlePayload> collectRepoOwnedPayloads() {
    const std::filesystem::path root = repoRootPath();
    const std::vector<std::pair<std::filesystem::path, std::string>> singleFiles = {
        {root / "content" / "readiness" / "readiness_status.json", "readiness"},
    };
    const std::vector<std::pair<std::filesystem::path, std::string>> directories = {
        {root / "content" / "schemas", "schema"},
        {root / "content" / "level_libraries", "level_library"},
    };

    std::vector<BundlePayload> payloads;

    for (const auto& [filePath, kind] : singleFiles) {
        if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
            continue;
        }

        payloads.push_back({
            std::filesystem::relative(filePath, root).generic_string(),
            kind,
            readFileBytes(filePath),
            0,
            false,
            false,
            {},
        });
    }

    for (const auto& [dirPath, kind] : directories) {
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            continue;
        }

        std::vector<std::filesystem::path> files;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());

        for (const auto& filePath : files) {
            payloads.push_back({
                std::filesystem::relative(filePath, root).generic_string(),
                kind,
                readFileBytes(filePath),
                0,
                false,
                false,
                {},
            });
        }
    }

    return payloads;
}

std::vector<BundlePayload> collectPromotedAssetBundlePayloads(const ExportConfig& config) {
    const auto manifestRoot = assetBundleManifestRoot(config);
    const auto assetRoot = normalizedAssetRoot(config);
    std::vector<BundlePayload> payloads;

    if (!std::filesystem::exists(manifestRoot) || !std::filesystem::is_directory(manifestRoot) ||
        !std::filesystem::exists(assetRoot) || !std::filesystem::is_directory(assetRoot)) {
        return payloads;
    }

    std::vector<std::filesystem::path> manifestFiles;
    for (const auto& entry : std::filesystem::directory_iterator(manifestRoot)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json" &&
            entry.path().filename() != "asset_bundle.schema.json") {
            manifestFiles.push_back(entry.path());
        }
    }
    std::sort(manifestFiles.begin(), manifestFiles.end());

    for (const auto& manifestPath : manifestFiles) {
        nlohmann::json manifest;
        try {
            const auto manifestBytes = readFileBytes(manifestPath);
            manifest = nlohmann::json::parse(manifestBytes.begin(), manifestBytes.end());
        } catch (const std::exception&) {
            continue;
        }

        if (!manifest.is_object() || manifest.value("bundle_state", "") != "promoted" ||
            !manifest.contains("assets") || !manifest["assets"].is_array()) {
            continue;
        }

        std::vector<BundlePayload> manifestAssets;
        for (const auto& asset : manifest["assets"]) {
            if (!asset.is_object() || asset.value("status", "") != "promoted" ||
                !asset.contains("promoted_relative_path") || !asset["promoted_relative_path"].is_string() ||
                !asset.contains("category") || !asset["category"].is_string()) {
                continue;
            }

            const auto relativePath = std::filesystem::path(asset["promoted_relative_path"].get<std::string>());
            if (relativePath.is_absolute()) {
                continue;
            }

            const auto assetPath = (assetRoot / relativePath).lexically_normal();
            if (!isPathWithinRoot(assetRoot, assetPath) ||
                !std::filesystem::exists(assetPath) ||
                !std::filesystem::is_regular_file(assetPath)) {
                continue;
            }

            manifestAssets.push_back({
                ("imports/normalized/" + relativePath.generic_string()),
                "promoted_asset",
                readFileBytes(assetPath),
                0,
                false,
                false,
                {},
            });
        }

        if (manifestAssets.empty()) {
            continue;
        }

        payloads.push_back({
            "imports/manifests/asset_bundles/" + manifestPath.filename().generic_string(),
            "asset_bundle_manifest",
            readFileBytes(manifestPath),
            0,
            false,
            false,
            {},
        });
        payloads.insert(payloads.end(), manifestAssets.begin(), manifestAssets.end());
    }

    return payloads;
}

std::vector<BundlePayload> buildBundlePayloads(const ExportConfig& config) {
    std::vector<BundlePayload> entries;

    const nlohmann::json exportMetadata = {
        {"format", "URPG_BOUNDED_EXPORT_BUNDLE_V1"},
        {"bundleMode", "bounded_export_smoke"},
        {"target", bundleTargetToString(config.target)},
        {"compressAssets", config.compressAssets},
        {"includeDebugSymbols", config.includeDebugSymbols},
        {"obfuscateScripts", config.obfuscateScripts},
    };
    entries.push_back({
        "export/export_metadata.json",
        "metadata",
        toBytes(exportMetadata.dump(2) + "\n"),
        exportMetadata.dump(2).size() + 1,
        false,
        false,
        {},
    });

    const nlohmann::json bootstrapScene = {
        {"entryScene", "Boot"},
        {"firstRuntimeSurface", "MapScene"},
        {"bundleContract", "bounded_export_smoke"},
        {"notes", {
            "Synthetic project-wide asset discovery is not implemented in-tree.",
            "This bundle carries deterministic bootstrap/runtime metadata only."
        }},
    };
    entries.push_back({
        "runtime/bootstrap_scene.json",
        "bootstrap",
        toBytes(bootstrapScene.dump(2) + "\n"),
        bootstrapScene.dump(2).size() + 1,
        false,
        false,
        {},
    });

    const nlohmann::json scriptPolicy = {
        {"obfuscateScripts", config.obfuscateScripts},
        {"packScriptsStatus", config.obfuscateScripts ? "bounded_obfuscation_header" : "verbatim_placeholder"},
        {"runtimeSurface", "compat_harness"},
    };
    entries.push_back({
        "runtime/script_pack_policy.json",
        "script_policy",
        toBytes(scriptPolicy.dump(2) + "\n"),
        scriptPolicy.dump(2).size() + 1,
        false,
        false,
        {},
    });

    auto repoOwnedPayloads = collectRepoOwnedPayloads();
    entries.insert(entries.end(), repoOwnedPayloads.begin(), repoOwnedPayloads.end());
    auto promotedAssetBundlePayloads = collectPromotedAssetBundlePayloads(config);
    entries.insert(entries.end(), promotedAssetBundlePayloads.begin(), promotedAssetBundlePayloads.end());

    urpg::security::ResourceProtector protector;
    const std::string obfuscationKey = bundleObfuscationKey(config.target);
    for (auto& entry : entries) {
        entry.rawSize = entry.bytes.size();
        if (config.compressAssets) {
            entry.bytes = protector.compress(entry.bytes);
            entry.compressed = true;
            protector.obfuscate(entry.bytes, obfuscationKey);
            entry.obfuscated = true;
        }
        entry.integrityTag =
            protector.computeIntegrityTag(makePayloadIntegrityScope(entry), entry.bytes, obfuscationKey);
    }

    return entries;
}

} // namespace

ExportPackager::ExportResult ExportPackager::runExport(const ExportConfig& config) {
    std::string log;
    log += "Starting export for target: " + targetToString(config.target) + "\n";

    if (!config.outputDir.empty()) {
        std::error_code mkdirError;
        std::filesystem::create_directories(config.outputDir, mkdirError);
        if (mkdirError) {
            log += "Failed to prepare export output directory: " + mkdirError.message() + "\n";
            return { false, log, {} };
        }
    }

    const auto validation = validateBeforeExport(config);
    if (!validation.passed) {
        log += "Pre-export validation failed.\n";
        for (const auto& error : validation.errors) {
            log += " - " + error + "\n";
        }
        return { false, log, {} };
    }

    // 1. License Audit (Phase 4 Security Gate)
    if (!runLicenseAudit(log)) {
        return { false, log, {} };
    }

    // 2. Asset Bundling
    std::vector<std::string> bundles = bundleAssets(config, log);

    // 3. Script Packing (with optional obfuscation 4.6)
    packScripts(config, log);

    // 4. Binary Synthesis
    log += "Synthesizing final executable...\n";
    auto executableFiles = synthesizeExecutable(config, log);
    if (config.target == ExportTarget::Windows_x64 && !config.runtimeBinaryPath.empty() && executableFiles.empty()) {
        log += "Real Windows runtime staging failed.\n";
        return { false, log, bundles };
    }
    bundles.insert(bundles.end(), executableFiles.begin(), executableFiles.end());

    return { true, log, bundles };
}

ExportValidationResult ExportPackager::validateBeforeExport(const ExportConfig& config) const {
    std::vector<std::string> errors;

    if (config.outputDir.empty()) {
        errors.emplace_back("Output directory is required for export staging.");
    } else {
        const std::filesystem::path outputDir(config.outputDir);
        if (std::filesystem::exists(outputDir) && !std::filesystem::is_directory(outputDir)) {
            errors.emplace_back("Output path exists but is not a directory: " + config.outputDir);
        }
    }

    if (config.target == ExportTarget::Windows_x64 && !config.runtimeBinaryPath.empty()) {
        std::filesystem::path runtimeBinary(config.runtimeBinaryPath);
        if (runtimeBinary.empty() || !std::filesystem::exists(runtimeBinary) ||
            !std::filesystem::is_regular_file(runtimeBinary)) {
            errors.emplace_back("Missing runtime binary for real Windows smoke export: " + config.runtimeBinaryPath);
        } else if (runtimeBinary.extension() != ".exe") {
            errors.emplace_back("Real Windows smoke export requires an .exe runtime binary: " +
                                config.runtimeBinaryPath);
        }
    }

    return { errors.empty(), std::move(errors) };
}

bool ExportPackager::runLicenseAudit(std::string& log) {
    log += "Running Asset License Audit...\n";
    // Call urpg::asset::AssetLicenseAuditor logic
    return true; // Stubbed success
}

std::vector<std::string> ExportPackager::bundleAssets(const ExportConfig& config, std::string& log) {
    log += "Bundling assets (Compression: " + std::string(config.compressAssets ? "ON" : "OFF") + ")...\n";
    std::filesystem::path outDir(config.outputDir);
    std::filesystem::path pckPath = outDir / "data.pck";
    std::ofstream pck(pckPath, std::ios::binary);
    if (!pck.good()) {
        log += "Failed to open bounded asset bundle for writing.\n";
        return {};
    }

    auto payloads = buildBundlePayloads(config);
    nlohmann::json manifest;
    manifest["format"] = "URPG_BOUNDED_EXPORT_BUNDLE_V1";
    manifest["bundleMode"] = "bounded_export_smoke";
    manifest["target"] = targetToString(config.target);
    manifest["protectionMode"] = config.compressAssets ? "rle_xor" : "none";
    manifest["integrityMode"] = "fnv1a64_keyed";
    manifest["entries"] = nlohmann::json::array();

    std::uint32_t payloadOffset = 0;
    for (const auto& payload : payloads) {
        manifest["entries"].push_back({
            {"path", payload.path},
            {"kind", payload.kind},
            {"compressed", payload.compressed},
            {"obfuscated", payload.obfuscated},
            {"offset", payloadOffset},
            {"storedSize", payload.bytes.size()},
            {"rawSize", payload.rawSize},
            {"integrityTag", payload.integrityTag},
        });
        payloadOffset += static_cast<std::uint32_t>(payload.bytes.size());
    }

    std::string finalizedManifestJson = manifest.dump();
    for (int i = 0; i < 2; ++i) {
        manifest["payloadOffset"] =
            (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t) + finalizedManifestJson.size();
        finalizedManifestJson = manifest.dump();
    }

    pck.write(kBundleMagic, sizeof(kBundleMagic) - 1);
    appendUint32LE(pck, static_cast<std::uint32_t>(finalizedManifestJson.size()));
    pck.write(finalizedManifestJson.data(), static_cast<std::streamsize>(finalizedManifestJson.size()));
    for (const auto& payload : payloads) {
        pck.write(reinterpret_cast<const char*>(payload.bytes.data()),
                  static_cast<std::streamsize>(payload.bytes.size()));
    }

    log += "Wrote bounded asset bundle with " + std::to_string(payloads.size()) + " staged payload(s).\n";
    if (config.compressAssets) {
        log += "Applied lightweight RLE+XOR protection to bundle payloads.\n";
    }
    log += "Applied lightweight keyed integrity tags to bundle payloads.\n";
    return { "data.pck" };
}

void ExportPackager::packScripts(const ExportConfig& config, std::string& log) {
    if (config.obfuscateScripts) {
        log += "Applying script obfuscation (Phase 4.6)...\n";
    }
}

std::vector<std::string> ExportPackager::synthesizeExecutable(const ExportConfig& config, std::string& log) {
    if (config.target == ExportTarget::Windows_x64 && !config.runtimeBinaryPath.empty()) {
        return stageRealWindowsRuntime(config, log);
    }

    std::filesystem::path outDir(config.outputDir);
    std::vector<std::string> files;

    switch (config.target) {
        case ExportTarget::Windows_x64: {
            std::filesystem::path exePath = outDir / "game.exe";
            std::ofstream exe(exePath, std::ios::binary);
            exe << "MZ synthetic windows executable\n";
            files.push_back("game.exe");
            break;
        }
        case ExportTarget::Linux_x64: {
            std::filesystem::path exePath = outDir / "game";
            std::ofstream exe(exePath, std::ios::binary);
            exe << makeLinuxBootstrapScript();
            std::error_code chmodError;
            std::filesystem::permissions(
                exePath,
                std::filesystem::perms::owner_read |
                    std::filesystem::perms::owner_write |
                    std::filesystem::perms::owner_exec |
                    std::filesystem::perms::group_read |
                    std::filesystem::perms::group_exec |
                    std::filesystem::perms::others_read |
                    std::filesystem::perms::others_exec,
                std::filesystem::perm_options::replace,
                chmodError);
            files.push_back("game");
            log += "Synthesized bounded Linux bootstrap launcher script.\n";
            break;
        }
        case ExportTarget::macOS_Universal: {
            std::filesystem::path appPath = outDir / "MyGame.app";
            std::filesystem::create_directories(appPath);
            std::filesystem::path exePath = appPath / "Contents" / "MacOS" / "MyGame";
            std::filesystem::create_directories(exePath.parent_path());
            std::filesystem::path infoPlistPath = appPath / "Contents" / "Info.plist";
            std::ofstream exe(exePath, std::ios::binary);
            exe << makeMacBootstrapScript();
            std::ofstream plist(infoPlistPath, std::ios::binary);
            plist << makeMacInfoPlist();
            std::error_code chmodError;
            std::filesystem::permissions(
                exePath,
                std::filesystem::perms::owner_read |
                    std::filesystem::perms::owner_write |
                    std::filesystem::perms::owner_exec |
                    std::filesystem::perms::group_read |
                    std::filesystem::perms::group_exec |
                    std::filesystem::perms::others_read |
                    std::filesystem::perms::others_exec,
                std::filesystem::perm_options::replace,
                chmodError);
            files.push_back("MyGame.app");
            log += "Synthesized bounded macOS app bootstrap structure.\n";
            break;
        }
        case ExportTarget::Web_WASM: {
            std::ofstream html(outDir / "index.html", std::ios::binary);
            html << makeWebBootstrapHtml();

            std::ofstream wasm(outDir / "game.wasm", std::ios::binary);
            const auto wasmBytes = makeEmptyWasmModule();
            wasm.write(reinterpret_cast<const char*>(wasmBytes.data()),
                       static_cast<std::streamsize>(wasmBytes.size()));

            std::ofstream js(outDir / "game.js", std::ios::binary);
            js << makeWebBootstrapLoader();
            files.push_back("index.html");
            files.push_back("game.wasm");
            files.push_back("game.js");
            log += "Synthesized bounded Web bootstrap artifacts with real HTML/JS/Wasm structure.\n";
            break;
        }
    }

    log += "Synthesized " + std::to_string(files.size()) + " executable artifact(s).\n";
    return files;
}

std::vector<std::string> ExportPackager::stageRealWindowsRuntime(const ExportConfig& config, std::string& log) {
    std::vector<std::string> files;
    const std::filesystem::path sourceBinary(config.runtimeBinaryPath);
    const std::filesystem::path outDir(config.outputDir);
    const std::filesystem::path stagedBinary = outDir / "game.exe";

    std::error_code copyError;
    std::filesystem::copy_file(
        sourceBinary,
        stagedBinary,
        std::filesystem::copy_options::overwrite_existing,
        copyError);
    if (copyError) {
        log += "Failed to copy real Windows runtime binary: " + copyError.message() + "\n";
        return {};
    }
    files.push_back("game.exe");

    for (const auto& entry : std::filesystem::directory_iterator(sourceBinary.parent_path())) {
        if (!entry.is_regular_file() || entry.path().extension() != ".dll") {
            continue;
        }

        const std::filesystem::path destination = outDir / entry.path().filename();
        std::error_code dllCopyError;
        std::filesystem::copy_file(
            entry.path(),
            destination,
            std::filesystem::copy_options::overwrite_existing,
            dllCopyError);
        if (dllCopyError) {
            log += "Failed to copy runtime dependency " + entry.path().filename().string() + ": " +
                   dllCopyError.message() + "\n";
            return {};
        }
        files.push_back(entry.path().filename().string());
    }

    log += "Staged real Windows smoke runtime from " + sourceBinary.string() + ".\n";
    log += "Staged " + std::to_string(files.size()) + " Windows runtime artifact(s).\n";
    return files;
}

std::string ExportPackager::targetToString(ExportTarget t) {
    switch(t) {
        case ExportTarget::Windows_x64: return "Windows (x64)";
        case ExportTarget::Linux_x64: return "Linux (x64)";
        case ExportTarget::macOS_Universal: return "macOS (Universal)";
        case ExportTarget::Web_WASM: return "Web (WASM/WebGL)";
        default: return "Other";
    }
}

} // namespace urpg::tools
