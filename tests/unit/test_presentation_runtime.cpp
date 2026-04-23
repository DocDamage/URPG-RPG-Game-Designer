#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "engine/core/presentation/dialogue_translator.h"
#include "engine/core/presentation/presentation_bridge.h"
#include "engine/core/presentation/presentation_hotreload.h"
#include "engine/core/presentation/presentation_runtime.h"
#include "engine/core/presentation/render_backend_mock.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/scene_manager.h"
#include <memory>

using namespace urpg::presentation;

namespace {

class MockPresentationScene final : public urpg::scene::GameScene {
public:
    urpg::scene::SceneType getType() const override { return urpg::scene::SceneType::MAP; }
    std::string getName() const override { return "MockPresentationScene"; }
};

} // namespace

TEST_CASE("Presentation runtime resolves weighted fog and PostFX blends", "[presentation][runtime]") {
    PresentationFrameIntent intent;

    FogProfile baseFog;
    baseFog.density = 0.2f;
    baseFog.startDist = 2.0f;
    baseFog.endDist = 40.0f;

    FogProfile overrideFog;
    overrideFog.density = 0.5f;
    overrideFog.startDist = 10.0f;
    overrideFog.endDist = 100.0f;

    PostFXProfile baseFx;
    baseFx.exposure = 1.0f;
    baseFx.bloomThreshold = 0.8f;
    baseFx.bloomIntensity = 0.2f;
    baseFx.saturation = 1.0f;

    PostFXProfile overrideFx;
    overrideFx.exposure = 1.6f;
    overrideFx.bloomThreshold = 0.4f;
    overrideFx.bloomIntensity = 0.8f;
    overrideFx.saturation = 0.5f;

    intent.AddFog(baseFog, 1.0f);
    intent.AddFog(overrideFog, 0.5f);
    intent.AddPostFX(baseFx, 1.0f);
    intent.AddPostFX(overrideFx, 0.5f);
    intent.AddShadowProxy(99, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f});

    PresentationRuntime::ResolveEnvironmentCommands(intent);

    size_t fogCount = 0;
    size_t postFxCount = 0;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::SetFog) {
            fogCount++;
            REQUIRE(cmd.fogProfile != nullptr);
            CHECK(cmd.fogProfile->density == Catch::Approx(0.3f));
            CHECK(cmd.fogProfile->startDist == Catch::Approx(4.6666667f).margin(0.0001f));
            CHECK(cmd.fogProfile->endDist == Catch::Approx(60.0f));
        }
        if (cmd.type == PresentationCommand::Type::SetPostFX) {
            postFxCount++;
            REQUIRE(cmd.postFXProfile != nullptr);
            CHECK(cmd.postFXProfile->exposure == Catch::Approx(1.2f));
            CHECK(cmd.postFXProfile->bloomThreshold == Catch::Approx(0.6666667f).margin(0.0001f));
            CHECK(cmd.postFXProfile->bloomIntensity == Catch::Approx(0.4f));
            CHECK(cmd.postFXProfile->saturation == Catch::Approx(0.8333333f).margin(0.0001f));
        }
    }

    REQUIRE(fogCount == 1);
    REQUIRE(postFxCount == 1);
    REQUIRE(intent.commands.size() == 3);

    const FogProfile* resolvedFogProfile = intent.commands.front().fogProfile;
    const PostFXProfile* resolvedPostFxProfile = nullptr;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::SetPostFX) {
            resolvedPostFxProfile = cmd.postFXProfile;
            break;
        }
    }

    REQUIRE(resolvedFogProfile != nullptr);
    REQUIRE(resolvedPostFxProfile != nullptr);

    urpg::render::RenderBackendMock backend;
    backend.ConsumeFrame(intent);
    CHECK(backend.GetCurrentState().fogDensity == Catch::Approx(0.3f));
    CHECK(backend.GetCurrentState().bloomIntensity == Catch::Approx(0.4f));
    CHECK(backend.GetCurrentState().saturation == Catch::Approx(0.8333333f).margin(0.0001f));

    PresentationFrameIntent secondIntent;
    secondIntent.AddFog(overrideFog, 1.0f);
    secondIntent.AddPostFX(overrideFx, 1.0f);
    PresentationRuntime::ResolveEnvironmentCommands(secondIntent);

    REQUIRE(secondIntent.commands.front().fogProfile != nullptr);
    REQUIRE(secondIntent.commands.back().postFXProfile != nullptr);
    CHECK(secondIntent.commands.front().fogProfile != resolvedFogProfile);
    CHECK(secondIntent.commands.back().postFXProfile != resolvedPostFxProfile);
    CHECK(resolvedFogProfile->density == Catch::Approx(0.3f));
    CHECK(resolvedPostFxProfile->exposure == Catch::Approx(1.2f));
}

TEST_CASE("Presentation runtime preserves effect commands when resolving environment commands", "[presentation][runtime][effects]") {
    PresentationFrameIntent intent;

    FogProfile fog;
    fog.density = 0.2f;
    intent.AddFog(fog);
    intent.AddWorldEffect(101, {0.0f, 1.0f, 0.0f}, 7, 2.0f, 0.25f, 1.0f, {1.0f, 1.0f, 1.0f, 1.0f});
    intent.AddOverlayEffect(201, {3.0f, 4.0f, 5.0f}, 2, 0.08f, 1.0f, 1.0f, {1.0f, 1.0f, 1.0f, 1.0f}, 1.0f);

    PresentationRuntime::ResolveEnvironmentCommands(intent);

    bool foundWorld = false;
    bool foundOverlay = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawWorldEffect && cmd.effectOwnerId == 7) {
            foundWorld = true;
        }
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect && cmd.effectOwnerId == 2) {
            foundOverlay = true;
            CHECK(cmd.position.x == Catch::Approx(3.0f));
            CHECK(cmd.position.y == Catch::Approx(4.0f));
            CHECK(cmd.position.z == Catch::Approx(5.0f));
        }
    }

    REQUIRE(foundWorld);
    REQUIRE(foundOverlay);

    urpg::render::RenderBackendMock backend;
    backend.ConsumeFrame(intent);
    CHECK(backend.GetCurrentState().worldEffectCount == 1);
    CHECK(backend.GetCurrentState().overlayEffectCount == 1);
}

TEST_CASE("Presentation runtime builds map frame with resolved environment commands", "[presentation][runtime]") {
    PresentationRuntime runtime;
    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;
    context.mapState.mapId = "runtime_map";
    context.mapState.actors.push_back({1, "hero", 2.0f, 3.0f, false});

    PresentationAuthoringData data;
    SpatialMapOverlay runtimeOverlay;
    runtimeOverlay.mapId = "runtime_map";
    runtimeOverlay.elevation.width = 8;
    runtimeOverlay.elevation.height = 8;
    runtimeOverlay.elevation.levels.assign(64, 2);
    runtimeOverlay.fog.density = 0.25f;
    runtimeOverlay.fog.startDist = 4.0f;
    runtimeOverlay.fog.endDist = 60.0f;
    runtimeOverlay.postFX.exposure = 1.2f;
    runtimeOverlay.postFX.bloomThreshold = 0.6f;
    runtimeOverlay.postFX.bloomIntensity = 0.4f;
    runtimeOverlay.postFX.saturation = 0.85f;
    data.mapOverlays.push_back(runtimeOverlay);
    data.actorProfiles.push_back({"hero", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});

    const PresentationFrameIntent intent = runtime.BuildPresentationFrame(context, data);

    size_t fogCount = 0;
    size_t postFxCount = 0;
    size_t actorCount = 0;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::SetFog) {
            fogCount++;
            REQUIRE(cmd.fogProfile != nullptr);
            CHECK(cmd.fogProfile->density == Catch::Approx(0.25f));
        }
        if (cmd.type == PresentationCommand::Type::SetPostFX) {
            postFxCount++;
            REQUIRE(cmd.postFXProfile != nullptr);
            CHECK(cmd.postFXProfile->bloomIntensity == Catch::Approx(0.4f));
            CHECK(cmd.postFXProfile->saturation == Catch::Approx(0.85f));
        }
        if (cmd.type == PresentationCommand::Type::DrawActor) {
            actorCount++;
            CHECK(cmd.position.y == Catch::Approx(1.25f));
        }
    }

    REQUIRE(fogCount == 1);
    REQUIRE(postFxCount == 1);
    REQUIRE(actorCount == 1);
    REQUIRE(intent.activePasses.size() == 3);
    CHECK(intent.activePasses[1].passName == "WorldSpatial");
}

TEST_CASE("Dialogue readability overrides blend into resolved PostFX", "[presentation][runtime]") {
    PresentationFrameIntent intent;

    PostFXProfile worldFx;
    worldFx.exposure = 1.0f;
    worldFx.bloomThreshold = 1.0f;
    worldFx.bloomIntensity = 0.2f;
    worldFx.saturation = 1.0f;
    intent.AddPostFX(worldFx);

    DialogueTranslator dialogueTranslator;
    DialogueState dialogueState;
    dialogueState.isActive = true;
    dialogueState.requireHighContrast = true;

    DialoguePresentationConfig dialogueConfig;
    dialogueConfig.sceneSaturationMultiplier = 0.4f;
    dialogueConfig.contrastBgAlpha = 0.7f;
    dialogueTranslator.ApplyReadability(dialogueState, dialogueConfig, intent);

    PostFXProfile menuBlurFx;
    menuBlurFx.exposure = 1.0f;
    menuBlurFx.bloomThreshold = 0.05f;
    menuBlurFx.bloomIntensity = 5.0f;
    menuBlurFx.saturation = 1.0f;
    intent.AddPostFX(menuBlurFx);

    PresentationRuntime::ResolveEnvironmentCommands(intent);

    size_t postFxCount = 0;
    size_t shadowProxyCount = 0;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::SetPostFX) {
            postFxCount++;
            REQUIRE(cmd.postFXProfile != nullptr);
            CHECK(cmd.postFXProfile->bloomIntensity == Catch::Approx((0.2f + 0.5f + 5.0f) / 3.0f).margin(0.0001f));
            CHECK(cmd.postFXProfile->saturation == Catch::Approx(0.8f).margin(0.0001f));
        }
        if (cmd.type == PresentationCommand::Type::DrawShadowProxy) {
            shadowProxyCount++;
        }
    }

    REQUIRE(postFxCount == 1);
    REQUIRE(shadowProxyCount == 1);
}

TEST_CASE("PresentationBridge builds frame for active scene using runtime", "[presentation][bridge]") {
    auto runtime = std::make_shared<PresentationRuntime>();
    auto authoringData = std::make_shared<PresentationAuthoringData>();

    SpatialMapOverlay overlay;
    overlay.mapId = "bridge_map";
    overlay.elevation.width = 4;
    overlay.elevation.height = 4;
    overlay.elevation.levels.assign(16, 0);
    overlay.fog.density = 0.15f;
    authoringData->mapOverlays.push_back(overlay);
    authoringData->actorProfiles.push_back({"hero", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});

    PresentationBridge bridge(runtime, authoringData);

    urpg::scene::SceneManager sceneManager;
    sceneManager.pushScene(std::make_shared<MockPresentationScene>());

    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;
    context.mapState.mapId = "bridge_map";
    context.mapState.actors.push_back({7, "hero", 1.0f, 1.0f, false});

    const PresentationFrameIntent intent = bridge.BuildFrameForActiveScene(sceneManager, context);

    REQUIRE(intent.activeMode == PresentationMode::Spatial);
    REQUIRE(intent.activePasses.size() == 3);

    bool foundActor = false;
    bool foundFog = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor && cmd.id == 7) {
            foundActor = true;
        }
        if (cmd.type == PresentationCommand::Type::SetFog && cmd.fogProfile && cmd.fogProfile->density == Catch::Approx(0.15f)) {
            foundFog = true;
        }
    }

    REQUIRE(foundActor);
    REQUIRE(foundFog);
}

TEST_CASE("PresentationBridge derives battle frame from active BattleScene", "[presentation][bridge][battle]") {
    auto runtime = std::make_shared<PresentationRuntime>();
    auto authoringData = std::make_shared<PresentationAuthoringData>();
    authoringData->actorProfiles.push_back({"1", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});
    authoringData->actorProfiles.push_back({"2", {0.5f, 0.0f}, {0.0f, 0.5f, 0.0f}, true, 0.0f});
    authoringData->battleConfig.useDynamicCineCamera = true;
    authoringData->battleConfig.formation.type = BattleFormation::LayoutType::Staged;
    authoringData->battleConfig.formation.spreadWidth = 1.5f;
    authoringData->battleConfig.formation.depthSpacing = 2.0f;

    PresentationBridge bridge(runtime, authoringData);

    auto battleScene = std::make_shared<urpg::scene::BattleScene>(std::vector<std::string>{"2"});
    battleScene->addActor("1", "Hero", 120, 30, {0.0f, 0.0f}, nullptr);
    battleScene->addEnemy("2", "Slime", 40, 0, {100.0f, 0.0f}, nullptr);

    urpg::scene::SceneManager sceneManager;
    sceneManager.pushScene(battleScene);

    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;
    REQUIRE(context.battleState.battleArenaId.empty());

    const PresentationFrameIntent intent = bridge.BuildFrameForActiveScene(sceneManager, context);

    size_t drawActorCount = 0;
    bool foundHero = false;
    bool foundEnemy = false;
    bool foundCamera = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor) {
            drawActorCount++;
            if (cmd.id == 1) {
                foundHero = true;
            }
            if (cmd.id == 2) {
                foundEnemy = true;
            }
        }
        if (cmd.type == PresentationCommand::Type::SetCamera && cmd.cameraProfile != nullptr) {
            foundCamera = true;
        }
    }

    REQUIRE(drawActorCount == 2);
    REQUIRE(foundHero);
    REQUIRE(foundEnemy);
    REQUIRE(foundCamera);
}

TEST_CASE("PresentationBridge derives battle effect commands from active BattleScene", "[presentation][bridge][battle][effects]") {
    auto runtime = std::make_shared<PresentationRuntime>();
    auto authoringData = std::make_shared<PresentationAuthoringData>();
    authoringData->actorProfiles.push_back({"1", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});
    authoringData->actorProfiles.push_back({"2", {0.5f, 0.0f}, {0.0f, 0.5f, 0.0f}, true, 0.0f});
    authoringData->battleConfig.formation.type = BattleFormation::LayoutType::Staged;
    authoringData->battleConfig.formation.spreadWidth = 1.5f;
    authoringData->battleConfig.formation.depthSpacing = 2.0f;
    PresentationBridge bridge(runtime, authoringData);

    auto battleScene = std::make_shared<urpg::scene::BattleScene>(std::vector<std::string>{"2"});
    battleScene->addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battleScene->addEnemy("2", "Slime", 30, 0, {100.0f, 0.0f}, nullptr);

    urpg::scene::SceneManager sceneManager;
    sceneManager.pushScene(battleScene);

    urpg::presentation::effects::EffectCue ownerCue;
    ownerCue.frameTick = 0;
    ownerCue.kind = urpg::presentation::effects::EffectCueKind::Gameplay;
    ownerCue.anchorMode = urpg::presentation::effects::EffectAnchorMode::Owner;
    ownerCue.sourceId = 1;
    ownerCue.ownerId = 1;
    ownerCue.intensity = {1.0f};
    battleScene->enqueueEffectCue(ownerCue);

    urpg::presentation::effects::EffectCue targetCue;
    targetCue.frameTick = 0;
    targetCue.kind = urpg::presentation::effects::EffectCueKind::Gameplay;
    targetCue.anchorMode = urpg::presentation::effects::EffectAnchorMode::Target;
    targetCue.sourceId = 1;
    targetCue.ownerId = (1ull << 63) | 2ull;
    targetCue.overlayEmphasis = {1.0f};
    targetCue.intensity = {1.5f};
    battleScene->enqueueEffectCue(targetCue);

    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;

    const PresentationFrameIntent intent = bridge.BuildFrameForActiveScene(sceneManager, context);

    std::optional<Vec3> heroPosition;
    std::optional<Vec3> enemyPosition;
    std::optional<Vec3> ownerEffectPosition;
    std::optional<Vec3> targetEffectPosition;
    bool foundOverlayEffect = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor && cmd.id == 1) {
            heroPosition = cmd.position;
        }
        if (cmd.type == PresentationCommand::Type::DrawActor && cmd.id == 2) {
            enemyPosition = cmd.position;
        }
        if (cmd.type == PresentationCommand::Type::DrawWorldEffect) {
            if (cmd.effectOwnerId == 1) {
                ownerEffectPosition = cmd.position;
            }
            if (cmd.effectOwnerId == ((1ull << 63) | 2ull)) {
                targetEffectPosition = cmd.position;
            }
        }
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect) {
            foundOverlayEffect = true;
        }
    }

    REQUIRE(heroPosition.has_value());
    REQUIRE(enemyPosition.has_value());
    REQUIRE(ownerEffectPosition.has_value());
    REQUIRE(targetEffectPosition.has_value());
    REQUIRE(foundOverlayEffect);
    CHECK(ownerEffectPosition->x == Catch::Approx(heroPosition->x));
    CHECK(ownerEffectPosition->y == Catch::Approx(heroPosition->y));
    CHECK(ownerEffectPosition->z == Catch::Approx(heroPosition->z));
    CHECK(targetEffectPosition->x == Catch::Approx(enemyPosition->x));
    CHECK(targetEffectPosition->y == Catch::Approx(enemyPosition->y));
    CHECK(targetEffectPosition->z == Catch::Approx(enemyPosition->z));
    CHECK((ownerEffectPosition->x != Catch::Approx(0.0f)
        || ownerEffectPosition->y != Catch::Approx(0.0f)
        || ownerEffectPosition->z != Catch::Approx(0.0f)));
    CHECK((targetEffectPosition->x != Catch::Approx(0.0f)
        || targetEffectPosition->y != Catch::Approx(0.0f)
        || targetEffectPosition->z != Catch::Approx(0.0f)));
}

TEST_CASE("PresentationBridge does not replay consumed battle effect commands on consecutive frames", "[presentation][bridge][battle][effects]") {
    auto runtime = std::make_shared<PresentationRuntime>();
    auto authoringData = std::make_shared<PresentationAuthoringData>();
    PresentationBridge bridge(runtime, authoringData);

    auto battleScene = std::make_shared<urpg::scene::BattleScene>(std::vector<std::string>{"2"});
    battleScene->addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battleScene->addEnemy("2", "Slime", 30, 0, {100.0f, 0.0f}, nullptr);

    urpg::scene::SceneManager sceneManager;
    sceneManager.pushScene(battleScene);

    urpg::presentation::effects::EffectCue cue;
    cue.frameTick = 0;
    cue.kind = urpg::presentation::effects::EffectCueKind::Gameplay;
    cue.anchorMode = urpg::presentation::effects::EffectAnchorMode::Target;
    cue.sourceId = 1;
    cue.ownerId = 2;
    cue.overlayEmphasis = {1.0f};
    cue.intensity = {1.5f};
    battleScene->enqueueEffectCue(cue);

    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;

    const PresentationFrameIntent firstIntent = bridge.BuildFrameForActiveScene(sceneManager, context);
    const PresentationFrameIntent secondIntent = bridge.BuildFrameForActiveScene(sceneManager, context);

    size_t firstWorldEffectCount = 0;
    size_t firstOverlayEffectCount = 0;
    for (const auto& cmd : firstIntent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawWorldEffect) {
            firstWorldEffectCount++;
        }
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect) {
            firstOverlayEffectCount++;
        }
    }

    size_t secondWorldEffectCount = 0;
    size_t secondOverlayEffectCount = 0;
    for (const auto& cmd : secondIntent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawWorldEffect) {
            secondWorldEffectCount++;
        }
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect) {
            secondOverlayEffectCount++;
        }
    }

    REQUIRE(firstWorldEffectCount >= 1);
    REQUIRE(firstOverlayEffectCount >= 1);
    REQUIRE(secondWorldEffectCount == 0);
    REQUIRE(secondOverlayEffectCount == 0);
}

// ---------------------------------------------------------------------------
// S23-T02 / S23-T06: ProfileArenaHotReloader tests
// ---------------------------------------------------------------------------

TEST_CASE("ProfileArenaHotReloader starts empty with zero reload count", "[presentation][hotreload]") {
    ProfileArenaHotReloader reloader;
    REQUIRE(reloader.GetReloadCount() == 0);
    REQUIRE(reloader.GetRegistry().MapOverlayCount() == 0);
    REQUIRE(reloader.GetArenaUsedBytes() == 0);
}

TEST_CASE("ProfileArenaHotReloader Update returns false when no assets are pending", "[presentation][hotreload]") {
    ProfileArenaHotReloader reloader;
    REQUIRE_FALSE(reloader.Update());
    REQUIRE(reloader.GetReloadCount() == 0);
}

TEST_CASE("ProfileArenaHotReloader processes a reload and invokes callback", "[presentation][hotreload]") {
    ProfileArenaHotReloader reloader;

    std::string callbackPath;
    reloader.SetReloadCallback([&](const std::string& path, PresentationRegistry& registry) {
        callbackPath = path;
        SpatialMapOverlay overlay;
        overlay.mapId = "hot_map";
        overlay.elevation.width = 4;
        overlay.elevation.height = 4;
        overlay.elevation.levels.assign(16, 1);
        overlay.fog.density = 0.1f;
        registry.RegisterMapOverlay("hot_map", overlay);
    });

    reloader.OnAssetChanged("content/maps/hot_map.json");
    REQUIRE(reloader.Update());
    REQUIRE(reloader.GetReloadCount() == 1);
    REQUIRE(callbackPath == "content/maps/hot_map.json");
    REQUIRE(reloader.GetRegistry().MapOverlayCount() == 1);
    REQUIRE(reloader.GetRegistry().GetMapOverlay("hot_map") != nullptr);
    CHECK(reloader.GetRegistry().GetMapOverlay("hot_map")->fog.density == Catch::Approx(0.1f));
}

TEST_CASE("ProfileArenaHotReloader does not leak registry data across reload cycles", "[presentation][hotreload]") {
    ProfileArenaHotReloader reloader;

    // First reload: register one overlay
    reloader.SetReloadCallback([](const std::string&, PresentationRegistry& registry) {
        SpatialMapOverlay overlay;
        overlay.mapId = "map_a";
        overlay.elevation.width = 2;
        overlay.elevation.height = 2;
        overlay.elevation.levels.assign(4, 0);
        registry.RegisterMapOverlay("map_a", overlay);
    });
    reloader.OnAssetChanged("content/maps/map_a.json");
    reloader.Update();
    REQUIRE(reloader.GetRegistry().MapOverlayCount() == 1);
    REQUIRE(reloader.GetReloadCount() == 1);

    // Second reload: register a different overlay — the old one must be gone
    reloader.SetReloadCallback([](const std::string&, PresentationRegistry& registry) {
        SpatialMapOverlay overlay;
        overlay.mapId = "map_b";
        overlay.elevation.width = 2;
        overlay.elevation.height = 2;
        overlay.elevation.levels.assign(4, 0);
        registry.RegisterMapOverlay("map_b", overlay);
    });
    reloader.OnAssetChanged("content/maps/map_b.json");
    reloader.Update();

    REQUIRE(reloader.GetReloadCount() == 2);
    REQUIRE(reloader.GetRegistry().MapOverlayCount() == 1);
    REQUIRE(reloader.GetRegistry().GetMapOverlay("map_a") == nullptr);
    REQUIRE(reloader.GetRegistry().GetMapOverlay("map_b") != nullptr);
}

TEST_CASE("ProfileArenaHotReloader deduplicates pending paths", "[presentation][hotreload]") {
    ProfileArenaHotReloader reloader;
    int callbackCount = 0;
    reloader.SetReloadCallback([&](const std::string&, PresentationRegistry&) { ++callbackCount; });

    reloader.OnAssetChanged("content/maps/shared.json");
    reloader.OnAssetChanged("content/maps/shared.json"); // duplicate
    reloader.OnAssetChanged("content/maps/shared.json"); // duplicate

    reloader.Update();
    REQUIRE(callbackCount == 1);
    REQUIRE(reloader.GetReloadCount() == 1);
}

TEST_CASE("ProfileArenaHotReloader arena is reset between reload cycles (no leaked usage)", "[presentation][hotreload]") {
    ProfileArenaHotReloader reloader(64 * 1024); // small 64 KB arena

    // First reload
    reloader.SetReloadCallback([](const std::string&, PresentationRegistry&) {});
    reloader.OnAssetChanged("content/maps/any.json");
    reloader.Update();
    const size_t usageAfterFirst = reloader.GetArenaUsedBytes();

    // Second reload — arena must be reset, usage should be at most equal (not accumulated)
    reloader.OnAssetChanged("content/maps/any.json");
    reloader.Update();
    const size_t usageAfterSecond = reloader.GetArenaUsedBytes();

    // Both reloads had the same callback (no allocations) so usage stays at 0
    REQUIRE(usageAfterFirst == 0);
    REQUIRE(usageAfterSecond == 0);
    REQUIRE(reloader.GetReloadCount() == 2);
}

// ---------------------------------------------------------------------------
// S23-T06: Command-stream replay determinism
// ---------------------------------------------------------------------------

TEST_CASE("Presentation command-stream replay is deterministic across builds", "[presentation][replay]") {
    PresentationRuntime runtime;
    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;
    context.mapState.mapId = "replay_map";
    context.mapState.actors.push_back({10, "hero", 3.0f, 4.0f, false});
    context.mapState.actors.push_back({11, "npc",  1.0f, 2.0f, false});

    PresentationAuthoringData data;
    SpatialMapOverlay overlay;
    overlay.mapId = "replay_map";
    overlay.elevation.width = 4;
    overlay.elevation.height = 4;
    overlay.elevation.levels.assign(16, 1);
    overlay.fog.density = 0.12f;
    overlay.fog.startDist = 2.0f;
    overlay.fog.endDist = 50.0f;
    overlay.postFX.exposure = 1.0f;
    overlay.postFX.bloomIntensity = 0.3f;
    overlay.postFX.saturation = 0.9f;
    data.mapOverlays.push_back(overlay);
    data.actorProfiles.push_back({"hero", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});
    data.actorProfiles.push_back({"npc",  {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});

    // Build twice from the same input — command counts and key values must match
    const PresentationFrameIntent first  = runtime.BuildPresentationFrame(context, data);
    const PresentationFrameIntent second = runtime.BuildPresentationFrame(context, data);

    REQUIRE(first.commands.size() == second.commands.size());
    REQUIRE(first.activePasses.size() == second.activePasses.size());

    for (size_t i = 0; i < first.commands.size(); ++i) {
        CHECK(first.commands[i].type == second.commands[i].type);
        CHECK(first.commands[i].id == second.commands[i].id);
        CHECK(first.commands[i].position.x == Catch::Approx(second.commands[i].position.x));
        CHECK(first.commands[i].position.y == Catch::Approx(second.commands[i].position.y));
        CHECK(first.commands[i].position.z == Catch::Approx(second.commands[i].position.z));
    }
}

TEST_CASE("Presentation command-stream replay after hot-reload reflects updated profile data", "[presentation][hotreload][replay]") {
    PresentationRuntime runtime;

    auto buildFrame = [&](float fogDensity) -> PresentationFrameIntent {
        PresentationContext ctx;
        ctx.activeMode = PresentationMode::Spatial;
        ctx.activeTier = CapabilityTier::Tier1_Standard;
        ctx.mapState.mapId = "reload_replay_map";
        ctx.mapState.actors.push_back({1, "hero", 0.0f, 0.0f, false});

        PresentationAuthoringData d;
        SpatialMapOverlay ov;
        ov.mapId = "reload_replay_map";
        ov.elevation.width = 2;
        ov.elevation.height = 2;
        ov.elevation.levels.assign(4, 0);
        ov.fog.density = fogDensity;
        d.mapOverlays.push_back(ov);
        d.actorProfiles.push_back({"hero", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});
        return runtime.BuildPresentationFrame(ctx, d);
    };

    const PresentationFrameIntent before = buildFrame(0.05f);
    const PresentationFrameIntent after  = buildFrame(0.9f);

    float beforeFog = 0.0f;
    float afterFog  = 0.0f;
    for (const auto& cmd : before.commands) {
        if (cmd.type == PresentationCommand::Type::SetFog && cmd.fogProfile)
            beforeFog = cmd.fogProfile->density;
    }
    for (const auto& cmd : after.commands) {
        if (cmd.type == PresentationCommand::Type::SetFog && cmd.fogProfile)
            afterFog = cmd.fogProfile->density;
    }

    CHECK(beforeFog == Catch::Approx(0.05f));
    CHECK(afterFog  == Catch::Approx(0.9f));
    REQUIRE(beforeFog != Catch::Approx(afterFog));
}

TEST_CASE("ProfileArenaHotReloader reload count increments per unique change event", "[presentation][hotreload]") {
    ProfileArenaHotReloader reloader;
    reloader.SetReloadCallback([](const std::string&, PresentationRegistry&) {});

    REQUIRE(reloader.GetReloadCount() == 0);

    reloader.OnAssetChanged("content/maps/map_x.json");
    reloader.Update();
    REQUIRE(reloader.GetReloadCount() == 1);

    reloader.OnAssetChanged("content/maps/map_y.json");
    reloader.Update();
    REQUIRE(reloader.GetReloadCount() == 2);

    // No pending → no reload
    reloader.Update();
    REQUIRE(reloader.GetReloadCount() == 2);
}
