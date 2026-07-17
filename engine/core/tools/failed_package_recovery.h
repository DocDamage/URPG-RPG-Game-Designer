#pragma once

#include <filesystem>
#include <string>

namespace urpg::tools {

struct FailedPackageRecoveryResult {
    bool success = false;
    std::string code;
    std::string message;
    std::filesystem::path quarantined_output;
    std::filesystem::path receipt_path;
};

class FailedPackageRecovery {
public:
    static FailedPackageRecoveryResult quarantinePartialOutput(
        const std::filesystem::path& project_root, const std::filesystem::path& partial_output,
        const std::string& package_id);
};

} // namespace urpg::tools
