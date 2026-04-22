#include "engine/core/mod/mod_registry_validator.h"

#include <unordered_set>

namespace urpg::mod {

std::vector<ModIssue> ModRegistryValidator::validate(const std::vector<ModManifest>& manifests) const {
    std::vector<ModIssue> issues;

    for (const auto& manifest : manifests) {
        if (manifest.id.empty()) {
            issues.push_back({
                ModIssueSeverity::Error,
                ModIssueCategory::EmptyId,
                manifest.id,
                "Mod id is required.",
            });
        }

        if (manifest.name.empty()) {
            issues.push_back({
                ModIssueSeverity::Error,
                ModIssueCategory::EmptyName,
                manifest.id,
                "Mod name is required.",
            });
        }

        if (manifest.version.empty()) {
            issues.push_back({
                ModIssueSeverity::Error,
                ModIssueCategory::EmptyVersion,
                manifest.id,
                "Mod version is required.",
            });
        }

        if (manifest.entryPoint.empty()) {
            issues.push_back({
                ModIssueSeverity::Warning,
                ModIssueCategory::MissingEntryPoint,
                manifest.id,
                "Mod entry point is empty.",
            });
        }

        std::unordered_set<std::string> seenDependencies;
        for (const auto& dependency : manifest.dependencies) {
            if (dependency == manifest.id && !manifest.id.empty()) {
                issues.push_back({
                    ModIssueSeverity::Error,
                    ModIssueCategory::SelfDependency,
                    manifest.id,
                    "Mod depends on itself.",
                });
            }

            if (!seenDependencies.insert(dependency).second) {
                issues.push_back({
                    ModIssueSeverity::Warning,
                    ModIssueCategory::DuplicateDependency,
                    manifest.id,
                    "Mod dependency '" + dependency + "' is duplicated.",
                });
            }
        }
    }

    return issues;
}

} // namespace urpg::mod
