#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace urpg::release {

struct ProviderProfileStatus {
    std::string status = "disabled";
    std::string credentialSourceCategory = "none";
    std::string reviewStatus = "not_required";
    std::string lastTestResult = "not_run";
    bool releasePackagingAllowed = false;
};

inline nlohmann::json providerProfileStatusToJson(const ProviderProfileStatus& status) {
    return {
        {"status", status.status},
        {"credentialSourceCategory", status.credentialSourceCategory},
        {"reviewStatus", status.reviewStatus},
        {"lastTestResult", status.lastTestResult},
        {"releasePackagingAllowed", status.releasePackagingAllowed},
    };
}

} // namespace urpg::release
