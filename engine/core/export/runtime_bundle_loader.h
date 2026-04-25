#pragma once

#include "engine/core/tools/export_packager.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::exporting {

struct RuntimeBundleLoadResult {
    bool loaded = false;
    nlohmann::json manifest;
    std::vector<std::string> errors;
};

RuntimeBundleLoadResult LoadRuntimeBundle(const std::filesystem::path& bundle_path,
                                          urpg::tools::ExportTarget target);

bool PublishRuntimeBundleAtomic(const std::filesystem::path& source_bundle,
                                const std::filesystem::path& destination_bundle,
                                std::string* error_message = nullptr);

} // namespace urpg::exporting
