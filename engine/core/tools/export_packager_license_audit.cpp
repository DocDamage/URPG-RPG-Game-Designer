#include "engine/core/tools/export_packager_license_audit.h"

#include "engine/core/asset/asset_license_audit.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>

namespace urpg::tools::export_packager_detail {

namespace {

constexpr char kAssetLicenseManifestFilename[] = "asset_licenses.json";
constexpr char kAssetLicenseManifestFormat[] = "URPG_ASSET_LICENSES_V1";

struct AssetDiscoveryRoot {
    std::filesystem::path sourcePath;
    std::string configuredLabel;
    std::string bundlePrefix;
    bool configuredFromAbsolutePath = false;
};

struct AssetLicenseManifestEntry {
    urpg::asset::AssetLicense license;
    bool present = false;
};

std::filesystem::path repoRootPath() {
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();
}

std::filesystem::path assetBundleManifestRoot(const ExportConfig& config) {
    if (!config.assetBundleManifestRootOverride.empty()) {
        return std::filesystem::path(config.assetBundleManifestRootOverride);
    }
    return repoRootPath() / "imports" / "manifests" / "asset_bundles";
}

std::filesystem::path assetSourceManifestRoot(const ExportConfig& config) {
    const auto bundleRoot = assetBundleManifestRoot(config);
    if (bundleRoot.filename() == "asset_bundles") {
        return bundleRoot.parent_path() / "asset_sources";
    }
    return bundleRoot.parent_path() / "asset_sources";
}

std::string sanitizeBundleSegment(std::string value) {
    for (auto& ch : value) {
        const unsigned char byte = static_cast<unsigned char>(ch);
        if (!std::isalnum(byte)) {
            ch = '_';
        }
    }

    value.erase(
        std::unique(value.begin(), value.end(),
                    [](char lhs, char rhs) { return lhs == '_' && rhs == '_'; }),
        value.end());

    while (!value.empty() && value.front() == '_') {
        value.erase(value.begin());
    }
    while (!value.empty() && value.back() == '_') {
        value.pop_back();
    }

    return value.empty() ? "root" : value;
}

std::vector<AssetDiscoveryRoot> assetDiscoveryRoots(const ExportConfig& config) {
    std::vector<AssetDiscoveryRoot> roots;
    if (!config.enableAutoAssetDiscovery) {
        return roots;
    }

    std::vector<std::string> configuredRoots = config.assetDiscoveryRoots;
    if (configuredRoots.empty()) {
        configuredRoots = {
            "assets",
            "audio",
            "fonts",
            "images",
            "content/projects",
            "content/scenes",
            "content/localization",
            "content/ui",
        };
    }

    const auto repoRoot = repoRootPath();
    for (std::size_t index = 0; index < configuredRoots.size(); ++index) {
        const std::filesystem::path configuredPath(configuredRoots[index]);
        const bool absolute = configuredPath.is_absolute();
        const auto resolvedPath = absolute ? configuredPath : (repoRoot / configuredPath).lexically_normal();

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

std::string toLowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool isLicenseEvidenceFile(const std::filesystem::path& path) {
    return path.filename() == kAssetLicenseManifestFilename;
}

std::optional<urpg::asset::LicenseType> parseLicenseType(const std::string& value) {
    const auto normalized = toLowerAscii(value);
    if (normalized == "publicdomain" || normalized == "public_domain" || normalized == "cc0") {
        return urpg::asset::LicenseType::PublicDomain;
    }
    if (normalized == "mit") {
        return urpg::asset::LicenseType::MIT;
    }
    if (normalized == "creativecommons_by" || normalized == "creative_commons_by" ||
        normalized == "cc-by" || normalized == "cc_by") {
        return urpg::asset::LicenseType::CreativeCommons_BY;
    }
    if (normalized == "creativecommons_by_sa" || normalized == "creative_commons_by_sa" ||
        normalized == "cc-by-sa" || normalized == "cc_by_sa") {
        return urpg::asset::LicenseType::CreativeCommons_BY_SA;
    }
    if (normalized == "proprietary") {
        return urpg::asset::LicenseType::Proprietary;
    }
    return std::nullopt;
}

std::optional<urpg::asset::LicenseType> licenseTypeFromLegalDisposition(const std::string& value) {
    const auto normalized = toLowerAscii(value);
    if (normalized.find("reference_only") != std::string::npos ||
        normalized.find("unknown") != std::string::npos ||
        normalized.find("no_license") != std::string::npos) {
        return std::nullopt;
    }
    if (normalized.find("cc0") != std::string::npos ||
        normalized.find("public_domain") != std::string::npos) {
        return urpg::asset::LicenseType::PublicDomain;
    }
    if (normalized.find("mit") != std::string::npos) {
        return urpg::asset::LicenseType::MIT;
    }
    if (normalized.find("cc_by_sa") != std::string::npos ||
        normalized.find("creative_commons_by_sa") != std::string::npos) {
        return urpg::asset::LicenseType::CreativeCommons_BY_SA;
    }
    if (normalized.find("cc_by") != std::string::npos ||
        normalized.find("creative_commons_by") != std::string::npos) {
        return urpg::asset::LicenseType::CreativeCommons_BY;
    }
    if (normalized.find("proprietary_distribution_rights_confirmed") != std::string::npos) {
        return urpg::asset::LicenseType::Proprietary;
    }
    return std::nullopt;
}

std::map<std::string, AssetLicenseManifestEntry> readAssetLicenseManifest(
    const std::filesystem::path& manifestPath,
    std::vector<std::string>& errors) {
    std::map<std::string, AssetLicenseManifestEntry> entries;
    nlohmann::json manifest;
    try {
        const auto manifestBytes = readFileBytes(manifestPath);
        manifest = nlohmann::json::parse(manifestBytes.begin(), manifestBytes.end());
    } catch (const std::exception& ex) {
        errors.push_back("Malformed asset license manifest: " + manifestPath.string() + " (" + ex.what() + ")");
        return entries;
    }

    if (!manifest.is_object() || manifest.value("format", "") != kAssetLicenseManifestFormat ||
        !manifest.contains("assets") || !manifest["assets"].is_array()) {
        errors.push_back("Malformed asset license manifest: " + manifestPath.string());
        return entries;
    }

    for (const auto& asset : manifest["assets"]) {
        if (!asset.is_object() || !asset.contains("path") || !asset["path"].is_string() ||
            !asset.contains("license") || !asset["license"].is_string()) {
            errors.push_back("Malformed asset license entry in: " + manifestPath.string());
            continue;
        }

        const auto licenseType = parseLicenseType(asset["license"].get<std::string>());
        if (!licenseType.has_value()) {
            errors.push_back("Disallowed or unknown asset license for " +
                             asset["path"].get<std::string>() + " in: " + manifestPath.string());
            continue;
        }

        urpg::asset::AssetLicense license;
        license.type = *licenseType;
        license.attribution = asset.value("attribution", "");
        license.sourceUrl = asset.value("sourceUrl", "");
        entries[std::filesystem::path(asset["path"].get<std::string>()).generic_string()] = {license, true};
    }

    return entries;
}

} // namespace

bool auditPromotedAssetBundleLicenses(const ExportConfig& config, std::vector<std::string>& errors) {
    const auto manifestRoot = assetBundleManifestRoot(config);
    const auto sourceRoot = assetSourceManifestRoot(config);
    if (!std::filesystem::exists(manifestRoot) || !std::filesystem::is_directory(manifestRoot)) {
        return true;
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
        } catch (const std::exception& ex) {
            errors.push_back("Malformed asset bundle manifest: " + manifestPath.string() + " (" + ex.what() + ")");
            continue;
        }

        if (!manifest.is_object() || manifest.value("bundle_state", "") != "promoted") {
            continue;
        }
        if (!manifest.contains("source_id") || !manifest["source_id"].is_string() ||
            !manifest.contains("assets") || !manifest["assets"].is_array()) {
            errors.push_back("Promoted asset bundle is missing source or asset governance: " +
                             manifestPath.string());
            continue;
        }

        bool hasPromotedAsset = false;
        for (const auto& asset : manifest["assets"]) {
            hasPromotedAsset = hasPromotedAsset || (asset.is_object() && asset.value("status", "") == "promoted");
        }
        if (!hasPromotedAsset) {
            continue;
        }

        const auto sourceId = manifest["source_id"].get<std::string>();
        const auto sourcePath = sourceRoot / (sourceId + ".json");
        if (!std::filesystem::exists(sourcePath) || !std::filesystem::is_regular_file(sourcePath)) {
            errors.push_back("Promoted asset bundle is missing source license evidence " +
                             sourcePath.string() + " for " + manifestPath.filename().string());
            continue;
        }

        nlohmann::json sourceManifest;
        try {
            const auto sourceBytes = readFileBytes(sourcePath);
            sourceManifest = nlohmann::json::parse(sourceBytes.begin(), sourceBytes.end());
        } catch (const std::exception& ex) {
            errors.push_back("Malformed asset source license evidence: " + sourcePath.string() +
                             " (" + ex.what() + ")");
            continue;
        }

        if (!sourceManifest.is_object() || sourceManifest.value("source_id", "") != sourceId ||
            !sourceManifest.contains("source_url") || !sourceManifest["source_url"].is_string() ||
            sourceManifest["source_url"].get<std::string>().empty() ||
            !sourceManifest.contains("legal_disposition") ||
            !sourceManifest["legal_disposition"].is_string()) {
            errors.push_back("Malformed asset source license evidence: " + sourcePath.string());
            continue;
        }

        const auto licenseType =
            licenseTypeFromLegalDisposition(sourceManifest["legal_disposition"].get<std::string>());
        if (!licenseType.has_value()) {
            errors.push_back("Disallowed asset source legal disposition in " + sourcePath.string());
            continue;
        }

        urpg::asset::AssetLicense license;
        license.type = *licenseType;
        license.sourceUrl = sourceManifest["source_url"].get<std::string>();
        license.attribution = sourceManifest.value("repo_name", sourceId);
        const auto audit = urpg::asset::AssetLicenseAuditor::auditAsset(sourceId, license);
        if (!audit.isExportSafe) {
            errors.push_back("Asset source failed license audit: " + sourcePath.string() +
                             " (" + audit.warningMessage + ")");
        }
    }

    return errors.empty();
}

bool auditAutoDiscoveredAssetLicenses(const ExportConfig& config, std::vector<std::string>& errors) {
    for (const auto& root : assetDiscoveryRoots(config)) {
        std::vector<std::filesystem::path> files;
        if (std::filesystem::exists(root.sourcePath) && std::filesystem::is_directory(root.sourcePath)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(root.sourcePath)) {
                if (entry.is_regular_file() && !isLicenseEvidenceFile(entry.path())) {
                    files.push_back(entry.path());
                }
            }
        } else if (std::filesystem::exists(root.sourcePath) && std::filesystem::is_regular_file(root.sourcePath) &&
                   !isLicenseEvidenceFile(root.sourcePath)) {
            files.push_back(root.sourcePath);
        }

        if (files.empty()) {
            continue;
        }

        const auto licenseManifestPath =
            std::filesystem::is_directory(root.sourcePath)
                ? root.sourcePath / kAssetLicenseManifestFilename
                : root.sourcePath.parent_path() / kAssetLicenseManifestFilename;
        if (!std::filesystem::exists(licenseManifestPath) ||
            !std::filesystem::is_regular_file(licenseManifestPath)) {
            errors.push_back("Missing asset license manifest for discovered export assets: " +
                             licenseManifestPath.string());
            continue;
        }

        auto manifestEntries = readAssetLicenseManifest(licenseManifestPath, errors);
        for (const auto& filePath : files) {
            const auto relativePath =
                std::filesystem::is_directory(root.sourcePath)
                    ? std::filesystem::relative(filePath, root.sourcePath)
                    : filePath.filename();
            const auto key = relativePath.generic_string();
            const auto found = manifestEntries.find(key);
            if (found == manifestEntries.end() || !found->second.present) {
                errors.push_back("Missing asset license evidence for discovered asset: " + filePath.string());
                continue;
            }

            const auto audit = urpg::asset::AssetLicenseAuditor::auditAsset(
                root.configuredLabel + "/" + key,
                found->second.license);
            if (!audit.isExportSafe) {
                errors.push_back("Discovered asset failed license audit: " + filePath.string() +
                                 " (" + audit.warningMessage + ")");
            }
        }
    }

    return errors.empty();
}

} // namespace urpg::tools::export_packager_detail
