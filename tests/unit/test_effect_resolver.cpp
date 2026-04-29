#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "engine/core/presentation/battle_scene_state.h"
#include "engine/core/presentation/effects/effect_resolver.h"

using namespace urpg::presentation;
using namespace urpg::presentation::effects;

TEST_CASE("EffectResolver expands heavy hit cues into world and overlay output", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::Gameplay;
    cue.intensity.value = 1.75f;
    cue.overlayEmphasis.value = 0.35f;
    cue.sourceId = 11;
    cue.ownerId = 42;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 2);
    CHECK(resolved[0].placement == EffectPlacement::World);
    CHECK(resolved[1].placement == EffectPlacement::Overlay);
    CHECK(resolved[0].ownerId == 42);
    CHECK(resolved[1].ownerId == 42);
    CHECK(resolved[1].overlayEmphasis.value > resolved[0].overlayEmphasis.value);
}

TEST_CASE("EffectResolver routes overlay and screen anchors through the overlay preset", "[presentation][effects][resolver]") {
    EffectResolver resolver;

    SECTION("Overlay anchor stays overlay-placed") {
        EffectCue cue;
        cue.anchorMode = EffectAnchorMode::Overlay;
        cue.overlayEmphasis.value = 0.25f;

        const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

        REQUIRE(resolved.size() == 1);
        CHECK(resolved.front().placement == EffectPlacement::Overlay);
        CHECK(resolved.front().overlayEmphasis.value == Catch::Approx(0.25f));
    }

    SECTION("Screen anchor stays overlay-placed") {
        EffectCue cue;
        cue.anchorMode = EffectAnchorMode::Screen;
        cue.overlayEmphasis.value = 0.4f;

        const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

        REQUIRE(resolved.size() == 1);
        CHECK(resolved.front().placement == EffectPlacement::Overlay);
        CHECK(resolved.front().overlayEmphasis.value == Catch::Approx(0.4f));
    }
}

TEST_CASE("EffectResolver collapses critical-hit hybrid output on low tier", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::Gameplay;
    cue.intensity.value = 2.0f;
    cue.overlayEmphasis.value = 0.95f;
    cue.sourceId = 7;
    cue.ownerId = 19;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier0_Basic);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved.front().placement == EffectPlacement::Overlay);
    CHECK(resolved.front().ownerId == 19);
    CHECK(resolved.front().overlayEmphasis.value == Catch::Approx(0.95f));
}

TEST_CASE("EffectResolver falls back to sourceId when ownerId is zero", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.anchorMode = EffectAnchorMode::World;
    cue.sourceId = 77;
    cue.ownerId = 0;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved.front().placement == EffectPlacement::World);
    CHECK(resolved.front().ownerId == 77);
}

TEST_CASE("EffectResolver anchors owner and target world cues to battle participant positions", "[presentation][effects][resolver]") {
    BattleSceneState battleState;
    BattleParticipantState ownerParticipant;
    ownerParticipant.actorId = 1;
    ownerParticipant.classId = "1";
    ownerParticipant.formationIndex = 0;
    ownerParticipant.isEnemy = false;
    ownerParticipant.currentHPPulse = 1.0f;
    ownerParticipant.cueId = 1;
    ownerParticipant.anchorPosition = {-5.0f, 0.25f, 0.0f};
    ownerParticipant.hasAnchorPosition = true;
    battleState.participants.push_back(ownerParticipant);

    BattleParticipantState targetParticipant;
    targetParticipant.actorId = 2;
    targetParticipant.classId = "2";
    targetParticipant.formationIndex = 0;
    targetParticipant.isEnemy = true;
    targetParticipant.currentHPPulse = 1.0f;
    targetParticipant.cueId = (1ull << 63) | 2ull;
    targetParticipant.anchorPosition = {5.0f, 0.5f, 0.0f};
    targetParticipant.hasAnchorPosition = true;
    battleState.participants.push_back(targetParticipant);

    EffectCue ownerCue;
    ownerCue.anchorMode = EffectAnchorMode::Owner;
    ownerCue.sourceId = 1;
    ownerCue.ownerId = 1;

    EffectCue targetCue;
    targetCue.anchorMode = EffectAnchorMode::Target;
    targetCue.sourceId = 1;
    targetCue.ownerId = (1ull << 63) | 2ull;

    EffectResolver resolver;
    const auto ownerResolved = resolver.resolve(ownerCue, CapabilityTier::Tier1_Standard, &battleState);
    const auto targetResolved = resolver.resolve(targetCue, CapabilityTier::Tier1_Standard, &battleState);

    REQUIRE(ownerResolved.size() == 1);
    CHECK(ownerResolved.front().position[0] == Catch::Approx(-5.0f));
    CHECK(ownerResolved.front().position[1] == Catch::Approx(0.25f));
    CHECK(ownerResolved.front().position[2] == Catch::Approx(0.0f));

    REQUIRE(targetResolved.size() == 1);
    CHECK(targetResolved.front().position[0] == Catch::Approx(5.0f));
    CHECK(targetResolved.front().position[1] == Catch::Approx(0.5f));
    CHECK(targetResolved.front().position[2] == Catch::Approx(0.0f));
}

TEST_CASE("EffectResolver routes HealPulse through healGlow preset", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::HealPulse;
    cue.sourceId = 1;
    cue.ownerId = 2;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved[0].placement == EffectPlacement::World);
    CHECK(resolved[0].color[1] > resolved[0].color[0]);
    CHECK(resolved[0].color[1] > resolved[0].color[2]);
}

TEST_CASE("EffectResolver routes MissSweep through missSweep preset", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::MissSweep;
    cue.sourceId = 1;
    cue.ownerId = 2;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved[0].placement == EffectPlacement::World);
    CHECK(resolved[0].scale < 1.0f);
}

TEST_CASE("EffectResolver routes DefeatFade through defeatFade preset", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::DefeatFade;
    cue.sourceId = 1;
    cue.ownerId = 2;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved[0].placement == EffectPlacement::Overlay);
    CHECK(resolved[0].duration > 0.3f);
}

TEST_CASE("EffectResolver routes PhaseBanner through phaseBanner preset", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::PhaseBanner;
    cue.sourceId = 1;
    cue.ownerId = 2;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved[0].placement == EffectPlacement::Overlay);
    CHECK(resolved[0].overlayEmphasis.value >= 0.8f);
}

TEST_CASE("EffectResolver routes CastStart through castSmall preset", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::CastStart;
    cue.sourceId = 1;
    cue.ownerId = 2;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved[0].placement == EffectPlacement::World);
    CHECK(resolved[0].color[2] > resolved[0].color[0]);
    CHECK(resolved[0].color[2] > resolved[0].color[1]);
}

TEST_CASE("EffectResolver routes CriticalHit through critBurst preset", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::CriticalHit;
    cue.sourceId = 1;
    cue.ownerId = 2;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 2);
    CHECK(resolved[0].placement == EffectPlacement::World);
    CHECK(resolved[0].color[0] > resolved[0].color[1]);
    CHECK(resolved[0].color[0] > resolved[0].color[2]);
    CHECK(resolved[1].placement == EffectPlacement::Overlay);
}

TEST_CASE("EffectResolver routes GuardClash as single world impact", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::GuardClash;
    cue.sourceId = 1;
    cue.ownerId = 2;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved[0].placement == EffectPlacement::World);
}

TEST_CASE("EffectResolver routes BloodSplatter as native battle impact feedback",
          "[presentation][effects][resolver][native-plugin-absorption]") {
    EffectCue cue;
    cue.kind = EffectCueKind::BloodSplatter;
    cue.sourceId = 1;
    cue.ownerId = 2;
    cue.overlayEmphasis.value = 0.2f;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved[0].placement == EffectPlacement::World);
    CHECK(resolved[0].ownerId == 2);
    CHECK(resolved[0].overlayEmphasis.value >= 0.45f);
}
