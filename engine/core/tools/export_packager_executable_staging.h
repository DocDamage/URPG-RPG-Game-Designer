#pragma once

#include "engine/core/tools/export_packager.h"

#include <string>
#include <vector>

namespace urpg::tools::export_packager_detail {

struct ExecutableStageResult {
    std::vector<std::string> files;
    std::string log;
};

ExecutableStageResult stageExecutableArtifacts(const ExportConfig& config);

} // namespace urpg::tools::export_packager_detail
