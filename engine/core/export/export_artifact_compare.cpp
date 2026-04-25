#include "engine/core/export/export_artifact_compare.h"

#include <algorithm>
#include <map>

namespace urpg::exporting {

namespace {

std::map<std::string, nlohmann::json> entriesByPath(const nlohmann::json& manifest) {
    std::map<std::string, nlohmann::json> entries;
    if (!manifest.contains("entries") || !manifest["entries"].is_array()) {
        return entries;
    }
    for (const auto& entry : manifest["entries"]) {
        if (entry.is_object() && entry.contains("path") && entry["path"].is_string()) {
            entries[entry["path"].get<std::string>()] = entry;
        }
    }
    return entries;
}

bool isSchemaPath(const std::string& path) {
    return path.find("schema") != std::string::npos || path.ends_with(".schema.json");
}

} // namespace

ExportArtifactCompareResult CompareExportArtifacts(const nlohmann::json& baseline_manifest,
                                                   const nlohmann::json& candidate_manifest) {
    ExportArtifactCompareResult result;
    const auto baseline_entries = entriesByPath(baseline_manifest);
    const auto candidate_entries = entriesByPath(candidate_manifest);

    for (const auto& [path, base_entry] : baseline_entries) {
        const auto candidate_it = candidate_entries.find(path);
        if (candidate_it == candidate_entries.end()) {
            result.missing_files.push_back(path);
            continue;
        }
        if (base_entry.value("integrityTag", "") != candidate_it->second.value("integrityTag", "") ||
            base_entry.value("rawSize", 0u) != candidate_it->second.value("rawSize", 0u)) {
            if (isSchemaPath(path)) {
                result.changed_schemas.push_back(path);
            } else {
                result.changed_assets.push_back(path);
            }
        }
    }

    result.signature_changed =
        baseline_manifest.value("bundleSignature", "") != candidate_manifest.value("bundleSignature", "");
    auto baseline_view = baseline_manifest;
    auto candidate_view = candidate_manifest;
    baseline_view.erase("bundleSignature");
    candidate_view.erase("bundleSignature");
    result.manifest_changed = baseline_view != candidate_view;

    return result;
}

nlohmann::json ExportArtifactCompareResultToJson(const ExportArtifactCompareResult& result) {
    return {
        {"changed_assets", result.changed_assets},
        {"changed_schemas", result.changed_schemas},
        {"missing_files", result.missing_files},
        {"signature_changed", result.signature_changed},
        {"manifest_changed", result.manifest_changed},
    };
}

} // namespace urpg::exporting
