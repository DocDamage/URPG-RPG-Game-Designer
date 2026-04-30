#include "engine/core/tools/export_packager_bundle_writer.h"

#include "engine/core/export/export_bundle_contract.h"
#include "engine/core/security/resource_protector.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace urpg::tools::export_packager_detail {

namespace {

constexpr char kBundleMagic[] = "URPGPCK1";

bool checkedAddUint32(std::uint32_t& value, std::size_t increment) {
    if (increment > std::numeric_limits<std::uint32_t>::max() - value) {
        return false;
    }
    value += static_cast<std::uint32_t>(increment);
    return true;
}

std::string bundleSignatureKey(ExportTarget target) {
    return urpg::exporting::bundle_contract::bundleSignatureKey(target);
}

std::string bundleTargetLabel(ExportTarget target) {
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

void appendUint32LE(std::ofstream& out, std::uint32_t value) {
    const char encoded[4] = {
        static_cast<char>(value & 0xFF),
        static_cast<char>((value >> 8) & 0xFF),
        static_cast<char>((value >> 16) & 0xFF),
        static_cast<char>((value >> 24) & 0xFF),
    };
    out.write(encoded, sizeof(encoded));
}

void removeIfExists(const std::filesystem::path& path) {
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

bool replaceFileAtomically(const std::filesystem::path& source, const std::filesystem::path& destination,
                           std::string& error) {
#ifdef _WIN32
    if (MoveFileExW(source.wstring().c_str(), destination.wstring().c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0) {
        error = std::system_category().message(static_cast<int>(GetLastError()));
        return false;
    }
    return true;
#else
    std::error_code ec;
    std::filesystem::rename(source, destination, ec);
    if (ec) {
        error = ec.message();
        return false;
    }
    return true;
#endif
}

} // namespace

bool fitsUint32(std::size_t value) {
    return value <= std::numeric_limits<std::uint32_t>::max();
}

std::vector<std::uint8_t> concatPayloadBytes(const std::vector<BundlePayload>& payloads) {
    std::vector<std::uint8_t> bytes;
    std::size_t totalSize = 0;
    for (const auto& payload : payloads) {
        totalSize += payload.bytes.size();
    }
    bytes.reserve(totalSize);

    for (const auto& payload : payloads) {
        bytes.insert(bytes.end(), payload.bytes.begin(), payload.bytes.end());
    }
    return bytes;
}

nlohmann::json buildBundleSignatureView(const nlohmann::json& manifest) {
    return urpg::exporting::bundle_contract::buildBundleSignatureView(manifest);
}

BundleWriteResult writeBundleFile(const std::filesystem::path& outputDir, ExportTarget target, bool compressAssets,
                                  const std::string& assetDiscoveryMode, std::vector<BundlePayload> payloads) {
    BundleWriteResult result;
    const std::filesystem::path pckPath = outputDir / "data.pck";
    const std::filesystem::path tempPath = outputDir / "data.pck.tmp";

    nlohmann::json manifest;
    manifest["format"] = "URPG_BOUNDED_EXPORT_BUNDLE_V1";
    manifest["bundleMode"] = "project_content_bundle_v1";
    manifest["target"] = bundleTargetLabel(target);
    manifest["assetDiscoveryMode"] = assetDiscoveryMode;
    manifest["protectionMode"] = compressAssets ? "rle_xor" : "none";
    manifest["integrityMode"] = urpg::exporting::bundle_contract::kIntegrityMode;
    manifest["signatureMode"] = urpg::exporting::bundle_contract::kSignatureMode;
    manifest["bundleSignature"] = std::string(64, '0');
    manifest["entries"] = nlohmann::json::array();

    std::uint32_t payloadOffset = 0;
    for (const auto& payload : payloads) {
        if (!fitsUint32(payload.bytes.size())) {
            result.errors.push_back("Bundle payload exceeds uint32 stored-size limit: " + payload.path);
            return result;
        }
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
        if (!checkedAddUint32(payloadOffset, payload.bytes.size())) {
            result.errors.push_back("Bundle payload offsets exceed uint32 limit before: " + payload.path);
            return result;
        }
    }

    std::string finalizedManifestJson = manifest.dump();
    for (int i = 0; i < 2; ++i) {
        manifest["payloadOffset"] = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t) + finalizedManifestJson.size();
        finalizedManifestJson = manifest.dump();
    }
    if (!fitsUint32(finalizedManifestJson.size())) {
        result.errors.push_back("Bundle manifest exceeds uint32 length limit.");
        return result;
    }

    urpg::security::ResourceProtector protector;
    manifest["bundleSignature"] = protector.computeCryptographicSignature(
        buildBundleSignatureView(manifest).dump(), concatPayloadBytes(payloads), bundleSignatureKey(target));
    finalizedManifestJson = manifest.dump();
    if (!fitsUint32(finalizedManifestJson.size())) {
        result.errors.push_back("Bundle manifest exceeds uint32 length limit.");
        return result;
    }

    removeIfExists(tempPath);
    std::ofstream pck(tempPath, std::ios::binary | std::ios::trunc);
    if (!pck.good()) {
        result.errors.push_back(
            "bundle_publish.temp_open_failed: Failed to open temporary bounded asset bundle for writing.");
        removeIfExists(tempPath);
        return result;
    }

    pck.write(kBundleMagic, sizeof(kBundleMagic) - 1);
    appendUint32LE(pck, static_cast<std::uint32_t>(finalizedManifestJson.size()));
    pck.write(finalizedManifestJson.data(), static_cast<std::streamsize>(finalizedManifestJson.size()));
    for (const auto& payload : payloads) {
        pck.write(reinterpret_cast<const char*>(payload.bytes.data()),
                  static_cast<std::streamsize>(payload.bytes.size()));
    }
    pck.flush();
    if (!pck.good()) {
        result.errors.push_back(
            "bundle_publish.temp_flush_failed: Failed while flushing temporary bounded asset bundle.");
        pck.close();
        removeIfExists(tempPath);
        return result;
    }
    pck.close();
    if (!pck.good()) {
        result.errors.push_back(
            "bundle_publish.temp_close_failed: Failed while closing temporary bounded asset bundle.");
        removeIfExists(tempPath);
        return result;
    }

    const auto validation = urpg::exporting::bundle_contract::validateBundleFile(tempPath, target);
    if (!validation.valid) {
        result.errors.push_back(
            "bundle_publish.temp_validation_failed: Temporary bounded asset bundle failed validation.");
        for (const auto& error : validation.errors) {
            result.errors.push_back("bundle_publish.validation." + error);
        }
        removeIfExists(tempPath);
        return result;
    }

    std::string replaceError;
    if (!replaceFileAtomically(tempPath, pckPath, replaceError)) {
        result.errors.push_back("bundle_publish.replace_failed: " + replaceError);
        removeIfExists(tempPath);
        return result;
    }

    result.success = true;
    result.fileName = "data.pck";
    result.log += "Wrote bounded asset bundle with " + std::to_string(payloads.size()) + " staged payload(s).\n";
    const auto discoveredAssetCount = std::count_if(payloads.begin(), payloads.end(), [](const BundlePayload& payload) {
        return payload.kind == "auto_discovered_asset";
    });
    result.log += "Auto-discovered " + std::to_string(discoveredAssetCount) + " project asset(s).\n";
    if (compressAssets) {
        result.log += "Applied lightweight RLE+XOR protection to bundle payloads.\n";
    }
    result.log += "Applied lightweight keyed integrity tags to bundle payloads.\n";
    result.log += "Applied keyed SHA-256 bundle signature across manifest metadata and staged payload bytes.\n";
    return result;
}

} // namespace urpg::tools::export_packager_detail
