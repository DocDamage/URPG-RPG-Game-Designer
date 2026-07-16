#include "engine/core/presentation/battle_feedback.h"
#include "engine/core/presentation/exploration_feedback.h"
#include "engine/core/presentation/runtime_feedback_stack.h"
#include "engine/core/scene/runtime_shell_flow.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("Runtime feedback presets acknowledge immediately and obey every user setting",
          "[presentation][feedback_stack][pcq604]") {
    using namespace urpg::presentation;
    RuntimeFeedbackStack stack;
    stack.setSettings({false, false, false, false, false, true, 0.5F});
    REQUIRE(stack.submit({"focus", RuntimeFeedbackSignal::Focus, RuntimeFeedbackPriority::Ambient,
                          "Focus moved", {}}).success);
    REQUIRE(stack.submit({"confirm", RuntimeFeedbackSignal::Confirm, RuntimeFeedbackPriority::Secondary,
                          "Confirmed", {}}).success);
    REQUIRE(stack.submit({"cancel", RuntimeFeedbackSignal::Cancel, RuntimeFeedbackPriority::Secondary,
                          "Cancelled", {}}).success);
    REQUIRE(stack.submit({"error", RuntimeFeedbackSignal::Error, RuntimeFeedbackPriority::Critical,
                          "Action unavailable", {}}).success);
    REQUIRE_FALSE(stack.submit({"error", RuntimeFeedbackSignal::Error, RuntimeFeedbackPriority::Critical,
                                "Duplicate", {}}).success);

    while (stack.snapshot().active || !stack.snapshot().queued.empty()) stack.advance(2000);
    const auto snapshot = stack.snapshot();
    REQUIRE(snapshot.timeline.size() == 4);
    REQUIRE(snapshot.timeline.front().signal == RuntimeFeedbackSignal::Error);
    for (const auto& cue : snapshot.timeline) {
        REQUIRE(cue.acknowledged_immediately);
        REQUIRE_FALSE(cue.control_blocking);
        REQUIRE(cue.completed);
        REQUIRE(cue.treatment.duration_ms > 0);
        REQUIRE(cue.treatment.sound.empty());
        REQUIRE(cue.treatment.haptic.empty());
        REQUIRE(cue.treatment.particle.empty());
        REQUIRE(cue.treatment.camera.empty());
        REQUIRE(cue.treatment.focus_animation == "instant_focus");
        REQUIRE(cue.treatment.screen_shake_pixels == 0.0F);
        REQUIRE(cue.treatment.hit_stop_ms == 0);
    }
}

TEST_CASE("Runtime feedback custom treatments are bounded and the stack cannot grow without limit",
          "[presentation][feedback_stack][pcq604]") {
    using namespace urpg::presentation;
    RuntimeFeedbackStack stack;
    RuntimeFeedbackTreatment treatment{"urpg.sound.custom", "urpg.haptic.custom", "paper_spark",
                                       "bounded_impact", "focus_ring_in", 999.0F, 999, 9999};
    REQUIRE(stack.submit({"custom-0", RuntimeFeedbackSignal::Custom, RuntimeFeedbackPriority::Primary,
                          "Custom feedback", treatment}).success);
    const auto active = stack.snapshot().active;
    REQUIRE(active);
    REQUIRE(active->treatment.screen_shake_pixels == Catch::Approx(6.0F));
    REQUIRE(active->treatment.hit_stop_ms == 65);
    REQUIRE(active->treatment.duration_ms == 900);

    for (int index = 1; index < 32; ++index) {
        REQUIRE(stack.submit({"custom-" + std::to_string(index), RuntimeFeedbackSignal::Custom,
                              RuntimeFeedbackPriority::Ambient, "Queued feedback", treatment}).success);
    }
    REQUIRE(stack.snapshot().queued.size() == 31);
    REQUIRE_FALSE(stack.submit({"custom-overflow", RuntimeFeedbackSignal::Custom,
                                RuntimeFeedbackPriority::Critical, "Overflow", treatment}).success);
}

TEST_CASE("Exploration and battle directors use the shared runtime feedback constraints",
          "[presentation][feedback_stack][pcq604]") {
    using namespace urpg::presentation;
    ExplorationFeedbackDirector exploration;
    exploration.setSettings({false, true, true, 1.0F, false});
    REQUIRE(exploration.submit({"reward", ExplorationFeedbackKind::Reward, "Reward", "quest", ""}).success);
    REQUIRE(exploration.activeCue()->sound.empty());
    REQUIRE(exploration.activeCue()->haptic_cue.empty());
    REQUIRE(exploration.activeCue()->particle_motif.empty());
    REQUIRE(exploration.activeCue()->camera_cue.empty());
    REQUIRE_FALSE(exploration.activeCue()->control_blocking);

    BattleFeedbackDirector battle;
    battle.setSettings({false, true, BattleColorFilter::Monochrome, BattlePlaybackMode::Normal, false});
    REQUIRE(battle.submit({"impact", BattleFeedbackKind::Impact, "Impact", "hero", "enemy", 20, false}).success);
    REQUIRE(battle.activeCue()->sound.empty());
    REQUIRE(battle.activeCue()->haptic_cue.empty());
    REQUIRE(battle.activeCue()->particle_motif.empty());
    REQUIRE(battle.activeCue()->camera_cue.empty());
    REQUIRE(battle.activeCue()->screen_shake_pixels == 0.0F);
    REQUIRE(battle.activeCue()->hit_stop_ms == 0);
    REQUIRE_FALSE(battle.activeCue()->control_blocking);
}

TEST_CASE("Runtime shell routes focus confirmation cancellation and errors through the shared stack",
          "[presentation][feedback_stack][pcq604]") {
    using namespace urpg::presentation;
    urpg::scene::RuntimeShellFlow shell;
    REQUIRE(shell.completeStartup(true).success);
    urpg::input::InputCore input;
    input.updateActionState(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Pressed);
    REQUIRE(shell.handleInput(input).success);
    REQUIRE(shell.activate("new_game").success);
    REQUIRE(shell.activate("back").success);
    REQUIRE_FALSE(shell.activate("missing").success);
    while (shell.feedbackStack().snapshot().active || !shell.feedbackStack().snapshot().queued.empty()) {
        shell.advanceFeedback(2000);
    }
    const auto timeline = shell.feedbackStack().snapshot().timeline;
    REQUIRE(timeline.size() == 5);
    REQUIRE(timeline.front().signal == RuntimeFeedbackSignal::Error);
    REQUIRE(std::count_if(timeline.begin(), timeline.end(), [](const auto& cue) {
                return cue.signal == RuntimeFeedbackSignal::Confirm;
            }) == 2);
    REQUIRE(std::count_if(timeline.begin(), timeline.end(), [](const auto& cue) {
                return cue.signal == RuntimeFeedbackSignal::Cancel;
            }) == 1);
    REQUIRE(std::count_if(timeline.begin(), timeline.end(), [](const auto& cue) {
                return cue.signal == RuntimeFeedbackSignal::Focus;
            }) == 1);
}
