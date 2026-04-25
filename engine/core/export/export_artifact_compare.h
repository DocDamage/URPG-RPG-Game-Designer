#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::exporting {

struct ExportArtifactCompareResult {
    std::vector<std::string> changed_assets;
    std::vector<std::string> changed_schemas;
    std::vector<std::string> missing_files;
    bool signature_changed = false;
    bool manifest_changed = false;
};

ExportArtifactCompareResult CompareExportArtifacts(const nlohmann::json& baseline_manifest,
                                                   const nlohmann::json& candidate_manifest);
nlohmann::json ExportArtifactCompareResultToJson(const ExportArtifactCompareResult& result);

} // namespace urpg::exporting
