#pragma once

#include "engine/core/export/export_artifact_compare.h"

#include <nlohmann/json.hpp>

#include <set>
#include <string>
#include <vector>

namespace urpg::exporting {

struct PatchManifest {
    std::string id;
    std::string base_version;
    std::string target_version;
    std::vector<std::string> changed_data;
    std::vector<std::string> changed_assets;
    std::vector<std::string> dependencies;
};

std::vector<std::string> ValidatePatchManifest(const PatchManifest& manifest,
                                               const std::set<std::string>& available_dependencies);
PatchManifest BuildPatchManifestFromExportDiff(std::string id,
                                               std::string base_version,
                                               std::string target_version,
                                               const ExportArtifactCompareResult& diff,
                                               std::vector<std::string> dependencies = {});
nlohmann::json PatchManifestToJson(const PatchManifest& manifest);
PatchManifest PatchManifestFromJson(const nlohmann::json& json);

} // namespace urpg::exporting
