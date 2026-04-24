#pragma once

#include "engine/core/tools/export_packager.h"

#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::tools::export_packager_detail {

struct BundlePayload {
    std::string path;
    std::string kind;
    std::vector<std::uint8_t> bytes;
    std::size_t rawSize = 0;
    bool compressed = false;
    bool obfuscated = false;
    std::string integrityTag;
};

struct BundleWriteResult {
    bool success = false;
    std::string fileName;
    std::string log;
    std::vector<std::string> errors;
};

bool fitsUint32(std::size_t value);

std::vector<std::uint8_t> concatPayloadBytes(const std::vector<BundlePayload>& payloads);
nlohmann::json buildBundleSignatureView(const nlohmann::json& manifest);

BundleWriteResult writeBundleFile(
    const std::filesystem::path& outputDir,
    ExportTarget target,
    bool compressAssets,
    const std::string& assetDiscoveryMode,
    std::vector<BundlePayload> payloads);

} // namespace urpg::tools::export_packager_detail
