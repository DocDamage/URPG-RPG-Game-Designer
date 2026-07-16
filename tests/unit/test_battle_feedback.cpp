#include "engine/core/presentation/battle_feedback.h"
#include "engine/core/scene/battle_scene.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Battle feedback keeps a representative encounter readable without audio motion or color",
          "[presentation][battle][feedback][pcq603]") {
    using namespace urpg::presentation;
    BattleFeedbackDirector director;
    director.setSettings({false, true, BattleColorFilter::Monochrome, BattlePlaybackMode::Normal});
    const std::vector<BattleFeedbackRequest> requests = {
        {"select", BattleFeedbackKind::Selection, "Attack", "hero", "", 0, false},
        {"target", BattleFeedbackKind::Targeting, "Slime", "hero", "slime", 0, false},
        {"turn", BattleFeedbackKind::TurnState, "Hero turn", "battle", "", 0, false},
        {"anticipate", BattleFeedbackKind::Anticipation, "Hero attacks", "hero", "slime", 0, false},
        {"impact", BattleFeedbackKind::Impact, "Direct hit", "hero", "slime", 0, false},
        {"damage", BattleFeedbackKind::Damage, "42 damage", "hero", "slime", 42, true},
        {"heal", BattleFeedbackKind::Heal, "18 HP restored", "mage", "hero", 18, false},
        {"status", BattleFeedbackKind::Status, "Poisoned", "slime", "hero", 0, false},
        {"victory", BattleFeedbackKind::Victory, "Victory", "battle", "", 0, false},
        {"defeat", BattleFeedbackKind::Defeat, "Defeat", "battle", "", 0, false},
        {"results", BattleFeedbackKind::Results, "100 EXP and 20 gold", "battle", "", 0, false}
    };
    for (const auto& request : requests) REQUIRE(director.submit(request).success);
    REQUIRE_FALSE(director.submit(requests.front()).success);
    while (director.activeCue() || !director.snapshot().queued.empty()) director.advance(4000);

    const auto snapshot = director.snapshot();
    REQUIRE(snapshot.timeline.size() == requests.size());
    for (const auto& cue : snapshot.timeline) {
        REQUIRE(cue.completed);
        REQUIRE(cue.non_color_cue);
        REQUIRE_FALSE(cue.icon.empty());
        REQUIRE_FALSE(cue.shape_cue.empty());
        REQUIRE(cue.sound.empty());
        REQUIRE(cue.particle_motif.empty());
        REQUIRE(cue.camera_cue.empty());
        REQUIRE(cue.hit_stop_ms == 0);
        REQUIRE_FALSE(cue.control_blocking);
    }
    REQUIRE(snapshot.timeline.front().kind == BattleFeedbackKind::Victory);
    REQUIRE(snapshot.timeline[1].kind == BattleFeedbackKind::Defeat);
    REQUIRE(snapshot.timeline[2].kind == BattleFeedbackKind::Results);
}

TEST_CASE("Battle feedback fast-forward and skip preserve critical result clarity",
          "[presentation][battle][feedback][pcq603]") {
    using namespace urpg::presentation;
    BattleFeedbackDirector director;
    director.setSettings({true, false, BattleColorFilter::None, BattlePlaybackMode::FastForward});
    REQUIRE(director.submit({"fast-impact", BattleFeedbackKind::Impact, "Hit", "hero", "enemy", 5, false}).success);
    REQUIRE(director.activeCue()->duration_ms == 90);
    REQUIRE(director.activeCue()->hit_stop_ms <= 24);
    director.clear();

    director.setSettings({true, false, BattleColorFilter::None, BattlePlaybackMode::Skip});
    REQUIRE(director.submit({"skip-select", BattleFeedbackKind::Selection, "Attack", "hero", "", 0, false}).success);
    REQUIRE(director.activeCue()->duration_ms == 1);
    REQUIRE(director.submit({"keep-victory", BattleFeedbackKind::Victory, "Victory", "battle", "", 0, false}).success);
    REQUIRE(director.activeCue()->kind == BattleFeedbackKind::Victory);
    REQUIRE(director.activeCue()->duration_ms == 1400);
    REQUIRE_FALSE(director.activeCue()->skippable);
}

TEST_CASE("BattleScene publishes phase feedback through the shared battle director",
          "[scene][battle][feedback][pcq603]") {
    using namespace urpg::presentation;
    urpg::scene::BattleScene battle({});
    battle.onStart();
    battle.setPhase(urpg::scene::BattlePhase::START);
    battle.setPhase(urpg::scene::BattlePhase::INPUT);
    battle.setPhase(urpg::scene::BattlePhase::ACTION);
    battle.setPhase(urpg::scene::BattlePhase::VICTORY);
    const auto snapshot = battle.battleFeedback().snapshot();
    REQUIRE(snapshot.active);
    REQUIRE(snapshot.active->kind == BattleFeedbackKind::Victory);
    REQUIRE(snapshot.queued.size() == 3);
}
