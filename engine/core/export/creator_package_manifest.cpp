#include "engine/core/export/creator_package_manifest.h"

namespace urpg::exporting {

std::vector<std::string> ValidateCreatorPackageManifest(const CreatorPackageManifest& manifest) {
    std::vector<std::string> errors;
    if (manifest.id.empty()) {
        errors.push_back("missing_package_id");
    }
    if (manifest.type.empty()) {
        errors.push_back("missing_package_type");
    }
    if (manifest.license_evidence.empty()) {
        errors.push_back("missing_license_evidence");
    }
    if (manifest.compatibility_target.empty()) {
        errors.push_back("missing_compatibility_target");
    }
    if (manifest.validation_summary.empty()) {
        errors.push_back("missing_validation_summary");
    }
    return errors;
}

nlohmann::json CreatorPackageManifestToJson(const CreatorPackageManifest& manifest) {
    return {
        {"id", manifest.id},
        {"type", manifest.type},
        {"license_evidence", manifest.license_evidence},
        {"compatibility_target", manifest.compatibility_target},
        {"dependencies", manifest.dependencies},
        {"validation_summary", manifest.validation_summary},
    };
}

} // namespace urpg::exporting
