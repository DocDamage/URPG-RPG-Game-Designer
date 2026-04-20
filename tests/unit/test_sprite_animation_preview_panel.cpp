#include "editor/sprite/sprite_animation_preview_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <cmath>

namespace {

urpg::tools::SpriteAtlas makePreviewAtlas() {
    urpg::tools::SpriteAtlas atlas;
    atlas.atlasName = "heroes";
    atlas.texturePath = "heroes.png";
    atlas.width = 128;
    atlas.height = 64;
    atlas.sprites = {
        {"walk_0", 0, 0, 32, 32, 16, 16},
        {"walk_1", 32, 0, 32, 32, 16, 16},
        {"walk_2", 64, 0, 32, 32, 16, 16},
        {"cast_0", 0, 32, 32, 32, 16, 16},
        {"cast_1", 32, 32, 32, 32, 16, 16}
    };
    atlas.animations = {
        {"preview_loop", {"walk_0", "walk_1", "walk_2"}, 0.15f, true},
        {"cast_once", {"cast_0", "cast_1"}, 0.2f, false}
    };
    atlas.preview.defaultAnimation = "preview_loop";
    atlas.preview.frameCount = 3;
    atlas.preview.orderedSpriteIds = {"walk_0", "walk_1", "walk_2", "cast_0", "cast_1"};
    return atlas;
}

} // namespace

TEST_CASE("SpriteAnimationPreviewPanel exposes atlas preview snapshot", "[sprite][editor][panel]") {
    urpg::editor::SpriteAnimationPreviewPanel panel;
    panel.update(makePreviewAtlas());
    panel.render();

    const auto& snapshot = panel.getRenderSnapshot();
    REQUIRE(snapshot.visible);
    REQUIRE(snapshot.has_data);
    REQUIRE(snapshot.atlas_name == "heroes");
    REQUIRE(snapshot.texture_path == "heroes.png");
    REQUIRE(snapshot.selected_animation_id == std::optional<std::string>{"preview_loop"});
    REQUIRE(snapshot.animation_rows.size() == 2);
    REQUIRE(snapshot.animation_rows[0].is_selected);
    REQUIRE(snapshot.animation_rows[0].frame_count == 3);
    REQUIRE(snapshot.active_frame_ids.size() == 3);
    REQUIRE(snapshot.active_frame_id == "walk_0");
    REQUIRE(snapshot.active_frame_width == 32);
    REQUIRE(snapshot.active_frame_height == 32);
    REQUIRE(snapshot.selected_loop);
}

TEST_CASE("SpriteAnimationPreviewPanel advances deterministically and supports tuning", "[sprite][editor][panel]") {
    urpg::editor::SpriteAnimationPreviewPanel panel;
    panel.update(makePreviewAtlas());

    REQUIRE(panel.selectAnimation("cast_once"));
    REQUIRE(panel.setSelectedAnimationFrameDuration(0.05f));
    REQUIRE(panel.setSelectedAnimationLoop(false));
    panel.setPreviewPlaying(true);
    panel.advancePreview(0.06f);

    auto snapshot = panel.getRenderSnapshot();
    REQUIRE(snapshot.selected_animation_id == std::optional<std::string>{"cast_once"});
    REQUIRE(snapshot.active_frame_id == "cast_1");
    REQUIRE(snapshot.active_frame_index == 1);
    REQUIRE(std::fabs(snapshot.selected_frame_duration - 0.05f) <= 0.0001f);
    REQUIRE_FALSE(snapshot.selected_loop);
    REQUIRE(snapshot.preview_playing);

    panel.advancePreview(0.06f);
    snapshot = panel.getRenderSnapshot();
    REQUIRE(snapshot.active_frame_id == "cast_1");
    REQUIRE(snapshot.active_frame_index == 1);
    REQUIRE_FALSE(snapshot.preview_playing);
}

TEST_CASE("SpriteAnimationPreviewPanel clear resets snapshot state", "[sprite][editor][panel]") {
    urpg::editor::SpriteAnimationPreviewPanel panel;
    panel.update(makePreviewAtlas());
    panel.render();
    REQUIRE(panel.getRenderSnapshot().has_data);

    panel.clear();

    const auto& snapshot = panel.getRenderSnapshot();
    REQUIRE_FALSE(snapshot.has_data);
    REQUIRE(snapshot.animation_rows.empty());
    REQUIRE(snapshot.active_frame_ids.empty());
    REQUIRE(snapshot.active_frame_id.empty());
    REQUIRE_FALSE(snapshot.selected_animation_id.has_value());
}
