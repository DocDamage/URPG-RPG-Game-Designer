#pragma once

#include "engine/core/analytics/analytics_uploader.h"
#include "engine/core/release/provider_profile_status.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::analytics {

enum class AnalyticsEndpointMode { Disabled, LocalJsonl, HttpJson };

struct AnalyticsPrivacyReviewEvidence {
    bool approved = false;
    std::string reviewer;
    std::string approvedAt;
    std::string notes;
    std::vector<std::string> dataCategories;
};

struct AnalyticsEndpointProfile {
    std::string profileId;
    std::string providerId;
    AnalyticsEndpointMode mode = AnalyticsEndpointMode::Disabled;
    std::filesystem::path localJsonlPath;
    std::string url;
    std::unordered_map<std::string, std::string> headers;
    std::string bearerToken;
    std::string curlExecutable = "curl";
    std::string credentialSourceCategory = "not_required";
    bool credentialsRequired = false;
    bool reviewed = false;
    std::string reviewedBy;
    std::string reviewedAt;
    std::string lastTestResult = "not_run";
    AnalyticsPrivacyReviewEvidence privacyReview;

    nlohmann::json toJson() const;
    static AnalyticsEndpointProfile fromJson(const nlohmann::json& json);
};

struct AnalyticsEndpointProfileDiagnostic {
    std::string code;
    std::string message;
    std::string target;
};

struct AnalyticsEndpointProfileApplyResult {
    bool applied = false;
    std::string uploadMode = "disabled";
    std::vector<AnalyticsEndpointProfileDiagnostic> diagnostics;
};

const char* analyticsEndpointModeName(AnalyticsEndpointMode mode);
AnalyticsEndpointMode analyticsEndpointModeFromString(const std::string& value);
std::vector<AnalyticsEndpointProfileDiagnostic> validateAnalyticsEndpointProfile(
    const AnalyticsEndpointProfile& profile);
urpg::release::ProviderProfileStatus analyticsEndpointProfileStatus(
    const AnalyticsEndpointProfile& profile);
AnalyticsEndpointProfileApplyResult applyAnalyticsEndpointProfile(
    AnalyticsUploader& uploader, const AnalyticsEndpointProfile& profile);
nlohmann::json analyticsEndpointProfileApplyResultToJson(const AnalyticsEndpointProfileApplyResult& result);

} // namespace urpg::analytics
