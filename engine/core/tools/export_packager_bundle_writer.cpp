#include "engine/core/tools/export_packager_bundle_writer.h"

#include "engine/core/security/resource_protector.h"

#include <algorithm>
#include <fstream>
#include <limits>

namespace urpg::tools::export_packager_detail {

namespace {

constexpr char kBundleMagic[] = "URPGPCK1";
constexpr char kBundleSignatureMode[] = "sha256_keyed_bundle_v1";

bool checkedAddUint32(std::uint32_t& value, std::size_t increment) {
    if (increment > std::numeric_limits<std::uint32_t>::max() - value) {
        return false;
    }
    value += static_cast<std::uint32_t>(increment);
    return true;
}

std::string bundleSignatureKey(ExportTarget target) {
    switch (target) {
        case ExportTarget::Windows_x64: return "urpg-export-signature-win-v1";
        case ExportTarget::Linux_x64: return "urpg-export-signature-linux-v1";
        case ExportTarget::macOS_Universal: return "urpg-export-signature-macos-v1";
        case ExportTarget::Web_WASM: return "urpg-export-signature-web-v1";
        default: return "urpg-export-signature-v1";
    }
}

std::string bundleTargetLabel(ExportTarget target) {
    switch (target) {
        case ExportTarget::Windows_x64: return "Windows (x64)";
        case ExportTarget::Linux_x64: return "Linux (x64)";
        case ExportTarget::macOS_Universal: return "macOS (Universal)";
        case ExportTarget::Web_WASM: return "Web (WASM/WebGL)";
        default: return "Other";
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
    auto signatureView = manifest;
    signatureView.erase("payloadOffset");
    signatureView.erase("bundleSignature");
    return signatureView;
}

BundleWriteResult writeBundleFile(
    const std::filesystem::path& outputDir,
    ExportTarget target,
    bool compressAssets,
    const std::string& assetDiscoveryMode,
    std::vector<BundlePayload> payloads) {
    BundleWriteResult result;
    const std::filesystem::path pckPath = outputDir / "data.pck";
    std::ofstream pck(pckPath, std::ios::binary);
    if (!pck.good()) {
        result.errors.push_back("Failed to open bounded asset bundle for writing.");
        return result;
    }

    nlohmann::json manifest;
    manifest["format"] = "URPG_BOUNDED_EXPORT_BUNDLE_V1";
    manifest["bundleMode"] = "bounded_export_smoke";
    manifest["target"] = bundleTargetLabel(target);
    manifest["assetDiscoveryMode"] = assetDiscoveryMode;
    manifest["protectionMode"] = compressAssets ? "rle_xor" : "none";
    manifest["integrityMode"] = "fnv1a64_keyed";
    manifest["signatureMode"] = kBundleSignatureMode;
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
        manifest["payloadOffset"] =
            (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t) + finalizedManifestJson.size();
        finalizedManifestJson = manifest.dump();
    }
    if (!fitsUint32(finalizedManifestJson.size())) {
        result.errors.push_back("Bundle manifest exceeds uint32 length limit.");
        return result;
    }

    urpg::security::ResourceProtector protector;
    manifest["bundleSignature"] = protector.computeCryptographicSignature(
        buildBundleSignatureView(manifest).dump(),
        concatPayloadBytes(payloads),
        bundleSignatureKey(target));
    finalizedManifestJson = manifest.dump();
    if (!fitsUint32(finalizedManifestJson.size())) {
        result.errors.push_back("Bundle manifest exceeds uint32 length limit.");
        return result;
    }

    pck.write(kBundleMagic, sizeof(kBundleMagic) - 1);
    appendUint32LE(pck, static_cast<std::uint32_t>(finalizedManifestJson.size()));
    pck.write(finalizedManifestJson.data(), static_cast<std::streamsize>(finalizedManifestJson.size()));
    for (const auto& payload : payloads) {
        pck.write(reinterpret_cast<const char*>(payload.bytes.data()),
                  static_cast<std::streamsize>(payload.bytes.size()));
    }
    pck.close();
    if (!pck.good()) {
        result.errors.push_back("Failed while writing bounded asset bundle.");
        return result;
    }

    result.success = true;
    result.fileName = "data.pck";
    result.log += "Wrote bounded asset bundle with " + std::to_string(payloads.size()) + " staged payload(s).\n";
    const auto discoveredAssetCount = std::count_if(
        payloads.begin(), payloads.end(),
        [](const BundlePayload& payload) { return payload.kind == "auto_discovered_asset"; });
    result.log += "Auto-discovered " + std::to_string(discoveredAssetCount) + " project asset(s).\n";
    if (compressAssets) {
        result.log += "Applied lightweight RLE+XOR protection to bundle payloads.\n";
    }
    result.log += "Applied lightweight keyed integrity tags to bundle payloads.\n";
    result.log += "Applied keyed SHA-256 bundle signature across manifest metadata and staged payload bytes.\n";
    return result;
}

} // namespace urpg::tools::export_packager_detail
