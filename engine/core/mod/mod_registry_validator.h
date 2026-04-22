#pragma once

#include "engine/core/mod/mod_registry.h"

#include <string>
#include <vector>

namespace urpg::mod {

enum class ModIssueSeverity {
    Warning,
    Error
};

enum class ModIssueCategory {
    EmptyId,
    EmptyName,
    EmptyVersion,
    DuplicateDependency,
    SelfDependency,
    MissingEntryPoint
};

struct ModIssue {
    ModIssueSeverity severity = ModIssueSeverity::Error;
    ModIssueCategory category = ModIssueCategory::EmptyId;
    std::string modId;
    std::string message;
};

/**
 * @brief Validates bounded mod manifest quality rules for native registry usage.
 */
class ModRegistryValidator {
public:
    std::vector<ModIssue> validate(const std::vector<ModManifest>& manifests) const;
};

} // namespace urpg::mod
