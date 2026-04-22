#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "engine/core/analytics/analytics_dispatcher.h"

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
