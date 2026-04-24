#pragma once

#include "engine/core/tools/export_packager.h"

#include <string>
#include <vector>

namespace urpg::tools::export_packager_detail {

bool auditPromotedAssetBundleLicenses(const ExportConfig& config, std::vector<std::string>& errors);
bool auditAutoDiscoveredAssetLicenses(const ExportConfig& config, std::vector<std::string>& errors);

} // namespace urpg::tools::export_packager_detail
