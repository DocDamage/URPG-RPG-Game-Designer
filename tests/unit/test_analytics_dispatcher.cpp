#include <catch2/catch_test_macros.hpp>

#include "engine/core/analytics/analytics_dispatcher.h"

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
