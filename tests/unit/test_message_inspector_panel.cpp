#include "editor/message/message_inspector_panel.h"

#include "editor/message/dialogue_preview_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path messagePanelRepoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

nlohmann::json loadMessagePanelJson(const std::filesystem::path& path) {
    std::ifstream stream(path);
    REQUIRE(stream.is_open());
    nlohmann::json json;
    stream >> json;
    return json;
}

urpg::localization::LocaleCatalog makeGuideLocale() {
    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson({
        {"locale", "en-US"},
        {"keys",
         {
             {"dialogue.guide.intro", "Hello \\P[1], you have \\V[1] crystals."},
             {"dialogue.guide.choice.accept", "Light the gate"},
             {"dialogue.guide.choice.locked", "Ask about the sealed path"},
         }},
    });
    return catalog;
}

} // namespace

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

TEST_CASE("MessageInspectorPanel delegates page mutations to model", "[message][editor]") {
    urpg::message::MessageFlowRunner runner;
    runner.begin({
        {"speaker_a", "Hello.", urpg::message::variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
    });
    urpg::message::RichTextLayoutEngine layout;

    urpg::editor::MessageInspectorPanel panel;
    panel.bindRuntime(runner, layout);
    panel.refresh();
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().total_pages == 1);

    urpg::message::DialoguePage new_page;
    new_page.id = "speaker_b";
    new_page.body = "World.";
    new_page.variant = urpg::message::variantFromCompatRoute("speaker", "Bob", 2);
    panel.addPage(new_page);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.total_pages == 2);
    REQUIRE(snapshot.visible_row_count == 2);
    REQUIRE(snapshot.visible_rows[1].page_id == "speaker_b");
}

TEST_CASE("Dialogue preview resolves portraits, choices, variables, localization, and runtime flow",
          "[message][editor][dialogue_preview][wysiwyg]") {
    const auto json = loadMessagePanelJson(
        messagePanelRepoRoot() / "content" / "fixtures" / "dialogue_preview_fixture.json");
    const auto document = urpg::message::DialoguePreviewDocument::fromJson(json);

    urpg::editor::DialoguePreviewPanel panel;
    panel.loadDocument(document, makeGuideLocale());
    panel.selectPage("intro");
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().preview_id == "guide_intro_preview");
    REQUIRE(panel.snapshot().page_id == "intro");
    REQUIRE(panel.snapshot().locale == "en-US");
    REQUIRE(panel.snapshot().speaker == "Guide");
    REQUIRE(panel.snapshot().body == "Hello Mira, you have 7 crystals.");
    REQUIRE(panel.snapshot().portrait_face_name == "portraits/guide.png");
    REQUIRE(panel.snapshot().choice_count == 2);
    REQUIRE(panel.snapshot().enabled_choice_count == 1);
    REQUIRE(panel.snapshot().diagnostic_count == 0);
    REQUIRE(panel.snapshot().runtime_page_index == 0);
    REQUIRE(panel.snapshot().runtime_command_count == 5);
    REQUIRE(panel.snapshot().status_message == "Dialogue preview is ready.");

    const auto& preview = panel.preview();
    REQUIRE(preview.flow_snapshot.state == urpg::message::MessageFlowState::AwaitingChoice);
    REQUIRE(preview.choices[0].label == "Light the gate");
    REQUIRE(preview.choices[0].target_page_id == "accepted");
    REQUIRE(preview.choices[1].label == "Ask about the sealed path");
    REQUIRE_FALSE(preview.choices[1].enabled);
    REQUIRE(preview.layout_metrics.width > 0);
    REQUIRE_FALSE(panel.snapshot().saved_project_json.empty());
}

TEST_CASE("Dialogue preview confirms authored choice effects through runtime trace",
          "[message][editor][dialogue_preview][wysiwyg]") {
    const auto json = loadMessagePanelJson(
        messagePanelRepoRoot() / "content" / "fixtures" / "dialogue_preview_fixture.json");
    const auto document = urpg::message::DialoguePreviewDocument::fromJson(json);

    urpg::editor::DialoguePreviewPanel panel;
    panel.loadDocument(document, makeGuideLocale());
    panel.selectPage("intro");
    panel.selectChoice(0);
    panel.confirmSelectedChoice(true);
    panel.render();

    REQUIRE(panel.snapshot().selected_choice_index == 0);
    REQUIRE(panel.snapshot().confirmed_choice_id == "accept");
    REQUIRE(panel.snapshot().next_page_id == "accepted");
    REQUIRE(panel.snapshot().variable_after_choice_count == 2);
    REQUIRE(panel.snapshot().variables_after_choice_json.find("\"2\":1") != std::string::npos);
    REQUIRE(panel.preview().variables_after_choice.at(2) == 1);

    const auto& commands = panel.preview().runtime_commands;
    REQUIRE(std::find(commands.begin(), commands.end(), "select_choice:accept") != commands.end());
    REQUIRE(std::find(commands.begin(), commands.end(), "confirm_choice:accept") != commands.end());
    REQUIRE(std::find(commands.begin(), commands.end(), "set_variable:2=1") != commands.end());
    REQUIRE(std::find(commands.begin(), commands.end(), "choice_command:open_gate") != commands.end());
    REQUIRE(std::find(commands.begin(), commands.end(), "goto_page:accepted") != commands.end());
}

TEST_CASE("Dialogue preview saved project data round-trips through schema surface",
          "[message][editor][dialogue_preview][wysiwyg]") {
    const auto json = loadMessagePanelJson(
        messagePanelRepoRoot() / "content" / "fixtures" / "dialogue_preview_fixture.json");
    const auto document = urpg::message::DialoguePreviewDocument::fromJson(json);
    const auto saved = document.toJson();
    const auto restored = urpg::message::DialoguePreviewDocument::fromJson(saved);

    REQUIRE(saved["schema"] == "urpg.dialogue_preview.v1");
    REQUIRE(restored.id == document.id);
    REQUIRE(restored.pages.size() == 2);
    REQUIRE(restored.variables.at(1) == 7);
    REQUIRE(restored.actor_names.at(1) == "Mira");
    REQUIRE(restored.pages.front().choices.front().target_page_id == "accepted");
    REQUIRE(restored.pages.front().choices.front().variable_writes.at(2) == 1);

    const auto result = urpg::message::PreviewDialoguePage(restored, makeGuideLocale(), "intro");
    REQUIRE(result.diagnostics.empty());
    REQUIRE(result.body == "Hello Mira, you have 7 crystals.");
    REQUIRE(result.portrait.has_value());
}

TEST_CASE("Dialogue preview diagnostics block false complete claims",
          "[message][editor][dialogue_preview][wysiwyg]") {
    urpg::message::DialoguePreviewDocument document;
    document.id = "broken_dialogue";
    document.locale = "en-US";
    document.pages.push_back({
        "intro",
        "",
        "dialogue.missing",
        urpg::message::variantFromCompatRoute("speaker", "", 4),
        {
            {"", "", "dialogue.choice.missing", "missing_target", "", {}, false, ""},
        },
        0,
        true,
    });

    urpg::editor::DialoguePreviewPanel panel;
    panel.loadDocument(document, makeGuideLocale());
    panel.selectPage("intro");
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().diagnostic_count >= 5);
    REQUIRE(panel.snapshot().status_message == "Dialogue preview has diagnostics.");

    const auto& diagnostics = panel.preview().diagnostics;
    const auto hasCode = [&diagnostics](const std::string& code) {
        return std::any_of(diagnostics.begin(), diagnostics.end(), [&code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
    };
    REQUIRE(hasCode("missing_localization_key"));
    REQUIRE(hasCode("missing_speaker"));
    REQUIRE(hasCode("missing_portrait_binding"));
    REQUIRE(hasCode("choice_unreachable"));
    REQUIRE(hasCode("missing_choice_id"));
    REQUIRE(hasCode("missing_choice_label"));
    REQUIRE(hasCode("missing_choice_target"));
}
