#include <algorithm>
#include <array>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "engine/core/presentation/effects/effect_cue.h"
#include "engine/core/presentation/effects/effect_instance.h"

using namespace urpg::presentation::effects;

TEST_CASE("EffectCue defaults are deterministic and render-agnostic", "[presentation][effects]") {
    EffectCue cue;

    CHECK(cue.frameTick == 0);
    CHECK(cue.sequenceIndex == 0);
    CHECK(cue.sourceId == 0);
    CHECK(cue.ownerId == 0);
    CHECK(cue.kind == EffectCueKind::Gameplay);
    CHECK(cue.anchorMode == EffectAnchorMode::World);
    CHECK(cue.overlayEmphasis.value == Catch::Approx(0.0f));
    CHECK(cue.intensity.value == Catch::Approx(1.0f));

    ResolvedEffectInstance instance;
    CHECK(instance.placement == EffectPlacement::World);
    CHECK(instance.position[0] == Catch::Approx(0.0f));
    CHECK(instance.position[1] == Catch::Approx(0.0f));
    CHECK(instance.position[2] == Catch::Approx(0.0f));
    CHECK(instance.ownerId == 0);
    CHECK(instance.duration == Catch::Approx(0.0f));
    CHECK(instance.scale == Catch::Approx(1.0f));
    CHECK(instance.intensity.value == Catch::Approx(1.0f));
    CHECK(instance.color[0] == Catch::Approx(1.0f));
    CHECK(instance.color[1] == Catch::Approx(1.0f));
    CHECK(instance.color[2] == Catch::Approx(1.0f));
    CHECK(instance.color[3] == Catch::Approx(1.0f));
    CHECK(instance.overlayEmphasis.value == Catch::Approx(0.0f));
}

TEST_CASE("EffectCue ordering sorts by frame tick then sequence index then ids", "[presentation][effects]") {
    std::array<EffectCue, 5> cues = {
        EffectCue{.frameTick = 2, .sequenceIndex = 1, .sourceId = 7, .ownerId = 3, .kind = EffectCueKind::Gameplay, .anchorMode = EffectAnchorMode::Owner, .overlayEmphasis = {0.2f}, .intensity = {1.0f}},
        EffectCue{.frameTick = 1, .sequenceIndex = 9, .sourceId = 2, .ownerId = 8, .kind = EffectCueKind::Status, .anchorMode = EffectAnchorMode::Target, .overlayEmphasis = {0.9f}, .intensity = {0.5f}},
        EffectCue{.frameTick = 2, .sequenceIndex = 1, .sourceId = 4, .ownerId = 1, .kind = EffectCueKind::System, .anchorMode = EffectAnchorMode::Screen, .overlayEmphasis = {0.1f}, .intensity = {0.8f}},
        EffectCue{.frameTick = 2, .sequenceIndex = 0, .sourceId = 9, .ownerId = 2, .kind = EffectCueKind::Gameplay, .anchorMode = EffectAnchorMode::Overlay, .overlayEmphasis = {0.7f}, .intensity = {1.2f}},
        EffectCue{.frameTick = 1, .sequenceIndex = 2, .sourceId = 5, .ownerId = 4, .kind = EffectCueKind::Gameplay, .anchorMode = EffectAnchorMode::World, .overlayEmphasis = {0.4f}, .intensity = {0.6f}},
    };

    std::array<EffectCue, 5> compareBySpaceship = cues;
    std::array<EffectCue, 5> compareByHelper = cues;

    std::sort(compareBySpaceship.begin(), compareBySpaceship.end());
    std::sort(compareByHelper.begin(), compareByHelper.end(), effectCueLess);

    REQUIRE(compareBySpaceship == compareByHelper);

    REQUIRE(compareBySpaceship[0].frameTick == 1);
    REQUIRE(compareBySpaceship[0].sequenceIndex == 2);
    REQUIRE(compareBySpaceship[0].sourceId == 5);
    REQUIRE(compareBySpaceship[0].ownerId == 4);

    REQUIRE(compareBySpaceship[1].frameTick == 1);
    REQUIRE(compareBySpaceship[1].sequenceIndex == 9);
    REQUIRE(compareBySpaceship[1].sourceId == 2);
    REQUIRE(compareBySpaceship[1].ownerId == 8);

    REQUIRE(compareBySpaceship[2].frameTick == 2);
    REQUIRE(compareBySpaceship[2].sequenceIndex == 0);
    REQUIRE(compareBySpaceship[2].sourceId == 9);
    REQUIRE(compareBySpaceship[2].ownerId == 2);

    REQUIRE(compareBySpaceship[3].frameTick == 2);
    REQUIRE(compareBySpaceship[3].sequenceIndex == 1);
    REQUIRE(compareBySpaceship[3].sourceId == 4);
    REQUIRE(compareBySpaceship[3].ownerId == 1);

    REQUIRE(compareBySpaceship[4].frameTick == 2);
    REQUIRE(compareBySpaceship[4].sequenceIndex == 1);
    REQUIRE(compareBySpaceship[4].sourceId == 7);
    REQUIRE(compareBySpaceship[4].ownerId == 3);
}
