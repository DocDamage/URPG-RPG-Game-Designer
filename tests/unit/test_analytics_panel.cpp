#include <catch2/catch_test_macros.hpp>

#include "editor/analytics/analytics_panel.h"
#include "engine/core/analytics/analytics_dispatcher.h"
#include "engine/core/analytics/analytics_endpoint_profile.h"
#include "engine/core/analytics/analytics_privacy_controller.h"
#include "engine/core/analytics/analytics_uploader.h"

#include <filesystem>
#include <string>
#include <vector>

using urpg::analytics::AnalyticsDispatcher;
using urpg::analytics::AnalyticsEndpointMode;
using urpg::analytics::AnalyticsEndpointProfile;
using urpg::analytics::AnalyticsPrivacyController;
using urpg::analytics::AnalyticsUploader;
using urpg::analytics::ConsentState;
using urpg::analytics::RetentionPolicy;
using urpg::editor::AnalyticsPanel;

TEST_CASE("AnalyticsPanel empty snapshot when no dispatcher bound", "[analytics][editor][panel]") {
    AnalyticsPanel panel;
    panel.render();
    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["optIn"] == false);
    REQUIRE(snapshot["sessionEventCount"] == 0);
    REQUIRE(snapshot["allowedCategories"].is_array());
    REQUIRE(snapshot["validationIssueCount"] == 0);
    REQUIRE(snapshot["validationIssues"].is_array());
    REQUIRE(snapshot["errorCount"] == 0);
    REQUIRE(snapshot["warningCount"] == 0);
    REQUIRE(snapshot["recentEvents"].is_array());
    REQUIRE(snapshot["recentEvents"].empty());
    REQUIRE(snapshot["dispatcherBound"] == false);
    REQUIRE(snapshot["actions"]["setOptIn"] == false);
    REQUIRE(snapshot["actionDetails"]["setOptIn"]["enabled"] == false);
    REQUIRE(snapshot["actionDetails"]["setOptIn"]["disabledReason"] == "No analytics dispatcher is bound.");
    REQUIRE(snapshot["actionDetails"]["clearQueue"]["disabledReason"] == "No analytics dispatcher is bound.");
    REQUIRE(snapshot["actionDetails"]["applyEndpointProfile"]["disabledReason"] == "No analytics uploader is bound.");
    REQUIRE(snapshot["uploadStatus"] == "disabled");
    REQUIRE(snapshot["disabledUploadMessage"] == "No analytics dispatcher is bound.");
    REQUIRE(snapshot["statusMessages"].size() == 2);
    REQUIRE(snapshot["statusMessages"][0] == "No analytics dispatcher is bound; analytics actions are disabled.");
    REQUIRE(snapshot["statusMessages"][1] == "No analytics privacy controller is bound.");
}

TEST_CASE("AnalyticsPanel snapshot reflects events after dispatch", "[analytics][editor][panel]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.setAllowedCategories({"editor_category"});
    dispatcher.dispatchEvent("editor_event", "editor_category");

    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["optIn"] == true);
    REQUIRE(snapshot["sessionEventCount"] == 1);
    REQUIRE(snapshot["allowedCategories"].size() == 1);
    REQUIRE(snapshot["allowedCategories"][0] == "editor_category");
    REQUIRE(snapshot["validationIssueCount"] == 0);
    REQUIRE(snapshot["validationIssues"].empty());
    REQUIRE(snapshot["recentEvents"].size() == 1);
    REQUIRE(snapshot["recentEvents"][0]["eventName"] == "editor_event");
    REQUIRE(snapshot["recentEvents"][0]["category"] == "editor_category");
    REQUIRE(snapshot["queuedEventCount"] == 1);
    REQUIRE(snapshot["actions"]["clearQueue"] == true);
    REQUIRE(snapshot["actionDetails"]["clearQueue"]["enabled"] == true);
    REQUIRE(snapshot["statusMessages"].size() == 3);
    REQUIRE(snapshot["statusMessages"][0] ==
            "No analytics privacy controller is bound; dispatcher opt-in is used as fallback.");
    REQUIRE(snapshot["statusMessages"][1] == "No analytics endpoint profile is bound.");
    REQUIRE(snapshot["statusMessages"][2] == "No analytics uploader is bound; uploads are disabled.");
}

TEST_CASE("AnalyticsPanel opt-in toggle reflected in snapshot", "[analytics][editor][panel]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(false);
    dispatcher.dispatchEvent("hidden_event", "hidden_category");

    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["optIn"] == false);
    REQUIRE(snapshot["sessionEventCount"] == 0);
    REQUIRE(snapshot["recentEvents"].empty());

    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("visible_event", "visible_category");
    panel.render();

    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["optIn"] == true);
    REQUIRE(snapshot["sessionEventCount"] == 1);
    REQUIRE(snapshot["recentEvents"].size() == 1);
}

TEST_CASE("AnalyticsPanel surfaces validator-backed diagnostics", "[analytics][editor][panel]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.setAllowedCategories({"combat", "ui"});
    dispatcher.dispatchEvent("combat_start", "debug", {{"source", ""}});
    dispatcher.dispatchEvent("", "");

    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["validationIssueCount"] == 4);
    REQUIRE(snapshot["errorCount"] == 3);
    REQUIRE(snapshot["warningCount"] == 1);
    REQUIRE(snapshot["validationIssues"].size() == 4);
    REQUIRE(snapshot["validationIssues"][0]["category"] == "disallowed_category");
    REQUIRE(snapshot["validationIssues"][1]["category"] == "empty_parameter_value");
    REQUIRE(snapshot["validationIssues"][2]["category"] == "empty_event_name");
    REQUIRE(snapshot["validationIssues"][0]["severity"] == "error");
    REQUIRE(snapshot["validationIssues"][1]["severity"] == "warning");
}

TEST_CASE("AnalyticsPanel consent toggle updates dispatcher and privacy controller",
          "[analytics][editor][panel][controls]") {
    AnalyticsDispatcher dispatcher;
    AnalyticsPrivacyController privacy;
    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.bindPrivacyController(&privacy);

    REQUIRE(panel.setOptIn(true));
    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(dispatcher.isOptIn());
    REQUIRE(privacy.getConsentState() == ConsentState::Granted);
    REQUIRE(snapshot["optIn"] == true);
    REQUIRE(snapshot["privacyStatus"] == "granted");
    REQUIRE(snapshot["analyticsPermitted"] == true);
    REQUIRE(snapshot["lastAction"]["action"] == "set_opt_in");

    REQUIRE(panel.setOptIn(false));
    snapshot = panel.lastRenderSnapshot();
    REQUIRE_FALSE(dispatcher.isOptIn());
    REQUIRE(privacy.getConsentState() == ConsentState::Denied);
    REQUIRE(snapshot["privacyStatus"] == "denied");
}

TEST_CASE("AnalyticsPanel clear queue action removes buffered events without resetting session count",
          "[analytics][editor][panel][controls]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("queued", "ui");

    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["queuedEventCount"] == 1);

    REQUIRE(panel.clearQueuedEvents() == 1);
    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["queuedEventCount"] == 0);
    REQUIRE(snapshot["sessionEventCount"] == 1);
    REQUIRE(snapshot["lastAction"]["action"] == "clear_queue");
    REQUIRE(snapshot["lastAction"]["affectedEvents"] == 1);
    REQUIRE(snapshot["actions"]["clearQueue"] == false);
    REQUIRE(snapshot["actionDetails"]["clearQueue"]["enabled"] == false);
    REQUIRE(snapshot["actionDetails"]["clearQueue"]["disabledReason"] == "No queued analytics events to clear.");
}

TEST_CASE("AnalyticsPanel flush upload respects disabled states and consent", "[analytics][editor][panel][controls]") {
    AnalyticsDispatcher dispatcher;
    AnalyticsUploader uploader;
    AnalyticsPrivacyController privacy;
    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.bindUploader(&uploader);
    panel.bindPrivacyController(&privacy);

    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("queued", "ui");
    panel.render();

    REQUIRE_FALSE(panel.flushQueuedEvents());
    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["uploadStatus"] == "disabled");
    REQUIRE(snapshot["lastAction"]["message"] == "Upload disabled: no upload handler configured.");
    REQUIRE(snapshot["actionDetails"]["flushUpload"]["enabled"] == false);
    REQUIRE(snapshot["actionDetails"]["flushUpload"]["disabledReason"] ==
            "Upload disabled: no upload handler configured.");

    uploader.setUploadHandler([](const std::string&) { return true; });
    privacy.recordConsentDecision(ConsentState::Denied);
    REQUIRE_FALSE(panel.flushQueuedEvents());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["lastAction"]["message"] == "Upload disabled: analytics consent is not granted.");
    REQUIRE(snapshot["actionDetails"]["flushUpload"]["disabledReason"] ==
            "Upload disabled: analytics consent is not granted.");
}

TEST_CASE("AnalyticsPanel requires explicit consent before upload is enabled",
          "[analytics][editor][panel][privacy][consent]") {
    AnalyticsDispatcher dispatcher;
    AnalyticsUploader uploader;
    AnalyticsPrivacyController privacy;
    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.bindUploader(&uploader);
    panel.bindPrivacyController(&privacy);

    uploader.setUploadHandler([](const std::string&) { return true; });
    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("queued", "ui");
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["privacyStatus"] == "unknown");
    REQUIRE(snapshot["requiresConsentPrompt"] == true);
    REQUIRE(snapshot["analyticsPermitted"] == false);
    REQUIRE(snapshot["actions"]["flushUpload"] == false);
    REQUIRE(snapshot["disabledUploadMessage"] == "Upload disabled: analytics consent is not granted.");
    REQUIRE_FALSE(panel.flushQueuedEvents());
    REQUIRE(panel.lastRenderSnapshot()["lastAction"]["message"] ==
            "Upload disabled: analytics consent is not granted.");

    REQUIRE(panel.setOptIn(true));
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["privacyStatus"] == "granted");
    REQUIRE(snapshot["requiresConsentPrompt"] == false);
    REQUIRE(snapshot["actions"]["flushUpload"] == true);

    REQUIRE(panel.setOptIn(false));
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["privacyStatus"] == "denied");
    REQUIRE(snapshot["analyticsPermitted"] == false);
    REQUIRE(snapshot["actions"]["flushUpload"] == false);
}

TEST_CASE("AnalyticsPanel flush upload sends queued events and clears queue on success",
          "[analytics][editor][panel][controls]") {
    AnalyticsDispatcher dispatcher;
    AnalyticsUploader uploader;
    AnalyticsPrivacyController privacy;
    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.bindUploader(&uploader);
    panel.bindPrivacyController(&privacy);
    panel.setSessionId("analytics-panel-test");

    std::vector<std::string> uploadedPayloads;
    uploader.setUploadHandler([&uploadedPayloads](const std::string& payload) {
        uploadedPayloads.push_back(payload);
        return true;
    });
    privacy.recordConsentDecision(ConsentState::Granted);
    RetentionPolicy policy;
    policy.maxAgeTicks = 240;
    policy.maxEventCount = 25;
    privacy.setRetentionPolicy(policy);
    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("queued", "ui");

    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["actions"]["flushUpload"] == true);
    REQUIRE(panel.flushQueuedEvents());

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["queuedEventCount"] == 0);
    REQUIRE(snapshot["uploadStatus"] == "disabled");
    REQUIRE(snapshot["disabledUploadMessage"] == "No queued analytics events to upload.");
    REQUIRE(snapshot["retentionMaxAgeTicks"] == 240);
    REQUIRE(snapshot["retentionMaxEventCount"] == 25);
    REQUIRE(snapshot["lastAction"]["action"] == "flush_upload");
    REQUIRE(snapshot["lastAction"]["success"] == true);
    REQUIRE(snapshot["lastAction"]["affectedEvents"] == 1);
    REQUIRE(uploadedPayloads.size() == 1);
    REQUIRE(uploadedPayloads[0].find("analytics-panel-test") != std::string::npos);
}

TEST_CASE("AnalyticsPanel reports release local export handler as upload mode",
          "[analytics][editor][panel][privacy][consent]") {
    AnalyticsDispatcher dispatcher;
    AnalyticsUploader uploader;
    AnalyticsPrivacyController privacy;
    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.bindUploader(&uploader);
    panel.bindPrivacyController(&privacy);

    const auto exportRoot = std::filesystem::temp_directory_path() / "urpg_analytics_panel_local_export";
    const auto exportPath = exportRoot / "reports" / "analytics" / "editor_analytics.jsonl";
    uploader.setLocalJsonlExportPath(exportPath);
    privacy.recordConsentDecision(ConsentState::Granted);
    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("queued", "ui");

    panel.render();
    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["uploadMode"] == "local_jsonl");
    REQUIRE(snapshot["localExportPath"] == exportPath.generic_string());
    REQUIRE(snapshot["actions"]["flushUpload"] == true);

    REQUIRE(panel.flushQueuedEvents());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["lastAction"]["success"] == true);
    REQUIRE(snapshot["lastAction"]["affectedEvents"] == 1);

    std::error_code ec;
    std::filesystem::remove_all(exportRoot, ec);
}

TEST_CASE("AnalyticsPanel applies reviewed endpoint profile before upload",
          "[analytics][editor][panel][endpoint_profile][privacy]") {
    AnalyticsDispatcher dispatcher;
    AnalyticsUploader uploader;
    AnalyticsPrivacyController privacy;
    AnalyticsEndpointProfile profile;
    profile.profileId = "reviewed-http";
    profile.mode = AnalyticsEndpointMode::HttpJson;
    profile.url = "https://telemetry.example.invalid/collect";
    profile.headers["X-URPG-Channel"] = "test";
    profile.privacyReview.approved = true;
    profile.privacyReview.reviewer = "privacy";
    profile.privacyReview.dataCategories = {"editor_usage"};

    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.bindUploader(&uploader);
    panel.bindPrivacyController(&privacy);
    panel.bindEndpointProfile(&profile);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["endpointProfileBound"] == true);
    REQUIRE(snapshot["endpointProfile"]["profileId"] == "reviewed-http");
    REQUIRE(snapshot["endpointProfilePrivacyApproved"] == true);
    REQUIRE(snapshot["actions"]["applyEndpointProfile"] == true);
    REQUIRE(snapshot["actionDetails"]["applyEndpointProfile"]["enabled"] == true);
    REQUIRE(snapshot["actionDetails"]["applyEndpointProfile"]["disabledReason"] == "");

    REQUIRE(panel.applyEndpointProfile());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["uploadMode"] == "http_json");
    REQUIRE(snapshot["uploadEndpoint"] == "https://telemetry.example.invalid/collect");
    REQUIRE(snapshot["lastAction"]["action"] == "apply_endpoint_profile");
    REQUIRE(snapshot["lastAction"]["success"] == true);
}

TEST_CASE("AnalyticsPanel reports endpoint profile privacy diagnostics",
          "[analytics][editor][panel][endpoint_profile][privacy]") {
    AnalyticsDispatcher dispatcher;
    AnalyticsUploader uploader;
    AnalyticsEndpointProfile profile;
    profile.profileId = "unreviewed-http";
    profile.mode = AnalyticsEndpointMode::HttpJson;
    profile.url = "https://telemetry.example.invalid/collect";

    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.bindUploader(&uploader);
    panel.bindEndpointProfile(&profile);

    REQUIRE_FALSE(panel.applyEndpointProfile());
    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["endpointProfileDiagnostics"].is_array());
    REQUIRE_FALSE(snapshot["endpointProfileDiagnostics"].empty());
    REQUIRE(snapshot["lastAction"]["action"] == "apply_endpoint_profile");
    REQUIRE(snapshot["lastAction"]["success"] == false);
}

TEST_CASE("AnalyticsPanel endpoint profile action exposes disabled reasons",
          "[analytics][editor][panel][controls]") {
    AnalyticsDispatcher dispatcher;
    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["actions"]["applyEndpointProfile"] == false);
    REQUIRE(snapshot["actionDetails"]["applyEndpointProfile"]["enabled"] == false);
    REQUIRE(snapshot["actionDetails"]["applyEndpointProfile"]["disabledReason"] == "No analytics uploader is bound.");

    AnalyticsUploader uploader;
    panel.bindUploader(&uploader);
    panel.render();

    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["actions"]["applyEndpointProfile"] == false);
    REQUIRE(snapshot["actionDetails"]["applyEndpointProfile"]["enabled"] == false);
    REQUIRE(snapshot["actionDetails"]["applyEndpointProfile"]["disabledReason"] ==
            "No analytics endpoint profile is bound.");
}
