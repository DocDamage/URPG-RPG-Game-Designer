#include "engine/core/presentation/perspective2d_render_order.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Perspective 2D render order uses layer feet and stable id", "[presentation][perspective2d]") {
    using namespace urpg::presentation;
    std::vector<Perspective2DRenderEntry> entries = {
        {"prop_b", Perspective2DRenderLayer::Prop, {0, 10, 16, 16, 0}},
        {"prop_a", Perspective2DRenderLayer::Prop, {0, 10, 16, 16, 0}},
        {"ground", Perspective2DRenderLayer::Ground, {0, 99, 16, 16, 0}},
        {"occluder", Perspective2DRenderLayer::Occluder, {0, 5, 16, 16, 0}},
    };
    sortPerspective2DRenderOrder(entries);
    REQUIRE(entries[0].objectId == "ground");
    REQUIRE(entries[1].objectId == "occluder");
    REQUIRE(entries[2].objectId == "prop_a");
    REQUIRE(entries[3].objectId == "prop_b");
    REQUIRE(entries[2].anchor.feetX() == 8.0f);
    REQUIRE(entries[2].anchor.feetY() == 26.0f);
}
