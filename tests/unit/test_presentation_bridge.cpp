#include <catch2/catch_test_macros.hpp>
#include "engine/core/presentation/presentation_bridge.h"
#include "engine/core/presentation/presentation_runtime.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/scene_manager.h"
#include <memory>

using namespace urpg::presentation;

namespace {

class StubNonBattleScene final : public urpg::scene::GameScene {
public:
    urpg::scene::SceneType getType() const override { return urpg::scene::SceneType::MAP; }
    std::string getName() const override { return "StubNonBattleScene"; }
};

size_t CountEffectCommands(const PresentationFrameIntent& intent) {
    size_t count = 0;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawWorldEffect ||
            cmd.type == PresentationCommand::Type::DrawOverlayEffect) {
            count++;
        }
    }
    return count;
}

} // namespace

TEST_CASE("BuildFrameForActiveScene returns baseline intent when no active scene", "[presentation][bridge]") {
    auto runtime = std::make_shared<PresentationRuntime>();
    auto authoringData = std::make_shared<PresentationAuthoringData>();
    PresentationBridge bridge(runtime, authoringData);

    urpg::scene::SceneManager sceneManager;
    PresentationContext context;

    const PresentationFrameIntent intent = bridge.BuildFrameForActiveScene(sceneManager, context);

    REQUIRE(intent.activeMode == PresentationMode::Classic2D);
    REQUIRE(intent.activeTier == CapabilityTier::Tier0_Baseline);
}

TEST_CASE("Battle scene type triggers battle state extraction", "[presentation][bridge]") {
    auto runtime = std::make_shared<PresentationRuntime>();
    auto authoringData = std::make_shared<PresentationAuthoringData>();
    authoringData->actorProfiles.push_back({"1", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});
    PresentationBridge bridge(runtime, authoringData);

    auto battleScene = std::make_shared<urpg::scene::BattleScene>(std::vector<std::string>{});
    battleScene->addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);

    urpg::scene::SceneManager sceneManager;
    sceneManager.pushScene(battleScene);

    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;

    const PresentationFrameIntent intent = bridge.BuildFrameForActiveScene(sceneManager, context);

    bool foundParticipant = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor && cmd.id == 1) {
            foundParticipant = true;
        }
    }
    REQUIRE(foundParticipant);
}

TEST_CASE("Non-battle scene resets battle cue cursor", "[presentation][bridge]") {
    auto runtime = std::make_shared<PresentationRuntime>();
    auto authoringData = std::make_shared<PresentationAuthoringData>();
    PresentationBridge bridge(runtime, authoringData);

    auto battleScene = std::make_shared<urpg::scene::BattleScene>(std::vector<std::string>{});
    battleScene->addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);

    urpg::scene::SceneManager sceneManager;
    sceneManager.pushScene(battleScene);

    urpg::presentation::effects::EffectCue cue;
    cue.frameTick = 0;
    cue.kind = urpg::presentation::effects::EffectCueKind::Gameplay;
    cue.anchorMode = urpg::presentation::effects::EffectAnchorMode::Owner;
    cue.sourceId = 1;
    cue.ownerId = 1;
    battleScene->enqueueEffectCue(cue);

    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;

    const PresentationFrameIntent firstBattleIntent = bridge.BuildFrameForActiveScene(sceneManager, context);
    REQUIRE(CountEffectCommands(firstBattleIntent) == 1);

    // Second call with same battle scene: cursor has advanced, no cues should replay.
    const PresentationFrameIntent secondBattleIntent = bridge.BuildFrameForActiveScene(sceneManager, context);
    REQUIRE(CountEffectCommands(secondBattleIntent) == 0);

    // Switch to non-battle scene: cursor resets.
    sceneManager.gotoScene(std::make_shared<StubNonBattleScene>());
    const PresentationFrameIntent nonBattleIntent = bridge.BuildFrameForActiveScene(sceneManager, context);
    REQUIRE(CountEffectCommands(nonBattleIntent) == 0);

    // Return to same battle scene and enqueue a new cue.
    // Because the cursor was reset by the non-battle scene, the new cue
    // (sequenceIndex 0 after onStart reset) is emitted. Without the reset,
    // the advanced cursor would suppress it.
    sceneManager.gotoScene(battleScene);
    battleScene->enqueueEffectCue(cue);
    const PresentationFrameIntent thirdBattleIntent = bridge.BuildFrameForActiveScene(sceneManager, context);
    REQUIRE(CountEffectCommands(thirdBattleIntent) == 1);
}

TEST_CASE("Cue cursor sequencing works across multiple calls with same battle scene", "[presentation][bridge]") {
    auto runtime = std::make_shared<PresentationRuntime>();
    auto authoringData = std::make_shared<PresentationAuthoringData>();
    PresentationBridge bridge(runtime, authoringData);

    auto battleScene = std::make_shared<urpg::scene::BattleScene>(std::vector<std::string>{});
    battleScene->addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);

    urpg::scene::SceneManager sceneManager;
    sceneManager.pushScene(battleScene);

    urpg::presentation::effects::EffectCue cue1;
    cue1.frameTick = 0;
    cue1.kind = urpg::presentation::effects::EffectCueKind::Gameplay;
    cue1.anchorMode = urpg::presentation::effects::EffectAnchorMode::Owner;
    cue1.sourceId = 1;
    cue1.ownerId = 1;
    battleScene->enqueueEffectCue(cue1);

    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;

    const PresentationFrameIntent firstIntent = bridge.BuildFrameForActiveScene(sceneManager, context);
    REQUIRE(CountEffectCommands(firstIntent) == 1);

    // Enqueue a second cue.
    urpg::presentation::effects::EffectCue cue2;
    cue2.frameTick = 1;
    cue2.kind = urpg::presentation::effects::EffectCueKind::Gameplay;
    cue2.anchorMode = urpg::presentation::effects::EffectAnchorMode::Owner;
    cue2.sourceId = 1;
    cue2.ownerId = 1;
    battleScene->enqueueEffectCue(cue2);

    const PresentationFrameIntent secondIntent = bridge.BuildFrameForActiveScene(sceneManager, context);
    REQUIRE(CountEffectCommands(secondIntent) == 1);

    // Enqueue a third cue.
    urpg::presentation::effects::EffectCue cue3;
    cue3.frameTick = 2;
    cue3.kind = urpg::presentation::effects::EffectCueKind::Gameplay;
    cue3.anchorMode = urpg::presentation::effects::EffectAnchorMode::Owner;
    cue3.sourceId = 1;
    cue3.ownerId = 1;
    battleScene->enqueueEffectCue(cue3);

    const PresentationFrameIntent thirdIntent = bridge.BuildFrameForActiveScene(sceneManager, context);
    REQUIRE(CountEffectCommands(thirdIntent) == 1);
}
