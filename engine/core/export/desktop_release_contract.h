#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::exporting {

enum class DesktopPackageRole : uint8_t { Executable, RuntimeLibrary, Content, Licenses, Notices };
enum class ReleaseTarget : uint8_t { Windows, MacOS, Linux, Web };
enum class TargetSupportStatus : uint8_t { Qualified, Experimental, Unsupported, Unavailable };

struct DesktopPackageEntry {
    DesktopPackageRole role = DesktopPackageRole::Executable;
    std::string relative_path;
    std::string sha256;
};

struct DesktopInstallPolicy {
    std::string platform;
    std::vector<DesktopPackageEntry> entries;
    bool install_supported = false;
    bool uninstall_supported = false;
    bool portable_mode_defined = false;
    std::string installed_save_location;
    std::string portable_save_location;
    bool repair_supported = false;
    bool clean_machine_smoke_passed = false;
};

struct TargetSupportRecord {
    ReleaseTarget target = ReleaseTarget::Windows;
    TargetSupportStatus ui_status = TargetSupportStatus::Unavailable;
    TargetSupportStatus cli_status = TargetSupportStatus::Unavailable;
    TargetSupportStatus docs_status = TargetSupportStatus::Unavailable;
    TargetSupportStatus readiness_status = TargetSupportStatus::Unavailable;
    std::string evidence;
};

struct DesktopReleaseContractResult {
    bool complete = false;
    bool release_ready = false;
    std::vector<std::string> diagnostics;
};

class DesktopReleaseContract {
public:
    DesktopReleaseContractResult evaluate(const std::vector<DesktopInstallPolicy>& policies,
                                          const std::vector<TargetSupportRecord>& targets) const;
};

const char* desktopPackageRoleName(DesktopPackageRole role);
const char* releaseTargetName(ReleaseTarget target);
const char* targetSupportStatusName(TargetSupportStatus status);

} // namespace urpg::exporting
