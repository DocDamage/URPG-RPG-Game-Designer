#include "editor/message/message_inspector_model.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Message inspector model builds preview rows and summary", "[message][editor][model]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {
            "speaker_intro",
            "Welcome back, hero.",
            urpg::message::variantFromCompatRoute("speaker", "Alicia", 3),
            true,
            {{"advance", "Continue", true, ""}},
            0,
        },
        {
            "narration_step",
            "The door creaks open...",
            urpg::message::variantFromCompatRoute("narration", "Alicia", 3),
            true,
            {},
            0,
        },
        {
            "",
            std::string(160, 'W'),
            urpg::message::variantFromCompatRoute("speaker", "", 2),
            true,
            {{"locked_1", "Locked 1", false, "missing_key"}, {"locked_2", "Locked 2", false, "missing_flag"}},
            0,
        },
        {
            "system_notice",
            "\\C[16]System\\C[0]: Autosave complete",
            urpg::message::variantFromCompatRoute("system", "System", 0),
            true,
            {},
            0,
        },
    });

    urpg::message::RichTextLayoutEngine layout;
    urpg::editor::MessageInspectorModel model;
    model.LoadFromFlow(runner, layout);

    const auto& summary = model.Summary();
    REQUIRE(summary.total_pages == 4);
    REQUIRE(summary.speaker_pages == 2);
    REQUIRE(summary.narration_pages == 1);
    REQUIRE(summary.system_pages == 1);
    REQUIRE(summary.choice_pages == 2);
    REQUIRE(summary.issue_count >= 3);
    REQUIRE(summary.max_preview_width > 640);
    REQUIRE(summary.has_active_flow);

    const auto& rows = model.VisibleRows();
    REQUIRE(rows.size() == 4);
    REQUIRE(rows[0].page_id == "speaker_intro");
    REQUIRE(rows[0].route == "speaker");
    REQUIRE(rows[1].route == "narration");
    REQUIRE(rows[2].issue_count >= 3);
    REQUIRE(rows[3].route == "system");

    const auto& issues = model.Issues();
    REQUIRE_FALSE(issues.empty());
}

TEST_CASE("Message inspector model route and issue filters narrow visible rows", "[message][editor][model]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {
            "speaker_clean",
            "Plain speaker line.",
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
    urpg::editor::MessageInspectorModel model;
    model.LoadFromFlow(runner, layout);

    model.SetRouteFilter(urpg::message::MessagePresentationMode::Narration);
    auto rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].page_id == "narration_issue");

    model.SetRouteFilter(std::nullopt);
    model.SetShowIssuesOnly(true);
    rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].page_id == "narration_issue");

    REQUIRE(model.SelectRow(0));
    const auto selected = model.SelectedPageId();
    REQUIRE(selected.has_value());
    REQUIRE(*selected == "narration_issue");

    REQUIRE_FALSE(model.SelectRow(9));
    REQUIRE_FALSE(model.SelectedPageId().has_value());
}
