#include "engine/core/export/export_bundle_contract.h"

#include "engine/core/security/resource_protector.h"

#include <fstream>
#include <iterator>
#include <limits>

namespace urpg::exporting::bundle_contract {

namespace {

constexpr std::size_t kMaxManifestBytes = 16u * 1024u * 1024u;

bool checkedRange(std::size_t total, std::size_t offset, std::size_t size) {
    return offset <= total && size <= total - offset;
}

std::vector<std::uint8_t> concatStoredPayloadBytes(const nlohmann::json& entries,
                                                   const std::vector<std::uint8_t>& bundleBytes,
                                                   std::size_t payloadOffset) {
    std::vector<std::uint8_t> payloadBytes;
    for (const auto& entry : entries) {
        const auto offset = entry["offset"].get<std::size_t>();
        const auto storedSize = entry["storedSize"].get<std::size_t>();
        const auto begin = bundleBytes.begin() + static_cast<std::ptrdiff_t>(payloadOffset + offset);
        const auto end = begin + static_cast<std::ptrdiff_t>(storedSize);
        payloadBytes.insert(payloadBytes.end(), begin, end);
    }
    return payloadBytes;
}

} // namespace

std::uint32_t readUint32LE(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) | (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::vector<std::uint8_t> readFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::string bundleObfuscationKey(urpg::tools::ExportTarget target) {
    switch (target) {
    case urpg::tools::ExportTarget::Windows_x64:
        return "urpg-export-bundle-win";
    case urpg::tools::ExportTarget::Linux_x64:
        return "urpg-export-bundle-linux";
    case urpg::tools::ExportTarget::macOS_Universal:
        return "urpg-export-bundle-macos";
    case urpg::tools::ExportTarget::Web_WASM:
        return "urpg-export-bundle-web";
    default:
        return "urpg-export-bundle";
    }
}

std::string bundleSignatureKey(urpg::tools::ExportTarget target) {
    switch (target) {
    case urpg::tools::ExportTarget::Windows_x64:
        return "urpg-export-signature-win-v1";
    case urpg::tools::ExportTarget::Linux_x64:
        return "urpg-export-signature-linux-v1";
    case urpg::tools::ExportTarget::macOS_Universal:
        return "urpg-export-signature-macos-v1";
    case urpg::tools::ExportTarget::Web_WASM:
        return "urpg-export-signature-web-v1";
    default:
        return "urpg-export-signature-v1";
    }
}

std::string makePayloadIntegrityScope(const nlohmann::json& entry) {
    return entry.value("path", "") + "|" + entry.value("kind", "") + "|" +
           (entry.value("compressed", false) ? "1" : "0") + "|" + (entry.value("obfuscated", false) ? "1" : "0") + "|" +
           std::to_string(entry.value("rawSize", 0u));
}

nlohmann::json buildBundleSignatureView(const nlohmann::json& manifest) {
    auto signatureView = manifest;
    signatureView.erase("payloadOffset");
    signatureView.erase("bundleSignature");
    return signatureView;
}

std::vector<std::uint8_t> decodeBundleEntryBytes(const nlohmann::json& entry,
                                                 const std::vector<std::uint8_t>& bundleBytes,
                                                 std::size_t payloadOffset, urpg::tools::ExportTarget target) {
    const auto offset = entry["offset"].get<std::size_t>();
    const auto storedSize = entry["storedSize"].get<std::size_t>();
    const auto begin = bundleBytes.begin() + static_cast<std::ptrdiff_t>(payloadOffset + offset);
    const auto end = begin + static_cast<std::ptrdiff_t>(storedSize);
    std::vector<std::uint8_t> decoded(begin, end);

    urpg::security::ResourceProtector protector;
    if (entry.value("obfuscated", false)) {
        protector.obfuscate(decoded, bundleObfuscationKey(target));
    }
    if (entry.value("compressed", false)) {
        decoded = urpg::core::AssetCompressor::instance().decompress(decoded);
    }
    return decoded;
}

BundleValidationResult validateBundleFile(const std::filesystem::path& bundlePath, urpg::tools::ExportTarget target) {
    BundleValidationResult result;

    if (!std::filesystem::exists(bundlePath) || !std::filesystem::is_regular_file(bundlePath)) {
        result.errors.emplace_back("missing_bundle");
        return result;
    }

    result.bytes = readFileBytes(bundlePath);
    const auto headerSize = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t);
    if (result.bytes.size() < headerSize) {
        result.errors.emplace_back("truncated_bundle_header");
        return result;
    }

    if (std::string(result.bytes.begin(), result.bytes.begin() + (sizeof(kBundleMagic) - 1)) != kBundleMagic) {
        result.errors.emplace_back("invalid_bundle_magic");
        return result;
    }

    const auto manifestSize = readUint32LE(result.bytes, sizeof(kBundleMagic) - 1);
    const auto manifestOffset = headerSize;
    if (manifestSize == 0 || manifestSize > kMaxManifestBytes) {
        result.errors.emplace_back("invalid_bundle_manifest_size");
        return result;
    }
    if (!checkedRange(result.bytes.size(), manifestOffset, manifestSize)) {
        result.errors.emplace_back("truncated_bundle_manifest");
        return result;
    }

    const std::string manifestText(result.bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset),
                                   result.bytes.begin() + static_cast<std::ptrdiff_t>(manifestOffset + manifestSize));
    result.manifest = nlohmann::json::parse(manifestText, nullptr, false);
    if (result.manifest.is_discarded() || !result.manifest.is_object()) {
        result.errors.emplace_back("invalid_bundle_manifest_json");
        return result;
    }

    if (!result.manifest.contains("entries") || !result.manifest["entries"].is_array()) {
        result.errors.emplace_back("missing_manifest_entries");
        return result;
    }
    if (!result.manifest.contains("payloadOffset") || !result.manifest["payloadOffset"].is_number_unsigned()) {
        result.errors.emplace_back("missing_payload_offset");
        return result;
    }
    if (!result.manifest.contains("integrityMode") || !result.manifest["integrityMode"].is_string()) {
        result.errors.emplace_back("missing_integrity_mode");
        return result;
    }
    if (result.manifest["integrityMode"].get<std::string>() != kIntegrityMode) {
        result.errors.emplace_back("unsupported_integrity_mode");
        return result;
    }
    if (!result.manifest.contains("signatureMode") || !result.manifest["signatureMode"].is_string()) {
        result.errors.emplace_back("missing_signature_mode");
        return result;
    }
    if (result.manifest["signatureMode"].get<std::string>() != kSignatureMode) {
        result.errors.emplace_back("unsupported_signature_mode");
        return result;
    }
    if (!result.manifest.contains("bundleSignature") || !result.manifest["bundleSignature"].is_string() ||
        result.manifest["bundleSignature"].get<std::string>().empty()) {
        result.errors.emplace_back("missing_bundle_signature");
        return result;
    }

    const auto payloadOffset = result.manifest["payloadOffset"].get<std::size_t>();
    if (payloadOffset < manifestOffset + manifestSize || payloadOffset > result.bytes.size()) {
        result.errors.emplace_back("payload_offset_out_of_bounds");
        return result;
    }

    urpg::security::ResourceProtector protector;
    const auto integrityKey = bundleObfuscationKey(target);
    for (const auto& entry : result.manifest["entries"]) {
        if (!entry.is_object()) {
            result.errors.emplace_back("manifest_entry_not_object");
            continue;
        }
        if (!entry.contains("path") || !entry["path"].is_string() || !entry.contains("kind") ||
            !entry["kind"].is_string() || !entry.contains("offset") || !entry["offset"].is_number_unsigned() ||
            !entry.contains("storedSize") || !entry["storedSize"].is_number_unsigned() || !entry.contains("rawSize") ||
            !entry["rawSize"].is_number_unsigned() || !entry.contains("integrityTag") ||
            !entry["integrityTag"].is_string()) {
            result.errors.emplace_back("manifest_entry_missing_required_fields");
            continue;
        }

        const auto offset = entry["offset"].get<std::size_t>();
        const auto storedSize = entry["storedSize"].get<std::size_t>();
        if (offset > result.bytes.size() - payloadOffset ||
            !checkedRange(result.bytes.size(), payloadOffset + offset, storedSize)) {
            result.errors.emplace_back("payload_entry_extends_past_bundle:" + entry["path"].get<std::string>());
            continue;
        }

        const auto begin = result.bytes.begin() + static_cast<std::ptrdiff_t>(payloadOffset + offset);
        const auto end = begin + static_cast<std::ptrdiff_t>(storedSize);
        const std::vector<std::uint8_t> storedBytes(begin, end);
        const auto expectedTag =
            protector.computeIntegrityTag(makePayloadIntegrityScope(entry), storedBytes, integrityKey);
        if (expectedTag != entry["integrityTag"].get<std::string>()) {
            result.errors.emplace_back("entry_integrity_mismatch:" + entry["path"].get<std::string>());
        }
    }

    if (!result.errors.empty()) {
        return result;
    }

    const auto expectedSignature = protector.computeCryptographicSignature(
        buildBundleSignatureView(result.manifest).dump(),
        concatStoredPayloadBytes(result.manifest["entries"], result.bytes, payloadOffset), bundleSignatureKey(target));
    if (expectedSignature != result.manifest["bundleSignature"].get<std::string>()) {
        result.errors.emplace_back("bundle_signature_mismatch");
        return result;
    }

    result.valid = true;
    return result;
}

} // namespace urpg::exporting::bundle_contract
