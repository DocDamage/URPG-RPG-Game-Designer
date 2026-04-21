#include <catch2/catch_test_macros.hpp>

#include "editor/analytics/analytics_panel.h"
#include "engine/core/analytics/analytics_dispatcher.h"

using urpg::analytics::AnalyticsDispatcher;
using urpg::editor::AnalyticsPanel;

TEST_CASE("AnalyticsPanel empty snapshot when no dispatcher bound", "[analytics][editor][panel]") {
    AnalyticsPanel panel;
    panel.render();
    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["optIn"] == false);
    REQUIRE(snapshot["sessionEventCount"] == 0);
    REQUIRE(snapshot["recentEvents"].is_array());
    REQUIRE(snapshot["recentEvents"].empty());
}

TEST_CASE("AnalyticsPanel snapshot reflects events after dispatch", "[analytics][editor][panel]") {
    AnalyticsDispatcher dispatcher;
    dispatcher.setOptIn(true);
    dispatcher.dispatchEvent("editor_event", "editor_category");

    AnalyticsPanel panel;
    panel.bindDispatcher(&dispatcher);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["optIn"] == true);
    REQUIRE(snapshot["sessionEventCount"] == 1);
    REQUIRE(snapshot["recentEvents"].size() == 1);
    REQUIRE(snapshot["recentEvents"][0]["eventName"] == "editor_event");
    REQUIRE(snapshot["recentEvents"][0]["category"] == "editor_category");
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
