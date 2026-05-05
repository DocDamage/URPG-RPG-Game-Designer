#include "engine/core/tools/export_packager_payload_builder.h"

#include "engine/core/security/resource_protector.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>

namespace urpg::tools::export_packager_detail {

namespace {

constexpr char kAssetDiscoveryManifestPath[] = "export/asset_discovery_manifest.json";
constexpr char kAssetDiscoveryFormat[] = "URPG_PROJECT_ASSET_DISCOVERY_V1";
constexpr char kAssetLicenseManifestFilename[] = "asset_licenses.json";
constexpr char kProjectContentBundleMode[] = "project_content_bundle_v1";

struct AssetDiscoveryRoot {
    std::filesystem::path sourcePath;
    std::string configuredLabel;
    std::string bundlePrefix;
    bool configuredFromAbsolutePath = false;
};

std::string bundleTargetToString(ExportTarget target) {
    switch (target) {
    case ExportTarget::Windows_x64:
        return "Windows (x64)";
    case ExportTarget::Linux_x64:
        return "Linux (x64)";
    case ExportTarget::macOS_Universal:
        return "macOS (Universal)";
    case ExportTarget::Web_WASM:
        return "Web (WASM/WebGL)";
    default:
        return "Other";
    }
}

std::string bundleObfuscationKey(ExportTarget target) {
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

std::vector<std::uint8_t> toBytes(const std::string& text) {
    return std::vector<std::uint8_t>(text.begin(), text.end());
}

std::string sanitizeBundleSegment(std::string value) {
    for (auto& ch : value) {
        const unsigned char byte = static_cast<unsigned char>(ch);
        if (!std::isalnum(byte)) {
            ch = '_';
        }
    }

    value.erase(std::unique(value.begin(), value.end(), [](char lhs, char rhs) { return lhs == '_' && rhs == '_'; }),
                value.end());

    while (!value.empty() && value.front() == '_') {
        value.erase(value.begin());
    }
    while (!value.empty() && value.back() == '_') {
        value.pop_back();
    }

    return value.empty() ? "root" : value;
}

std::string makePayloadIntegrityScope(const BundlePayload& payload) {
    return payload.path + "|" + payload.kind + "|" + (payload.compressed ? "1" : "0") + "|" +
           (payload.obfuscated ? "1" : "0") + "|" + std::to_string(payload.rawSize);
}

bool isPathWithinRoot(const std::filesystem::path& root, const std::filesystem::path& candidate) {
    const auto normalizedRoot = root.lexically_normal();
    const auto normalizedCandidate = candidate.lexically_normal();
    const auto mismatch = std::mismatch(normalizedRoot.begin(), normalizedRoot.end(), normalizedCandidate.begin(),
                                        normalizedCandidate.end());
    return mismatch.first == normalizedRoot.end();
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

std::vector<AssetDiscoveryRoot> assetDiscoveryRoots(const ExportConfig& config, std::vector<std::string>& errors) {
    std::vector<AssetDiscoveryRoot> roots;
    if (!config.enableAutoAssetDiscovery) {
        return roots;
    }

    std::vector<std::string> configuredRoots = config.assetDiscoveryRoots;
    if (configuredRoots.empty()) {
        configuredRoots = {
            "assets",     "audio", "fonts", "images", "content/projects", "content/scenes", "content/localization",
            "content/ui", "content/assets/imported", "content/assets/manifests",
        };
    }

    const auto repoRoot = repoRootPath();
    for (std::size_t index = 0; index < configuredRoots.size(); ++index) {
        const std::filesystem::path configuredPath(configuredRoots[index]);
        const bool absolute = configuredPath.is_absolute();
        const auto resolvedPath = absolute ? configuredPath : (repoRoot / configuredPath).lexically_normal();
        if (!absolute && !isPathWithinRoot(repoRoot, resolvedPath)) {
            errors.push_back("Asset discovery root escapes repository: " + configuredRoots[index]);
            continue;
        }

        std::string configuredLabel;
        std::string bundlePrefix;
        if (absolute) {
            std::ostringstream label;
            label << "configured_absolute_root_" << std::setw(2) << std::setfill('0') << (index + 1u);
            configuredLabel = label.str();

            std::ostringstream prefix;
            prefix << "project_assets/root_" << std::setw(2) << std::setfill('0') << (index + 1u);
            bundlePrefix = prefix.str();
        } else {
            configuredLabel = configuredPath.generic_string();
            bundlePrefix = "project_assets/" + sanitizeBundleSegment(configuredLabel);
        }

        roots.push_back({
            resolvedPath,
            configuredLabel,
            bundlePrefix,
            absolute,
        });
    }

    return roots;
}

std::vector<std::uint8_t> readFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

bool isLicenseEvidenceFile(const std::filesystem::path& path) {
    return path.filename() == kAssetLicenseManifestFilename;
}

bool isManagedGlobalAssetLibraryPath(const std::filesystem::path& path) {
    auto normalized = path.lexically_normal().generic_string();
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    return normalized.find("/.urpg/asset-library/sources/") != std::string::npos ||
           normalized.find("/.urpg/asset-library/promoted/") != std::string::npos ||
           normalized.rfind(".urpg/asset-library/sources/", 0) == 0 ||
           normalized.rfind(".urpg/asset-library/promoted/", 0) == 0;
}

bool fileFitsBundleLimits(const std::filesystem::path& path, std::vector<std::string>& errors) {
    std::error_code sizeError;
    const auto size = std::filesystem::file_size(path, sizeError);
    if (sizeError) {
        errors.push_back("Unable to read asset size for export payload: " + path.string() + " (" + sizeError.message() +
                         ")");
        return false;
    }
    if (size > std::numeric_limits<std::uint32_t>::max()) {
        errors.push_back("Asset payload exceeds uint32 bundle size limit: " + path.string());
        return false;
    }
    return true;
}

std::vector<BundlePayload> collectRepoOwnedPayloads() {
    const std::filesystem::path root = repoRootPath();
    const std::vector<std::pair<std::filesystem::path, std::string>> directories = {
        {root / "content" / "fixtures", "project_fixture"},
        {root / "content" / "readiness", "readiness"},
        {root / "content" / "schemas", "schema"},
        {root / "content" / "level_libraries", "level_library"},
        {root / "content" / "ui", "ui_theme_asset"},
    };

    std::vector<BundlePayload> payloads;

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

nlohmann::json buildProjectEntryPayload() {
    return {
        {"format", "URPG_PROJECT_ENTRY_V1"},
        {"bundleMode", kProjectContentBundleMode},
        {"entryScene", "content/fixtures/map_worldbuilding_fixture.json"},
        {"projectConfig", "content/fixtures/project_governance_fixture.json"},
        {"exportFixture", "content/fixtures/export_packaging_fixture.json"},
        {"levelLibrary", "content/level_libraries/starter_dungeon.json"},
        {"assetDiscoveryManifest", kAssetDiscoveryManifestPath},
        {"runtimeSurface", "runtime_bundle_loader"},
    };
}

nlohmann::json buildScriptPolicyPayload() {
    return {
        {"format", "URPG_SCRIPT_EXPORT_POLICY_V1"},
        {"scriptExportMode", "verbatim"},
        {"supportedModes", nlohmann::json::array({"verbatim"})},
        {"unsupportedModes", nlohmann::json::array({"obfuscateScripts"})},
        {"failClosedForUnsupportedModes", true},
        {"runtimeSurface", "quickjs_compat_harness"},
        {"scriptRoots", nlohmann::json::array({
                            {
                                {"path", "runtimes/compat_js"},
                                {"kind", "compat_runtime"},
                                {"exportMode", "verbatim"},
                            },
                            {
                                {"path", "content/fixtures/mod_sdk_sample/scripts"},
                                {"kind", "project_mod_scripts"},
                                {"exportMode", "verbatim"},
                            },
                        })},
    };
}

std::vector<BundlePayload> collectPromotedAssetBundlePayloads(const ExportConfig& config,
                                                              std::vector<std::string>& errors) {
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

        if (!manifest.is_object() || manifest.value("bundle_state", "") != "promoted" || !manifest.contains("assets") ||
            !manifest["assets"].is_array()) {
            continue;
        }

        std::vector<BundlePayload> manifestAssets;
        for (const auto& asset : manifest["assets"]) {
            if (!asset.is_object() || asset.value("status", "") != "promoted" ||
                !asset.contains("promoted_relative_path") || !asset["promoted_relative_path"].is_string() ||
                !asset.contains("category") || !asset["category"].is_string()) {
                continue;
            }
            if (asset.value("distribution", "bundled") != "bundled") {
                continue;
            }

            const auto relativePath = std::filesystem::path(asset["promoted_relative_path"].get<std::string>());
            if (relativePath.is_absolute()) {
                continue;
            }

            const auto assetPath = (assetRoot / relativePath).lexically_normal();
            if (!isPathWithinRoot(assetRoot, assetPath) || !std::filesystem::exists(assetPath) ||
                !std::filesystem::is_regular_file(assetPath)) {
                continue;
            }
            if (!fileFitsBundleLimits(assetPath, errors)) {
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

nlohmann::json buildAssetDiscoveryManifest(const ExportConfig& config,
                                           const std::vector<AssetDiscoveryRoot>& discoveryRoots,
                                           const std::vector<BundlePayload>& payloads) {
    nlohmann::json manifest;
    manifest["format"] = kAssetDiscoveryFormat;
    manifest["mode"] = config.enableAutoAssetDiscovery ? "enabled" : "disabled";
    manifest["roots"] = nlohmann::json::array();
    manifest["assets"] = nlohmann::json::array();

    std::map<std::string, std::size_t> rootCounts;
    for (const auto& payload : payloads) {
        const auto separator = payload.path.find('/', std::string("project_assets/").size());
        const std::string rootPrefix =
            separator == std::string::npos ? payload.path : payload.path.substr(0, separator);
        rootCounts[rootPrefix] += 1u;

        manifest["assets"].push_back({
            {"path", payload.path},
            {"kind", payload.kind},
            {"rawSize", payload.rawSize},
            {"sourceRootPrefix", rootPrefix},
        });
    }

    for (const auto& root : discoveryRoots) {
        manifest["roots"].push_back({
            {"configuredLabel", root.configuredLabel},
            {"bundlePrefix", root.bundlePrefix},
            {"exists", std::filesystem::exists(root.sourcePath)},
            {"isDirectory", std::filesystem::is_directory(root.sourcePath)},
            {"configuredFromAbsolutePath", root.configuredFromAbsolutePath},
            {"assetCount", rootCounts[root.bundlePrefix]},
        });
    }

    manifest["discoveredAssetCount"] = payloads.size();
    return manifest;
}

std::vector<BundlePayload> collectAutoDiscoveredProjectAssetPayloads(const ExportConfig& config,
                                                                     nlohmann::json& discoveryManifest,
                                                                     std::vector<std::string>& errors) {
    const auto discoveryRoots = assetDiscoveryRoots(config, errors);
    std::vector<BundlePayload> payloads;
    std::set<std::string> seenBundlePaths;

    for (const auto& root : discoveryRoots) {
        std::vector<std::filesystem::path> files;
        if (std::filesystem::exists(root.sourcePath) && std::filesystem::is_directory(root.sourcePath)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(root.sourcePath)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path());
                }
            }
        } else if (std::filesystem::exists(root.sourcePath) && std::filesystem::is_regular_file(root.sourcePath)) {
            files.push_back(root.sourcePath);
        }

        std::sort(files.begin(), files.end());
        for (const auto& filePath : files) {
            if (isLicenseEvidenceFile(filePath)) {
                continue;
            }
            if (isManagedGlobalAssetLibraryPath(filePath)) {
                continue;
            }

            std::filesystem::path relativePath;
            if (std::filesystem::is_directory(root.sourcePath)) {
                relativePath = std::filesystem::relative(filePath, root.sourcePath);
            } else {
                relativePath = filePath.filename();
            }

            const auto bundlePath = root.bundlePrefix + "/" + relativePath.generic_string();
            if (!seenBundlePaths.insert(bundlePath).second) {
                continue;
            }
            if (!fileFitsBundleLimits(filePath, errors)) {
                continue;
            }

            payloads.push_back({
                bundlePath,
                "auto_discovered_asset",
                readFileBytes(filePath),
                0,
                false,
                false,
                {},
            });
        }
    }

    for (auto& payload : payloads) {
        payload.rawSize = payload.bytes.size();
    }
    discoveryManifest = buildAssetDiscoveryManifest(config, discoveryRoots, payloads);
    return payloads;
}

} // namespace

BundleBuildResult buildBundlePayloads(const ExportConfig& config) {
    BundleBuildResult result;
    auto& entries = result.payloads;

    if (config.obfuscateScripts) {
        result.errors.push_back(
            "Unsupported script export mode: obfuscateScripts=true requires a real script transform pipeline.");
        return result;
    }

    const nlohmann::json exportMetadata = {
        {"format", "URPG_PROJECT_EXPORT_METADATA_V1"},           {"bundleMode", kProjectContentBundleMode},
        {"target", bundleTargetToString(config.target)},         {"compressAssets", config.compressAssets},
        {"includeDebugSymbols", config.includeDebugSymbols},     {"obfuscateScripts", config.obfuscateScripts},
        {"projectEntry", "runtime/project_entry.json"},          {"scriptPolicy", "runtime/script_pack_policy.json"},
        {"assetDiscoveryManifest", kAssetDiscoveryManifestPath},
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

    const nlohmann::json projectEntry = buildProjectEntryPayload();
    entries.push_back({
        "runtime/project_entry.json",
        "project_entry",
        toBytes(projectEntry.dump(2) + "\n"),
        projectEntry.dump(2).size() + 1,
        false,
        false,
        {},
    });

    const nlohmann::json scriptPolicy = buildScriptPolicyPayload();
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
    auto promotedAssetBundlePayloads = collectPromotedAssetBundlePayloads(config, result.errors);
    entries.insert(entries.end(), promotedAssetBundlePayloads.begin(), promotedAssetBundlePayloads.end());
    nlohmann::json assetDiscoveryManifest;
    auto autoDiscoveredPayloads =
        collectAutoDiscoveredProjectAssetPayloads(config, assetDiscoveryManifest, result.errors);
    entries.insert(entries.end(), autoDiscoveredPayloads.begin(), autoDiscoveredPayloads.end());
    const auto assetDiscoveryText = assetDiscoveryManifest.dump(2) + "\n";
    entries.push_back({
        kAssetDiscoveryManifestPath,
        "asset_discovery_manifest",
        toBytes(assetDiscoveryText),
        assetDiscoveryText.size(),
        false,
        false,
        {},
    });

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

    for (const auto& entry : entries) {
        if (!fitsUint32(entry.bytes.size())) {
            result.errors.push_back("Bundle payload exceeds uint32 stored-size limit: " + entry.path);
        }
    }

    return result;
}

} // namespace urpg::tools::export_packager_detail
