#pragma once

#include "engine/core/tools/export_packager.h"
#include "engine/core/tools/export_packager_bundle_writer.h"

#include <string>
#include <vector>

namespace urpg::tools::export_packager_detail {

struct BundleBuildResult {
    std::vector<BundlePayload> payloads;
    std::vector<std::string> errors;
};

BundleBuildResult buildBundlePayloads(const ExportConfig& config);

} // namespace urpg::tools::export_packager_detail
