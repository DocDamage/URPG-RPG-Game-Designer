#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "engine/core/analytics/analytics_dispatcher.h"
#include "engine/core/analytics/analytics_uploader.h"
#include "engine/core/analytics/analytics_privacy_controller.h"

using nlohmann::json;
using urpg::analytics::AnalyticsDispatcher;

TEST_CASE("AnalyticsDispatcher records events when opt-in is true", "[analytics]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("test_event", "test_category");
    REQUIRE(dispatcher.getSessionEventCount() == 1);
    REQUIRE(dispatcher.getBufferSnapshot().size() == 1);
}

TEST_CASE("AnalyticsDispatcher drops events when opt-in is false", "[analytics]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(false);
    dispatcher.dispatchEvent("test_event", "test_category");
    REQUIRE(dispatcher.getSessionEventCount() == 0);
    REQUIRE(dispatcher.getBufferSnapshot().empty());
}

TEST_CASE("AnalyticsDispatcher session event count increments correctly", "[analytics]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("event_1", "category_a");
    dispatcher.dispatchEvent("event_2", "category_a");
    dispatcher.dispatchEvent("event_3", "category_b");
    REQUIRE(dispatcher.getSessionEventCount() == 3);
}

TEST_CASE("AnalyticsDispatcher buffer snapshot returns events as JSON", "[analytics]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("snapshot_event", "snapshot_category");
    auto snapshot = dispatcher.getBufferSnapshot();
    REQUIRE(snapshot.is_array());
    REQUIRE(snapshot.size() == 1);
    REQUIRE(snapshot[0]["eventName"] == "snapshot_event");
    REQUIRE(snapshot[0]["category"] == "snapshot_category");
    REQUIRE(snapshot[0]["timestamp"] == 1);
}

TEST_CASE("AnalyticsDispatcher buffer circular drop when exceeding 1000 events", "[analytics]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    for (size_t i = 0; i < 1005; ++i) {
        dispatcher.dispatchEvent("event_" + std::to_string(i), "category");
    }
    REQUIRE(dispatcher.getBufferSnapshot().size() == 1000);
    REQUIRE(dispatcher.getSessionEventCount() == 1005);
    auto snapshot = dispatcher.getBufferSnapshot();
    REQUIRE(snapshot[0]["eventName"] == "event_5");
}

TEST_CASE("AnalyticsDispatcher reset session clears buffer and resets tick counter", "[analytics]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("event_before", "category");
    REQUIRE(dispatcher.getSessionEventCount() == 1);
    REQUIRE(dispatcher.getBufferSnapshot()[0]["timestamp"] == 1);

    dispatcher.resetSession();
    REQUIRE(dispatcher.getSessionEventCount() == 0);
    REQUIRE(dispatcher.getBufferSnapshot().empty());

    dispatcher.dispatchEvent("event_after", "category");
    REQUIRE(dispatcher.getBufferSnapshot()[0]["timestamp"] == 1);
}

TEST_CASE("AnalyticsDispatcher event parameters are preserved", "[analytics]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    std::map<std::string, std::string> params;
    params["key1"] = "value1";
    params["key2"] = "value2";
    dispatcher.dispatchEvent("param_event", "param_category", params);
    auto snapshot = dispatcher.getBufferSnapshot();
    REQUIRE(snapshot.size() == 1);
    REQUIRE(snapshot[0]["parameters"]["key1"] == "value1");
    REQUIRE(snapshot[0]["parameters"]["key2"] == "value2");
}

TEST_CASE("AnalyticsDispatcher validator reports disallowed categories and empty event fields", "[analytics]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.setAllowedCategories({"combat", "ui"});

    dispatcher.dispatchEvent("", "", {{"source", "editor"}});
    dispatcher.dispatchEvent("debug_click", "debug", {{"source", ""}});

    const auto issues = dispatcher.getValidationIssues();
    REQUIRE(issues.size() == 4);
    REQUIRE(issues[0].category == urpg::analytics::AnalyticsValidationCategory::EmptyEventName);
    REQUIRE(issues[1].category == urpg::analytics::AnalyticsValidationCategory::EmptyCategory);
    REQUIRE(issues[2].category == urpg::analytics::AnalyticsValidationCategory::DisallowedCategory);
    REQUIRE(issues[3].category == urpg::analytics::AnalyticsValidationCategory::EmptyParameterValue);
}

TEST_CASE("AnalyticsDispatcher validator reports excessive parameter count", "[analytics]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.setAllowedCategories({"combat"});

    std::map<std::string, std::string> params;
    for (int i = 0; i < 9; ++i) {
        params["key" + std::to_string(i)] = "value";
    }

    dispatcher.dispatchEvent("combat_tick", "combat", params);

    const auto issues = dispatcher.getValidationIssues();
    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].category == urpg::analytics::AnalyticsValidationCategory::ExcessiveParameterCount);
    REQUIRE(issues[0].severity == urpg::analytics::AnalyticsValidationSeverity::Warning);
}

TEST_CASE("AnalyticsDispatcher governance script validates artifacts", "[analytics][project_audit_cli]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto scriptPath = repoRoot / "tools" / "ci" / "check_analytics_governance.ps1";
    const auto uniqueSuffix =
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto outputPath =
        std::filesystem::temp_directory_path() / ("urpg_analytics_gov_out_" + uniqueSuffix + ".json");

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.is_open());

    std::string jsonStr((std::istreambuf_iterator<char>(resultFile)),
                        std::istreambuf_iterator<char>());
    resultFile.close();

    const auto result = json::parse(jsonStr);
    REQUIRE(result["passed"].get<bool>() == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());
}

// ─── S28-T07: Analytics upload and session aggregation ───────────────────────

using namespace urpg::analytics;

TEST_CASE("AnalyticsUploader: flush empty events succeeds with zero flushed",
          "[analytics][upload][s28t07]") {
    AnalyticsUploader uploader;
    uploader.setUploadHandler([](const std::string&) { return true; });

    auto result = uploader.flush({}, "sess1");
    REQUIRE(result.success);
    REQUIRE(result.eventsFlushed == 0);
}

TEST_CASE("AnalyticsUploader: flush forwards events to handler as JSON",
          "[analytics][upload][s28t07]") {
    std::vector<std::string> received;
    AnalyticsUploader uploader;
    uploader.setUploadHandler([&](const std::string& payload) {
        received.push_back(payload);
        return true;
    });

    std::vector<AnalyticsEvent> events;
    events.push_back({"player.move", "gameplay", 1, {}});
    events.push_back({"ui.click",    "ui",       2, {}});

    auto result = uploader.flush(events, "s1");
    REQUIRE(result.success);
    REQUIRE(result.eventsFlushed == 2);
    REQUIRE_FALSE(received.empty());

    // Verify the batch is valid JSON containing our events
    const auto batch = nlohmann::json::parse(received[0]);
    REQUIRE(batch.is_array());
    REQUIRE(batch[0]["eventName"] == "player.move");
    REQUIRE(batch[0]["sessionId"] == "s1");
}

TEST_CASE("AnalyticsUploader: local JSONL export writes consent-gated batches",
          "[analytics][upload][privacy][consent]") {
    const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto root = std::filesystem::temp_directory_path() / ("urpg_analytics_export_" + unique);
    const auto exportPath = root / "reports" / "analytics" / "events.jsonl";

    AnalyticsUploader uploader;
    uploader.setLocalJsonlExportPath(exportPath);

    std::vector<AnalyticsEvent> events = {
        {"editor.open", "ui", 1, {{"panel", "analytics"}}},
    };

    const auto result = uploader.flush(events, "local-session");
    REQUIRE(result.success);
    REQUIRE(result.eventsFlushed == 1);
    REQUIRE(uploader.localJsonlExportPath().has_value());
    REQUIRE(*uploader.localJsonlExportPath() == exportPath);

    std::ifstream in(exportPath, std::ios::binary);
    REQUIRE(in.good());
    std::string line;
    std::getline(in, line);
    const auto exported = nlohmann::json::parse(line);
    REQUIRE(exported["schema"] == "urpg.analytics.local_export.v1");
    REQUIRE(exported["transport"] == "local_jsonl");
    REQUIRE(exported["batch"].is_array());
    REQUIRE(exported["batch"][0]["eventName"] == "editor.open");
    REQUIRE(exported["batch"][0]["sessionId"] == "local-session");

    std::error_code ec;
    std::filesystem::remove_all(root, ec);
}

TEST_CASE("AnalyticsUploader: batching splits events into multiple handler calls",
          "[analytics][upload][s28t07]") {
    int callCount = 0;
    AnalyticsUploader uploader;
    uploader.setBatchSize(2);
    uploader.setUploadHandler([&](const std::string&) { ++callCount; return true; });

    std::vector<AnalyticsEvent> events;
    for (int i = 0; i < 5; ++i) {
        events.push_back({"event", "cat", static_cast<uint64_t>(i), {}});
    }

    auto result = uploader.flush(events);
    REQUIRE(result.success);
    REQUIRE(result.eventsFlushed == 5);
    REQUIRE(callCount == 3);  // ceil(5/2)
}

TEST_CASE("AnalyticsUploader: session aggregate accumulates across flushes",
          "[analytics][upload][s28t07]") {
    AnalyticsUploader uploader;
    uploader.setUploadHandler([](const std::string&) { return true; });

    std::vector<AnalyticsEvent> sess1 = {
        {"level.start", "gameplay", 1, {}},
        {"level.start", "gameplay", 2, {}},
    };
    std::vector<AnalyticsEvent> sess2 = {
        {"ui.click", "ui", 3, {}},
    };

    uploader.flush(sess1);
    uploader.flush(sess2);

    const auto& agg = uploader.getAggregate();
    REQUIRE(agg.totalSessions == 2);
    REQUIRE(agg.totalEvents == 3);
    REQUIRE(agg.countsByEvent.at("level.start") == 2);
    REQUIRE(agg.countsByCategory.at("gameplay") == 2);
    REQUIRE(agg.countsByCategory.at("ui") == 1);
}

TEST_CASE("AnalyticsUploader: handler failure propagates as flush failure",
          "[analytics][upload][s28t07]") {
    AnalyticsUploader uploader;
    uploader.setUploadHandler([](const std::string&) { return false; });

    std::vector<AnalyticsEvent> events = {{"e", "c", 1, {}}};
    auto result = uploader.flush(events);
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.errorMessage.empty());
}

TEST_CASE("AnalyticsUploader: flush without handler succeeds with 0 flushed",
          "[analytics][upload][s28t07]") {
    AnalyticsUploader uploader;
    std::vector<AnalyticsEvent> events = {{"e", "c", 1, {}}};
    auto result = uploader.flush(events);
    REQUIRE(result.success);
    REQUIRE(result.eventsFlushed == 0);
    // Aggregate is still updated
    REQUIRE(uploader.getAggregate().totalEvents == 1);
}

// ─── S28-T08: Privacy/consent and retention/export workflows ─────────────────

TEST_CASE("AnalyticsPrivacyController: default consent is Unknown (analytics suppressed)",
          "[analytics][privacy][s28t08]") {
    AnalyticsPrivacyController ctrl;
    REQUIRE(ctrl.getConsentState() == ConsentState::Unknown);
    REQUIRE_FALSE(ctrl.isAnalyticsPermitted());
}

TEST_CASE("AnalyticsPrivacyController: Granted consent permits analytics",
          "[analytics][privacy][s28t08]") {
    AnalyticsPrivacyController ctrl;
    ctrl.recordConsentDecision(ConsentState::Granted);
    REQUIRE(ctrl.isAnalyticsPermitted());
}

TEST_CASE("AnalyticsPrivacyController: Denied consent suppresses analytics",
          "[analytics][privacy][s28t08]") {
    AnalyticsPrivacyController ctrl;
    ctrl.recordConsentDecision(ConsentState::Denied);
    REQUIRE_FALSE(ctrl.isAnalyticsPermitted());
}

TEST_CASE("AnalyticsPrivacyController: retention policy purges old events by age",
          "[analytics][privacy][s28t08]") {
    AnalyticsPrivacyController ctrl;
    RetentionPolicy policy;
    policy.maxAgeTicks = 10;
    ctrl.setRetentionPolicy(policy);

    std::vector<AnalyticsEvent> events = {
        {"old", "c", 1, {}},   // age = 99 at tick 100 → purge
        {"old2", "c", 5, {}},  // age = 95 → purge
        {"recent", "c", 95, {}}, // age = 5 → keep
    };

    const size_t removed = ctrl.applyRetentionPolicy(events, 100);
    REQUIRE(removed == 2);
    REQUIRE(events.size() == 1);
    REQUIRE(events[0].eventName == "recent");
}

TEST_CASE("AnalyticsPrivacyController: retention policy purges excess events by count",
          "[analytics][privacy][s28t08]") {
    AnalyticsPrivacyController ctrl;
    RetentionPolicy policy;
    policy.maxEventCount = 2;
    ctrl.setRetentionPolicy(policy);

    std::vector<AnalyticsEvent> events = {
        {"e1", "c", 1, {}},
        {"e2", "c", 2, {}},
        {"e3", "c", 3, {}},
    };

    ctrl.applyRetentionPolicy(events, 100);
    REQUIRE(events.size() == 2);
    REQUIRE(events[0].eventName == "e2");
    REQUIRE(events[1].eventName == "e3");
}

TEST_CASE("AnalyticsPrivacyController: exportUserData redacts PII keys",
          "[analytics][privacy][s28t08]") {
    AnalyticsPrivacyController ctrl;
    ctrl.recordConsentDecision(ConsentState::Granted);

    std::vector<AnalyticsEvent> events = {
        {"login", "auth", 1, {{"username", "alice"}, {"device", "pc"}}}
    };

    const auto doc = ctrl.exportUserData(events, {"username"});
    REQUIRE(doc["consentState"] == "Granted");
    REQUIRE(doc["eventCount"] == 1);
    REQUIRE(doc["events"][0]["parameters"]["username"] == "[REDACTED]");
    REQUIRE(doc["events"][0]["parameters"]["device"] == "pc");
}

TEST_CASE("AnalyticsPrivacyController: eraseUserData clears all events",
          "[analytics][privacy][s28t08]") {
    AnalyticsPrivacyController ctrl;
    std::vector<AnalyticsEvent> events = {
        {"e1", "c", 1, {}},
        {"e2", "c", 2, {}},
    };

    const size_t erased = ctrl.eraseUserData(events);
    REQUIRE(erased == 2);
    REQUIRE(events.empty());
}
