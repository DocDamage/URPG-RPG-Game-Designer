#include "engine/core/export/creator_package_manifest.h"

#include <algorithm>
#include <cctype>

namespace urpg::exporting {

namespace {

bool isBlank(const std::string& value) {
    return std::all_of(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch) != 0; });
}

std::vector<std::string> normalizedDependencies(const std::vector<std::string>& dependencies) {
    std::vector<std::string> normalized;
    for (const auto& dependency : dependencies) {
        if (!isBlank(dependency)) {
            normalized.push_back(dependency);
        }
    }
    std::sort(normalized.begin(), normalized.end());
    normalized.erase(std::unique(normalized.begin(), normalized.end()), normalized.end());
    return normalized;
}

} // namespace

std::vector<std::string> ValidateCreatorPackageManifest(const CreatorPackageManifest& manifest) {
    std::vector<std::string> errors;
    if (isBlank(manifest.id)) {
        errors.push_back("missing_package_id");
    }
    if (isBlank(manifest.type)) {
        errors.push_back("missing_package_type");
    }
    if (isBlank(manifest.license_evidence)) {
        errors.push_back("missing_license_evidence");
    }
    if (isBlank(manifest.compatibility_target)) {
        errors.push_back("missing_compatibility_target");
    }
    if (isBlank(manifest.validation_summary)) {
        errors.push_back("missing_validation_summary");
    }
    if (std::any_of(manifest.dependencies.begin(), manifest.dependencies.end(), isBlank)) {
        errors.push_back("invalid_dependency");
    }
    if (normalizedDependencies(manifest.dependencies).size() !=
        static_cast<size_t>(std::count_if(manifest.dependencies.begin(), manifest.dependencies.end(),
                                          [](const std::string& dependency) { return !isBlank(dependency); }))) {
        errors.push_back("duplicate_dependency");
    }
    return errors;
}

nlohmann::json CreatorPackageManifestToJson(const CreatorPackageManifest& manifest) {
    return {
        {"id", manifest.id},
        {"type", manifest.type},
        {"license_evidence", manifest.license_evidence},
        {"compatibility_target", manifest.compatibility_target},
        {"dependencies", normalizedDependencies(manifest.dependencies)},
        {"validation_summary", manifest.validation_summary},
    };
}

} // namespace urpg::exporting
