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

TEST_CASE("Spatial Editor Tooling Integration", "[editor][spatial]") {
    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);

    SECTION("ElevationBrush modifies grid data") {
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

    SECTION("SpatialAuthoringWorkspace surfaces target binding states") {
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

    SECTION("PropPlacement adds instances") {
        PropPlacementPanel placement;
        placement.SetTarget(&overlay);
        placement.SetSelectedAssetId("rock_01");

        placement.AddProp("rock_01", 1.5f, 0.0f, 2.5f);
        REQUIRE(overlay.props.size() == 1);
        REQUIRE(overlay.props[0].assetId == "rock_01");
        REQUIRE(overlay.props[0].posX == 1.5f);
        REQUIRE(placement.lastRenderSnapshot().has_target);
        REQUIRE(placement.lastRenderSnapshot().selected_asset_id == "rock_01");
        REQUIRE(placement.lastRenderSnapshot().prop_count == 1);
        REQUIRE(placement.lastRenderSnapshot().last_added_asset_id == std::optional<std::string>{"rock_01"});
    }

    SECTION("MapAbilityBinding panel discovers project abilities and binds them to map interaction runtime") {
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

    SECTION("MapAbilityBinding panel supports tile and prop placement-oriented bindings") {
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

    SECTION("MapAbilityBinding panel supports click-to-place tiles, prop picking from overlay, and painted regions") {
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

    SECTION("MapAbilityBinding panel tracks multiple painted regions and exposes overlay-ready snapshots") {
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

    SECTION("SpatialAbilityCanvas panel supports direct click selection and rebinding of existing targets") {
        const auto projectRoot = std::filesystem::temp_directory_path() / "urpg_spatial_ability_canvas_panel";
        std::filesystem::remove_all(projectRoot);
        std::filesystem::create_directories(projectRoot / "content" / "abilities");

        const auto saveAsset = [&](const std::string& fileName, const std::string& abilityId,
                                   const std::string& attribute, float value) {
            urpg::ability::AuthoredAbilityAsset asset;
            asset.ability_id = abilityId;
            asset.effect_id = abilityId + ".effect";
            asset.effect_attribute = attribute;
            asset.effect_value = value;
            return urpg::ability::saveAuthoredAbilityAssetToFile(asset,
                                                                 projectRoot / "content" / "abilities" / fileName);
        };

        REQUIRE(saveAsset("tile_old.json", "skill.canvas_tile_old", "Attack", 5.0f));
        REQUIRE(saveAsset("tile_new.json", "skill.canvas_tile_new", "Attack", 8.0f));
        REQUIRE(saveAsset("prop_old.json", "skill.canvas_prop_old", "Defense", 6.0f));
        REQUIRE(saveAsset("prop_new.json", "skill.canvas_prop_new", "Defense", 9.0f));
        REQUIRE(saveAsset("region_old.json", "skill.canvas_region_old", "MagicDefense", 4.0f));
        REQUIRE(saveAsset("region_new.json", "skill.canvas_region_new", "MagicDefense", 10.0f));

        overlay.props.push_back({"banner_01", 6.1f, 0.0f, 5.9f, 0.0f, 1.0f});

        urpg::scene::MapScene map("001", 10, 10);
        MapAbilityBindingPanel bindingPanel;
        bindingPanel.SetTarget(&map);
        bindingPanel.SetSpatialTarget(&overlay);
        REQUIRE(bindingPanel.SetProjectRoot(projectRoot.string()));

        const auto selectAbility = [&](const std::string& abilityId) {
            const auto snapshot = bindingPanel.lastRenderSnapshot();
            for (size_t i = 0; i < snapshot.assets.size(); ++i) {
                if (snapshot.assets[i].ability_id == abilityId) {
                    bindingPanel.SelectAsset(i);
                    return true;
                }
            }
            return false;
        };

        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 0.1f;

        REQUIRE(selectAbility("skill.canvas_tile_old"));
        REQUIRE(bindingPanel.BindSelectedAbilityToTileFromScreen(110.0f, 40.0f, projection));

        REQUIRE(selectAbility("skill.canvas_prop_old"));
        REQUIRE(bindingPanel.SelectPropFromScreen(121.0f, 59.0f, projection, 1.0f));
        REQUIRE(bindingPanel.BindSelectedAbilityToSelectedProp());

        REQUIRE(selectAbility("skill.canvas_region_old"));
        REQUIRE(bindingPanel.BeginPaintRegionFromScreen(100.0f, 50.0f, projection));
        REQUIRE(bindingPanel.UpdatePaintRegionFromScreen(130.0f, 70.0f, projection));
        REQUIRE(bindingPanel.BindSelectedAbilityToPaintedRegion());

        SpatialAbilityCanvasPanel canvasPanel;
        canvasPanel.SetSpatialTarget(&overlay);
        canvasPanel.SetBindingPanel(&bindingPanel);
        canvasPanel.SetProjectionSettings(projection);

        REQUIRE(canvasPanel.ClickAtScreen(110.0f, 40.0f));
        auto canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasSnapshot.has_target_overlay);
        REQUIRE(canvasSnapshot.has_binding_panel);
        REQUIRE(canvasSnapshot.selection.kind == SpatialAbilityCanvasPanel::SelectionKind::Tile);
        REQUIRE(canvasSnapshot.selection.tile_x == 5);
        REQUIRE(canvasSnapshot.selection.tile_y == 4);
        REQUIRE(selectAbility("skill.canvas_tile_new"));
        REQUIRE(canvasPanel.ApplySelectedAssetToSelection());
        auto bindingSnapshot = bindingPanel.lastRenderSnapshot();
        const auto reboundTileBinding =
            std::find_if(bindingSnapshot.bindings.begin(), bindingSnapshot.bindings.end(), [](const auto& binding) {
                return binding.scope == "tile" && binding.ability_id == "skill.canvas_tile_new";
            });
        REQUIRE(reboundTileBinding != bindingSnapshot.bindings.end());

        REQUIRE(canvasPanel.ClickAtScreen(121.0f, 59.0f));
        canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasSnapshot.selection.kind == SpatialAbilityCanvasPanel::SelectionKind::Prop);
        REQUIRE(canvasSnapshot.selection.prop_asset_id == "banner_01");
        REQUIRE(selectAbility("skill.canvas_prop_new"));
        REQUIRE(canvasPanel.ApplySelectedAssetToSelection());
        bindingSnapshot = bindingPanel.lastRenderSnapshot();
        const auto reboundPropBinding =
            std::find_if(bindingSnapshot.bindings.begin(), bindingSnapshot.bindings.end(), [](const auto& binding) {
                return binding.scope == "prop" && binding.ability_id == "skill.canvas_prop_new";
            });
        REQUIRE(reboundPropBinding != bindingSnapshot.bindings.end());

        REQUIRE(canvasPanel.ClickAtScreen(100.0f, 60.0f));
        canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasSnapshot.selection.kind == SpatialAbilityCanvasPanel::SelectionKind::Region);
        REQUIRE(canvasSnapshot.selection.region_min_x == 4);
        REQUIRE(canvasSnapshot.selection.region_min_y == 5);
        REQUIRE(canvasSnapshot.selection.region_max_x == 7);
        REQUIRE(canvasSnapshot.selection.region_max_y == 7);
        REQUIRE(selectAbility("skill.canvas_region_new"));
        REQUIRE(canvasPanel.ApplySelectedAssetToSelection());
        bindingSnapshot = bindingPanel.lastRenderSnapshot();
        const auto reboundRegionBinding =
            std::find_if(bindingSnapshot.bindings.begin(), bindingSnapshot.bindings.end(), [](const auto& binding) {
                return binding.scope == "region" && binding.ability_id == "skill.canvas_region_new";
            });
        REQUIRE(reboundRegionBinding != bindingSnapshot.bindings.end());

        std::filesystem::remove_all(projectRoot);
    }

    SECTION("SpatialAbilityCanvas panel supports dragging bindings, resizing regions, and exposing inline badges") {
        const auto projectRoot = std::filesystem::temp_directory_path() / "urpg_spatial_ability_canvas_drag";
        std::filesystem::remove_all(projectRoot);
        std::filesystem::create_directories(projectRoot / "content" / "abilities");

        const auto saveAsset = [&](const std::string& fileName, const std::string& abilityId,
                                   const std::string& attribute, float value) {
            urpg::ability::AuthoredAbilityAsset asset;
            asset.ability_id = abilityId;
            asset.effect_id = abilityId + ".effect";
            asset.effect_attribute = attribute;
            asset.effect_value = value;
            return urpg::ability::saveAuthoredAbilityAssetToFile(asset,
                                                                 projectRoot / "content" / "abilities" / fileName);
        };

        REQUIRE(saveAsset("tile_drag.json", "skill.canvas_tile_drag", "Attack", 5.0f));
        REQUIRE(saveAsset("prop_drag.json", "skill.canvas_prop_drag", "Defense", 6.0f));
        REQUIRE(saveAsset("region_drag.json", "skill.canvas_region_drag", "MagicDefense", 4.0f));

        overlay.props.push_back({"banner_01", 6.1f, 0.0f, 5.9f, 0.0f, 1.0f});
        overlay.props.push_back({"chest_01", 8.1f, 0.0f, 5.9f, 0.0f, 1.0f});

        urpg::scene::MapScene map("001", 10, 10);
        MapAbilityBindingPanel bindingPanel;
        bindingPanel.SetTarget(&map);
        bindingPanel.SetSpatialTarget(&overlay);
        REQUIRE(bindingPanel.SetProjectRoot(projectRoot.string()));

        const auto selectAbility = [&](const std::string& abilityId) {
            const auto snapshot = bindingPanel.lastRenderSnapshot();
            for (size_t i = 0; i < snapshot.assets.size(); ++i) {
                if (snapshot.assets[i].ability_id == abilityId) {
                    bindingPanel.SelectAsset(i);
                    return true;
                }
            }
            return false;
        };

        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 0.1f;

        REQUIRE(selectAbility("skill.canvas_tile_drag"));
        REQUIRE(bindingPanel.BindSelectedAbilityToTileFromScreen(110.0f, 40.0f, projection));

        REQUIRE(selectAbility("skill.canvas_prop_drag"));
        REQUIRE(bindingPanel.SelectPropFromScreen(121.0f, 59.0f, projection, 1.0f));
        REQUIRE(bindingPanel.BindSelectedAbilityToSelectedProp());

        REQUIRE(selectAbility("skill.canvas_region_drag"));
        REQUIRE(bindingPanel.BeginPaintRegionFromScreen(100.0f, 50.0f, projection));
        REQUIRE(bindingPanel.UpdatePaintRegionFromScreen(130.0f, 70.0f, projection));
        REQUIRE(bindingPanel.BindSelectedAbilityToPaintedRegion());

        SpatialAbilityCanvasPanel canvasPanel;
        canvasPanel.SetSpatialTarget(&overlay);
        canvasPanel.SetBindingPanel(&bindingPanel);
        canvasPanel.SetProjectionSettings(projection);

        REQUIRE(canvasPanel.ClickAtScreen(110.0f, 40.0f));
        REQUIRE(canvasPanel.DragSelectionToScreen(130.0f, 40.0f));
        auto bindingSnapshot = bindingPanel.lastRenderSnapshot();
        const auto movedTileBinding =
            std::find_if(bindingSnapshot.bindings.begin(), bindingSnapshot.bindings.end(), [](const auto& binding) {
                return binding.scope == "tile" && binding.ability_id == "skill.canvas_tile_drag";
            });
        REQUIRE(movedTileBinding != bindingSnapshot.bindings.end());
        REQUIRE(movedTileBinding->tile_x == 7);
        REQUIRE(movedTileBinding->tile_y == 4);

        REQUIRE(canvasPanel.ClickAtScreen(121.0f, 59.0f));
        REQUIRE(canvasPanel.DragSelectionToScreen(141.0f, 59.0f));
        bindingSnapshot = bindingPanel.lastRenderSnapshot();
        const auto movedPropBinding =
            std::find_if(bindingSnapshot.bindings.begin(), bindingSnapshot.bindings.end(), [](const auto& binding) {
                return binding.scope == "prop" && binding.ability_id == "skill.canvas_prop_drag";
            });
        REQUIRE(movedPropBinding != bindingSnapshot.bindings.end());
        REQUIRE(movedPropBinding->prop_asset_id == "chest_01");

        REQUIRE(canvasPanel.ClickAtScreen(100.0f, 60.0f));
        REQUIRE(canvasPanel.ResizeSelectedRegionToScreen(140.0f, 80.0f));
        bindingSnapshot = bindingPanel.lastRenderSnapshot();
        const auto resizedRegionBinding =
            std::find_if(bindingSnapshot.bindings.begin(), bindingSnapshot.bindings.end(), [](const auto& binding) {
                return binding.scope == "region" && binding.ability_id == "skill.canvas_region_drag";
            });
        REQUIRE(resizedRegionBinding != bindingSnapshot.bindings.end());
        REQUIRE(resizedRegionBinding->region_min_x == 4);
        REQUIRE(resizedRegionBinding->region_min_y == 5);
        REQUIRE(resizedRegionBinding->region_max_x == 8);
        REQUIRE(resizedRegionBinding->region_max_y == 8);

        const auto canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasSnapshot.badge_count >= 3);
        REQUIRE(canvasSnapshot.badges[0].label.find("@") != std::string::npos);

        std::filesystem::remove_all(projectRoot);
    }

    SECTION("SpatialAbilityCanvas panel surfaces hover previews, direct trigger switching, and overlap warnings") {
        const auto projectRoot = std::filesystem::temp_directory_path() / "urpg_spatial_ability_canvas_wysiwyg";
        std::filesystem::remove_all(projectRoot);
        std::filesystem::create_directories(projectRoot / "content" / "abilities");

        const auto saveAsset = [&](const std::string& fileName, const std::string& abilityId,
                                   const std::string& attribute, float value) {
            urpg::ability::AuthoredAbilityAsset asset;
            asset.ability_id = abilityId;
            asset.effect_id = abilityId + ".effect";
            asset.effect_attribute = attribute;
            asset.effect_value = value;
            return urpg::ability::saveAuthoredAbilityAssetToFile(asset,
                                                                 projectRoot / "content" / "abilities" / fileName);
        };

        REQUIRE(saveAsset("hover_tile.json", "skill.hover_tile", "Attack", 5.0f));
        REQUIRE(saveAsset("hover_prop.json", "skill.hover_prop", "Defense", 6.0f));
        REQUIRE(saveAsset("hover_region.json", "skill.hover_region", "MagicDefense", 4.0f));
        REQUIRE(saveAsset("hover_region_alt.json", "skill.hover_region_alt", "MagicDefense", 7.0f));

        overlay.props.push_back({"banner_01", 6.1f, 0.0f, 5.9f, 0.0f, 1.0f});

        urpg::scene::MapScene map("001", 10, 10);
        MapAbilityBindingPanel bindingPanel;
        bindingPanel.SetTarget(&map);
        bindingPanel.SetSpatialTarget(&overlay);
        REQUIRE(bindingPanel.SetProjectRoot(projectRoot.string()));

        const auto selectAbility = [&](const std::string& abilityId) {
            const auto snapshot = bindingPanel.lastRenderSnapshot();
            for (size_t i = 0; i < snapshot.assets.size(); ++i) {
                if (snapshot.assets[i].ability_id == abilityId) {
                    bindingPanel.SelectAsset(i);
                    return true;
                }
            }
            return false;
        };

        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 0.1f;

        REQUIRE(selectAbility("skill.hover_tile"));
        REQUIRE(bindingPanel.BindSelectedAbilityToTileFromScreen(110.0f, 40.0f, projection));
        REQUIRE(bindingPanel.SetSelectedTriggerId("touch_interact"));
        REQUIRE_FALSE(bindingPanel.SetSelectedTriggerId("touch_interact"));
        REQUIRE(bindingPanel.BindSelectedAbilityToTileFromScreen(110.0f, 40.0f, projection));

        REQUIRE(selectAbility("skill.hover_prop"));
        bindingPanel.SetSelectedTriggerId("confirm_interact");
        REQUIRE(bindingPanel.SelectPropFromScreen(121.0f, 59.0f, projection, 1.0f));
        REQUIRE(bindingPanel.BindSelectedAbilityToSelectedProp());

        REQUIRE(selectAbility("skill.hover_region"));
        bindingPanel.SetSelectedTriggerId("confirm_interact");
        REQUIRE(bindingPanel.BeginPaintRegionFromScreen(100.0f, 50.0f, projection));
        REQUIRE(bindingPanel.UpdatePaintRegionFromScreen(130.0f, 70.0f, projection));
        REQUIRE(bindingPanel.BindSelectedAbilityToPaintedRegion());

        REQUIRE(selectAbility("skill.hover_region_alt"));
        REQUIRE(bindingPanel.SetSelectedTriggerId("touch_interact"));
        REQUIRE(bindingPanel.BeginPaintRegionFromScreen(110.0f, 60.0f, projection));
        REQUIRE(bindingPanel.UpdatePaintRegionFromScreen(140.0f, 80.0f, projection));
        REQUIRE(bindingPanel.BindSelectedAbilityToPaintedRegion());
        REQUIRE(bindingPanel.SetPlacementTile(0, 0));

        SpatialAbilityCanvasPanel canvasPanel;
        canvasPanel.SetSpatialTarget(&overlay);
        canvasPanel.SetBindingPanel(&bindingPanel);
        canvasPanel.SetProjectionSettings(projection);

        REQUIRE(canvasPanel.ClickAtScreen(110.0f, 40.0f));
        REQUIRE(canvasPanel.HoverAtScreen(120.0f, 40.0f));
        auto canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasSnapshot.hover_preview.active);
        REQUIRE(canvasSnapshot.hover_preview.kind == SpatialAbilityCanvasPanel::SelectionKind::Tile);
        REQUIRE(canvasSnapshot.hover_preview.tile_x == 6);
        REQUIRE_FALSE(canvasSnapshot.hover_preview.would_conflict);
        REQUIRE(canvasSnapshot.has_conflicts);
        REQUIRE(canvasSnapshot.conflict_count >= 2);
        REQUIRE(canvasSnapshot.mode_badge_count == 4);
        REQUIRE(canvasSnapshot.hover_affordance_count >= 2);
        REQUIRE(canvasSnapshot.conflict_action_chip_count >= canvasSnapshot.conflict_count);
        REQUIRE(canvasSnapshot.available_triggers.size() >= 4);
        REQUIRE_FALSE(canvasSnapshot.selection_trigger_menu.empty());
        const auto abilitiesBadge = std::find_if(canvasSnapshot.mode_badges.begin(), canvasSnapshot.mode_badges.end(),
                                                 [](const auto& badge) { return badge.action_id == "abilities"; });
        REQUIRE(abilitiesBadge != canvasSnapshot.mode_badges.end());
        REQUIRE(abilitiesBadge->enabled);
        const auto bindAbilityAffordance =
            std::find_if(canvasSnapshot.hover_affordances.begin(), canvasSnapshot.hover_affordances.end(),
                         [](const auto& affordance) { return affordance.action_id == "abilities"; });
        REQUIRE(bindAbilityAffordance != canvasSnapshot.hover_affordances.end());
        REQUIRE(bindAbilityAffordance->target_kind == "tile");
        const auto tileRecommendedTrigger =
            std::find_if(canvasSnapshot.selection_trigger_menu.begin(), canvasSnapshot.selection_trigger_menu.end(),
                         [](const auto& entry) { return entry.recommended; });
        REQUIRE(tileRecommendedTrigger != canvasSnapshot.selection_trigger_menu.end());
        REQUIRE(tileRecommendedTrigger->trigger_id == "confirm_interact");
        const auto tileConflictWarning =
            std::find_if(canvasSnapshot.conflicts.begin(), canvasSnapshot.conflicts.end(),
                         [](const auto& warning) { return warning.kind == "tile_multi_trigger"; });
        REQUIRE(tileConflictWarning != canvasSnapshot.conflicts.end());
        REQUIRE(tileConflictWarning->can_swap_triggers);
        REQUIRE(tileConflictWarning->can_replace_secondary);
        const size_t tileConflictIndex =
            static_cast<size_t>(std::distance(canvasSnapshot.conflicts.begin(), tileConflictWarning));
        const auto swapChip =
            std::find_if(canvasSnapshot.conflict_action_chips.begin(), canvasSnapshot.conflict_action_chips.end(),
                         [&](const auto& chip) {
                             return chip.conflict_index == tileConflictIndex &&
                                    chip.action_id == "conflict:" + std::to_string(tileConflictIndex) + ":swap";
                         });
        REQUIRE(swapChip != canvasSnapshot.conflict_action_chips.end());
        REQUIRE(canvasPanel.SwapConflictTriggers(tileConflictIndex));
        canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasPanel.ReplaceSecondaryWithPrimaryAsset(tileConflictIndex));
        canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasPanel.ResolveConflictByRemovingSecondary(tileConflictIndex));
        canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasSnapshot.conflict_count >= 1);

        const std::string switchedTriggerId =
            canvasSnapshot.selection.trigger_id == "touch_interact" ? "confirm_interact" : "touch_interact";
        REQUIRE(canvasPanel.SetSelectionTriggerId(switchedTriggerId));
        canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasSnapshot.selection.trigger_id == switchedTriggerId);

        const auto switchedBindingsSnapshot = bindingPanel.lastRenderSnapshot();
        const auto switchedTileBinding =
            std::find_if(switchedBindingsSnapshot.bindings.begin(), switchedBindingsSnapshot.bindings.end(),
                         [&](const auto& binding) {
                             return binding.scope == "tile" && binding.ability_id == "skill.hover_tile" &&
                                    binding.trigger_id == switchedTriggerId;
                         });
        REQUIRE(switchedTileBinding != switchedBindingsSnapshot.bindings.end());

        REQUIRE(canvasPanel.ClickAtScreen(100.0f, 60.0f));
        REQUIRE(canvasPanel.HoverAtScreen(120.0f, 70.0f));
        canvasSnapshot = canvasPanel.lastRenderSnapshot();
        REQUIRE(canvasSnapshot.selection.kind == SpatialAbilityCanvasPanel::SelectionKind::Region);
        REQUIRE(canvasSnapshot.hover_preview.kind == SpatialAbilityCanvasPanel::SelectionKind::Region);
        REQUIRE_FALSE(canvasSnapshot.hover_preview.would_conflict);

        const auto overlapWarning = std::find_if(canvasSnapshot.conflicts.begin(), canvasSnapshot.conflicts.end(),
                                                 [](const auto& warning) { return warning.kind == "region_layered"; });
        if (overlapWarning != canvasSnapshot.conflicts.end()) {
            REQUIRE(overlapWarning->severity == "warning");
            REQUIRE(overlapWarning->policy == "layered_region");
        }

        std::filesystem::remove_all(projectRoot);
    }

    SECTION("SpatialAuthoringWorkspace composes elevation, prop, binding, and canvas authoring into one surface") {
        const auto projectRoot = std::filesystem::temp_directory_path() / "urpg_spatial_authoring_workspace";
        std::filesystem::remove_all(projectRoot);
        std::filesystem::create_directories(projectRoot / "content" / "abilities");

        urpg::ability::AuthoredAbilityAsset asset;
        asset.ability_id = "skill.workspace";
        asset.effect_id = "effect.workspace";
        asset.effect_attribute = "Attack";
        asset.effect_value = 5.0f;
        REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(asset, projectRoot / "content" / "abilities" /
                                                                         "workspace.json"));

        urpg::scene::MapScene map("001", 10, 10);
        SpatialAuthoringWorkspace workspace;
        workspace.SetTargets(&map, &overlay);
        REQUIRE(workspace.SetProjectRoot(projectRoot.string()));
        REQUIRE(workspace.bindingPanel().RefreshProjectAssets());

        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 0.1f;
        workspace.SetProjectionSettings(projection);
        workspace.SetAvailableTriggers({"confirm_interact", "touch_interact"});
        workspace.SetActiveMode(SpatialAuthoringWorkspace::ToolMode::Abilities);

        workspace.elevationPanel().SetBrushSize(1);
        workspace.elevationPanel().ApplyBrush(4, 4, 2);
        workspace.propPanel().SetSelectedAssetId("rock_01");
        workspace.propPanel().AddProp("rock_01", 4.5f, 0.0f, 4.5f);

        const auto bindingAssets = workspace.bindingPanel().lastRenderSnapshot().assets;
        REQUIRE_FALSE(bindingAssets.empty());
        workspace.bindingPanel().BindSelectedAbilityToTileFromScreen(110.0f, 40.0f, projection);
        workspace.canvasPanel().ClickAtScreen(110.0f, 40.0f);
        workspace.canvasPanel().HoverAtScreen(120.0f, 40.0f);

        urpg::FrameContext frameContext{0.016f, 1};
        workspace.Render(frameContext);
        const auto snapshot = workspace.lastRenderSnapshot();
        REQUIRE(snapshot.has_target_scene);
        REQUIRE(snapshot.has_target_overlay);
        REQUIRE(snapshot.elevation.has_target);
        REQUIRE(snapshot.props.has_target);
        REQUIRE(snapshot.bindings.has_target_scene);
        REQUIRE(snapshot.canvas.has_binding_panel);
        REQUIRE(snapshot.canvas.hover_preview.active);
        REQUIRE(snapshot.canvas.available_triggers.size() == 2);
        REQUIRE(snapshot.canvas.mode_badge_count == 4);
        REQUIRE(snapshot.canvas.hover_affordance_count >= 2);
        REQUIRE(snapshot.toolbar.active_mode == "abilities");
        REQUIRE(snapshot.toolbar.has_conflicts == snapshot.canvas.has_conflicts);
        REQUIRE(snapshot.toolbar.actions.size() == 5);
        REQUIRE_FALSE(snapshot.elevation.visible);
        REQUIRE_FALSE(snapshot.props.visible);
        REQUIRE(snapshot.bindings.visible);
        REQUIRE(snapshot.canvas.visible);

        std::filesystem::remove_all(projectRoot);
    }

    SECTION("SpatialAuthoringWorkspace toolbar drives shared canvas workflows and suggested conflict resolution") {
        const auto projectRoot = std::filesystem::temp_directory_path() / "urpg_spatial_authoring_workspace_toolbar";
        std::filesystem::remove_all(projectRoot);
        std::filesystem::create_directories(projectRoot / "content" / "abilities");

        auto saveAsset = [&](const std::string& file_name, const std::string& ability_id, float value) {
            urpg::ability::AuthoredAbilityAsset asset;
            asset.ability_id = ability_id;
            asset.effect_id = ability_id + ".effect";
            asset.effect_attribute = "Attack";
            asset.effect_value = value;
            return urpg::ability::saveAuthoredAbilityAssetToFile(asset,
                                                                 projectRoot / "content" / "abilities" / file_name);
        };

        REQUIRE(saveAsset("toolbar_tile.json", "skill.toolbar_tile", 5.0f));
        REQUIRE(saveAsset("toolbar_tile_alt.json", "skill.toolbar_tile_alt", 7.0f));

        urpg::scene::MapScene map("001", 10, 10);
        SpatialAuthoringWorkspace workspace;
        workspace.SetTargets(&map, &overlay);
        REQUIRE(workspace.SetProjectRoot(projectRoot.string()));

        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 0.1f;
        workspace.SetProjectionSettings(projection);
        workspace.SetAvailableTriggers({"confirm_interact", "touch_interact"});

        workspace.elevationPanel().SetBrushHeight(3.0f);
        REQUIRE(workspace.ActivateToolbarAction("elevation"));
        REQUIRE(workspace.RouteCanvasPrimaryAction(100.0f, 50.0f));
        REQUIRE(overlay.elevation.levels[5 * 10 + 4] == 3);

        workspace.propPanel().SetSelectedAssetId("oak_01");
        REQUIRE(workspace.ActivateToolbarAction("props"));
        REQUIRE(workspace.RouteCanvasPrimaryAction(120.0f, 50.0f));
        REQUIRE_FALSE(overlay.props.empty());
        REQUIRE(overlay.props.back().assetId == "oak_01");

        REQUIRE(workspace.ActivateToolbarAction("abilities"));
        const auto selectWorkspaceAbility = [&](const std::string& ability_id) {
            const auto binding_snapshot = workspace.bindingPanel().lastRenderSnapshot();
            REQUIRE_FALSE(binding_snapshot.assets.empty());
            for (size_t i = 0; i < binding_snapshot.assets.size(); ++i) {
                if (binding_snapshot.assets[i].ability_id == ability_id) {
                    return workspace.bindingPanel().SelectAsset(i) || binding_snapshot.assets[i].selected;
                }
            }
            return false;
        };
        REQUIRE(selectWorkspaceAbility("skill.toolbar_tile"));
        REQUIRE(workspace.bindingPanel().BindSelectedAbilityToTileFromScreen(110.0f, 40.0f, projection));
        REQUIRE(workspace.bindingPanel().SetSelectedTriggerId("touch_interact"));
        REQUIRE(selectWorkspaceAbility("skill.toolbar_tile_alt"));
        REQUIRE(workspace.bindingPanel().BindSelectedAbilityToTileFromScreen(110.0f, 40.0f, projection));

        REQUIRE(workspace.RouteCanvasPrimaryAction(110.0f, 40.0f));
        auto snapshot = workspace.lastRenderSnapshot();
        REQUIRE(snapshot.toolbar.active_mode == "abilities");
        REQUIRE(snapshot.toolbar.selected_prop_asset_id == "oak_01");
        const auto propsBadge = std::find_if(snapshot.canvas.mode_badges.begin(), snapshot.canvas.mode_badges.end(),
                                             [](const auto& badge) { return badge.action_id == "props"; });
        REQUIRE(propsBadge != snapshot.canvas.mode_badges.end());
        REQUIRE_FALSE(propsBadge->active);
        REQUIRE(workspace.ActivateCanvasAction("props"));
        snapshot = workspace.lastRenderSnapshot();
        REQUIRE(snapshot.toolbar.active_mode == "props");
        const auto propsHoverAffordance =
            std::find_if(snapshot.canvas.hover_affordances.begin(), snapshot.canvas.hover_affordances.end(),
                         [](const auto& affordance) { return affordance.action_id == "props"; });
        if (propsHoverAffordance != snapshot.canvas.hover_affordances.end()) {
            REQUIRE(workspace.ActivateCanvasAction(propsHoverAffordance->action_id));
            snapshot = workspace.lastRenderSnapshot();
            REQUIRE(snapshot.toolbar.active_mode == "props");
        }
        REQUIRE(workspace.ActivateCanvasAction("abilities"));
        snapshot = workspace.lastRenderSnapshot();
        REQUIRE(snapshot.toolbar.active_mode == "abilities");
        if (snapshot.toolbar.can_apply_suggested_conflict_resolution) {
            REQUIRE(snapshot.toolbar.conflict_count >= 1);
            const auto preferredChip =
                std::find_if(snapshot.canvas.conflict_action_chips.begin(), snapshot.canvas.conflict_action_chips.end(),
                             [](const auto& chip) { return chip.recommended; });
            REQUIRE(preferredChip != snapshot.canvas.conflict_action_chips.end());
            REQUIRE(workspace.ActivateCanvasAction(preferredChip->action_id));
            snapshot = workspace.lastRenderSnapshot();
            REQUIRE(snapshot.toolbar.conflict_count < 2);
            REQUIRE(snapshot.canvas.conflict_count < 2);
        }

        std::filesystem::remove_all(projectRoot);
    }

    SECTION("PropPlacement projects viewport center to camera-centered world position") {
        overlay.elevation.levels[5 * 10 + 4] = 3;

        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 1.0f;

        float worldX = -1.0f;
        float worldY = -1.0f;
        float worldZ = -1.0f;
        const bool projected =
            PropPlacementPanel::TryProjectScreenToGround(overlay, 100.0f, 50.0f, projection, worldX, worldY, worldZ);

        REQUIRE(projected);
        REQUIRE(worldX == 4.0f);
        REQUIRE(worldY == 1.5f);
        REQUIRE(worldZ == 5.0f);
    }

    SECTION("PropPlacement converts screen offsets into world xz offsets") {
        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 0.1f;

        float worldX = -1.0f;
        float worldY = -1.0f;
        float worldZ = -1.0f;
        const bool projected =
            PropPlacementPanel::TryProjectScreenToGround(overlay, 120.0f, 40.0f, projection, worldX, worldY, worldZ);

        REQUIRE(projected);
        REQUIRE(worldX == 6.0f);
        REQUIRE(worldY == 0.0f);
        REQUIRE(worldZ == 4.0f);
    }

    SECTION("PropPlacement adds projected props using sampled elevation") {
        overlay.elevation.levels[6 * 10 + 7] = 2;

        PropPlacementPanel placement;
        placement.SetTarget(&overlay);

        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 0.1f;

        const bool placed = placement.AddPropFromScreen("oak_01", 130.0f, 60.0f, projection);

        REQUIRE(placed);
        REQUIRE(overlay.props.size() == 1);
        REQUIRE(overlay.props[0].assetId == "oak_01");
        REQUIRE(overlay.props[0].posX == 7.0f);
        REQUIRE(overlay.props[0].posY == 1.0f);
        REQUIRE(overlay.props[0].posZ == 6.0f);
    }

    SECTION("PropPlacement rejects screen projection without a target overlay") {
        PropPlacementPanel placement;

        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 0.1f;

        REQUIRE_FALSE(placement.AddPropFromScreen("oak_01", 100.0f, 50.0f, projection));
    }

    SECTION("PropPlacement rejects out-of-bounds projected props") {
        PropPlacementPanel placement;
        placement.SetTarget(&overlay);

        PropPlacementPanel::ScreenProjectionSettings projection;
        projection.viewportWidth = 200.0f;
        projection.viewportHeight = 100.0f;
        projection.cameraCenterX = 4.0f;
        projection.cameraCenterZ = 5.0f;
        projection.worldUnitsPerPixel = 0.25f;
        projection.rejectOutOfBounds = true;

        REQUIRE_FALSE(placement.AddPropFromScreen("oak_01", -50.0f, 50.0f, projection));
        REQUIRE(overlay.props.empty());
    }

    SECTION("PropPlacement adds projected props from camera ray intersection") {
        overlay.elevation.width = 8;
        overlay.elevation.height = 8;
        overlay.elevation.levels.assign(64, 0);
        overlay.elevation.stepHeight = 0.5f;
        overlay.elevation.levels[3 * 8 + 4] = 4;

        PropPlacementPanel placement;
        placement.SetTarget(&overlay);

        CameraState cameraState;
        cameraState.targetPos = {4.5f, 2.0f, 3.5f};
        cameraState.currentPitch = 45.0f;
        cameraState.currentYaw = 0.0f;
        cameraState.currentDistance = 8.0f;

        CameraProfile profile;
        profile.lookAtOffset = {0.0f, 0.0f, 0.0f};
        profile.fov = 60.0f;

        const ViewportRect viewport{0.0f, 0.0f, 1600.0f, 900.0f};
        const Vector2f screenPoint{800.0f, 450.0f};

        const bool placed = placement.AddPropFromScreen("banner_01", screenPoint, viewport, cameraState, profile);

        REQUIRE(placed);
        REQUIRE(overlay.props.size() == 1);
        REQUIRE(overlay.props[0].assetId == "banner_01");
        CHECK(overlay.props[0].posX == Catch::Approx(4.5f).margin(0.1f));
        CHECK(overlay.props[0].posY == Catch::Approx(2.0f).margin(0.05f));
        CHECK(overlay.props[0].posZ == Catch::Approx(3.5f).margin(0.1f));
    }

    SECTION("PropPlacement rejects camera ray misses outside authored grid") {
        overlay.elevation.width = 8;
        overlay.elevation.height = 8;
        overlay.elevation.levels.assign(64, 0);

        PropPlacementPanel placement;
        placement.SetTarget(&overlay);

        CameraState cameraState;
        cameraState.targetPos = {20.0f, 0.0f, 20.0f};
        cameraState.currentPitch = 45.0f;
        cameraState.currentYaw = 0.0f;
        cameraState.currentDistance = 8.0f;

        CameraProfile profile;
        profile.lookAtOffset = {0.0f, 0.0f, 0.0f};
        profile.fov = 60.0f;

        const ViewportRect viewport{0.0f, 0.0f, 1280.0f, 720.0f};
        const Vector2f screenPoint{640.0f, 360.0f};

        REQUIRE_FALSE(placement.AddPropFromScreen("banner_01", screenPoint, viewport, cameraState, profile));
        REQUIRE(overlay.props.empty());
    }

    SECTION("SpatialMapOverlay Serialization Round-trip") {
        overlay.mapId = "test_map";
        overlay.fog.density = 0.12f;
        overlay.props.push_back({"pine_tree", 10, 0, 10, 0, 1.0f});

        auto json = urpg::presentation::PresentationMigrationTool::ToJson(overlay);
        auto revived = urpg::presentation::PresentationMigrationTool::FromJson(json);

        REQUIRE(revived.mapId == "test_map");
        REQUIRE(revived.elevation.width == 10);
        REQUIRE(revived.fog.density == 0.12f);
        REQUIRE(revived.props.size() == 1);
        REQUIRE(revived.props[0].assetId == "pine_tree");
    }

    SECTION("MapSceneTranslator emits Light and Fog intents") {
        SpatialMapOverlay myMap;
        myMap.mapId = "test_env";
        myMap.fog.density = 0.5f;
        myMap.postFX.bloomIntensity = 0.9f;

        LightProfile light;
        light.lightId = 101;
        light.type = LightProfile::LightType::Point;
        light.position = {0, 5, 0};
        light.color[0] = 1.0f;
        light.color[1] = 1.0f;
        light.color[2] = 1.0f;
        light.intensity = 2.0f;
        light.range = 15.0f;
        myMap.lights.push_back(light);

        PresentationAuthoringData data;
        data.mapOverlays.push_back(myMap);
        data.actorProfiles.push_back({"hero", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});

        PresentationContext ctx;
        ctx.activeMode = PresentationMode::Spatial;
        ctx.activeTier = CapabilityTier::Tier1_Standard;

        MapSceneState state;
        state.mapId = "test_env";
        state.actors.push_back({7, "hero", 2.0f, 3.0f, false});

        PresentationFrameIntent intent;
        MapSceneTranslatorImpl translator;
        translator.Translate(ctx, data, state, intent);

        bool foundLight = false;
        bool foundFog = false;
        bool foundPostFx = false;
        bool foundShadowProxy = false;
        for (const auto& cmd : intent.commands) {
            if (cmd.type == PresentationCommand::Type::SetLight && cmd.id == 101)
                foundLight = true;
            if (cmd.type == PresentationCommand::Type::SetFog && cmd.fogProfile && cmd.fogProfile->density == 0.5f)
                foundFog = true;
            if (cmd.type == PresentationCommand::Type::SetPostFX && cmd.postFXProfile &&
                cmd.postFXProfile->bloomIntensity == 0.9f)
                foundPostFx = true;
            if (cmd.type == PresentationCommand::Type::DrawShadowProxy && cmd.id == 7)
                foundShadowProxy = true;
        }

        REQUIRE(foundLight);
        REQUIRE(foundFog);
        REQUIRE(foundPostFx);
        REQUIRE(foundShadowProxy);
    }

    SECTION("RenderBackendMock consumes environment commands") {
        PresentationFrameIntent intent;

        FogProfile fog;
        fog.density = 0.2f;

        PostFXProfile fx;
        fx.bloomIntensity = 0.35f;

        intent.AddFog(fog);
        intent.AddPostFX(fx);
        intent.AddShadowProxy(88, {1.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 0.0f});

        urpg::render::RenderBackendMock backend;
        backend.ConsumeFrame(intent);

        REQUIRE(backend.GetCurrentState().fogEnabled);
        REQUIRE(backend.GetCurrentState().postFxEnabled);
        REQUIRE(backend.GetCurrentState().bloomIntensity == 0.35f);
        REQUIRE(backend.GetDrawCalls().size() == 1);
        REQUIRE(backend.GetDrawCalls()[0].type == "ShadowProxy");
    }

    SECTION("SpatialProjection resolves center-screen ray to the target point on flat ground") {
        CameraState cameraState;
        cameraState.targetPos = {5.0f, 0.0f, 6.0f};
        cameraState.currentPitch = 45.0f;
        cameraState.currentYaw = 0.0f;
        cameraState.currentDistance = 10.0f;

        CameraProfile profile;
        profile.lookAtOffset = {0.0f, 0.0f, 0.0f};
        profile.fov = 60.0f;

        const ViewportRect viewport{0.0f, 0.0f, 1280.0f, 720.0f};
        const urpg::Vector2f screenPoint{640.0f, 360.0f};

        const WorldRay ray = SpatialProjection::ScreenPointToWorldRay(screenPoint, viewport, cameraState, profile);
        const auto hit = SpatialProjection::IntersectGroundPlane(ray, 0.0f);

        REQUIRE(hit.has_value());
        CHECK(hit->x == Catch::Approx(5.0f).margin(0.05f));
        CHECK(hit->y == Catch::Approx(0.0f).margin(0.01f));
        CHECK(hit->z == Catch::Approx(6.0f).margin(0.05f));
    }

    SECTION("SpatialProjection intersects elevated tiles before the base plane") {
        overlay.elevation.width = 8;
        overlay.elevation.height = 8;
        overlay.elevation.levels.assign(64, 0);
        overlay.elevation.stepHeight = 0.5f;
        overlay.elevation.levels[3 * 8 + 4] = 4; // 2.0 world units at tile (4,3)

        CameraState cameraState;
        cameraState.targetPos = {4.5f, 2.0f, 3.5f};
        cameraState.currentPitch = 45.0f;
        cameraState.currentYaw = 0.0f;
        cameraState.currentDistance = 8.0f;

        CameraProfile profile;
        profile.lookAtOffset = {0.0f, 0.0f, 0.0f};
        profile.fov = 60.0f;

        const ViewportRect viewport{0.0f, 0.0f, 1600.0f, 900.0f};
        const urpg::Vector2f screenPoint{800.0f, 450.0f};

        const WorldRay ray = SpatialProjection::ScreenPointToWorldRay(screenPoint, viewport, cameraState, profile);
        const auto hit = SpatialProjection::IntersectElevationGrid(ray, overlay.elevation);

        REQUIRE(hit.has_value());
        CHECK(hit->x == Catch::Approx(4.5f).margin(0.1f));
        CHECK(hit->y == Catch::Approx(2.0f).margin(0.05f));
        CHECK(hit->z == Catch::Approx(3.5f).margin(0.1f));
    }
}

TEST_CASE("Spatial authoring output flows into presentation runtime frame generation", "[presentation][spatial][e2e]") {
    // 1. Set up a spatial map overlay with a flat 8x8 grid
    SpatialMapOverlay overlay;
    overlay.mapId = "e2e_village";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.stepHeight = 0.5f;
    overlay.elevation.levels.assign(64, 0);
    overlay.fog.density = 0.1f;
    overlay.postFX.exposure = 1.0f;

    // 2. Use ElevationBrushPanel to raise a hill at (4,4)
    ElevationBrushPanel brush;
    brush.SetTarget(&overlay);
    brush.SetBrushSize(1);
    brush.ApplyBrush(4, 4, 4); // level 4 => 2.0 world units

    // Verify the hill was applied
    REQUIRE(overlay.elevation.levels[4 * 8 + 4] == 4);
    REQUIRE(overlay.elevation.GetWorldHeight(4, 4) == 2.0f);

    // 3. Use PropPlacementPanel to place a prop on the hill
    PropPlacementPanel placement;
    placement.SetTarget(&overlay);
    const float hillHeight = overlay.elevation.GetWorldHeight(4, 4);
    placement.AddProp("house_01", 4.5f, hillHeight, 4.5f);

    REQUIRE(overlay.props.size() == 1);
    REQUIRE(overlay.props[0].assetId == "house_01");
    // Prop Y should reflect the edited elevation
    REQUIRE(overlay.props[0].posY == Catch::Approx(2.0f));

    // 4. Feed into PresentationAuthoringData
    PresentationAuthoringData data;
    data.mapOverlays.push_back(overlay);
    data.actorProfiles.push_back({"hero", {0.5f, 0.0f}, {0.0f, 0.25f, 0.0f}, true, 0.0f});

    // 5. Set up PresentationContext with an actor standing on the hill
    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;
    context.mapState.mapId = "e2e_village";
    context.mapState.actors.push_back({1, "hero", 4.0f, 4.0f, false});

    // 6. Build the presentation frame
    PresentationRuntime runtime;
    const PresentationFrameIntent intent = runtime.BuildPresentationFrame(context, data);

    // 7. Verify the frame contains commands with elevation-resolved Y positions
    bool foundActor = false;
    bool foundProp = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawActor && cmd.id == 1) {
            foundActor = true;
            // Actor standing on tile (4,4) with elevation level 4 => 2.0 world units + 0.25 anchor offset
            CHECK(cmd.position.y == Catch::Approx(2.25f));
        }
        if (cmd.type == PresentationCommand::Type::DrawProp && cmd.id == 1) {
            // Prop id is index+1 in the translator; first prop has id==1
            foundProp = true;
            CHECK(cmd.position.y == Catch::Approx(2.0f));
        }
    }

    REQUIRE(foundActor);
    REQUIRE(foundProp);
    REQUIRE(intent.activePasses.size() == 3);
}

// ---------------------------------------------------------------------------
// S23-T04: Scene render-command coverage — Menu / UI / Status HUD / Chat
// ---------------------------------------------------------------------------

TEST_CASE("MenuSceneTranslator emits background shadow proxy command", "[presentation][scene][menu]") {
    MenuSceneStateTranslator translator;
    MenuSceneState state;
    state.menuId = "main_menu";
    state.drawBackground = true;
    state.blurBackground = false;
    state.zDepth = 2.5f;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    bool foundBackground = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawShadowProxy && cmd.id == 0xBB) {
            foundBackground = true;
            CHECK(cmd.position.z == Catch::Approx(2.5f));
        }
    }
    REQUIRE(foundBackground);
}

TEST_CASE("MenuSceneTranslator emits blur PostFX when blurBackground is set", "[presentation][scene][menu]") {
    MenuSceneStateTranslator translator;
    MenuSceneState state;
    state.drawBackground = true;
    state.blurBackground = true;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    bool foundPostFX = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::SetPostFX) {
            foundPostFX = true;
            REQUIRE(cmd.postFXProfile != nullptr);
            CHECK(cmd.postFXProfile->bloomIntensity == Catch::Approx(5.0f));
        }
    }
    REQUIRE(foundPostFX);
}

TEST_CASE("MenuSceneTranslator emits no commands when drawBackground is false and blurBackground is false",
          "[presentation][scene][menu]") {
    MenuSceneStateTranslator translator;
    MenuSceneState state;
    state.drawBackground = false;
    state.blurBackground = false;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    REQUIRE(intent.commands.empty());
}

TEST_CASE("StatusHUDTranslator emits no commands when invisible", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = false;
    state.activeActorId = 5;
    state.activeStatusIconCount = 3;
    state.showMinimap = true;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    REQUIRE(intent.commands.empty());
}

TEST_CASE("StatusHUDTranslator emits turn-indicator overlay for active actor", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = true;
    state.activeActorId = 42;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    bool foundTurnIndicator = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect && cmd.id == 42) {
            foundTurnIndicator = true;
            CHECK(cmd.effectColor[0] == Catch::Approx(1.0f));
            CHECK(cmd.effectColor[2] == Catch::Approx(0.5f)); // yellow tint
        }
    }
    REQUIRE(foundTurnIndicator);
}

TEST_CASE("StatusHUDTranslator emits one overlay command per status icon", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = true;
    state.activeActorId = 0;
    state.activeStatusIconCount = 4;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    size_t iconCount = 0;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect) {
            ++iconCount;
        }
    }
    REQUIRE(iconCount == 4);
}

TEST_CASE("StatusHUDTranslator emits minimap overlay when showMinimap is true", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = true;
    state.activeActorId = 0;
    state.showMinimap = true;
    state.minimapOpacity = 0.75f;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    bool foundMinimap = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect && cmd.id == 0xFFFF) {
            foundMinimap = true;
            CHECK(cmd.effectIntensity == Catch::Approx(0.75f));
        }
    }
    REQUIRE(foundMinimap);
}

TEST_CASE("StatusHUDTranslator combined: turn indicator + status icons + minimap", "[presentation][scene][status]") {
    StatusHUDTranslator translator;
    StatusHUDState state;
    state.visible = true;
    state.activeActorId = 7;
    state.activeStatusIconCount = 2;
    state.showMinimap = true;
    state.minimapOpacity = 1.0f;

    PresentationFrameIntent intent;
    translator.Translate(state, intent);

    // Expected: 1 turn indicator + 2 status icons + 1 minimap = 4 overlay commands
    size_t overlayCount = 0;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect)
            ++overlayCount;
    }
    REQUIRE(overlayCount == 4);
}

TEST_CASE("Chat/Dialogue overlay scene produces shadow proxy for contrast background", "[presentation][scene][chat]") {
    // The DialogueTranslator is responsible for chat-scene render commands.
    // This test verifies that an active high-contrast dialogue state emits
    // a shadow-proxy background command (the chat contrast background).
    DialogueTranslator translator;
    DialogueState dialogueState;
    dialogueState.isActive = true;
    dialogueState.requireHighContrast = true;

    DialoguePresentationConfig config;
    config.sceneSaturationMultiplier = 0.4f;
    config.contrastBgAlpha = 0.85f;

    PresentationFrameIntent intent;
    // Apply readability modifiers (adds shadow proxy + PostFX saturation override)
    translator.ApplyReadability(dialogueState, config, intent);

    bool foundContrastProxy = false;
    bool foundSaturationOverride = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawShadowProxy) {
            foundContrastProxy = true;
        }
        if (cmd.type == PresentationCommand::Type::SetPostFX && cmd.postFXProfile) {
            // Saturation should be reduced to sceneSaturationMultiplier
            foundSaturationOverride = cmd.postFXProfile->saturation < 1.0f;
        }
    }
    REQUIRE(foundContrastProxy);
    REQUIRE(foundSaturationOverride);
}

TEST_CASE("Chat/Dialogue overlay scene is silent when dialogue is inactive", "[presentation][scene][chat]") {
    DialogueTranslator translator;
    DialogueState dialogueState;
    dialogueState.isActive = false;

    DialoguePresentationConfig config;
    config.sceneSaturationMultiplier = 0.4f;
    config.contrastBgAlpha = 0.85f;

    PresentationFrameIntent intent;
    translator.ApplyReadability(dialogueState, config, intent);

    // Inactive dialogue must not inject any presentation commands
    REQUIRE(intent.commands.empty());
}
