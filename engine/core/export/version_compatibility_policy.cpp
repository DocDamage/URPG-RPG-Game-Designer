#include "engine/core/export/version_compatibility_policy.h"

#include "engine/core/semver.h"

#include <array>
#include <set>

namespace urpg::exporting {
namespace {

constexpr std::array<CompatibilitySurface, 6> kSurfaces = {
    CompatibilitySurface::ProjectSchema, CompatibilitySurface::SaveData, CompatibilitySurface::Runtime,
    CompatibilitySurface::PluginMod, CompatibilitySurface::Package, CompatibilitySurface::UpdateChannel};

} // namespace

const char* compatibilitySurfaceName(const CompatibilitySurface surface) {
    switch (surface) {
    case CompatibilitySurface::ProjectSchema: return "project_schema";
    case CompatibilitySurface::SaveData: return "save_data";
    case CompatibilitySurface::Runtime: return "runtime";
    case CompatibilitySurface::PluginMod: return "plugin_mod";
    case CompatibilitySurface::Package: return "package";
    case CompatibilitySurface::UpdateChannel: return "update_channel";
    }
    return "unknown";
}

VersionCompatibilityResult VersionCompatibilityPolicy::evaluate(
    const std::vector<VersionCompatibilityRule>& rules) const {
    VersionCompatibilityResult result;
    std::set<CompatibilitySurface> covered;
    for (const auto& rule : rules) {
        const auto surface = std::string(compatibilitySurfaceName(rule.surface));
        if (!covered.insert(rule.surface).second) result.diagnostics.push_back("version_rule_duplicate:" + surface);
        urpg::SemVer current;
        urpg::SemVer minimum;
        if (!urpg::SemVer::TryParse(rule.current_version, current) ||
            !urpg::SemVer::TryParse(rule.minimum_readable_version, minimum) || current < minimum) {
            result.diagnostics.push_back("version_range_invalid:" + surface);
        }
        if (rule.rollback_boundary.empty()) result.diagnostics.push_back("version_rollback_boundary_missing:" + surface);
        if (rule.downgrade_message.empty()) result.diagnostics.push_back("version_downgrade_message_missing:" + surface);
        if (!rule.n_minus_one_upgrade_passed) result.diagnostics.push_back("version_n_minus_one_unqualified:" + surface);
        if (!rule.failed_update_recovery_passed) result.diagnostics.push_back("version_failed_update_recovery_missing:" + surface);
    }
    for (const auto surface : kSurfaces) {
        if (!covered.contains(surface)) result.diagnostics.push_back(std::string("version_surface_missing:") + compatibilitySurfaceName(surface));
    }
    result.complete = covered.size() == kSurfaces.size();
    result.qualified = result.complete && result.diagnostics.empty();
    return result;
}

} // namespace urpg::exporting
