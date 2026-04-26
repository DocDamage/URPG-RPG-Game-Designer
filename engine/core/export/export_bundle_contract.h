#pragma once

#include "engine/core/tools/export_packager.h"

#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::exporting::bundle_contract {

constexpr char kBundleMagic[] = "URPGPCK1";
constexpr char kIntegrityMode[] = "fnv1a64_keyed";
constexpr char kSignatureMode[] = "sha256_keyed_bundle_v1";

struct BundleValidationResult {
    bool valid = false;
    nlohmann::json manifest;
    std::vector<std::uint8_t> bytes;
    std::vector<std::string> errors;
};

std::uint32_t readUint32LE(const std::vector<std::uint8_t>& bytes, std::size_t offset);
std::vector<std::uint8_t> readFileBytes(const std::filesystem::path& path);

std::string bundleObfuscationKey(urpg::tools::ExportTarget target);
std::string bundleSignatureKey(urpg::tools::ExportTarget target);
std::string makePayloadIntegrityScope(const nlohmann::json& entry);
nlohmann::json buildBundleSignatureView(const nlohmann::json& manifest);

std::vector<std::uint8_t> decodeBundleEntryBytes(const nlohmann::json& entry,
                                                 const std::vector<std::uint8_t>& bundleBytes,
                                                 std::size_t payloadOffset, urpg::tools::ExportTarget target);

BundleValidationResult validateBundleFile(const std::filesystem::path& bundlePath, urpg::tools::ExportTarget target);

} // namespace urpg::exporting::bundle_contract
