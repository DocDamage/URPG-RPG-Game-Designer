#include "engine/core/message/message_core.h"

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

    SECTION("setMaxWidth enables word-wrapping") {
        RichTextLayoutEngine rapping_layout;
        rapping_layout.setBaseFontSize(10); // small font
        
        // Long text should wrap
        rapping_layout.setMaxWidth(50); // very narrow
        const auto wrapped = rapping_layout.layout("This is a very long text that should definitely wrap multiple times.");
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
        {"p2", "Choose route", variantFromCompatRoute("speaker", "Alicia", 3), true,
         {{"branch_a", "Branch A", true, ""}, {"branch_b", "Branch B", true, ""}}, 0},
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
        {"choice", "Choice", variantFromCompatRoute("speaker", "Alicia", 3), true,
         {{"a", "A", true, ""}, {"b", "B", true, ""}, {"c", "C", true, ""}}, 0},
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
