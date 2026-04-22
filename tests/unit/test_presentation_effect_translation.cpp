#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "engine/core/presentation/effects/effect_translator.h"

using namespace urpg::presentation;
using namespace urpg::presentation::effects;

TEST_CASE("EffectTranslator emits world and overlay effect commands in resolver order", "[presentation][effects][translation]") {
    std::vector<ResolvedEffectInstance> instances{
        {
            EffectPlacement::World,
            {1.0f, 2.0f, 0.0f},
            101,
            2.0f,
            0.25f,
            EffectIntensity{1.2f},
            {1.0f, 1.0f, 1.0f, 1.0f},
            OverlayEmphasis{0.0f}
        },
        {
            EffectPlacement::Overlay,
            {4.0f, 5.0f, 6.0f},
            201,
            2.0f,
            0.08f,
            EffectIntensity{1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
            OverlayEmphasis{0.75f}
        }
    };

    PresentationFrameIntent intent;
    EffectTranslator translator;
    translator.append(instances, intent);

    REQUIRE(intent.commands.size() == 2);
    CHECK(intent.commands[0].type == PresentationCommand::Type::DrawWorldEffect);
    CHECK(intent.commands[1].type == PresentationCommand::Type::DrawOverlayEffect);
    CHECK(intent.commands[0].effectOwnerId == 101);
    CHECK(intent.commands[1].effectOwnerId == 201);
    CHECK(intent.commands[1].position.x == Catch::Approx(4.0f));
    CHECK(intent.commands[1].position.y == Catch::Approx(5.0f));
    CHECK(intent.commands[1].position.z == Catch::Approx(6.0f));
}
