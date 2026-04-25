#include "engine/core/export/patch_manifest.h"

#include <utility>

namespace urpg::exporting {

std::vector<std::string> ValidatePatchManifest(const PatchManifest& manifest,
                                               const std::set<std::string>& available_dependencies) {
    std::vector<std::string> errors;
    for (const auto& dependency : manifest.dependencies) {
        if (!available_dependencies.contains(dependency)) {
            errors.push_back("missing_patch_dependency:" + dependency);
        }
    }
    if (manifest.base_version.empty() || manifest.target_version.empty()) {
        errors.push_back("missing_patch_version");
    }
    if (!manifest.base_version.empty() && manifest.base_version == manifest.target_version) {
        errors.push_back("patch_target_matches_base_version");
    }
    return errors;
}

PatchManifest BuildPatchManifestFromExportDiff(std::string id,
                                               std::string base_version,
                                               std::string target_version,
                                               const ExportArtifactCompareResult& diff,
                                               std::vector<std::string> dependencies) {
    PatchManifest manifest;
    manifest.id = std::move(id);
    manifest.base_version = std::move(base_version);
    manifest.target_version = std::move(target_version);
    manifest.changed_data = diff.changed_schemas;
    manifest.changed_assets = diff.changed_assets;
    manifest.dependencies = std::move(dependencies);
    return manifest;
}

nlohmann::json PatchManifestToJson(const PatchManifest& manifest) {
    return {
        {"id", manifest.id},
        {"base_version", manifest.base_version},
        {"target_version", manifest.target_version},
        {"changed_data", manifest.changed_data},
        {"changed_assets", manifest.changed_assets},
        {"dependencies", manifest.dependencies},
    };
}

PatchManifest PatchManifestFromJson(const nlohmann::json& json) {
    PatchManifest manifest;
    if (!json.is_object()) {
        return manifest;
    }
    manifest.id = json.value("id", "");
    manifest.base_version = json.value("base_version", "");
    manifest.target_version = json.value("target_version", "");
    if (json.contains("changed_data") && json["changed_data"].is_array()) {
        manifest.changed_data = json["changed_data"].get<std::vector<std::string>>();
    }
    if (json.contains("changed_assets") && json["changed_assets"].is_array()) {
        manifest.changed_assets = json["changed_assets"].get<std::vector<std::string>>();
    }
    if (json.contains("dependencies") && json["dependencies"].is_array()) {
        manifest.dependencies = json["dependencies"].get<std::vector<std::string>>();
    }
    return manifest;
}

} // namespace urpg::exporting
