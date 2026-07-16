#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::exporting {

enum class CompatibilitySurface : uint8_t { ProjectSchema, SaveData, Runtime, PluginMod, Package, UpdateChannel };

struct VersionCompatibilityRule {
    CompatibilitySurface surface = CompatibilitySurface::ProjectSchema;
    std::string current_version;
    std::string minimum_readable_version;
    std::string rollback_boundary;
    std::string downgrade_message;
    bool n_minus_one_upgrade_passed = false;
    bool failed_update_recovery_passed = false;
};

struct VersionCompatibilityResult {
    bool complete = false;
    bool qualified = false;
    std::vector<std::string> diagnostics;
};

class VersionCompatibilityPolicy {
public:
    VersionCompatibilityResult evaluate(const std::vector<VersionCompatibilityRule>& rules) const;
};

const char* compatibilitySurfaceName(CompatibilitySurface surface);

} // namespace urpg::exporting
