#include "engine/core/export/runtime_bundle_loader.h"

#include "engine/core/security/resource_protector.h"
#include "engine/core/tools/export_packager_bundle_writer.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <system_error>

namespace urpg::exporting {

namespace {

constexpr char kBundleMagic[] = "URPGPCK1";
constexpr char kBundleSignatureMode[] = "sha256_keyed_bundle_v1";

std::vector<std::uint8_t> readFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::uint32_t readUint32LE(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
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
    }
    return "urpg-export-signature-v1";
}

std::vector<std::uint8_t> concatStoredPayloadBytes(const nlohmann::json& entries,
                                                   const std::vector<std::uint8_t>& bundle_bytes,
                                                   std::size_t payload_offset,
                                                   std::vector<std::string>& errors) {
    std::vector<std::uint8_t> payload;
    for (const auto& entry : entries) {
        if (!entry.contains("offset") || !entry.contains("storedSize")) {
            errors.push_back("manifest_entry_missing_payload_bounds");
            continue;
        }
        const auto offset = entry["offset"].get<std::size_t>();
        const auto stored_size = entry["storedSize"].get<std::size_t>();
        if (bundle_bytes.size() < payload_offset + offset + stored_size) {
            errors.push_back("payload_entry_extends_past_bundle");
            continue;
        }
        const auto begin = bundle_bytes.begin() + static_cast<std::ptrdiff_t>(payload_offset + offset);
        const auto end = begin + static_cast<std::ptrdiff_t>(stored_size);
        payload.insert(payload.end(), begin, end);
    }
    return payload;
}

} // namespace

RuntimeBundleLoadResult LoadRuntimeBundle(const std::filesystem::path& bundle_path,
                                          urpg::tools::ExportTarget target) {
    RuntimeBundleLoadResult result;
    const auto bytes = readFileBytes(bundle_path);
    if (bytes.size() < (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t)) {
        result.errors.push_back("truncated_bundle_header");
        return result;
    }
    if (std::string(bytes.begin(), bytes.begin() + (sizeof(kBundleMagic) - 1)) != kBundleMagic) {
        result.errors.push_back("invalid_bundle_magic");
        return result;
    }

    const auto manifest_size = readUint32LE(bytes, sizeof(kBundleMagic) - 1);
    const auto manifest_offset = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t);
    if (bytes.size() < manifest_offset + manifest_size) {
        result.errors.push_back("truncated_bundle_manifest");
        return result;
    }

    const std::string manifest_text(
        bytes.begin() + static_cast<std::ptrdiff_t>(manifest_offset),
        bytes.begin() + static_cast<std::ptrdiff_t>(manifest_offset + manifest_size));
    result.manifest = nlohmann::json::parse(manifest_text, nullptr, false);
    if (result.manifest.is_discarded() || !result.manifest.is_object()) {
        result.errors.push_back("invalid_bundle_manifest_json");
        return result;
    }
    if (result.manifest.value("signatureMode", "") != kBundleSignatureMode ||
        !result.manifest.contains("bundleSignature") ||
        !result.manifest["bundleSignature"].is_string()) {
        result.errors.push_back("missing_bundle_signature");
        return result;
    }
    if (!result.manifest.contains("payloadOffset") ||
        !result.manifest["payloadOffset"].is_number_unsigned() ||
        !result.manifest.contains("entries") ||
        !result.manifest["entries"].is_array()) {
        result.errors.push_back("invalid_bundle_payload_metadata");
        return result;
    }

    const auto payload_offset = result.manifest["payloadOffset"].get<std::size_t>();
    if (bytes.size() < payload_offset) {
        result.errors.push_back("payload_offset_past_end");
        return result;
    }

    std::vector<std::string> payload_errors;
    const auto stored_payload = concatStoredPayloadBytes(result.manifest["entries"], bytes, payload_offset, payload_errors);
    result.errors.insert(result.errors.end(), payload_errors.begin(), payload_errors.end());
    if (!payload_errors.empty()) {
        return result;
    }

    urpg::security::ResourceProtector protector;
    const auto expected_signature = protector.computeCryptographicSignature(
        urpg::tools::export_packager_detail::buildBundleSignatureView(result.manifest).dump(),
        stored_payload,
        bundleSignatureKey(target));
    if (expected_signature != result.manifest["bundleSignature"].get<std::string>()) {
        result.errors.push_back("bundle_signature_mismatch");
        return result;
    }

    result.loaded = true;
    return result;
}

bool PublishRuntimeBundleAtomic(const std::filesystem::path& source_bundle,
                                const std::filesystem::path& destination_bundle,
                                std::string* error_message) {
    if (error_message) {
        error_message->clear();
    }
    std::error_code ec;
    std::filesystem::create_directories(destination_bundle.parent_path(), ec);
    if (ec) {
        if (error_message) *error_message = ec.message();
        return false;
    }

    std::filesystem::path temp_path = destination_bundle;
    temp_path += ".tmp";
    std::filesystem::copy_file(source_bundle, temp_path, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        if (error_message) *error_message = ec.message();
        return false;
    }

    std::filesystem::rename(temp_path, destination_bundle, ec);
    if (ec) {
        std::filesystem::remove(temp_path);
        if (error_message) *error_message = ec.message();
        return false;
    }
    return true;
}

} // namespace urpg::exporting
