#include "engine/core/export/export_validator.h"

#include "engine/core/security/resource_protector.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

constexpr char kBundleMagic[] = "URPGPCK1";

std::uint32_t readUint32LE(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::string bundleObfuscationKey(urpg::tools::ExportTarget target) {
    switch (target) {
        case urpg::tools::ExportTarget::Windows_x64: return "urpg-export-bundle-win";
        case urpg::tools::ExportTarget::Linux_x64: return "urpg-export-bundle-linux";
        case urpg::tools::ExportTarget::macOS_Universal: return "urpg-export-bundle-macos";
        case urpg::tools::ExportTarget::Web_WASM: return "urpg-export-bundle-web";
        default: return "urpg-export-bundle";
    }
}

std::string makePayloadIntegrityScope(const nlohmann::json& entry) {
    return entry.value("path", "") + "|" + entry.value("kind", "") + "|" +
           (entry.value("compressed", false) ? "1" : "0") + "|" +
           (entry.value("obfuscated", false) ? "1" : "0") + "|" +
           std::to_string(entry.value("rawSize", 0u));
}

std::vector<std::uint8_t> readFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

} // namespace

namespace urpg::exporting {

std::vector<std::string> ExportValidator::validateExportDirectory(const std::string& path, tools::ExportTarget target) {
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
            return {
                {"*.exe", true, "Windows executable"},
                {"data.pck", true, "Asset package"},
                {"*.dll", false, "Dynamic libraries"}
            };
        case tools::ExportTarget::Linux_x64:
            return {
                {"executable_without_extension", true, "Linux executable"},
                {"data.pck", true, "Asset package"},
                {"*.so", false, "Shared libraries"}
            };
        case tools::ExportTarget::macOS_Universal:
            return {
                {"*.app", true, "macOS application bundle"},
                {"data.pck", true, "Asset package"}
            };
        case tools::ExportTarget::Web_WASM:
            return {
                {"index.html", true, "HTML entry point"},
                {"*.wasm", true, "WebAssembly binary"},
                {"*.js", true, "JavaScript loader"},
                {"data.pck", true, "Asset package"}
            };
        default:
            return {};
    }
}

nlohmann::json ExportValidator::buildReportJson(const std::vector<std::string>& errors, tools::ExportTarget target) const {
    nlohmann::json report;
    report["target"] = targetToString(target);
    report["passed"] = errors.empty();
    report["errors"] = errors;
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

    const auto bytes = readFileBytes(bundlePath);
    if (bytes.size() < (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t)) {
        errors.emplace_back("Invalid asset package: truncated bundle header");
        return errors;
    }

    if (std::string(bytes.begin(), bytes.begin() + (sizeof(kBundleMagic) - 1)) != kBundleMagic) {
        errors.emplace_back("Invalid asset package: missing URPGPCK1 header");
        return errors;
    }

    const auto manifestSize = readUint32LE(bytes, sizeof(kBundleMagic) - 1);
    const auto manifestOffset = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t);
    if (bytes.size() < manifestOffset + manifestSize) {
        errors.emplace_back("Invalid asset package: manifest extends past end of file");
        return errors;
    }

    nlohmann::json manifest;
    try {
        const std::string manifestText(bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset),
                                       bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset + manifestSize));
        manifest = nlohmann::json::parse(manifestText);
    } catch (const std::exception& ex) {
        errors.emplace_back(std::string("Invalid asset package: manifest JSON parse failed: ") + ex.what());
        return errors;
    }

    if (!manifest.contains("entries") || !manifest["entries"].is_array()) {
        errors.emplace_back("Invalid asset package: missing manifest entries array");
        return errors;
    }
    if (!manifest.contains("payloadOffset") || !manifest["payloadOffset"].is_number_unsigned()) {
        errors.emplace_back("Invalid asset package: missing payload offset");
        return errors;
    }
    if (manifest.value("integrityMode", "") != "fnv1a64_keyed") {
        errors.emplace_back("Invalid asset package: missing keyed integrity metadata");
        return errors;
    }

    const auto payloadOffset = manifest["payloadOffset"].get<std::size_t>();
    if (bytes.size() < payloadOffset) {
        errors.emplace_back("Invalid asset package: payload offset extends past end of file");
        return errors;
    }

    urpg::security::ResourceProtector protector;
    const auto key = bundleObfuscationKey(target);

    for (const auto& entry : manifest["entries"]) {
        if (!entry.is_object()) {
            errors.emplace_back("Invalid asset package: manifest entry is not an object");
            continue;
        }
        if (!entry.contains("path") || !entry["path"].is_string() ||
            !entry.contains("kind") || !entry["kind"].is_string() ||
            !entry.contains("offset") || !entry["offset"].is_number_unsigned() ||
            !entry.contains("storedSize") || !entry["storedSize"].is_number_unsigned() ||
            !entry.contains("rawSize") || !entry["rawSize"].is_number_unsigned() ||
            !entry.contains("integrityTag") || !entry["integrityTag"].is_string()) {
            errors.emplace_back("Invalid asset package: manifest entry is missing required fields");
            continue;
        }

        const auto offset = entry["offset"].get<std::size_t>();
        const auto storedSize = entry["storedSize"].get<std::size_t>();
        if (bytes.size() < payloadOffset + offset + storedSize) {
            errors.emplace_back("Invalid asset package: payload bytes extend past end of file for " +
                                entry["path"].get<std::string>());
            continue;
        }

        const auto begin = bytes.begin() + static_cast<std::ptrdiff_t>(payloadOffset + offset);
        const auto end = begin + static_cast<std::ptrdiff_t>(storedSize);
        const std::vector<std::uint8_t> storedBytes(begin, end);
        const auto expectedTag = protector.computeIntegrityTag(
            makePayloadIntegrityScope(entry),
            storedBytes,
            key);
        if (expectedTag != entry["integrityTag"].get<std::string>()) {
            errors.emplace_back("Asset package integrity mismatch: " + entry["path"].get<std::string>());
        }
    }

    return errors;
}

std::string ExportValidator::targetToString(tools::ExportTarget target) const {
    switch (target) {
        case tools::ExportTarget::Windows_x64: return "Windows_x64";
        case tools::ExportTarget::Linux_x64: return "Linux_x64";
        case tools::ExportTarget::macOS_Universal: return "macOS_Universal";
        case tools::ExportTarget::Web_WASM: return "Web_WASM";
        default: return "Unknown";
    }
}

} // namespace urpg::exporting
