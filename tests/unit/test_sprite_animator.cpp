#include "engine/core/render/sprite_animator.h"
#include "tools/sprite_pipeline/sprite_pipeline_defs.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <cmath>

using json = nlohmann::json;

namespace {

bool approxEqual(float lhs, float rhs) {
    return std::fabs(lhs - rhs) <= 0.0001f;
}

urpg::SpriteAnimator::AtlasDefinition makeTestAtlas() {
    urpg::SpriteAnimator::AtlasDefinition atlas;
    atlas.width = 128;
    atlas.height = 64;
    atlas.frames = {
        {"walk_0", 0, 0, 32, 32},
        {"walk_1", 32, 0, 32, 32},
        {"walk_2", 64, 0, 32, 32}
    };
    atlas.animations = {
        {"walk", {"walk_0", "walk_1", "walk_2"}, 0.1f, true}
    };
    return atlas;
}

} // namespace

TEST_CASE("SpriteAnimator consumes authored atlas animations", "[sprite][animator]") {
    urpg::SpriteAnimator animator(nullptr, makeTestAtlas(), "walk");

    SECTION("Atlas-backed frame view exposes normalized UVs") {
        const auto frame = animator.getCurrentFrameView();
        REQUIRE(frame.usingAtlas);
        REQUIRE(frame.frameId == "walk_0");
        REQUIRE(approxEqual(frame.u1, 0.0f));
        REQUIRE(approxEqual(frame.v1, 0.0f));
        REQUIRE(approxEqual(frame.u2, 0.25f));
        REQUIRE(approxEqual(frame.v2, 0.5f));
    }

    SECTION("Atlas-backed animation advances through the authored clip order") {
        animator.setMoving(true);
        animator.update(0.11f);
        REQUIRE(animator.getCurrentFrameView().frameId == "walk_1");

        animator.update(0.11f);
        REQUIRE(animator.getCurrentFrameView().frameId == "walk_2");
    }
}

TEST_CASE("Sprite pipeline emits preview animation artifacts", "[sprite][pipeline]") {
    urpg::tools::SpriteAtlas atlas;
    atlas.atlasName = "heroes";
    atlas.texturePath = "heroes.png";
    atlas.width = 128;
    atlas.height = 64;
    atlas.sprites = {
        {"walk_2", 64, 0, 32, 32, 16, 16},
        {"walk_0", 0, 0, 32, 32, 16, 16},
        {"walk_1", 32, 0, 32, 32, 16, 16}
    };

    urpg::tools::populatePreviewArtifacts(atlas);
    const json serialized = urpg::tools::toJson(atlas);

    REQUIRE(serialized.contains("animations"));
    REQUIRE(serialized["animations"].size() == 1);
    REQUIRE(serialized["animations"][0]["id"] == "preview_loop");
    REQUIRE(serialized["animations"][0]["frames"] == json::array({"walk_0", "walk_1", "walk_2"}));
    REQUIRE(serialized["preview"]["defaultAnimation"] == "preview_loop");
    REQUIRE(serialized["preview"]["frameCount"] == 3);
}
