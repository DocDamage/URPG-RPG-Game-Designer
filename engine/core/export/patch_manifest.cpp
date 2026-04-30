#include "engine/core/export/patch_manifest.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <utility>

namespace urpg::exporting {

namespace {

bool isBlank(const std::string& value) {
    return std::all_of(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch) != 0; });
}

std::vector<std::string> normalizedStrings(const std::vector<std::string>& values) {
    std::vector<std::string> normalized;
    for (const auto& value : values) {
        if (!isBlank(value)) {
            normalized.push_back(value);
        }
    }
    std::sort(normalized.begin(), normalized.end());
    normalized.erase(std::unique(normalized.begin(), normalized.end()), normalized.end());
    return normalized;
}

std::optional<std::vector<std::string>> stringArrayFromJson(const nlohmann::json& json, const char* key) {
    if (!json.contains(key)) {
        return std::vector<std::string>{};
    }
    const auto& array = json.at(key);
    if (!array.is_array()) {
        return std::nullopt;
    }

    std::vector<std::string> values;
    for (const auto& value : array) {
        if (!value.is_string()) {
            return std::nullopt;
        }
        values.push_back(value.get<std::string>());
    }
    return normalizedStrings(values);
}

} // namespace

std::vector<std::string> ValidatePatchManifest(const PatchManifest& manifest,
                                               const std::set<std::string>& available_dependencies) {
    std::vector<std::string> errors;
    if (std::any_of(manifest.dependencies.begin(), manifest.dependencies.end(), isBlank)) {
        errors.push_back("invalid_patch_dependency");
    }
    if (normalizedStrings(manifest.dependencies).size() !=
        static_cast<size_t>(std::count_if(manifest.dependencies.begin(), manifest.dependencies.end(),
                                          [](const std::string& dependency) { return !isBlank(dependency); }))) {
        errors.push_back("duplicate_patch_dependency");
    }
    for (const auto& dependency : manifest.dependencies) {
        if (isBlank(dependency)) {
            continue;
        }
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
        {"changed_data", normalizedStrings(manifest.changed_data)},
        {"changed_assets", normalizedStrings(manifest.changed_assets)},
        {"dependencies", normalizedStrings(manifest.dependencies)},
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

    const auto changedData = stringArrayFromJson(json, "changed_data");
    const auto changedAssets = stringArrayFromJson(json, "changed_assets");
    const auto dependencies = stringArrayFromJson(json, "dependencies");
    if (!changedData.has_value() || !changedAssets.has_value() || !dependencies.has_value()) {
        return {};
    }
    manifest.changed_data = *changedData;
    manifest.changed_assets = *changedAssets;
    manifest.dependencies = *dependencies;
    return manifest;
}

} // namespace urpg::exporting
