#include "engine/core/export/desktop_release_contract.h"

#include "engine/core/security/release_security_audit.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <set>

namespace urpg::exporting {
namespace {

constexpr std::array<DesktopPackageRole, 5> kRequiredRoles = {
    DesktopPackageRole::Executable, DesktopPackageRole::RuntimeLibrary, DesktopPackageRole::Content,
    DesktopPackageRole::Licenses, DesktopPackageRole::Notices};
constexpr std::array<ReleaseTarget, 4> kTargets = {
    ReleaseTarget::Windows, ReleaseTarget::MacOS, ReleaseTarget::Linux, ReleaseTarget::Web};

bool validHash(const std::string& value) {
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](const unsigned char character) {
        return std::isxdigit(character) != 0;
    });
}

} // namespace

const char* desktopPackageRoleName(const DesktopPackageRole role) {
    switch (role) {
    case DesktopPackageRole::Executable: return "executable";
    case DesktopPackageRole::RuntimeLibrary: return "runtime_library";
    case DesktopPackageRole::Content: return "content";
    case DesktopPackageRole::Licenses: return "licenses";
    case DesktopPackageRole::Notices: return "notices";
    }
    return "unknown";
}

const char* releaseTargetName(const ReleaseTarget target) {
    switch (target) {
    case ReleaseTarget::Windows: return "windows";
    case ReleaseTarget::MacOS: return "macos";
    case ReleaseTarget::Linux: return "linux";
    case ReleaseTarget::Web: return "web";
    }
    return "unknown";
}

const char* targetSupportStatusName(const TargetSupportStatus status) {
    switch (status) {
    case TargetSupportStatus::Qualified: return "qualified";
    case TargetSupportStatus::Experimental: return "experimental";
    case TargetSupportStatus::Unsupported: return "unsupported";
    case TargetSupportStatus::Unavailable: return "unavailable";
    }
    return "unknown";
}

DesktopReleaseContractResult DesktopReleaseContract::evaluate(
    const std::vector<DesktopInstallPolicy>& policies, const std::vector<TargetSupportRecord>& targets) const {
    DesktopReleaseContractResult result;
    std::set<std::string> platforms;
    for (const auto& policy : policies) {
        if (policy.platform.empty() || !platforms.insert(policy.platform).second) {
            result.diagnostics.push_back("desktop_policy_invalid_or_duplicate:" + policy.platform);
            continue;
        }
        std::set<DesktopPackageRole> roles;
        std::set<std::string> paths;
        for (const auto& entry : policy.entries) {
            roles.insert(entry.role);
            if (urpg::security::ReleaseSecurityAudit::isUnsafeRelativePath(entry.relative_path) ||
                !paths.insert(entry.relative_path).second || !validHash(entry.sha256)) {
                result.diagnostics.push_back("desktop_package_entry_invalid:" + policy.platform + ":" + entry.relative_path);
            }
        }
        for (const auto role : kRequiredRoles) {
            if (!roles.contains(role)) result.diagnostics.push_back("desktop_package_role_missing:" + policy.platform + ":" + desktopPackageRoleName(role));
        }
        if (!policy.install_supported || !policy.uninstall_supported || !policy.portable_mode_defined ||
            policy.installed_save_location.empty() || policy.portable_save_location.empty() || !policy.repair_supported ||
            !policy.clean_machine_smoke_passed) {
            result.diagnostics.push_back("desktop_lifecycle_incomplete:" + policy.platform);
        }
    }

    std::set<ReleaseTarget> coveredTargets;
    for (const auto& target : targets) {
        const auto name = std::string(releaseTargetName(target.target));
        if (!coveredTargets.insert(target.target).second) result.diagnostics.push_back("target_support_duplicate:" + name);
        if (target.ui_status != target.cli_status || target.ui_status != target.docs_status ||
            target.ui_status != target.readiness_status) result.diagnostics.push_back("target_support_disagrees:" + name);
        if (target.ui_status == TargetSupportStatus::Qualified && target.evidence.empty()) {
            result.diagnostics.push_back("target_qualification_evidence_missing:" + name);
        }
        if (target.target == ReleaseTarget::Web && target.ui_status != TargetSupportStatus::Unsupported &&
            target.ui_status != TargetSupportStatus::Unavailable) {
            result.diagnostics.push_back("target_web_claim_unsupported:" + name);
        }
    }
    for (const auto target : kTargets) {
        if (!coveredTargets.contains(target)) result.diagnostics.push_back(std::string("target_support_missing:") + releaseTargetName(target));
    }
    result.complete = !policies.empty() && coveredTargets.size() == kTargets.size();
    result.release_ready = result.complete && result.diagnostics.empty();
    return result;
}

} // namespace urpg::exporting
