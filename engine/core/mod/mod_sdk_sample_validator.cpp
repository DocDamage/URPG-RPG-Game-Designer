#include "engine/core/mod/mod_sdk_sample_validator.h"

#include <fstream>
#include <unordered_set>

namespace urpg::mod {

ModSdkSampleValidationResult ModSdkSampleValidator::validateSample(const std::filesystem::path& sampleRoot) const {
    ModSdkSampleValidationResult result;
    const auto manifestPath = sampleRoot / "manifest.json";
    const auto docsPath = sampleRoot / "README.md";
    const auto diagnosticsPath = sampleRoot / "expected_diagnostics.json";

    if (!std::filesystem::exists(manifestPath)) {
        result.issues.push_back({ModSdkSampleSeverity::Error, "missing_manifest", "manifest.json is required."});
    } else {
        std::ifstream in(manifestPath);
        try {
            result.manifest = nlohmann::json::parse(in);
        } catch (const nlohmann::json::exception& e) {
            result.issues.push_back({ModSdkSampleSeverity::Error, "invalid_manifest_json", e.what()});
        }
    }

    if (!std::filesystem::exists(docsPath)) {
        result.issues.push_back({ModSdkSampleSeverity::Error, "missing_docs", "README.md is required."});
    }
    if (!std::filesystem::exists(diagnosticsPath)) {
        result.issues.push_back(
            {ModSdkSampleSeverity::Error, "missing_expected_diagnostics", "expected_diagnostics.json is required."});
    }

    if (!result.manifest.empty()) {
        for (const auto* field : {"id", "name", "version", "entryPoint"}) {
            if (!result.manifest.contains(field) || !result.manifest[field].is_string() ||
                result.manifest[field].get<std::string>().empty()) {
                result.issues.push_back(
                    {ModSdkSampleSeverity::Error, std::string("missing_") + field, std::string(field) + " is required."});
            }
        }

        if (!result.manifest.contains("permissions") || !result.manifest["permissions"].is_array()) {
            result.issues.push_back(
                {ModSdkSampleSeverity::Error, "missing_permissions", "permissions array is required."});
        } else {
            for (const auto& permission : result.manifest["permissions"]) {
                if (!permission.is_string()) {
                    result.issues.push_back({ModSdkSampleSeverity::Error,
                                             "invalid_permission",
                                             "Permission entries must be strings."});
                    continue;
                }
                const auto value = permission.get<std::string>();
                if (!isAllowedPermission(value)) {
                    result.issues.push_back({ModSdkSampleSeverity::Error,
                                             "forbidden_permission",
                                             "Permission '" + value + "' is not allowed in the SDK sample."});
                }
            }
        }
    }

    result.passed = true;
    for (const auto& issue : result.issues) {
        if (issue.severity == ModSdkSampleSeverity::Error) {
            result.passed = false;
            break;
        }
    }
    return result;
}

bool ModSdkSampleValidator::isAllowedPermission(const std::string& permission) const {
    static const std::unordered_set<std::string> allowed = {
        "read_project_data",
        "register_commands",
        "emit_diagnostics",
    };
    return allowed.find(permission) != allowed.end();
}

} // namespace urpg::mod
