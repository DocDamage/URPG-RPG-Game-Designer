#include "engine/core/message/message_core.h"
#include "engine/core/message/picture_task_document.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::message;

TEST_CASE("Message route mapping matches compat message fixture lanes", "[message][route]") {
    const auto speaker = variantFromCompatRoute("speaker", "Alicia", 3);
    REQUIRE(speaker.mode == MessagePresentationMode::Speaker);
    REQUIRE(speaker.tone == MessageTone::Portrait);
    REQUIRE(speaker.speaker == "Alicia");
    REQUIRE(speaker.face_actor_id == 3);
    REQUIRE(speaker.route_token == "speaker:speaker:portrait");

    const auto narration = variantFromCompatRoute("narration", "Alicia", 3);
    REQUIRE(narration.mode == MessagePresentationMode::Narration);
    REQUIRE(narration.tone == MessageTone::Neutral);
    REQUIRE(narration.speaker.empty());
    REQUIRE(narration.face_actor_id == 0);
    REQUIRE(narration.route_token == "narration:narration:neutral");

    const auto system = variantFromCompatRoute("system", "System", 3);
    REQUIRE(system.mode == MessagePresentationMode::System);
    REQUIRE(system.tone == MessageTone::System);
    REQUIRE(system.speaker == "System");
    REQUIRE(system.face_actor_id == 0);
    REQUIRE(system.route_token == "system:system:system");
}

TEST_CASE("PortraitBindingRegistry resolves actor bindings deterministically", "[message][portrait]") {
    PortraitBindingRegistry registry;

    registry.registerBinding(2, PortraitBinding{"Actor2Face", 1, false});
    registry.registerBinding(7, PortraitBinding{"Actor7Face", 3, true});

    const auto* actor2 = registry.resolveBinding(2);
    REQUIRE(actor2 != nullptr);
    REQUIRE(actor2->face_name == "Actor2Face");
    REQUIRE(actor2->face_index == 1);

    const auto* actor7 = registry.resolveBinding(7);
    REQUIRE(actor7 != nullptr);
    REQUIRE(actor7->mirror);

    REQUIRE(registry.resolveBinding(999) == nullptr);

    registry.clear();
    REQUIRE(registry.resolveBinding(2) == nullptr);
}

TEST_CASE("RichTextLayoutEngine expands escapes and measures deterministic layout", "[message][richtext]") {
    RichTextLayoutEngine layout;
    layout.setVariableResolver([](int32_t id) { return id == 2 ? 777 : 0; });
    layout.setActorNameResolver([](int32_t actor_id) { return actor_id == 3 ? "Alicia" : "Unknown"; });
    layout.setPartyMemberResolver([](int32_t party_index) { return party_index == 0 ? 3 : 0; });

    const auto baseline = layout.layout("AB");
    const auto with_icon = layout.layout("A\\I[5]B");
    REQUIRE(with_icon.metrics.width > baseline.metrics.width);

    const auto with_tokens = layout.layout("\\C[2]HP\\I[5]\\V[2]\\G\\N[3]\\P[1]");
    REQUIRE(with_tokens.metrics.width > 0);
    REQUIRE(with_tokens.metrics.height >= with_tokens.metrics.line_height);
    REQUIRE(with_tokens.metrics.line_count == 1);

    bool has_color = false;
    bool has_icon = false;
    bool has_expanded_text = false;
    for (const auto& token : with_tokens.tokens) {
        if (token.type == RichTextTokenType::Color) {
            has_color = true;
            REQUIRE(token.value == 2);
        }
        if (token.type == RichTextTokenType::Icon) {
            has_icon = true;
            REQUIRE(token.value == 5);
        }
        if (token.type == RichTextTokenType::Text && token.text.find("777") != std::string::npos &&
            token.text.find("Alicia") != std::string::npos) {
            has_expanded_text = true;
        }
    }
    REQUIRE(has_color);
    REQUIRE(has_icon);
    REQUIRE(has_expanded_text);

    const auto multi = layout.layout("Line1\n\\{Line2\\}");
    REQUIRE(multi.metrics.line_count == 2);
    REQUIRE(multi.metrics.height > multi.metrics.line_height);

    SECTION("resolveEscapes expands variables without full tokenize") {
        const std::string text = "Points: \\V[2]";
        const std::string expanded = layout.resolveEscapes(text);
        REQUIRE(expanded == "Points: 777");
    }

    SECTION("nested variable escapes resolve recursively") {
        RichTextLayoutEngine nested_layout;
        nested_layout.setVariableResolver([](int32_t id) {
            if (id == 1) {
                return 2;
            }
            if (id == 2) {
                return 777;
            }
            return 0;
        });
        REQUIRE(nested_layout.resolveEscapes("Nested: \\V[\\V[1]]") == "Nested: 777");
        const auto nested_tokens = nested_layout.layout("Nested: \\V[\\V[1]]");
        bool found_nested_value = false;
        for (const auto& token : nested_tokens.tokens) {
            found_nested_value = found_nested_value ||
                                 (token.type == RichTextTokenType::Text &&
                                  token.text.find("777") != std::string::npos);
        }
        REQUIRE(found_nested_value);
    }

    SECTION("setMaxWidth enables word-wrapping") {
        RichTextLayoutEngine rapping_layout;
        rapping_layout.setBaseFontSize(10); // small font

        // Long text should wrap
        rapping_layout.setMaxWidth(50); // very narrow
        const auto wrapped =
            rapping_layout.layout("This is a very long text that should definitely wrap multiple times.");
        REQUIRE(wrapped.metrics.line_count > 1);
        REQUIRE(wrapped.metrics.width <= 70); // allow slight overflow for long words
    }

    SECTION("setAlignment adds LineOffset tokens for non-zero offsets") {
        RichTextLayoutEngine alignment_layout;
        alignment_layout.setBaseFontSize(10);
        alignment_layout.setMaxWidth(200);

        const std::string text = "Centered line";
        const int32_t text_w = alignment_layout.textWidth(text);

        alignment_layout.setAlignment(MessageAlignment::Center);
        const auto centered = alignment_layout.layout(text);

        bool has_offset = false;
        for (const auto& token : centered.tokens) {
            if (token.type == RichTextTokenType::LineOffset) {
                has_offset = true;
                const int32_t expected_offset = (200 - text_w) / 2;
                REQUIRE(token.value == expected_offset);
            }
        }
        REQUIRE(has_offset);

        alignment_layout.setAlignment(MessageAlignment::Right);
        const auto right = alignment_layout.layout(text);

        has_offset = false;
        for (const auto& token : right.tokens) {
            if (token.type == RichTextTokenType::LineOffset) {
                has_offset = true;
                const int32_t expected_offset = (200 - text_w);
                REQUIRE(token.value == expected_offset);
            }
        }
        REQUIRE(has_offset);
    }
}

TEST_CASE("ChoicePromptState skips disabled options and confirms selected route", "[message][choice]") {
    ChoicePromptState choices;
    choices.open({
        {"locked", "Locked", false, "quest_missing"},
        {"yes", "Yes", true, ""},
        {"later", "Later", true, ""},
    });

    REQUIRE(choices.isOpen());
    REQUIRE(choices.optionCount() == 3);
    REQUIRE(choices.selectedOption() != nullptr);
    REQUIRE(choices.selectedOption()->id == "yes");

    REQUIRE(choices.moveNext());
    REQUIRE(choices.selectedOption()->id == "later");

    REQUIRE(choices.movePrev());
    REQUIRE(choices.selectedOption()->id == "yes");

    const auto selected = choices.confirmSelection();
    REQUIRE(selected.has_value());
    REQUIRE(*selected == "yes");
}

TEST_CASE("MessageFlowRunner drives presentation, advance, and choice lifecycle", "[message][flow]") {
    MessageFlowRunner runner;
    runner.begin({
        {"p1", "Speaker line", variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
        {"p2",
         "Choose route",
         variantFromCompatRoute("speaker", "Alicia", 3),
         true,
         {{"branch_a", "Branch A", true, ""}, {"branch_b", "Branch B", true, ""}},
         0},
        {"p3", "Narration continuation", variantFromCompatRoute("narration", "Alicia", 3), false, {}, 0},
    });

    REQUIRE(runner.isActive());
    REQUIRE(runner.currentPage() != nullptr);
    REQUIRE(runner.currentPage()->id == "p1");
    REQUIRE(runner.state() == MessageFlowState::Presenting);

    REQUIRE(runner.markPagePresented());
    REQUIRE(runner.state() == MessageFlowState::AwaitingAdvance);

    REQUIRE(runner.advance());
    REQUIRE(runner.currentPage() != nullptr);
    REQUIRE(runner.currentPage()->id == "p2");
    REQUIRE(runner.state() == MessageFlowState::Presenting);

    REQUIRE(runner.markPagePresented());
    REQUIRE(runner.state() == MessageFlowState::AwaitingChoice);
    REQUIRE(runner.choicePrompt().isOpen());

    REQUIRE(runner.moveChoiceNext());
    const auto chosen = runner.confirmChoice();
    REQUIRE(chosen.has_value());
    REQUIRE(*chosen == "branch_b");
    REQUIRE(runner.currentPage() != nullptr);
    REQUIRE(runner.currentPage()->id == "p3");
    REQUIRE(runner.state() == MessageFlowState::Presenting);

    REQUIRE(runner.markPagePresented());
    REQUIRE(runner.state() == MessageFlowState::Completed);
    REQUIRE_FALSE(runner.isActive());
}

TEST_CASE("MessageFlowRunner advance on final page transitions to Completed", "[message][flow]") {
    MessageFlowRunner runner;
    runner.begin({
        {"p1", "Only page", variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
    });

    REQUIRE(runner.isActive());
    REQUIRE(runner.currentPage() != nullptr);
    REQUIRE(runner.currentPage()->id == "p1");
    REQUIRE(runner.state() == MessageFlowState::Presenting);

    REQUIRE(runner.markPagePresented());
    REQUIRE(runner.state() == MessageFlowState::AwaitingAdvance);

    REQUIRE(runner.advance());
    REQUIRE(runner.state() == MessageFlowState::Completed);
    REQUIRE_FALSE(runner.isActive());
    REQUIRE(runner.currentPage() == nullptr);
}

TEST_CASE("MessageFlowRunner cancel resets to Idle", "[message][flow]") {
    MessageFlowRunner runner;
    runner.begin({
        {"p1", "Page one", variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
        {"p2", "Page two", variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
    });

    REQUIRE(runner.isActive());
    REQUIRE(runner.state() == MessageFlowState::Presenting);

    runner.cancel();
    REQUIRE(runner.state() == MessageFlowState::Idle);
    REQUIRE_FALSE(runner.isActive());
    REQUIRE(runner.currentPage() == nullptr);
    REQUIRE(runner.pages().empty());
}

TEST_CASE("ChoicePromptState with all disabled options cannot confirm", "[message][choice]") {
    ChoicePromptState choices;
    choices.open({
        {"a", "A", false, "reason_a"},
        {"b", "B", false, "reason_b"},
    });

    REQUIRE(choices.isOpen());
    REQUIRE(choices.optionCount() == 2);
    REQUIRE(choices.selectedOption() != nullptr);
    REQUIRE_FALSE(choices.selectedOption()->enabled);
    REQUIRE_FALSE(choices.canConfirm());
    REQUIRE(choices.confirmSelection() == std::nullopt);
    REQUIRE_FALSE(choices.moveNext());
    REQUIRE_FALSE(choices.movePrev());
}

TEST_CASE("MessageFlowRunner snapshot and restore keeps in-flight choice state", "[message][flow][snapshot]") {
    MessageFlowRunner runner;
    const std::vector<DialoguePage> pages = {
        {"intro", "Intro", variantFromCompatRoute("speaker", "Alicia", 3), true, {}, 0},
        {"choice",
         "Choice",
         variantFromCompatRoute("speaker", "Alicia", 3),
         true,
         {{"a", "A", true, ""}, {"b", "B", true, ""}, {"c", "C", true, ""}},
         0},
    };
    runner.begin(pages);

    REQUIRE(runner.markPagePresented());
    REQUIRE(runner.advance());
    REQUIRE(runner.currentPage() != nullptr);
    REQUIRE(runner.currentPage()->id == "choice");
    REQUIRE(runner.markPagePresented());
    REQUIRE(runner.state() == MessageFlowState::AwaitingChoice);
    REQUIRE(runner.moveChoiceNext());
    REQUIRE(runner.moveChoiceNext());
    REQUIRE(runner.choicePrompt().selectedOption() != nullptr);
    REQUIRE(runner.choicePrompt().selectedOption()->id == "c");

    const auto snapshot = runner.snapshot();

    MessageFlowRunner restored;
    restored.begin(pages);
    REQUIRE(restored.restore(snapshot));
    REQUIRE(restored.state() == MessageFlowState::AwaitingChoice);
    REQUIRE(restored.currentPage() != nullptr);
    REQUIRE(restored.currentPage()->id == "choice");
    REQUIRE(restored.choicePrompt().selectedOption() != nullptr);
    REQUIRE(restored.choicePrompt().selectedOption()->id == "c");
}

TEST_CASE("PictureTaskDocument supports high-count picture slots and common-event bindings", "[message][picture][tasks]") {
    PictureTaskDocument document;
    document.setMaxPictures(1000);
    document.addBinding({350, "open_codex_hotspot", "common_event.open_codex", "click", true});
    document.addBinding({350, "hover_codex_hotspot", "common_event.preview_codex", "hover", false});

    const auto bindings = document.bindingsForPicture(350);
    REQUIRE(document.maxPictures() == 1000);
    REQUIRE(bindings.size() == 1);
    REQUIRE(bindings.front().task_id == "open_codex_hotspot");
    REQUIRE(bindings.front().common_event_id == "common_event.open_codex");
    REQUIRE(document.validate().empty());

    PictureTaskDocument invalid;
    invalid.setMaxPictures(2);
    invalid.addBinding({5, "", "", "click", true});
    const auto diagnostics = invalid.validate();
    REQUIRE(diagnostics.size() == 3);
}

TEST_CASE("PictureTaskDocument builds runtime preview rows for WYSIWYG picture UI",
          "[message][picture][tasks][preview]") {
    PictureTaskDocument document;
    document.setMaxPictures(1000);
    document.addBinding({350, "open_codex_hotspot", "common_event.open_codex", "click", true});
    document.addBinding({350, "hover_codex_hotspot", "common_event.preview_codex", "hover", true});
    document.addBinding({999, "open_map_hotspot", "common_event.open_map", "click", true});
    document.addBinding({999, "disabled_map_hover", "common_event.disabled", "hover", false});

    const std::vector<PictureRuntimeSlot> pictures = {
        {999, "asset://ui/map_button.png", 200, 20, 48, 48, 5, 1.0F, true},
        {350, "asset://ui/codex_button.png", 10, 10, 64, 32, 2, 0.75F, true},
        {1001, "asset://ui/out_of_range.png", 0, 0, 16, 16, 1, 1.0F, true},
        {120, "asset://ui/hidden.png", 0, 0, 32, 32, 0, 0.0F, true},
    };

    const auto preview = document.previewRuntime(pictures, 20, 20);

    REQUIRE(preview.max_pictures == 1000);
    REQUIRE(preview.visible_picture_count == 2);
    REQUIRE(preview.bound_picture_count == 2);
    REQUIRE(preview.clickable_picture_count == 2);
    REQUIRE(preview.hoverable_picture_count == 1);
    REQUIRE(preview.rows.size() == 3);
    REQUIRE(preview.rows[0].picture_id == 120);
    REQUIRE_FALSE(preview.rows[0].visible);
    REQUIRE(preview.rows[1].picture_id == 350);
    REQUIRE(preview.rows[1].visible);
    REQUIRE(preview.rows[1].clickable);
    REQUIRE(preview.rows[1].hoverable);
    REQUIRE(preview.rows[1].hovered);
    REQUIRE(preview.rows[1].bindings.size() == 2);
    REQUIRE(preview.rows[1].common_event_ids.size() == 2);
    REQUIRE(preview.rows[2].picture_id == 999);
    REQUIRE(preview.rows[2].bindings.size() == 1);
    REQUIRE_FALSE(preview.rows[2].hoverable);
    REQUIRE_FALSE(preview.rows[2].hovered);
    REQUIRE(preview.diagnostics.size() == 1);
    REQUIRE(preview.diagnostics[0].code == "picture_runtime_slot_out_of_range");

    const auto json = preview.toJson();
    REQUIRE(json["rows"].size() == 3);
    REQUIRE(json["rows"][1]["picture_id"] == 350);
    REQUIRE(json["rows"][1]["hovered"] == true);
    REQUIRE(json["rows"][1]["bindings"].size() == 2);
    REQUIRE(json["diagnostics"][0]["picture_id"] == 1001);
}
