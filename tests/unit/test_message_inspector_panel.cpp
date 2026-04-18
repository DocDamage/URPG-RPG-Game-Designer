#include "editor/message/message_inspector_panel.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MessageInspectorPanel - Refresh projects runtime message state", "[message][editor][panel]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {
            "speaker_clean",
            "Welcome.",
            urpg::message::variantFromCompatRoute("speaker", "Alicia", 3),
            true,
            {},
            0,
        },
        {
            "narration_issue",
            "",
            urpg::message::variantFromCompatRoute("narration", "Alicia", 3),
            true,
            {},
            0,
        },
    });

    urpg::message::RichTextLayoutEngine layout;

    urpg::editor::MessageInspectorPanel panel;
    panel.bindRuntime(runner, layout);
    panel.refresh();

    REQUIRE(panel.getModel().Summary().total_pages == 2);
    REQUIRE(panel.getModel().VisibleRows().size() == 2);
    REQUIRE(panel.getModel().Summary().issue_count >= 1);

    panel.setShowIssuesOnly(true);
    panel.update();
    REQUIRE(panel.getModel().VisibleRows().size() == 1);
    REQUIRE(panel.getModel().VisibleRows()[0].page_id == "narration_issue");

    panel.setShowIssuesOnly(false);
    panel.setRouteFilter(urpg::message::MessagePresentationMode::Speaker);
    panel.update();
    REQUIRE(panel.getModel().VisibleRows().size() == 1);
    REQUIRE(panel.getModel().VisibleRows()[0].page_id == "speaker_clean");

    panel.clearRuntime();
    panel.refresh();
    REQUIRE(panel.getModel().VisibleRows().empty());
}

TEST_CASE("MessageInspectorPanel - render populates snapshot", "[message][editor][panel]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {"speaker_a", "Hello.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
        {"narration_b", "", urpg::message::variantFromCompatRoute("narration", "Alicia", 3), true, {}, 0},
    });
    urpg::message::RichTextLayoutEngine layout;

    urpg::editor::MessageInspectorPanel panel;
    panel.bindRuntime(runner, layout);
    panel.refresh();
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.has_data);
    REQUIRE(snapshot.total_pages == 2);
    REQUIRE(snapshot.visible_row_count == 2);
    REQUIRE(snapshot.issue_count >= 1);
    REQUIRE(snapshot.visible_rows.size() == 2);
    REQUIRE(snapshot.visible_rows[0].page_id == "speaker_a");
    REQUIRE(snapshot.issues.size() == snapshot.issue_count);
    REQUIRE(snapshot.summary.total_pages == 2);
}

TEST_CASE("MessageInspectorPanel - clear resets snapshot state", "[message][editor][panel]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {"speaker_a", "Hello.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
    });
    urpg::message::RichTextLayoutEngine layout;

    urpg::editor::MessageInspectorPanel panel;
    panel.bindRuntime(runner, layout);
    panel.refresh();
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().has_data);

    panel.clear();
    REQUIRE_FALSE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().visible_rows.empty());
    REQUIRE(panel.lastRenderSnapshot().issues.empty());
    REQUIRE(panel.lastRenderSnapshot().total_pages == 0);
}
