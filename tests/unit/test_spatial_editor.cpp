#include "editor/spatial/elevation_brush_panel.h"
#include "editor/spatial/map_ability_binding_panel.h"
#include "editor/spatial/prop_placement_panel.h"
#include "editor/spatial/spatial_ability_canvas_panel.h"
#include "editor/spatial/spatial_authoring_workspace.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/presentation/dialogue_translator.h"
#include "engine/core/presentation/map_scene_translator.h"
#include "engine/core/presentation/menu_scene_translator.h"
#include "engine/core/presentation/presentation_migrate.h"
#include "engine/core/presentation/presentation_runtime.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/presentation/render_backend_mock.h"
#include "engine/core/presentation/spatial_projection.h"
#include "engine/core/scene/map_scene.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>

using namespace urpg::editor;
using namespace urpg::presentation;
using urpg::Vector2f;

TEST_CASE("Spatial Editor Tooling Integration - ElevationBrush modifies grid data", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    ElevationBrushPanel brush;
    brush.SetTarget(&overlay);
    brush.SetBrushSize(2);
    brush.SetBrushHeight(3.0f);

    brush.ApplyBrush(5, 5, 2);
    REQUIRE(overlay.elevation.levels[5 * 10 + 5] == 2);
    REQUIRE(overlay.elevation.levels[4 * 10 + 4] == 2);
    REQUIRE(brush.lastRenderSnapshot().has_target);
    REQUIRE(brush.lastRenderSnapshot().grid_width == 10);
    REQUIRE(brush.lastRenderSnapshot().grid_height == 10);
    REQUIRE(brush.lastRenderSnapshot().brush_size == 2);
    REQUIRE(brush.lastRenderSnapshot().brush_height == Catch::Approx(3.0f));

    // Out of bounds safety
    brush.ApplyBrush(20, 20, 5);
    // Should not crash
}

TEST_CASE("Spatial Editor Tooling Integration - SpatialAuthoringWorkspace surfaces target binding states", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    SpatialAuthoringWorkspace workspace;
    workspace.Render(urpg::FrameContext{});

    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.status == "disabled");
    REQUIRE_FALSE(snapshot.has_target_scene);
    REQUIRE_FALSE(snapshot.has_target_overlay);
    REQUIRE_FALSE(snapshot.remediation.empty());

    urpg::scene::MapScene map("001", 10, 10);
    workspace.SetTargets(&map, nullptr);
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.status == "error");
    REQUIRE(snapshot.has_target_scene);
    REQUIRE_FALSE(snapshot.has_target_overlay);
    REQUIRE(snapshot.remediation.find("SpatialMapOverlay") != std::string::npos);

    workspace.SetTargets(&map, &overlay);
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.status == "ready");
    REQUIRE(snapshot.has_target_scene);
    REQUIRE(snapshot.has_target_overlay);
    REQUIRE(snapshot.remediation.empty());
}

TEST_CASE("Spatial Editor Tooling Integration - PropPlacement adds instances", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    PropPlacementPanel placement;
    overlay.mapId = "001";
    placement.SetTarget(&overlay);
    placement.SetSelectedAssetId("rock_01");

    placement.AddProp("rock_01", 1.5f, 0.0f, 2.5f);
    REQUIRE(overlay.props.size() == 1);
    REQUIRE(overlay.props[0].instanceId == "001:rock_01:0");
    REQUIRE(overlay.props[0].assetId == "rock_01");
    REQUIRE(overlay.props[0].posX == 1.5f);
    REQUIRE(placement.lastRenderSnapshot().has_target);
    REQUIRE(placement.lastRenderSnapshot().selected_asset_id == "rock_01");
    REQUIRE(placement.lastRenderSnapshot().prop_count == 1);
    REQUIRE(placement.lastRenderSnapshot().last_added_asset_id == std::optional<std::string>{"rock_01"});

    placement.AddProp("tree_01", 2.5f, 0.0f, 3.5f);
    placement.AddProp("rock_01", 3.5f, 0.0f, 4.5f);
    REQUIRE(overlay.props[1].instanceId == "001:tree_01:0");
    REQUIRE(overlay.props[2].instanceId == "001:rock_01:1");
}

TEST_CASE("Spatial Editor Tooling Integration - PropPlacement consumes attached project asset picker rows",
          "[editor][spatial][asset_attachment]") {
    PropPlacementPanel placement;
    placement.SetProjectAssetOptions({
        {
            "asset.hero",
            "content/assets/imported/asset.hero/hero.png",
            "sprite",
            {"level_builder", "sprite_selector"},
        },
        {
            "asset.click",
            "content/assets/imported/asset.click/click.wav",
            "audio",
            {"audio_selector"},
        },
    });

    REQUIRE(placement.lastRenderSnapshot().project_asset_options.size() == 1);
    REQUIRE(placement.lastRenderSnapshot().project_asset_options[0].asset_id == "asset.hero");
    REQUIRE(placement.lastRenderSnapshot().project_asset_options[0].project_path ==
            "content/assets/imported/asset.hero/hero.png");
    REQUIRE(placement.lastRenderSnapshot().project_asset_options[0].picker_kind == "sprite");
    REQUIRE(placement.lastRenderSnapshot().project_asset_options[0].targeted_for_level_builder);
    REQUIRE(placement.SelectProjectAsset("content/assets/imported/asset.hero/hero.png"));
    REQUIRE(placement.lastRenderSnapshot().selected_asset_id == "content/assets/imported/asset.hero/hero.png");
    REQUIRE(placement.lastRenderSnapshot().selected_project_asset_id == "asset.hero");
    REQUIRE_FALSE(placement.SelectProjectAsset("content/assets/imported/asset.click/click.wav"));
    REQUIRE(placement.lastRenderSnapshot().selected_project_asset_id == "asset.hero");
}

TEST_CASE("Spatial Editor Tooling Integration - Spatial overlay migration assigns deterministic prop instance ids", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    overlay.mapId = "001";
    overlay.props.push_back({"banner_01", 6.1f, 0.0f, 5.9f, 0.0f, 1.0f});
    overlay.props.push_back({"chest_01", 8.1f, 0.0f, 5.9f, 0.0f, 1.0f});
    overlay.props.push_back({"banner_01", 7.1f, 0.0f, 5.9f, 0.0f, 1.0f});

    urpg::presentation::EnsureStablePropInstanceIds(overlay);

    REQUIRE(overlay.props[0].instanceId == "001:banner_01:0");
    REQUIRE(overlay.props[1].instanceId == "001:chest_01:0");
    REQUIRE(overlay.props[2].instanceId == "001:banner_01:1");
}
