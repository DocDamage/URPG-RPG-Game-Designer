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

TEST_CASE("MessageInspectorModel supports page body editing", "[message][editor]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {"page_a", "Original body.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
        {"page_b", "Second page.", urpg::message::variantFromCompatRoute("narration", "", 0), true, {}, 0},
    });

    urpg::message::RichTextLayoutEngine layout;
    urpg::editor::MessageInspectorModel model;
    model.LoadFromFlow(runner, layout);

    REQUIRE(model.SelectRow(0));
    REQUIRE(model.SelectedPageId().value() == "page_a");

    REQUIRE(model.updatePageBody(0, "Updated body text."));

    const auto& rows = model.VisibleRows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].page_id == "page_a");
    REQUIRE(rows[0].body_preview == "Updated body text.");

    REQUIRE(model.SelectedPageId().has_value());
    REQUIRE(model.SelectedPageId().value() == "page_a");
}

TEST_CASE("MessageInspectorModel supports adding and removing pages", "[message][editor]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {"page_a", "First page.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
    });

    urpg::message::RichTextLayoutEngine layout;
    urpg::editor::MessageInspectorModel model;
    model.LoadFromFlow(runner, layout);

    REQUIRE(model.Summary().total_pages == 1);

    urpg::message::DialoguePage new_page;
    new_page.id = "page_b";
    new_page.body = "Second page.";
    new_page.variant = urpg::message::variantFromCompatRoute("narration", "", 0);
    REQUIRE(model.addPage(new_page));

    REQUIRE(model.Summary().total_pages == 2);
    REQUIRE(model.VisibleRows().size() == 2);
    REQUIRE(model.VisibleRows()[1].page_id == "page_b");

    REQUIRE(model.removePage(1));
    REQUIRE(model.Summary().total_pages == 1);
    REQUIRE(model.VisibleRows().size() == 1);
    REQUIRE(model.VisibleRows()[0].page_id == "page_a");
}

TEST_CASE("MessageInspectorModel applyToRuntime reconstructs runner state", "[message][editor]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {"page_a", "First page.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
    });

    urpg::message::RichTextLayoutEngine layout;
    urpg::editor::MessageInspectorModel model;
    model.LoadFromFlow(runner, layout);

    REQUIRE(model.updatePageBody(0, "Mutated body."));

    urpg::message::DialoguePage new_page;
    new_page.id = "page_b";
    new_page.body = "Added page.";
    new_page.variant = urpg::message::variantFromCompatRoute("narration", "", 0);
    REQUIRE(model.addPage(new_page));

    urpg::message::MessageFlowRunner target_runner;
    REQUIRE(model.applyToRuntime(target_runner));

    REQUIRE(target_runner.pages().size() == 2);
    REQUIRE(target_runner.pages()[0].body == "Mutated body.");
    REQUIRE(target_runner.pages()[1].id == "page_b");
    REQUIRE(target_runner.isActive());
}

TEST_CASE("MessageInspectorModel clear resets to empty", "[message][editor]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {"page_a", "First page.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
    });

    urpg::message::RichTextLayoutEngine layout;
    urpg::editor::MessageInspectorModel model;
    model.LoadFromFlow(runner, layout);

    REQUIRE(model.Summary().total_pages == 1);
    model.clear();
    REQUIRE(model.Summary().total_pages == 0);
    REQUIRE(model.VisibleRows().empty());
    REQUIRE(model.Issues().empty());
    REQUIRE_FALSE(model.SelectedPageId().has_value());
}

TEST_CASE("MessageInspectorModel selectPageById finds correct row", "[message][editor]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {"page_a", "First page.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
        {"page_b", "Second page.", urpg::message::variantFromCompatRoute("narration", "", 0), true, {}, 0},
    });

    urpg::message::RichTextLayoutEngine layout;
    urpg::editor::MessageInspectorModel model;
    model.LoadFromFlow(runner, layout);

    REQUIRE(model.selectPageById("page_b"));
    REQUIRE(model.SelectedPageId().has_value());
    REQUIRE(model.SelectedPageId().value() == "page_b");

    REQUIRE_FALSE(model.selectPageById("nonexistent"));
}
