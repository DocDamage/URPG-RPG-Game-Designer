#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::mod {

enum class ModSdkSampleSeverity {
    Warning,
    Error
};

struct ModSdkSampleIssue {
    ModSdkSampleSeverity severity = ModSdkSampleSeverity::Error;
    std::string code;
    std::string detail;
};

struct ModSdkSampleValidationResult {
    bool passed = false;
    std::vector<ModSdkSampleIssue> issues;
    nlohmann::json manifest = nlohmann::json::object();
};

/**
 * @brief Validates the checked-in SDK sample as a local, permission-bounded mod contract.
 */
class ModSdkSampleValidator {
public:
    ModSdkSampleValidationResult validateSample(const std::filesystem::path& sampleRoot) const;

private:
    bool isAllowedPermission(const std::string& permission) const;
};

} // namespace urpg::mod
