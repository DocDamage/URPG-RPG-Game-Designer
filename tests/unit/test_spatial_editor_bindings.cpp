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

TEST_CASE("Spatial Editor Tooling Integration - MapAbilityBinding panel discovers project abilities and binds them to map interaction runtime", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    const auto projectRoot = std::filesystem::temp_directory_path() / "urpg_map_ability_binding_panel";
    std::filesystem::remove_all(projectRoot);
    std::filesystem::create_directories(projectRoot / "content" / "abilities");

    urpg::ability::AuthoredAbilityAsset asset;
    asset.ability_id = "skill.map_interact";
    asset.effect_id = "effect.map_interact";
    asset.effect_attribute = "Attack";
    asset.effect_value = 14.0f;
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(asset,
                                                          projectRoot / "content" / "abilities" / "interact.json"));

    urpg::scene::MapScene map("001", 10, 10);
    MapAbilityBindingPanel panel;
    panel.SetTarget(&map);
    REQUIRE(panel.SetProjectRoot(projectRoot.string()));

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.has_target_scene);
    REQUIRE(snapshot.available_asset_count == 1);
    REQUIRE(snapshot.selected_asset_path == "content/abilities/interact.json");
    REQUIRE(snapshot.selected_ability_id == "skill.map_interact");

    REQUIRE(panel.BindSelectedAbilityToConfirmInteraction());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.binding_count == 1);
    REQUIRE(snapshot.runtime_ability_count == 1);
    REQUIRE(snapshot.bindings[0].trigger_id == "confirm_interact");
    REQUIRE(snapshot.bindings[0].ability_id == "skill.map_interact");

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("Attack", 100.0f);
    REQUIRE(panel.ActivateConfirmInteraction());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.latest_ability_id == "skill.map_interact");
    REQUIRE(snapshot.latest_outcome == "executed");
    REQUIRE(map.playerAbilitySystem().getAttribute("Attack", 0.0f) == 114.0f);

    std::filesystem::remove_all(projectRoot);
}

TEST_CASE("Spatial Editor Tooling Integration - MapAbilityBinding panel supports tile and prop placement-oriented bindings", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    const auto projectRoot = std::filesystem::temp_directory_path() / "urpg_map_ability_binding_panel_targets";
    std::filesystem::remove_all(projectRoot);
    std::filesystem::create_directories(projectRoot / "content" / "abilities");

    urpg::ability::AuthoredAbilityAsset tileAsset;
    tileAsset.ability_id = "skill.tile_bind";
    tileAsset.effect_id = "effect.tile_bind";
    tileAsset.effect_attribute = "Defense";
    tileAsset.effect_value = 11.0f;
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(tileAsset, projectRoot / "content" / "abilities" /
                                                                         "tile_bind.json"));

    urpg::ability::AuthoredAbilityAsset propAsset;
    propAsset.ability_id = "skill.prop_bind";
    propAsset.effect_id = "effect.prop_bind";
    propAsset.effect_attribute = "MagicDefense";
    propAsset.effect_value = 7.0f;
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(propAsset, projectRoot / "content" / "abilities" /
                                                                         "prop_bind.json"));

    urpg::scene::MapScene map("001", 10, 10);
    MapAbilityBindingPanel panel;
    panel.SetTarget(&map);
    REQUIRE(panel.SetProjectRoot(projectRoot.string()));
    panel.SetSelectedTriggerId("confirm_interact");
    REQUIRE(panel.SetPlacementTile(3, 4));
    REQUIRE(panel.SelectAsset(1));
    REQUIRE(panel.BindSelectedAbilityToPlacementTile());

    REQUIRE(panel.SelectAsset(0));
    REQUIRE(panel.SetSelectedPropAssetId("rock_01"));
    REQUIRE(panel.BindSelectedAbilityToSelectedProp());

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.binding_count == 2);
    REQUIRE(snapshot.bindings[0].scope == "tile");
    REQUIRE(snapshot.bindings[0].tile_x == 3);
    REQUIRE(snapshot.bindings[0].tile_y == 4);
    REQUIRE(snapshot.bindings[1].scope == "prop");
    REQUIRE(snapshot.bindings[1].prop_asset_id == "rock_01");

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("Defense", 100.0f);
    REQUIRE(panel.ActivatePlacementTileInteraction());
    snapshot = panel.lastRenderSnapshot();
    const std::string tileAbilityId = snapshot.bindings.front().ability_id;
    REQUIRE(snapshot.latest_ability_id == tileAbilityId);

    map.playerAbilitySystem().setCooldown(tileAbilityId, 0.0f);
    map.playerAbilitySystem().setAttribute("MagicDefense", 100.0f);
    REQUIRE(panel.ActivateSelectedPropInteraction());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.latest_ability_id == "skill.prop_bind");
    REQUIRE(map.playerAbilitySystem().getAttribute("MagicDefense", 0.0f) == 107.0f);

    std::filesystem::remove_all(projectRoot);
}

TEST_CASE("Spatial Editor Tooling Integration - MapAbilityBinding panel supports click-to-place tiles, prop picking from overlay, and painted regions", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    const auto projectRoot = std::filesystem::temp_directory_path() / "urpg_map_ability_binding_panel_visual";
    std::filesystem::remove_all(projectRoot);
    std::filesystem::create_directories(projectRoot / "content" / "abilities");

    urpg::ability::AuthoredAbilityAsset tileAsset;
    tileAsset.ability_id = "skill.visual_tile";
    tileAsset.effect_id = "effect.visual_tile";
    tileAsset.effect_attribute = "Attack";
    tileAsset.effect_value = 5.0f;
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(tileAsset, projectRoot / "content" / "abilities" /
                                                                         "visual_tile.json"));

    urpg::ability::AuthoredAbilityAsset propAsset;
    propAsset.ability_id = "skill.visual_prop";
    propAsset.effect_id = "effect.visual_prop";
    propAsset.effect_attribute = "Defense";
    propAsset.effect_value = 6.0f;
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(propAsset, projectRoot / "content" / "abilities" /
                                                                         "visual_prop.json"));

    urpg::ability::AuthoredAbilityAsset regionAsset;
    regionAsset.ability_id = "skill.visual_region";
    regionAsset.effect_id = "effect.visual_region";
    regionAsset.effect_attribute = "MagicDefense";
    regionAsset.effect_value = 4.0f;
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(regionAsset, projectRoot / "content" / "abilities" /
                                                                           "visual_region.json"));

    overlay.props.push_back({"banner_01", 6.1f, 0.0f, 5.9f, 0.0f, 1.0f});

    urpg::scene::MapScene map("001", 10, 10);
    MapAbilityBindingPanel panel;
    panel.SetTarget(&map);
    panel.SetSpatialTarget(&overlay);
    REQUIRE(panel.SetProjectRoot(projectRoot.string()));
    REQUIRE(panel.SelectAsset(2));

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.1f;

    REQUIRE(panel.BindSelectedAbilityToTileFromScreen(110.0f, 40.0f, projection));
    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.has_target_overlay);
    REQUIRE(snapshot.tile_overlay_count >= 2);
    REQUIRE(snapshot.bindings[0].scope == "tile");
    REQUIRE(snapshot.bindings[0].tile_x == 5);
    REQUIRE(snapshot.bindings[0].tile_y == 4);
    REQUIRE(snapshot.bindings[0].ability_id == "skill.visual_tile");
    REQUIRE(snapshot.tile_overlays[0].source == "placement");
    REQUIRE(snapshot.tile_overlays[0].pending);

    REQUIRE(panel.SelectAsset(0));
    REQUIRE(panel.SelectPropFromScreen(121.0f, 59.0f, projection, 1.0f));
    REQUIRE(panel.SelectPropHandle(0) == false);
    REQUIRE(panel.BindSelectedAbilityToSelectedProp());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.prop_handle_count == 1);
    REQUIRE(snapshot.prop_handles[0].selected);
    REQUIRE(snapshot.prop_handles[0].has_binding);
    REQUIRE(snapshot.bindings[1].scope == "prop");
    REQUIRE(snapshot.bindings[1].prop_asset_id == "banner_01");
    REQUIRE(snapshot.bindings[1].ability_id == "skill.visual_prop");

    REQUIRE(panel.SelectAsset(1));
    REQUIRE(panel.BeginPaintRegionFromScreen(100.0f, 50.0f, projection));
    REQUIRE(panel.UpdatePaintRegionFromScreen(130.0f, 70.0f, projection));
    REQUIRE(panel.CommitPaintedRegion());
    REQUIRE(panel.BindSelectedAbilityToPaintedRegion());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.region_overlay_count >= 3);
    REQUIRE(snapshot.bindings[2].scope == "region");
    REQUIRE(snapshot.bindings[2].ability_id == "skill.visual_region");
    REQUIRE(snapshot.bindings[2].region_min_x == 4);
    REQUIRE(snapshot.bindings[2].region_min_y == 5);
    REQUIRE(snapshot.bindings[2].region_max_x == 7);
    REQUIRE(snapshot.bindings[2].region_max_y == 7);
    REQUIRE(snapshot.region_overlays[1].source == "painted_region");
    REQUIRE(snapshot.region_overlays[1].pending);
    REQUIRE(snapshot.region_overlays[2].source == "binding");
    REQUIRE_FALSE(snapshot.region_overlays[2].pending);

    std::filesystem::remove_all(projectRoot);
}

TEST_CASE("Spatial Editor Tooling Integration - MapAbilityBinding panel tracks multiple painted regions and exposes overlay-ready snapshots", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    const auto projectRoot = std::filesystem::temp_directory_path() / "urpg_map_ability_binding_panel_regions";
    std::filesystem::remove_all(projectRoot);
    std::filesystem::create_directories(projectRoot / "content" / "abilities");

    urpg::ability::AuthoredAbilityAsset regionAsset;
    regionAsset.ability_id = "skill.multi_region";
    regionAsset.effect_id = "effect.multi_region";
    regionAsset.effect_attribute = "Defense";
    regionAsset.effect_value = 9.0f;
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(regionAsset, projectRoot / "content" / "abilities" /
                                                                           "multi_region.json"));

    urpg::scene::MapScene map("001", 10, 10);
    MapAbilityBindingPanel panel;
    panel.SetTarget(&map);
    panel.SetSpatialTarget(&overlay);
    REQUIRE(panel.SetProjectRoot(projectRoot.string()));

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.1f;

    REQUIRE(panel.BeginPaintRegionFromScreen(90.0f, 40.0f, projection));
    REQUIRE(panel.UpdatePaintRegionFromScreen(110.0f, 60.0f, projection));
    REQUIRE(panel.CommitPaintedRegion());

    REQUIRE(panel.BeginPaintRegionFromScreen(120.0f, 50.0f, projection));
    REQUIRE(panel.UpdatePaintRegionFromScreen(140.0f, 80.0f, projection));
    REQUIRE(panel.CommitPaintedRegion());
    REQUIRE(panel.SelectPaintedRegion(0));

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.placement.painted_regions.size() == 2);
    REQUIRE(snapshot.placement.painted_regions[0].selected);
    REQUIRE_FALSE(snapshot.placement.painted_regions[1].selected);
    REQUIRE(snapshot.region_overlay_count == 3);
    REQUIRE(snapshot.region_overlays[1].source == "painted_region");
    REQUIRE(snapshot.region_overlays[2].source == "painted_region");

    REQUIRE(panel.BindSelectedAbilityToCommittedRegions());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.binding_count == 2);
    REQUIRE(snapshot.region_overlay_count == 5);
    REQUIRE(snapshot.bindings[0].scope == "region");
    REQUIRE(snapshot.bindings[1].scope == "region");
    REQUIRE(snapshot.region_overlays[3].source == "binding");
    REQUIRE(snapshot.region_overlays[4].source == "binding");

    REQUIRE(panel.ClearPaintedRegions());
    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.placement.painted_regions.empty());

    std::filesystem::remove_all(projectRoot);
}
