#include "engine/core/analytics/analytics_endpoint_profile.h"

#include <utility>

namespace urpg::analytics {
namespace {

std::vector<std::string> stringsFromJsonArray(const nlohmann::json& json) {
    std::vector<std::string> values;
    if (!json.is_array()) {
        return values;
    }
    for (const auto& value : json) {
        if (value.is_string()) {
            values.push_back(value.get<std::string>());
        }
    }
    return values;
}

bool isSupportedProviderId(const std::string& providerId) {
    return providerId.empty() || providerId == "disabled" || providerId == "local_jsonl" ||
           providerId == "http_json" || providerId == "command";
}

} // namespace

const char* analyticsEndpointModeName(AnalyticsEndpointMode mode) {
    switch (mode) {
    case AnalyticsEndpointMode::Disabled:
        return "disabled";
    case AnalyticsEndpointMode::LocalJsonl:
        return "local_jsonl";
    case AnalyticsEndpointMode::HttpJson:
        return "http_json";
    }
    return "disabled";
}

AnalyticsEndpointMode analyticsEndpointModeFromString(const std::string& value) {
    if (value == "local_jsonl") {
        return AnalyticsEndpointMode::LocalJsonl;
    }
    if (value == "http_json") {
        return AnalyticsEndpointMode::HttpJson;
    }
    return AnalyticsEndpointMode::Disabled;
}

nlohmann::json AnalyticsEndpointProfile::toJson() const {
    return {
        {"schema", "urpg.analytics_endpoint_profile.v1"},
        {"profileId", profileId},
        {"providerId", providerId},
        {"mode", analyticsEndpointModeName(mode)},
        {"localJsonlPath", localJsonlPath.generic_string()},
        {"url", url},
        {"headers", headers},
        {"bearerToken", bearerToken.empty() ? "" : "[configured]"},
        {"curlExecutable", curlExecutable},
        {"credentialSourceCategory", credentialSourceCategory},
        {"credentialsRequired", credentialsRequired},
        {"reviewed", reviewed},
        {"reviewedBy", reviewedBy},
        {"reviewedAt", reviewedAt},
        {"lastTestResult", lastTestResult},
        {"privacyReview",
         {
             {"approved", privacyReview.approved},
             {"reviewer", privacyReview.reviewer},
             {"approvedAt", privacyReview.approvedAt},
             {"notes", privacyReview.notes},
             {"dataCategories", privacyReview.dataCategories},
         }},
    };
}

AnalyticsEndpointProfile AnalyticsEndpointProfile::fromJson(const nlohmann::json& json) {
    AnalyticsEndpointProfile profile;
    if (!json.is_object()) {
        return profile;
    }

    profile.profileId = json.value("profileId", "");
    profile.providerId = json.value("providerId", "");
    profile.mode = analyticsEndpointModeFromString(json.value("mode", std::string("disabled")));
    profile.localJsonlPath = json.value("localJsonlPath", "");
    profile.url = json.value("url", "");
    profile.curlExecutable = json.value("curlExecutable", std::string("curl"));

    if (const auto headers = json.find("headers"); headers != json.end() && headers->is_object()) {
        for (auto it = headers->begin(); it != headers->end(); ++it) {
            if (it.value().is_string()) {
                profile.headers[it.key()] = it.value().get<std::string>();
            }
        }
    }
    profile.bearerToken = json.value("bearerToken", "");
    profile.credentialSourceCategory = json.value("credentialSourceCategory", std::string("not_required"));
    profile.credentialsRequired = json.value("credentialsRequired", false);
    profile.reviewed = json.value("reviewed", false);
    profile.reviewedBy = json.value("reviewedBy", "");
    profile.reviewedAt = json.value("reviewedAt", "");
    profile.lastTestResult = json.value("lastTestResult", std::string("not_run"));

    if (const auto review = json.find("privacyReview"); review != json.end() && review->is_object()) {
        profile.privacyReview.approved = review->value("approved", false);
        profile.privacyReview.reviewer = review->value("reviewer", "");
        profile.privacyReview.approvedAt = review->value("approvedAt", "");
        profile.privacyReview.notes = review->value("notes", "");
        profile.privacyReview.dataCategories =
            stringsFromJsonArray(review->value("dataCategories", nlohmann::json::array()));
    }

    return profile;
}

std::vector<AnalyticsEndpointProfileDiagnostic> validateAnalyticsEndpointProfile(
    const AnalyticsEndpointProfile& profile) {
    std::vector<AnalyticsEndpointProfileDiagnostic> diagnostics;
    if (profile.profileId.empty()) {
        diagnostics.push_back({"missing_profile_id", "Analytics endpoint profile requires a profile id.", ""});
    }
    if (!isSupportedProviderId(profile.providerId)) {
        diagnostics.push_back({"unsupported_provider", "Analytics endpoint profile provider is not supported.",
                               profile.profileId});
    }

    if (profile.mode == AnalyticsEndpointMode::LocalJsonl && profile.localJsonlPath.empty()) {
        diagnostics.push_back(
            {"missing_local_export_path", "Local analytics endpoint profile requires a JSONL export path.",
             profile.profileId});
    }

    if (profile.mode == AnalyticsEndpointMode::HttpJson) {
        if (profile.url.empty()) {
            diagnostics.push_back(
                {"missing_endpoint_url", "HTTP analytics endpoint profile requires a URL.", profile.profileId});
        }
        if (profile.credentialsRequired && profile.bearerToken.empty()) {
            diagnostics.push_back(
                {"missing_credentials", "HTTP analytics endpoint profile requires configured credentials.",
                 profile.profileId});
        }
        if (!profile.privacyReview.approved) {
            diagnostics.push_back({"privacy_review_required",
                                   "HTTP analytics endpoint profile requires approved privacy review evidence.",
                                   profile.profileId});
        }
        if (profile.privacyReview.reviewer.empty()) {
            diagnostics.push_back(
                {"missing_privacy_reviewer", "Privacy review evidence requires a reviewer.", profile.profileId});
        }
        if (profile.privacyReview.dataCategories.empty()) {
            diagnostics.push_back({"missing_data_categories",
                                   "Privacy review evidence requires declared analytics data categories.",
                                   profile.profileId});
        }
    }

    return diagnostics;
}

urpg::release::ProviderProfileStatus analyticsEndpointProfileStatus(
    const AnalyticsEndpointProfile& profile) {
    urpg::release::ProviderProfileStatus status;
    status.credentialSourceCategory =
        profile.credentialSourceCategory.empty() ? "not_required" : profile.credentialSourceCategory;
    status.lastTestResult = profile.lastTestResult.empty() ? "not_run" : profile.lastTestResult;

    if (profile.mode == AnalyticsEndpointMode::Disabled ||
        (!profile.providerId.empty() && profile.providerId == "disabled")) {
        status.status = "disabled";
        status.credentialSourceCategory = "none";
        return status;
    }
    if (!isSupportedProviderId(profile.providerId)) {
        status.status = "unsupported_provider";
        return status;
    }
    if (profile.mode == AnalyticsEndpointMode::LocalJsonl) {
        status.status = "dry_run";
        status.credentialSourceCategory = "none";
        return status;
    }
    if (profile.mode == AnalyticsEndpointMode::HttpJson &&
        profile.credentialsRequired && profile.bearerToken.empty()) {
        status.status = "missing_credentials";
        return status;
    }

    const bool reviewed = profile.reviewed || profile.privacyReview.approved;
    status.reviewStatus = reviewed ? "reviewed" : "unreviewed";
    status.status = reviewed ? "configured_reviewed" : "configured_unreviewed";
    status.releasePackagingAllowed = reviewed && (status.lastTestResult == "pass" || status.lastTestResult == "not_run");
    return status;
}

AnalyticsEndpointProfileApplyResult applyAnalyticsEndpointProfile(
    AnalyticsUploader& uploader, const AnalyticsEndpointProfile& profile) {
    AnalyticsEndpointProfileApplyResult result;
    result.diagnostics = validateAnalyticsEndpointProfile(profile);
    if (!result.diagnostics.empty()) {
        result.uploadMode = uploader.uploadMode();
        return result;
    }

    switch (profile.mode) {
    case AnalyticsEndpointMode::Disabled:
        uploader.setUploadHandler(nullptr);
        break;
    case AnalyticsEndpointMode::LocalJsonl:
        uploader.setLocalJsonlExportPath(profile.localJsonlPath);
        break;
    case AnalyticsEndpointMode::HttpJson: {
        AnalyticsUploadEndpoint endpoint;
        endpoint.url = profile.url;
        endpoint.headers = profile.headers;
        endpoint.bearerToken = profile.bearerToken;
        endpoint.curlExecutable = profile.curlExecutable;
        uploader.setHttpJsonEndpoint(std::move(endpoint));
        break;
    }
    }

    result.applied = true;
    result.uploadMode = uploader.uploadMode();
    return result;
}

nlohmann::json analyticsEndpointProfileApplyResultToJson(const AnalyticsEndpointProfileApplyResult& result) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : result.diagnostics) {
        diagnostics.push_back({
            {"code", diagnostic.code},
            {"message", diagnostic.message},
            {"target", diagnostic.target},
        });
    }

    return {
        {"applied", result.applied},
        {"uploadMode", result.uploadMode},
        {"diagnostics", diagnostics},
    };
}

} // namespace urpg::analytics
