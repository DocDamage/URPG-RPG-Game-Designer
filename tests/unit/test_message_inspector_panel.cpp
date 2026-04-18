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
