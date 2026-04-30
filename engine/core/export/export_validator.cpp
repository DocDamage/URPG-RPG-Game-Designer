#include "engine/core/export/export_validator.h"

#include "engine/core/export/export_bundle_contract.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <set>
#include <string_view>
#include <vector>

namespace {

constexpr char kBundleMagic[] = "URPGPCK1";
constexpr char kAssetDiscoveryManifestPath[] = "export/asset_discovery_manifest.json";
constexpr char kAssetDiscoveryFormat[] = "URPG_PROJECT_ASSET_DISCOVERY_V1";

std::string formatBundleContractError(const std::string& code) {
    if (code == "missing_bundle")
        return "Invalid asset package: missing bundle";
    if (code == "truncated_bundle_header")
        return "Invalid asset package: truncated bundle header";
    if (code == "invalid_bundle_magic")
        return "Invalid asset package: missing URPGPCK1 header";
    if (code == "invalid_bundle_manifest_size")
        return "Invalid asset package: invalid manifest size";
    if (code == "truncated_bundle_manifest")
        return "Invalid asset package: manifest extends past end of file";
    if (code == "invalid_bundle_manifest_json")
        return "Invalid asset package: manifest JSON parse failed";
    if (code == "missing_manifest_entries")
        return "Invalid asset package: missing manifest entries array";
    if (code == "missing_payload_offset")
        return "Invalid asset package: missing payload offset";
    if (code == "missing_integrity_mode")
        return "Invalid asset package: missing keyed integrity metadata";
    if (code == "unsupported_integrity_mode")
        return "Invalid asset package: unsupported keyed integrity metadata";
    if (code == "missing_signature_mode")
        return "Invalid asset package: missing keyed SHA-256 bundle signature metadata";
    if (code == "unsupported_signature_mode")
        return "Invalid asset package: unsupported keyed SHA-256 bundle signature metadata";
    if (code == "missing_bundle_signature")
        return "Invalid asset package: missing keyed SHA-256 bundle signature";
    if (code == "payload_offset_out_of_bounds")
        return "Invalid asset package: payload offset extends past end of file";
    if (code == "manifest_entry_not_object")
        return "Invalid asset package: manifest entry is not an object";
    if (code == "manifest_entry_missing_required_fields") {
        return "Invalid asset package: manifest entry is missing required fields";
    }
    if (code == "bundle_signature_mismatch")
        return "Asset package bundle signature mismatch";
    constexpr std::string_view integrityPrefix = "entry_integrity_mismatch:";
    if (code.rfind(std::string(integrityPrefix), 0) == 0) {
        return "Asset package integrity mismatch: " + code.substr(integrityPrefix.size());
    }
    constexpr std::string_view payloadPrefix = "payload_entry_extends_past_bundle:";
    if (code.rfind(std::string(payloadPrefix), 0) == 0) {
        return "Invalid asset package: payload bytes extend past end of file for " + code.substr(payloadPrefix.size());
    }
    return "Invalid asset package: " + code;
}

std::uint32_t readUint32LE(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) | (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::vector<std::uint8_t> readFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::vector<std::string> normalizedErrors(std::vector<std::string> errors) {
    std::sort(errors.begin(), errors.end());
    errors.erase(std::unique(errors.begin(), errors.end()), errors.end());
    return errors;
}

} // namespace

namespace urpg::exporting {

std::vector<std::string> ExportValidator::validateExportDirectory(const std::string& path,
                                                                  tools::ExportTarget target) const {
    std::vector<std::string> errors;
    std::filesystem::path dir(path);

    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        errors.emplace_back("Path does not exist or is not a directory: " + path);
        return errors;
    }

    auto requirements = getRequirementsForTarget(target);

    for (const auto& req : requirements) {
        bool found = false;

        if (req.filePattern == "*.app") {
            found = checkAnyAppDirectory(dir);
        } else if (req.filePattern == "executable_without_extension") {
            found = checkAnyExecutableWithoutExtension(dir);
        } else if (req.filePattern.find('*') != std::string::npos) {
            found = checkPatternExists(dir, req.filePattern);
        } else {
            found = std::filesystem::exists(dir / req.filePattern);
        }

        if (req.required && !found) {
            errors.emplace_back("Missing required file: " + req.filePattern + " (" + req.description + ")");
        }
    }

    if (!errors.empty()) {
        return errors;
    }

    const auto bundlePath = dir / "data.pck";
    if (std::filesystem::exists(bundlePath) && std::filesystem::is_regular_file(bundlePath)) {
        auto bundleErrors = validateBundleIntegrity(bundlePath, target);
        errors.insert(errors.end(), bundleErrors.begin(), bundleErrors.end());
    }

    return errors;
}

std::vector<PlatformRequirement> ExportValidator::getRequirementsForTarget(tools::ExportTarget target) const {
    switch (target) {
    case tools::ExportTarget::Windows_x64:
        return {{"*.exe", true, "Windows executable"},
                {"data.pck", true, "Asset package"},
                {"*.dll", false, "Dynamic libraries"}};
    case tools::ExportTarget::Linux_x64:
        return {{"executable_without_extension", true, "Linux executable"},
                {"data.pck", true, "Asset package"},
                {"*.so", false, "Shared libraries"}};
    case tools::ExportTarget::macOS_Universal:
        return {{"*.app", true, "macOS application bundle"}, {"data.pck", true, "Asset package"}};
    case tools::ExportTarget::Web_WASM:
        return {{"index.html", true, "HTML entry point"},
                {"*.wasm", true, "WebAssembly binary"},
                {"*.js", true, "JavaScript loader"},
                {"data.pck", true, "Asset package"}};
    default:
        return {};
    }
}

nlohmann::json ExportValidator::buildReportJson(const std::vector<std::string>& errors,
                                                tools::ExportTarget target) const {
    const auto normalized = normalizedErrors(errors);
    nlohmann::json report;
    report["target"] = targetToString(target);
    report["passed"] = normalized.empty();
    report["errors"] = normalized;
    return report;
}

nlohmann::json ExportValidator::buildReportJson(const std::string& path, tools::ExportTarget target) const {
    const auto errors = validateExportDirectory(path, target);
    auto report = buildReportJson(errors, target);

    const auto bundlePath = std::filesystem::path(path) / "data.pck";
    if (std::filesystem::exists(bundlePath) && std::filesystem::is_regular_file(bundlePath)) {
        const auto summary = inspectBundleSummary(bundlePath, target);
        if (summary.has_value()) {
            report["bundleSummary"] = *summary;
        }
    }

    return report;
}

bool ExportValidator::checkPatternExists(const std::filesystem::path& dir, const std::string& pattern) const {
    if (pattern.size() < 2 || pattern[0] != '*') {
        return std::filesystem::exists(dir / pattern);
    }

    std::string ext = pattern.substr(1);
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension().string() == ext) {
            return true;
        }
    }
    return false;
}

bool ExportValidator::checkAnyAppDirectory(const std::filesystem::path& dir) const {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            if (name.size() > 4 && name.substr(name.size() - 4) == ".app") {
                return true;
            }
        }
    }
    return false;
}

bool ExportValidator::checkAnyExecutableWithoutExtension(const std::filesystem::path& dir) const {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().filename().string();
            if (name.find('.') == std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::string> ExportValidator::validateBundleIntegrity(const std::filesystem::path& bundlePath,
                                                                  tools::ExportTarget target) const {
    std::vector<std::string> errors;

    const auto contract = bundle_contract::validateBundleFile(bundlePath, target);
    for (const auto& code : contract.errors) {
        errors.emplace_back(formatBundleContractError(code));
    }
    if (!contract.valid) {
        return errors;
    }

    const auto& bytes = contract.bytes;
    const auto& manifest = contract.manifest;
    const auto payloadOffset = manifest["payloadOffset"].get<std::size_t>();
    std::set<std::string> manifestPaths;

    for (const auto& entry : manifest["entries"]) {
        manifestPaths.insert(entry["path"].get<std::string>());
    }

    const auto discoveryEntryIt =
        std::find_if(manifest["entries"].begin(), manifest["entries"].end(), [](const nlohmann::json& entry) {
            return entry.is_object() && entry.contains("path") && entry["path"].is_string() &&
                   entry["path"].get<std::string>() == kAssetDiscoveryManifestPath;
        });

    if (discoveryEntryIt == manifest["entries"].end()) {
        errors.emplace_back("Invalid asset package: missing asset discovery manifest");
        return errors;
    }

    try {
        const auto decodedDiscoveryBytes =
            bundle_contract::decodeBundleEntryBytes(*discoveryEntryIt, bytes, payloadOffset, target);
        const std::string discoveryText(decodedDiscoveryBytes.begin(), decodedDiscoveryBytes.end());
        const auto discoveryManifest = nlohmann::json::parse(discoveryText);

        if (discoveryManifest.value("format", "") != kAssetDiscoveryFormat) {
            errors.emplace_back("Invalid asset package: asset discovery manifest format mismatch");
        }
        if (!discoveryManifest.contains("assets") || !discoveryManifest["assets"].is_array()) {
            errors.emplace_back("Invalid asset package: asset discovery manifest missing assets array");
        } else {
            const auto discoveredAssetCount = discoveryManifest.value("discoveredAssetCount", 0u);
            if (discoveredAssetCount != discoveryManifest["assets"].size()) {
                errors.emplace_back("Invalid asset package: asset discovery manifest asset count mismatch");
            }

            for (const auto& asset : discoveryManifest["assets"]) {
                if (!asset.is_object() || !asset.contains("path") || !asset["path"].is_string()) {
                    errors.emplace_back("Invalid asset package: asset discovery manifest entry is malformed");
                    continue;
                }

                const auto assetPath = asset["path"].get<std::string>();
                if (manifestPaths.find(assetPath) == manifestPaths.end()) {
                    errors.emplace_back("Invalid asset package: discovered asset missing from bundle entries: " +
                                        assetPath);
                }
            }
        }
    } catch (const std::exception& ex) {
        errors.emplace_back(std::string("Invalid asset package: asset discovery manifest parse failed: ") + ex.what());
    }

    return errors;
}

std::optional<nlohmann::json> ExportValidator::inspectBundleSummary(const std::filesystem::path& bundlePath,
                                                                    tools::ExportTarget target) const {
    const auto bytes = readFileBytes(bundlePath);
    if (bytes.size() < (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t)) {
        return std::nullopt;
    }
    if (std::string(bytes.begin(), bytes.begin() + (sizeof(kBundleMagic) - 1)) != kBundleMagic) {
        return std::nullopt;
    }

    const auto manifestSize = readUint32LE(bytes, sizeof(kBundleMagic) - 1);
    const auto manifestOffset = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t);
    if (bytes.size() < manifestOffset + manifestSize) {
        return std::nullopt;
    }

    nlohmann::json manifest;
    try {
        const std::string manifestText(bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset),
                                       bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset + manifestSize));
        manifest = nlohmann::json::parse(manifestText);
    } catch (const std::exception&) {
        return std::nullopt;
    }

    if (!manifest.contains("entries") || !manifest["entries"].is_array() || !manifest.contains("payloadOffset") ||
        !manifest["payloadOffset"].is_number_unsigned()) {
        return std::nullopt;
    }

    nlohmann::json summary;
    summary["path"] = bundlePath.generic_string();
    summary["format"] = manifest.value("format", "");
    summary["bundleMode"] = manifest.value("bundleMode", "");
    summary["target"] = manifest.value("target", targetToString(target));
    summary["entryCount"] = manifest["entries"].size();
    summary["assetDiscoveryMode"] = manifest.value("assetDiscoveryMode", "");
    summary["integrityMode"] = manifest.value("integrityMode", "");
    summary["signatureMode"] = manifest.value("signatureMode", "");
    summary["bundleSignaturePresent"] = manifest.contains("bundleSignature") &&
                                        manifest["bundleSignature"].is_string() &&
                                        !manifest["bundleSignature"].get<std::string>().empty();

    const auto payloadOffset = manifest["payloadOffset"].get<std::size_t>();
    const auto discoveryEntryIt =
        std::find_if(manifest["entries"].begin(), manifest["entries"].end(), [](const nlohmann::json& entry) {
            return entry.is_object() && entry.contains("path") && entry["path"].is_string() &&
                   entry["path"].get<std::string>() == kAssetDiscoveryManifestPath;
        });

    if (discoveryEntryIt != manifest["entries"].end()) {
        try {
            const auto decodedDiscoveryBytes =
                bundle_contract::decodeBundleEntryBytes(*discoveryEntryIt, bytes, payloadOffset, target);
            const std::string discoveryText(decodedDiscoveryBytes.begin(), decodedDiscoveryBytes.end());
            summary["assetDiscoveryManifest"] = nlohmann::json::parse(discoveryText);
        } catch (const std::exception& ex) {
            summary["assetDiscoveryManifestError"] = ex.what();
        }
    }

    return summary;
}

std::string ExportValidator::targetToString(tools::ExportTarget target) const {
    switch (target) {
    case tools::ExportTarget::Windows_x64:
        return "Windows_x64";
    case tools::ExportTarget::Linux_x64:
        return "Linux_x64";
    case tools::ExportTarget::macOS_Universal:
        return "macOS_Universal";
    case tools::ExportTarget::Web_WASM:
        return "Web_WASM";
    default:
        return "Unknown";
    }
}

} // namespace urpg::exporting
