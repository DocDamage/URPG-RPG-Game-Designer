#include "editor/spatial/elevation_brush_panel.h"
#include "editor/spatial/map_ability_binding_panel.h"
#include "editor/spatial/perspective_2d_product_workflow.h"
#include "editor/spatial/prop_placement_panel.h"
#include "editor/spatial/spatial_ability_canvas_panel.h"
#include "editor/spatial/spatial_authoring_workspace.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/global_state_hub.h"
#include "engine/core/input/input_core.h"
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
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace urpg::editor;
using namespace urpg::presentation;
using urpg::Vector2f;

TEST_CASE("Spatial Editor Tooling Integration - SpatialAbilityCanvas panel supports direct click selection and rebinding of existing targets", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
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

TEST_CASE("Spatial Editor Tooling Integration - SpatialAbilityCanvas panel supports dragging bindings, resizing regions, and exposing inline badges", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
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

TEST_CASE("Spatial Editor Tooling Integration - SpatialAbilityCanvas panel surfaces hover previews, direct trigger switching, and overlap warnings", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
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

TEST_CASE("Spatial Editor Tooling Integration - SpatialAuthoringWorkspace composes elevation, prop, binding, and canvas authoring into one surface", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
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
    REQUIRE(workspace.GetTitle() == "Perspective 2D Map Editor");
    REQUIRE(snapshot.has_target_scene);
    REQUIRE(snapshot.has_target_overlay);
    REQUIRE(snapshot.message == "Perspective 2D map editor is ready.");
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
    REQUIRE(snapshot.toolbar.actions.size() == 8);
    REQUIRE(snapshot.toolbar.actions[0].label == "Compose");
    const auto partsAction = std::find_if(snapshot.toolbar.actions.begin(), snapshot.toolbar.actions.end(),
                                          [](const auto& action) { return action.id == "parts"; });
    REQUIRE(partsAction != snapshot.toolbar.actions.end());
    REQUIRE_FALSE(partsAction->enabled);
    REQUIRE_FALSE(snapshot.elevation.visible);
    REQUIRE_FALSE(snapshot.props.visible);
    REQUIRE(snapshot.bindings.visible);
    REQUIRE(snapshot.canvas.visible);

    std::filesystem::remove_all(projectRoot);
}

TEST_CASE("Spatial Editor Tooling Integration - SpatialAuthoringWorkspace toolbar drives shared canvas workflows and suggested conflict resolution", "[editor][spatial]") {

    SpatialMapOverlay overlay;
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
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

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D workspace composes worldbuilding tools",
          "[editor][spatial][worldbuilding]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "perspective_world";
    overlay.elevation.width = 12;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(96, 0);

    urpg::scene::MapScene map("perspective_world", 12, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    urpg::map::TerrainBrush brush;
    brush.mode = urpg::map::TerrainBrushMode::Rectangle;
    brush.width = 2;
    brush.height = 2;
    brush.tile_id = 7;
    workspace.PreviewTerrainBrush(brush, 3, 4, 11);
    workspace.LoadRegionRules({
        {"rain_path", 2, 1, 4, 3, "", "rain_loop", "rain", "", "normal", ""},
    });
    workspace.GenerateProceduralMap({"perspective_seed", "dungeon", 12, 8, 19, false, false, false});
    workspace.LoadEnvironmentPreview(urpg::map::MapEnvironmentPreviewDocument::fromRegionRules(
        "perspective_world",
        12,
        8,
        {
            {"rain_path", 2, 1, 4, 3, "", "rain_loop", "rain", "", "normal", ""},
        }));
    workspace.SelectEnvironmentTile(3, 2);

    REQUIRE(workspace.ActivateToolbarAction("worldbuilding"));
    urpg::FrameContext frameContext{0.016f, 3};
    workspace.Render(frameContext);

    const auto& snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.toolbar.active_mode == "worldbuilding");
    REQUIRE(snapshot.worldbuilding_terrain.preview_point_count == 4);
    REQUIRE(snapshot.worldbuilding_regions.rule_count == 1);
    REQUIRE_FALSE(snapshot.worldbuilding_regions.disabled);
    REQUIRE(snapshot.worldbuilding_procedural.has_result);
    REQUIRE(snapshot.worldbuilding_procedural.width == 12);
    REQUIRE(snapshot.worldbuilding_environment.map_id == "perspective_world");
    REQUIRE(snapshot.worldbuilding_environment.region_id == "rain_path");
    REQUIRE(snapshot.worldbuilding_environment.status_message == "Map environment preview is ready.");

    const auto worldbuildingAction = std::find_if(snapshot.toolbar.actions.begin(), snapshot.toolbar.actions.end(),
                                                  [](const auto& action) { return action.id == "worldbuilding"; });
    REQUIRE(worldbuildingAction != snapshot.toolbar.actions.end());
    REQUIRE(worldbuildingAction->label == "Worldbuilding");
    REQUIRE(worldbuildingAction->active);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D supports tile layers palette events and draft IO",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_depth_map";
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);

    urpg::scene::MapScene map("p2d_depth_map", 10, 10);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTile("tileset_overworld", "grass_a"));
    REQUIRE(workspace.ActivateToolbarAction("tiles"));
    REQUIRE(workspace.RouteCanvasPrimaryAction(100.0f, 50.0f));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(110.0f, 50.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_intro", "Intro NPC", "confirm_interact", 120.0f, 60.0f));

    workspace.Render({0.016f, 4});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.toolbar.active_mode == "tiles");
    REQUIRE(snapshot.perspective_2d_project.layer_count == 2);
    REQUIRE(snapshot.perspective_2d_project.painted_tile_count == 2);
    REQUIRE(snapshot.perspective_2d_project.event_count == 1);
    REQUIRE(snapshot.perspective_2d_project.has_unsaved_changes);
    REQUIRE(snapshot.perspective_2d_palette.selected_tileset_id == "tileset_overworld");
    REQUIRE(snapshot.perspective_2d_palette.selected_tile_id == "grass_a");
    REQUIRE(snapshot.perspective_2d_layers[0].id == "ground");
    REQUIRE(snapshot.perspective_2d_layers[0].tile_count == 2);
    REQUIRE(snapshot.perspective_2d_layers[1].id == "events");
    REQUIRE(snapshot.perspective_2d_layers[1].event_count == 1);
    REQUIRE(snapshot.perspective_2d_events[0].event_id == "ev_intro");
    REQUIRE(snapshot.perspective_2d_events[0].tile_x == 6);
    REQUIRE(snapshot.perspective_2d_events[0].tile_y == 6);

    const auto save = workspace.SavePerspectiveMapDraft();
    REQUIRE(save.success);
    REQUIRE(save.message == "Perspective 2D map draft saved.");
    REQUIRE(save.layer_count == 2);
    REQUIRE(save.painted_tile_count == 2);
    REQUIRE(save.event_count == 1);
    const auto savedJson = nlohmann::json::parse(save.serialized_document_json);
    REQUIRE(savedJson["document_kind"] == "urpg.perspective_2d.map");
    REQUIRE(savedJson["selected_layer_id"] == "events");
    REQUIRE(savedJson["selected_tileset_id"] == "tileset_overworld");
    REQUIRE(savedJson["selected_tile_id"] == "grass_a");
    REQUIRE(savedJson["tiles"].size() == 2);
    REQUIRE(savedJson["events"].size() == 1);

    SpatialMapOverlay loadedOverlay;
    loadedOverlay.mapId = "p2d_depth_map";
    loadedOverlay.elevation.width = 10;
    loadedOverlay.elevation.height = 10;
    loadedOverlay.elevation.levels.resize(100, 0);
    urpg::scene::MapScene loadedMap("p2d_depth_map", 10, 10);
    SpatialAuthoringWorkspace loadedWorkspace;
    loadedWorkspace.SetTargets(&loadedMap, &loadedOverlay);
    loadedWorkspace.SetProjectionSettings(projection);
    const auto load = loadedWorkspace.LoadPerspectiveMapDraft(save.serialized_document_json);
    REQUIRE(load.success);
    REQUIRE(load.message == "Perspective 2D map draft loaded.");
    REQUIRE(load.layer_count == 2);
    REQUIRE(load.painted_tile_count == 2);
    REQUIRE(load.event_count == 1);

    loadedWorkspace.Render({0.016f, 5});
    const auto loadedSnapshot = loadedWorkspace.lastRenderSnapshot();
    REQUIRE_FALSE(loadedSnapshot.perspective_2d_project.has_unsaved_changes);
    REQUIRE(loadedSnapshot.perspective_2d_project.selected_layer_id == "events");
    REQUIRE(loadedSnapshot.perspective_2d_palette.selected_tile_id == "grass_a");
    REQUIRE(loadedSnapshot.perspective_2d_layers[0].tile_count == 2);
    REQUIRE(loadedSnapshot.perspective_2d_events[0].label == "Intro NPC");
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D gates playtest and export readiness",
          "[editor][spatial][p2d_depth]") {
    SpatialAuthoringWorkspace unbound;
    auto unboundPlaytest = unbound.RunPerspectiveMapPlaytest();
    REQUIRE_FALSE(unboundPlaytest.success);
    REQUIRE(unboundPlaytest.blocker_codes[0] == "p2d_targets_unbound");

    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_ready_map";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_ready_map", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    auto emptyPlaytest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE_FALSE(emptyPlaytest.success);
    REQUIRE(emptyPlaytest.blocker_codes[0] == "p2d_no_tile_layer");

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTile("dungeon", "floor"));
    auto noTilesPlaytest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE_FALSE(noTilesPlaytest.success);
    REQUIRE(noTilesPlaytest.blocker_codes[0] == "p2d_no_painted_tiles");

    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_door", "Door", "confirm_interact", 90.0f, 90.0f));
    REQUIRE(workspace.SetPerspectiveLayerLocked("events", true));
    auto lockedEventPlaytest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE_FALSE(lockedEventPlaytest.success);
    REQUIRE(lockedEventPlaytest.blocker_codes[0] == "p2d_event_layer_locked");

    REQUIRE(workspace.SetPerspectiveLayerLocked("events", false));
    auto playtest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE(playtest.success);
    REQUIRE(playtest.message == "Perspective 2D map playtest readiness passed.");
    REQUIRE(playtest.playtest_tile_count == 1);
    REQUIRE(playtest.playtest_event_count == 1);

    auto exportResult = workspace.ExportPerspectiveMap();
    REQUIRE(exportResult.success);
    REQUIRE(exportResult.message == "Perspective 2D map is exportable.");
    REQUIRE(exportResult.exported_tile_count == 1);
    REQUIRE(exportResult.exported_event_count == 1);
    const auto exportedJson = nlohmann::json::parse(exportResult.serialized_document_json);
    REQUIRE(exportedJson["map_id"] == "p2d_ready_map");
    REQUIRE(exportedJson["tiles"].size() == 1);
    REQUIRE(exportedJson["events"].size() == 1);

    workspace.Render({0.016f, 6});
    const auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_project.can_playtest);
    REQUIRE(snapshot.perspective_2d_project.can_export);
    REQUIRE(snapshot.last_perspective_2d_playtest.success);
    REQUIRE(snapshot.last_perspective_2d_export.success);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D emits runtime manifests for playtest and export",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_runtime_manifest";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_runtime_manifest", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("secret", "Secret", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveTile("overworld", "grass"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("secret"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(90.0f, 80.0f));
    REQUIRE(workspace.SetPerspectiveLayerVisible("secret", false));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_gate", "Gate", "confirm_interact", 100.0f, 90.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_gate", "closed", "Closed", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "closed", "show_text", "Closed."));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_gate", "open", "Open", "player_touch"));
    REQUIRE(workspace.AddPerspectiveEventPageCondition("ev_gate", "open", "switch", "gate_open", "true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "open", "transfer_player", "castle:2,7"));
    REQUIRE(workspace.SetPerspectiveEventConditionValue("switch", "gate_open", "true"));

    const auto playtest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE(playtest.success);
    REQUIRE(playtest.runtime_layer_count == 2);
    REQUIRE(playtest.runtime_tile_count == 1);
    REQUIRE(playtest.runtime_event_count == 1);
    const auto playtestManifest = nlohmann::json::parse(playtest.serialized_runtime_manifest_json);
    REQUIRE(playtestManifest["document_kind"] == "urpg.perspective_2d.runtime_manifest");
    REQUIRE(playtestManifest["map_id"] == "p2d_runtime_manifest");
    REQUIRE(playtestManifest["layers"].size() == 2);
    REQUIRE(playtestManifest["layers"][0]["id"] == "ground");
    REQUIRE(playtestManifest["tiles"].size() == 1);
    REQUIRE(playtestManifest["tiles"][0]["layer_id"] == "ground");
    REQUIRE(playtestManifest["events"][0]["active_page_id"] == "open");
    REQUIRE(playtestManifest["events"][0]["trigger_id"] == "player_touch");
    REQUIRE(playtestManifest["events"][0]["commands"][0]["code"] == "transfer_player");

    const auto exportResult = workspace.ExportPerspectiveMap();
    REQUIRE(exportResult.success);
    REQUIRE(exportResult.runtime_layer_count == 2);
    REQUIRE(exportResult.runtime_tile_count == 1);
    REQUIRE(exportResult.runtime_event_count == 1);
    const auto exportManifest = nlohmann::json::parse(exportResult.serialized_runtime_manifest_json);
    REQUIRE(exportManifest["events"][0]["commands"][0]["argument"] == "castle:2,7");
    const auto packageManifest = nlohmann::json::parse(exportResult.serialized_package_manifest_json);
    REQUIRE(packageManifest["document_kind"] == "urpg.perspective_2d.export_package");
    REQUIRE(packageManifest["map_id"] == "p2d_runtime_manifest");
    REQUIRE(packageManifest["draft_document_kind"] == "urpg.perspective_2d.map");
    REQUIRE(packageManifest["runtime_manifest_kind"] == "urpg.perspective_2d.runtime_manifest");
    REQUIRE(packageManifest["runtime_tile_count"] == 1);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D supports brush erase and layer ergonomics",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_brush_map";
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    urpg::scene::MapScene map("p2d_brush_map", 10, 10);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTile("overworld", "grass"));
    REQUIRE(workspace.SetPerspectiveBrushSize(2));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(100.0f, 50.0f));
    workspace.Render({0.016f, 7});
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 4);
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_palette.brush_size == 2);

    REQUIRE(workspace.ErasePerspectiveTileFromScreen(100.0f, 50.0f));
    workspace.Render({0.016f, 8});
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 0);

    REQUIRE(workspace.PaintPerspectiveTileFromScreen(100.0f, 50.0f));
    REQUIRE(workspace.DuplicatePerspectiveLayer("ground", "ground_copy", "Ground Copy"));
    workspace.Render({0.016f, 9});
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_project.layer_count == 2);
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 8);

    REQUIRE(workspace.ClearPerspectiveLayer("ground_copy"));
    workspace.Render({0.016f, 10});
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 4);

    REQUIRE(workspace.SetPerspectiveLayerLocked("ground", true));
    REQUIRE_FALSE(workspace.PaintPerspectiveTileFromScreen(120.0f, 50.0f));
    REQUIRE_FALSE(workspace.ErasePerspectiveTileFromScreen(100.0f, 50.0f));
    REQUIRE(workspace.SetPerspectiveLayerLocked("ground", false));
    REQUIRE(workspace.DeletePerspectiveLayer("ground_copy"));
    workspace.Render({0.016f, 11});
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_project.layer_count == 1);
    REQUIRE(workspace.lastRenderSnapshot().perspective_2d_project.painted_tile_count == 4);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D supports multi layer editing",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_multi_layer";
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    urpg::scene::MapScene map("p2d_multi_layer", 10, 10);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("detail", "Details", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveTile("town", "floor"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(100.0f, 50.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("detail"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(110.0f, 50.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_gate", "Gate", "confirm_interact", 120.0f, 60.0f));

    REQUIRE(workspace.SelectPerspectiveLayers({"ground", "detail"}));
    workspace.Render({0.016f, 27});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_project.selected_layer_count == 2);
    REQUIRE(snapshot.perspective_2d_layers[0].selected_for_bulk_edit);
    REQUIRE(snapshot.perspective_2d_layers[1].selected_for_bulk_edit);
    REQUIRE_FALSE(snapshot.perspective_2d_layers[2].selected_for_bulk_edit);

    REQUIRE(workspace.SetSelectedPerspectiveLayersVisible(false));
    REQUIRE(workspace.SetSelectedPerspectiveLayersLocked(true));
    workspace.Render({0.016f, 28});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE_FALSE(snapshot.perspective_2d_layers[0].visible);
    REQUIRE(snapshot.perspective_2d_layers[0].locked);
    REQUIRE_FALSE(snapshot.perspective_2d_layers[1].visible);
    REQUIRE(snapshot.perspective_2d_layers[1].locked);
    REQUIRE(snapshot.perspective_2d_layers[2].visible);

    REQUIRE(workspace.SetSelectedPerspectiveLayersVisible(true));
    REQUIRE(workspace.SetSelectedPerspectiveLayersLocked(false));
    REQUIRE(workspace.DuplicateSelectedPerspectiveLayers("_copy"));
    workspace.Render({0.016f, 29});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_project.layer_count == 5);
    REQUIRE(snapshot.perspective_2d_project.selected_layer_count == 2);
    REQUIRE(snapshot.perspective_2d_project.painted_tile_count == 4);
    REQUIRE(snapshot.perspective_2d_layers[3].id == "ground_copy");
    REQUIRE(snapshot.perspective_2d_layers[3].selected_for_bulk_edit);
    REQUIRE(snapshot.perspective_2d_layers[4].id == "detail_copy");
    REQUIRE(snapshot.perspective_2d_layers[4].selected_for_bulk_edit);

    REQUIRE(workspace.DeleteSelectedPerspectiveLayers());
    workspace.Render({0.016f, 30});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_project.layer_count == 3);
    REQUIRE(snapshot.perspective_2d_project.selected_layer_count == 1);
    REQUIRE(snapshot.perspective_2d_project.selected_layer_id == "ground");
    REQUIRE(snapshot.perspective_2d_project.painted_tile_count == 2);
    REQUIRE(snapshot.perspective_2d_project.event_count == 1);
    REQUIRE(snapshot.perspective_2d_layers[0].order == 0);
    REQUIRE(snapshot.perspective_2d_layers[1].order == 1);
    REQUIRE(snapshot.perspective_2d_layers[2].order == 2);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D supports event command editing and movement",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_event_commands";
    overlay.elevation.width = 10;
    overlay.elevation.height = 10;
    overlay.elevation.levels.resize(100, 0);
    urpg::scene::MapScene map("p2d_event_commands", 10, 10);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 200.0f;
    projection.viewportHeight = 100.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 5.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTile("town", "floor"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(100.0f, 50.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_intro", "Intro NPC", "confirm_interact", 110.0f, 60.0f));
    REQUIRE(workspace.AddPerspectiveEventCommand("ev_intro", "show_text", "Welcome to town."));
    REQUIRE(workspace.AddPerspectiveEventCommand("ev_intro", "transfer_player", "map002:4,6"));
    REQUIRE(workspace.MovePerspectiveEventFromScreen("ev_intro", 130.0f, 70.0f));

    workspace.Render({0.016f, 12});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_events[0].tile_x == 7);
    REQUIRE(snapshot.perspective_2d_events[0].tile_y == 7);
    REQUIRE(snapshot.perspective_2d_events[0].command_count == 2);
    REQUIRE(snapshot.perspective_2d_events[0].commands[0].code == "show_text");
    REQUIRE(snapshot.perspective_2d_events[0].commands[1].argument == "map002:4,6");

    const auto save = workspace.SavePerspectiveMapDraft();
    REQUIRE(save.success);
    const auto savedJson = nlohmann::json::parse(save.serialized_document_json);
    REQUIRE(savedJson["events"][0]["commands"].size() == 2);
    REQUIRE(savedJson["events"][0]["commands"][0]["code"] == "show_text");

    SpatialMapOverlay loadedOverlay;
    loadedOverlay.mapId = "p2d_event_commands";
    loadedOverlay.elevation.width = 10;
    loadedOverlay.elevation.height = 10;
    loadedOverlay.elevation.levels.resize(100, 0);
    urpg::scene::MapScene loadedMap("p2d_event_commands", 10, 10);
    SpatialAuthoringWorkspace loadedWorkspace;
    loadedWorkspace.SetTargets(&loadedMap, &loadedOverlay);
    REQUIRE(loadedWorkspace.LoadPerspectiveMapDraft(save.serialized_document_json).success);
    loadedWorkspace.Render({0.016f, 13});
    const auto loadedSnapshot = loadedWorkspace.lastRenderSnapshot();
    REQUIRE(loadedSnapshot.perspective_2d_events[0].command_count == 2);
    REQUIRE(loadedSnapshot.perspective_2d_events[0].commands[0].argument == "Welcome to town.");
    REQUIRE(loadedSnapshot.perspective_2d_events[0].tile_x == 7);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D exposes promoted asset palette rows",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_palette_assets";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_palette_assets", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    workspace.SetPerspectiveTilePaletteOptions({
        {"grass_01", "Grass A", "overworld", "grass_a", "asset.overworld.grass_a", "content/tiles/grass_a.png"},
        {"stone_01", "Stone Floor", "dungeon", "stone_floor", "asset.dungeon.stone_floor",
         "content/tiles/stone_floor.png"},
    });

    workspace.Render({0.016f, 14});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_palette.tile_option_count == 2);
    REQUIRE(snapshot.perspective_2d_palette.tile_options[0].option_id == "grass_01");
    REQUIRE(snapshot.perspective_2d_palette.tile_options[0].label == "Grass A");
    REQUIRE(snapshot.perspective_2d_palette.tile_options[0].asset_id == "asset.overworld.grass_a");
    REQUIRE_FALSE(snapshot.perspective_2d_palette.tile_options[0].selected);

    REQUIRE(workspace.SelectPerspectiveTilePaletteOption("stone_01"));
    REQUIRE_FALSE(workspace.SelectPerspectiveTilePaletteOption("missing"));

    workspace.Render({0.016f, 15});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_palette.selected_option_id == "stone_01");
    REQUIRE(snapshot.perspective_2d_palette.selected_tileset_id == "dungeon");
    REQUIRE(snapshot.perspective_2d_palette.selected_tile_id == "stone_floor");
    REQUIRE(snapshot.perspective_2d_palette.tile_options[1].selected);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    const auto save = workspace.SavePerspectiveMapDraft();
    REQUIRE(save.success);
    const auto savedJson = nlohmann::json::parse(save.serialized_document_json);
    REQUIRE(savedJson["selected_palette_option_id"] == "stone_01");
    REQUIRE(savedJson["tiles"][0]["tileset_id"] == "dungeon");
    REQUIRE(savedJson["tiles"][0]["tile_id"] == "stone_floor");
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D filters promoted tile palette options",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_palette_filtering";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_palette_filtering", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    workspace.SetPerspectiveTilePaletteOptions({
        {"grass_01", "Grass A", "overworld", "grass_a", "asset.overworld.grass_a",
         "content/tiles/grass_a.png", "field", "content/tiles/grass_a.preview.png"},
        {"stone_01", "Stone Floor", "dungeon", "stone_floor", "asset.dungeon.stone_floor",
         "content/tiles/stone_floor.png", "interior", "content/tiles/stone_floor.preview.png"},
        {"water_01", "Water Edge", "overworld", "water_edge", "asset.overworld.water_edge",
         "content/tiles/water_edge.png", "field", "content/tiles/water_edge.preview.png"},
    });
    REQUIRE(workspace.SelectPerspectiveTilePaletteOption("stone_01"));

    workspace.Render({0.016f, 22});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_palette.tile_option_count == 3);
    REQUIRE(snapshot.perspective_2d_palette.visible_tile_option_count == 3);
    REQUIRE(snapshot.perspective_2d_palette.selected_option_visible);
    REQUIRE(snapshot.perspective_2d_palette.tile_options[1].category_id == "interior");
    REQUIRE(snapshot.perspective_2d_palette.tile_options[1].thumbnail_path ==
            "content/tiles/stone_floor.preview.png");

    workspace.SetPerspectiveTilePaletteFilter("water", "", "");
    workspace.Render({0.016f, 23});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_palette.search_text == "water");
    REQUIRE(snapshot.perspective_2d_palette.tile_option_count == 3);
    REQUIRE(snapshot.perspective_2d_palette.visible_tile_option_count == 1);
    REQUIRE_FALSE(snapshot.perspective_2d_palette.selected_option_visible);
    REQUIRE(snapshot.perspective_2d_palette.tile_options[0].option_id == "water_01");

    workspace.SetPerspectiveTilePaletteFilter("", "overworld", "field");
    workspace.Render({0.016f, 24});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_palette.filtered_tileset_id == "overworld");
    REQUIRE(snapshot.perspective_2d_palette.filtered_category_id == "field");
    REQUIRE(snapshot.perspective_2d_palette.visible_tile_option_count == 2);
    REQUIRE(snapshot.perspective_2d_palette.tile_options[0].option_id == "grass_01");
    REQUIRE(snapshot.perspective_2d_palette.tile_options[1].option_id == "water_01");

    workspace.SetPerspectiveTilePaletteFilter("missing", "overworld", "field");
    workspace.Render({0.016f, 25});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_palette.visible_tile_option_count == 0);
    REQUIRE(snapshot.perspective_2d_palette.tile_options.empty());
    REQUIRE_FALSE(snapshot.perspective_2d_palette.selected_option_visible);

    workspace.ClearPerspectiveTilePaletteFilter();
    workspace.Render({0.016f, 26});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_palette.search_text.empty());
    REQUIRE(snapshot.perspective_2d_palette.filtered_tileset_id.empty());
    REQUIRE(snapshot.perspective_2d_palette.filtered_category_id.empty());
    REQUIRE(snapshot.perspective_2d_palette.visible_tile_option_count == 3);
    REQUIRE(snapshot.perspective_2d_palette.selected_option_visible);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D updates and removes event command rows",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_event_command_rows";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_event_command_rows", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_cutscene", "Cutscene", "autorun", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventCommand("ev_cutscene", "show_text", "Old line"));
    REQUIRE(workspace.AddPerspectiveEventCommand("ev_cutscene", "wait", "30"));

    REQUIRE(workspace.UpdatePerspectiveEventCommand("ev_cutscene", 0, "show_text", "New line"));
    REQUIRE(workspace.RemovePerspectiveEventCommand("ev_cutscene", 1));
    REQUIRE_FALSE(workspace.UpdatePerspectiveEventCommand("ev_cutscene", 1, "wait", "60"));
    REQUIRE_FALSE(workspace.RemovePerspectiveEventCommand("ev_missing", 0));

    workspace.Render({0.016f, 16});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_events[0].command_count == 1);
    REQUIRE(snapshot.perspective_2d_events[0].commands[0].code == "show_text");
    REQUIRE(snapshot.perspective_2d_events[0].commands[0].argument == "New line");

    const auto save = workspace.SavePerspectiveMapDraft();
    REQUIRE(save.success);
    const auto savedJson = nlohmann::json::parse(save.serialized_document_json);
    REQUIRE(savedJson["events"][0]["commands"].size() == 1);
    REQUIRE(savedJson["events"][0]["commands"][0]["argument"] == "New line");
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D supports event pages conditions and active preview",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_event_pages";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_event_pages", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_gate", "Castle Gate", "confirm_interact", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_gate", "closed", "Closed", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "closed", "show_text", "The gate is closed."));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_gate", "open", "Open", "player_touch"));
    REQUIRE(workspace.AddPerspectiveEventPageCondition("ev_gate", "open", "switch", "gate_open", "true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "open", "transfer_player", "castle:2,7"));
    REQUIRE(workspace.SelectPerspectiveEventPage("ev_gate", "closed"));

    workspace.Render({0.016f, 17});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_events[0].page_count == 2);
    REQUIRE(snapshot.perspective_2d_events[0].selected_page_id == "closed");
    REQUIRE(snapshot.perspective_2d_events[0].active_page_id == "closed");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].page_id == "closed");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].command_count == 1);
    REQUIRE(snapshot.perspective_2d_events[0].pages[1].condition_count == 1);
    REQUIRE(snapshot.perspective_2d_events[0].pages[1].conditions[0].key == "gate_open");
    REQUIRE_FALSE(snapshot.perspective_2d_events[0].pages[1].active_in_playtest);

    REQUIRE(workspace.SetPerspectiveEventConditionValue("switch", "gate_open", "true"));
    workspace.Render({0.016f, 18});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_events[0].active_page_id == "open");
    REQUIRE(snapshot.perspective_2d_events[0].pages[1].active_in_playtest);
    REQUIRE(snapshot.perspective_2d_events[0].trigger_id == "player_touch");
    REQUIRE(snapshot.perspective_2d_events[0].commands[0].code == "transfer_player");

    const auto save = workspace.SavePerspectiveMapDraft();
    REQUIRE(save.success);
    const auto savedJson = nlohmann::json::parse(save.serialized_document_json);
    REQUIRE(savedJson["events"][0]["selected_page_id"] == "closed");
    REQUIRE(savedJson["events"][0]["pages"].size() == 2);
    REQUIRE(savedJson["events"][0]["pages"][1]["conditions"][0]["type"] == "switch");
    REQUIRE(savedJson["events"][0]["pages"][1]["commands"][0]["code"] == "transfer_player");

    SpatialMapOverlay loadedOverlay;
    loadedOverlay.mapId = "p2d_event_pages";
    loadedOverlay.elevation.width = 8;
    loadedOverlay.elevation.height = 8;
    loadedOverlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene loadedMap("p2d_event_pages", 8, 8);
    SpatialAuthoringWorkspace loadedWorkspace;
    loadedWorkspace.SetTargets(&loadedMap, &loadedOverlay);
    REQUIRE(loadedWorkspace.LoadPerspectiveMapDraft(save.serialized_document_json).success);
    REQUIRE(loadedWorkspace.SetPerspectiveEventConditionValue("switch", "gate_open", "true"));

    loadedWorkspace.Render({0.016f, 19});
    const auto loadedSnapshot = loadedWorkspace.lastRenderSnapshot();
    REQUIRE(loadedSnapshot.perspective_2d_events[0].page_count == 2);
    REQUIRE(loadedSnapshot.perspective_2d_events[0].selected_page_id == "closed");
    REQUIRE(loadedSnapshot.perspective_2d_events[0].active_page_id == "open");
    REQUIRE(loadedSnapshot.perspective_2d_events[0].pages[1].commands[0].argument == "castle:2,7");
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D supports comparison condition pages",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_condition_rules";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_condition_rules", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTile("town", "floor"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_rank_gate", "Rank Gate", "confirm_interact", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_rank_gate", "default", "Default", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_rank_gate", "default", "show_text", "Come back later."));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_rank_gate", "ranked", "Ranked", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageConditionRule("ev_rank_gate", "ranked", "variable", "rank", "greater_equal", "3"));
    REQUIRE(workspace.AddPerspectiveEventPageConditionRule("ev_rank_gate", "ranked", "switch", "gate_open", "equals", "true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_rank_gate", "ranked", "show_text", "Welcome, ranked hero."));

    REQUIRE(workspace.SetPerspectiveEventConditionValue("variable", "rank", "2"));
    REQUIRE(workspace.SetPerspectiveEventConditionValue("switch", "gate_open", "true"));
    workspace.Render({0.016f, 31});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_events[0].active_page_id == "default");
    REQUIRE(snapshot.perspective_2d_events[0].pages[1].conditions[0].comparison == "greater_equal");
    REQUIRE(snapshot.perspective_2d_events[0].pages[1].conditions[0].value == "3");
    REQUIRE(snapshot.perspective_2d_events[0].pages[1].conditions[1].comparison == "equals");

    REQUIRE(workspace.SetPerspectiveEventConditionValue("variable", "rank", "3"));
    workspace.Render({0.016f, 32});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_events[0].active_page_id == "ranked");
    REQUIRE(snapshot.perspective_2d_events[0].commands[0].argument == "Welcome, ranked hero.");

    const auto save = workspace.SavePerspectiveMapDraft();
    REQUIRE(save.success);
    const auto savedJson = nlohmann::json::parse(save.serialized_document_json);
    REQUIRE(savedJson["events"][0]["pages"][1]["conditions"][0]["comparison"] == "greater_equal");
    REQUIRE(savedJson["events"][0]["pages"][1]["conditions"][1]["comparison"] == "equals");

    const auto playtest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE(playtest.success);
    const auto manifest = nlohmann::json::parse(playtest.serialized_runtime_manifest_json);
    REQUIRE(manifest["events"][0]["active_page_id"] == "ranked");
    REQUIRE(manifest["events"][0]["commands"][0]["argument"] == "Welcome, ranked hero.");
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D supports conditional branch event commands",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_branch_commands";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_branch_commands", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTile("town", "floor"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_guard", "Guard", "confirm_interact", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_guard", "main", "Main", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageConditionalBranch("ev_guard", "main", "switch", "guard_bribed", "equals", "true"));
    REQUIRE(workspace.AddPerspectiveEventPageBranchCommand("ev_guard", "main", 0, true, "show_text", "Go on through."));
    REQUIRE(workspace.AddPerspectiveEventPageBranchCommand("ev_guard", "main", 0, false, "show_text", "No entry."));

    workspace.Render({0.016f, 33});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].command_count == 1);
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].code == "conditional_branch");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].condition_type == "switch");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].condition_key == "guard_bribed");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].condition_comparison == "equals");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].condition_value == "true");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].true_command_count == 1);
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].false_command_count == 1);
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].true_commands[0].argument == "Go on through.");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].false_commands[0].argument == "No entry.");

    const auto save = workspace.SavePerspectiveMapDraft();
    REQUIRE(save.success);
    const auto savedJson = nlohmann::json::parse(save.serialized_document_json);
    const auto branchJson = savedJson["events"][0]["pages"][0]["commands"][0];
    REQUIRE(branchJson["code"] == "conditional_branch");
    REQUIRE(branchJson["condition"]["key"] == "guard_bribed");
    REQUIRE(branchJson["true_commands"][0]["argument"] == "Go on through.");
    REQUIRE(branchJson["false_commands"][0]["argument"] == "No entry.");

    const auto playtest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE(playtest.success);
    const auto manifest = nlohmann::json::parse(playtest.serialized_runtime_manifest_json);
    const auto manifestBranch = manifest["events"][0]["commands"][0];
    REQUIRE(manifestBranch["code"] == "conditional_branch");
    REQUIRE(manifestBranch["condition"]["comparison"] == "equals");
    REQUIRE(manifestBranch["true_commands"][0]["code"] == "show_text");
    REQUIRE(manifestBranch["false_commands"][0]["code"] == "show_text");
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D previews active event execution traces",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_event_execution";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_event_execution", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTile("town", "floor"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_guard", "Guard", "confirm_interact", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_guard", "main", "Main", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_guard", "main", "show_text", "Guard stops you."));
    REQUIRE(workspace.AddPerspectiveEventPageConditionalBranch("ev_guard", "main", "switch", "guard_bribed", "equals", "true"));
    REQUIRE(workspace.AddPerspectiveEventPageBranchCommand("ev_guard", "main", 1, true, "transfer_player", "town:4,1"));
    REQUIRE(workspace.AddPerspectiveEventPageBranchCommand("ev_guard", "main", 1, false, "show_text", "Bring a pass first."));

    auto blockedTrace = workspace.PreviewPerspectiveEventExecution("missing_event");
    REQUIRE_FALSE(blockedTrace.success);
    REQUIRE(blockedTrace.blocker_codes[0] == "p2d_event_missing");

    auto falseTrace = workspace.PreviewPerspectiveEventExecution("ev_guard");
    REQUIRE(falseTrace.success);
    REQUIRE(falseTrace.active_page_id == "main");
    REQUIRE(falseTrace.trigger_id == "confirm_interact");
    REQUIRE(falseTrace.executed_command_count == 3);
    REQUIRE(falseTrace.executed_commands[0].code == "show_text");
    REQUIRE(falseTrace.executed_commands[1].code == "conditional_branch");
    REQUIRE_FALSE(falseTrace.executed_commands[1].condition_matched);
    REQUIRE(falseTrace.executed_commands[2].branch_path == "false");
    REQUIRE(falseTrace.executed_commands[2].argument == "Bring a pass first.");

    REQUIRE(workspace.SetPerspectiveEventConditionValue("switch", "guard_bribed", "true"));
    auto trueTrace = workspace.PreviewPerspectiveEventExecution("ev_guard");
    REQUIRE(trueTrace.success);
    REQUIRE(trueTrace.executed_command_count == 3);
    REQUIRE(trueTrace.executed_commands[1].condition_matched);
    REQUIRE(trueTrace.executed_commands[2].branch_path == "true");
    REQUIRE(trueTrace.executed_commands[2].code == "transfer_player");
    REQUIRE(trueTrace.executed_commands[2].argument == "town:4,1");

    const auto traceJson = nlohmann::json::parse(trueTrace.serialized_execution_trace_json);
    REQUIRE(traceJson["document_kind"] == "urpg.perspective_2d.event_execution_trace");
    REQUIRE(traceJson["map_id"] == "p2d_event_execution");
    REQUIRE(traceJson["event_id"] == "ev_guard");
    REQUIRE(traceJson["active_page_id"] == "main");
    REQUIRE(traceJson["executed_commands"][1]["condition"]["matched"] == true);
    REQUIRE(traceJson["executed_commands"][2]["branch_path"] == "true");

    workspace.Render({0.016f, 34});
    const auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.last_perspective_2d_event_execution.success);
    REQUIRE(snapshot.last_perspective_2d_event_execution.executed_command_count == 3);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D completes playtest package UX and release gate",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_completion";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_completion", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    workspace.SetPerspectiveTilePaletteOptions({
        {"grass_01", "Grass A", "overworld", "grass_a", "asset.overworld.grass_a",
         "content/tiles/grass_a.png", "field", "content/tiles/grass_a.preview.png"},
    });
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTilePaletteOption("grass_01"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_guard", "Guard", "confirm_interact", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_guard", "main", "Main", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_guard", "main", "show_text", "Guard stops you."));
    REQUIRE(workspace.AddPerspectiveEventPageConditionalBranch("ev_guard", "main", "switch", "guard_bribed", "equals", "true"));
    REQUIRE(workspace.AddPerspectiveEventPageBranchCommand("ev_guard", "main", 1, true, "transfer_player", "town:4,1"));
    REQUIRE(workspace.AddPerspectiveEventPageBranchCommand("ev_guard", "main", 1, false, "show_text", "Bring a pass first."));
    REQUIRE(workspace.SetPerspectiveEventConditionValue("switch", "guard_bribed", "true"));

    const auto releaseGate = workspace.RecordPerspectiveReleaseAssetGate(2, 2, 32229, 32229);
    REQUIRE(releaseGate.success);
    REQUIRE(releaseGate.release_required_asset_count == 2);
    REQUIRE(releaseGate.verified_release_required_asset_count == 2);
    REQUIRE(releaseGate.optional_lfs_asset_count == 32229);
    REQUIRE(releaseGate.optional_lfs_deferred_count == 32229);
    REQUIRE(releaseGate.policy_state == "bounded_release_required_verified_optional_lfs_deferred");

    const auto playtest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE(playtest.success);
    REQUIRE(playtest.runtime_event_execution_trace_count == 1);
    const auto traceBundle = nlohmann::json::parse(playtest.serialized_event_execution_traces_json);
    REQUIRE(traceBundle["document_kind"] == "urpg.perspective_2d.event_execution_trace_bundle");
    REQUIRE(traceBundle["traces"][0]["event_id"] == "ev_guard");
    REQUIRE(traceBundle["traces"][0]["executed_commands"][2]["branch_path"] == "true");
    REQUIRE(traceBundle["traces"][0]["executed_commands"][2]["code"] == "transfer_player");

    const auto exportResult = workspace.ExportPerspectiveMap();
    REQUIRE(exportResult.success);
    REQUIRE(exportResult.runtime_event_execution_trace_count == 1);
    REQUIRE_FALSE(exportResult.package_signature.empty());
    REQUIRE(exportResult.package_files.size() == 4);
    REQUIRE(exportResult.package_files[0].path == "maps/p2d_completion.p2d.json");
    REQUIRE(exportResult.package_files[1].path == "maps/p2d_completion.runtime.json");
    REQUIRE(exportResult.package_files[2].path == "maps/p2d_completion.event_traces.json");
    REQUIRE(exportResult.package_files[3].path == "package/p2d_completion.package.json");
    REQUIRE(exportResult.package_files[2].kind == "event_execution_traces");
    REQUIRE(exportResult.package_files[2].byte_count > 0);
    REQUIRE_FALSE(exportResult.package_files[2].content_hash.empty());

    const auto packageManifest = nlohmann::json::parse(exportResult.serialized_package_manifest_json);
    REQUIRE(packageManifest["package_signature"] == exportResult.package_signature);
    REQUIRE(packageManifest["files"].size() == 4);
    REQUIRE(packageManifest["release_asset_gate"]["policy_state"] ==
            "bounded_release_required_verified_optional_lfs_deferred");
    REQUIRE(packageManifest["release_asset_gate"]["optional_lfs_asset_count"] == 32229);

    workspace.Render({0.016f, 35});
    const auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_project.creator_workflow_ready);
    REQUIRE(snapshot.perspective_2d_project.creator_next_step == "Ready to playtest and export.");
    REQUIRE(snapshot.perspective_2d_project.layer_workflow_ready);
    REQUIRE(snapshot.perspective_2d_project.palette_workflow_ready);
    REQUIRE(snapshot.perspective_2d_project.event_workflow_ready);
    REQUIRE(snapshot.perspective_2d_project.playtest_workflow_ready);
    REQUIRE(snapshot.perspective_2d_project.export_workflow_ready);
    REQUIRE(snapshot.perspective_2d_project.release_asset_gate_ready);
    REQUIRE(snapshot.last_perspective_2d_release_asset_gate.success);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D executes live event commands and exposes RPG editor UI",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_live_runtime";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_live_runtime", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    workspace.SetPerspectiveTilePaletteOptions({
        {"grass_01", "Grass A", "overworld", "grass_a", "asset.overworld.grass_a",
         "content/tiles/grass_a.png", "field", "content/tiles/grass_a.preview.png"},
    });
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTilePaletteOption("grass_01"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_runtime", "Runtime Event", "confirm_interact", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_runtime", "main", "Main", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "show_text", "Welcome."));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "show_choice", "accept_moonwell_quest"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "change_switch", "door_open=true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "change_variable", "rank+=2"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "change_self_switch", "A=true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "change_gold", "+50"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "change_item", "potion:+3"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "move_route", "right,down"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "call_common_event", "common_unlock"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "start_battle", "shrine_wisp"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_runtime", "main", "open_vendor", "rowan_tonics"));
    REQUIRE(workspace.AddPerspectiveEventPageConditionalBranch("ev_runtime", "main", "switch", "door_open", "equals", "true"));
    REQUIRE(workspace.AddPerspectiveEventPageBranchCommand("ev_runtime", "main", 11, true, "transfer_player", "castle:2,7"));
    REQUIRE(workspace.SelectPerspectiveEventPage("ev_runtime", "main"));

    const auto runtime = workspace.ExecutePerspectiveRuntimeEvent("ev_runtime");
    REQUIRE(runtime.success);
    REQUIRE(runtime.executed_command_count == 13);
    REQUIRE(runtime.messages[0] == "Welcome.");
    REQUIRE(runtime.dialogue_choices[0] == "accept_moonwell_quest");
    REQUIRE(runtime.switches[0].key == "door_open");
    REQUIRE(runtime.switches[0].value == "true");
    REQUIRE(runtime.variables[0].key == "rank");
    REQUIRE(runtime.variables[0].value == "2");
    REQUIRE(runtime.self_switches[0].key == "ev_runtime:A");
    REQUIRE(runtime.self_switches[0].value == "true");
    REQUIRE(runtime.gold == 50);
    REQUIRE(runtime.inventory[0].key == "potion");
    REQUIRE(runtime.inventory[0].value == "3");
    REQUIRE(runtime.common_events[0] == "common_unlock");
    REQUIRE(runtime.battles[0] == "shrine_wisp");
    REQUIRE(runtime.vendors[0] == "rowan_tonics");
    REQUIRE(runtime.movement_route_steps[0] == "right");
    REQUIRE(runtime.player_map_id == "castle");
    REQUIRE(runtime.player_tile_x == 2);
    REQUIRE(runtime.player_tile_y == 7);

    const auto restored = workspace.RestorePerspectiveRuntimeState(runtime.serialized_runtime_state_json);
    REQUIRE(restored.success);
    REQUIRE(restored.command_id == "restore_perspective_2d_runtime_state");
    REQUIRE(restored.switches[0].key == "door_open");
    REQUIRE(restored.inventory[0].value == "3");
    REQUIRE(restored.player_map_id == "castle");
    REQUIRE_FALSE(workspace.RestorePerspectiveRuntimeState("{}").success);

    workspace.Render({0.016f, 36});
    const auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.last_perspective_2d_runtime.success);
    REQUIRE(snapshot.perspective_2d_ui.map_canvas_visible);
    REQUIRE(snapshot.perspective_2d_ui.layer_panel_visible);
    REQUIRE(snapshot.perspective_2d_ui.tile_palette_visible);
    REQUIRE(snapshot.perspective_2d_ui.event_page_tabs_visible);
    REQUIRE(snapshot.perspective_2d_ui.condition_editor_visible);
    REQUIRE(snapshot.perspective_2d_ui.command_list_visible);
    REQUIRE(snapshot.perspective_2d_ui.branch_tree_visible);
    REQUIRE(snapshot.perspective_2d_ui.command_picker_visible);
    REQUIRE(snapshot.perspective_2d_ui.playtest_controls_visible);
    REQUIRE(snapshot.perspective_2d_ui.export_controls_visible);
    REQUIRE(snapshot.perspective_2d_ui.selected_event_id == "ev_runtime");
    REQUIRE(snapshot.perspective_2d_ui.selected_page_id == "main");
    REQUIRE(snapshot.perspective_2d_ui.command_picker_options.size() >= 9);
    REQUIRE(std::find(snapshot.perspective_2d_ui.command_picker_options.begin(),
                      snapshot.perspective_2d_ui.command_picker_options.end(),
                      "transfer_player") != snapshot.perspective_2d_ui.command_picker_options.end());
    REQUIRE(std::find(snapshot.perspective_2d_ui.command_picker_options.begin(),
                      snapshot.perspective_2d_ui.command_picker_options.end(),
                      "change_switch") != snapshot.perspective_2d_ui.command_picker_options.end());
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D map events start saved native dialogue graphs",
          "[editor][spatial][p2d_depth]") {
    auto& global_state = urpg::GlobalStateHub::getInstance();
    global_state.clearSessionState();
    const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto project_root = std::filesystem::temp_directory_path() / ("urpg_p2d_event_dialogue_" + unique);
    std::filesystem::create_directories(project_root / "content" / "dialogues");

    urpg::dialogue::DialogueGraph graph;
    urpg::dialogue::DialogueNode start;
    start.id = "start";
    start.speaker_id = "guide";
    start.speaker_name = "Guide";
    start.text_preview = "The saved dialogue graph is running.";
    start.ending = true;
    REQUIRE(graph.addNode(start));
    graph.setStartNode("start");
    {
        std::ofstream output(project_root / "content" / "dialogues" / "moonwell_intro.json", std::ios::binary);
        REQUIRE(output.good());
        output << graph.serialize().dump(2) << '\n';
    }

    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_dialogue_map";
    overlay.elevation.width = 4;
    overlay.elevation.height = 4;
    overlay.elevation.levels.resize(16, 0);
    urpg::scene::MapScene map("p2d_dialogue_map", 4, 4);
    map.setProjectRoot(project_root);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 80.0f;
    projection.viewportHeight = 80.0f;
    projection.cameraCenterX = 2.0f;
    projection.cameraCenterZ = 2.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);
    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    workspace.SetPerspectiveTilePaletteOptions({{"grass", "Grass", "overworld", "grass", "asset.grass",
                                                 "content/tiles/grass.png", "field", ""}});
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTilePaletteOption("grass"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(40.0f, 40.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("moonwell", "Moonwell", "confirm_interact", 40.0f, 40.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("moonwell", "main", "Main", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("moonwell", "main", "change_variable", "moonwell_visited=1"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("moonwell", "main", "change_self_switch", "A=true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("moonwell", "main", "start_dialogue", "moonwell_intro"));

    REQUIRE(map.authoredDialogueInteractions().size() == 1);
    const auto& interaction = map.authoredDialogueInteractions().front();
    REQUIRE(interaction.event_id == "moonwell");
    const auto original_interaction = interaction;
    map.getPlayerMovement().gridPos = {interaction.tile_x, interaction.tile_y};
    urpg::input::InputCore input;
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);
    REQUIRE(map.activeDialogueConversationId() == "project.dialogue.moonwell_intro");
    REQUIRE(map.isDialogueActive());
    REQUIRE(std::get<int32_t>(global_state.getVariable("moonwell_visited")) == 1);

    {
        std::ofstream output(project_root / "content" / "dialogues" / "moonwell_ranked.json", std::ios::binary);
        REQUIRE(output.good());
        output << graph.serialize().dump(2) << '\n';
    }
    REQUIRE(workspace.AddPerspectiveEventPage("moonwell", "ranked", "Ranked", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageConditionRule("moonwell", "ranked", "variable", "rank",
                                                            "greater_equal", "2"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("moonwell", "ranked", "start_dialogue", "moonwell_ranked"));
    REQUIRE(map.authoredDialogueInteractions().size() == 1);
    REQUIRE(map.authoredDialogueInteractions().front().page_candidates.size() == 2);
    global_state.setVariable("rank", int32_t{2});
    REQUIRE(map.triggerAuthoredDialogueInteractionAtTile("confirm_interact", original_interaction.tile_x,
                                                          original_interaction.tile_y));
    REQUIRE(map.activeDialogueConversationId() == "project.dialogue.moonwell_ranked");

    {
        std::ofstream output(project_root / "content" / "dialogues" / "moonwell_self_switch.json", std::ios::binary);
        REQUIRE(output.good());
        output << graph.serialize().dump(2) << '\n';
    }
    REQUIRE(workspace.AddPerspectiveEventPage("moonwell", "self_switch", "Self Switch", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageConditionRule("moonwell", "self_switch", "self_switch", "A", "equals",
                                                            "true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("moonwell", "self_switch", "start_dialogue",
                                                      "moonwell_self_switch"));
    REQUIRE(map.authoredDialogueInteractions().front().page_candidates.size() == 3);
    REQUIRE(map.triggerAuthoredDialogueInteractionAtTile("confirm_interact", original_interaction.tile_x,
                                                          original_interaction.tile_y));
    REQUIRE(map.activeDialogueConversationId() == "project.dialogue.moonwell_self_switch");

    auto duplicate = original_interaction;
    duplicate.event_id = "duplicate";
    REQUIRE_FALSE(map.setAuthoredDialogueInteractions({original_interaction, duplicate}));
    REQUIRE(map.authoredDialogueInteractions().size() == 1);

    urpg::scene::MapScene::AuthoredDialogueInteraction rejected_interaction;
    rejected_interaction.event_id = "invalid_graph";
    rejected_interaction.trigger_id = "confirm_interact";
    rejected_interaction.dialogue_id = "missing_graph";
    rejected_interaction.tile_x = original_interaction.tile_x;
    rejected_interaction.tile_y = original_interaction.tile_y;
    rejected_interaction.state_writes.push_back({
        urpg::scene::MapScene::AuthoredDialogueInteraction::StateWriteKind::SetVariable,
        "must_not_write",
        42,
    });
    REQUIRE(map.setAuthoredDialogueInteractions({rejected_interaction}));
    REQUIRE(map.triggerAuthoredDialogueInteractionAtTile("confirm_interact", original_interaction.tile_x,
                                                          original_interaction.tile_y));
    REQUIRE(std::get<int32_t>(global_state.getVariable("must_not_write")) == 0);

    const auto runtime = workspace.ExecutePerspectiveRuntimeEvent("moonwell");
    REQUIRE(runtime.success);
    REQUIRE(map.activeDialogueConversationId() == "project.dialogue.moonwell_intro");
    REQUIRE(map.isDialogueActive());
    REQUIRE(std::get<int32_t>(global_state.getVariable("moonwell_visited")) == 1);

    REQUIRE(workspace.AddPerspectiveEventFromScreen("invalid_dialogue", "Invalid", "confirm_interact", 40.0f, 40.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("invalid_dialogue", "main", "Main", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("invalid_dialogue", "main", "change_variable",
                                                      "manual_must_not_write=99"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("invalid_dialogue", "main", "start_dialogue", "../escape"));
    const auto invalid_runtime = workspace.ExecutePerspectiveRuntimeEvent("invalid_dialogue");
    REQUIRE_FALSE(invalid_runtime.success);
    REQUIRE(std::get<int32_t>(global_state.getVariable("manual_must_not_write")) == 0);
    REQUIRE(std::find(invalid_runtime.blocker_codes.begin(), invalid_runtime.blocker_codes.end(),
                      "p2d_event_dialogue_start_failed:../escape") != invalid_runtime.blocker_codes.end());
    REQUIRE(std::find(map.dialogueRuntimeDiagnostics().begin(), map.dialogueRuntimeDiagnostics().end(),
                      "authored_dialogue_project_id_invalid:../escape") != map.dialogueRuntimeDiagnostics().end());

    std::error_code cleanup_error;
    std::filesystem::remove_all(project_root, cleanup_error);
    global_state.clearSessionState();
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D exposes RPG Maker-grade tile metadata",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_tile_metadata";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_tile_metadata", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    std::vector<SpatialAuthoringWorkspace::Perspective2DTilesetPage> pages;
    for (char page = 'A'; page <= 'Z'; ++page) {
        const std::string page_id(1, page);
        pages.push_back({page_id,
                         "Page " + page_id,
                         "asset.tiles.page_" + page_id,
                         "content/tiles/page_" + page_id + ".png",
                         16,
                         16,
                         48,
                         48});
    }
    REQUIRE(workspace.SetPerspectiveTilesetPages(pages));

    SpatialAuthoringWorkspace::Perspective2DTileDefinition water_tile;
    water_tile.tileset_id = "overworld";
    water_tile.tile_id = "water_edge";
    water_tile.page_id = "A";
    water_tile.autotile = true;
    water_tile.autotile_kind = "water";
    water_tile.animated = true;
    water_tile.animation_frame_tile_ids = {"water_edge_0", "water_edge_1", "water_edge_2"};
    water_tile.animation_frame_ms = 180;
    water_tile.passable_down = false;
    water_tile.passable_left = true;
    water_tile.passable_right = true;
    water_tile.passable_up = false;
    water_tile.collision = true;
    water_tile.terrain_tag = 3;
    water_tile.region_id = 7;
    water_tile.priority = 5;
    water_tile.star_passability = true;
    water_tile.preview_path = "content/tiles/water_edge.preview.png";
    REQUIRE(workspace.SetPerspectiveTileDefinition(water_tile));

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    workspace.SetPerspectiveTilePaletteOptions({
        {"water", "Water Edge", "overworld", "water_edge", "asset.overworld.water_edge",
         "content/tiles/water_edge.png", "water", "content/tiles/water_edge.preview.png"},
    });
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTilePaletteOption("water"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));

    const auto preview = workspace.PreviewPerspectiveTileAt(4, 4);
    REQUIRE(preview.success);
    REQUIRE(preview.page_id == "A");
    REQUIRE(preview.autotile);
    REQUIRE(preview.autotile_kind == "water");
    REQUIRE(preview.animated);
    REQUIRE(preview.animation_frame_tile_ids.size() == 3);
    REQUIRE_FALSE(preview.passable_down);
    REQUIRE(preview.passable_left);
    REQUIRE(preview.collision);
    REQUIRE(preview.terrain_tag == 3);
    REQUIRE(preview.region_id == 7);
    REQUIRE(preview.priority == 5);
    REQUIRE(preview.star_passability);
    REQUIRE(preview.preview_path == "content/tiles/water_edge.preview.png");

    const auto playtest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE(playtest.success);
    const auto manifest = nlohmann::json::parse(playtest.serialized_runtime_manifest_json);
    REQUIRE(manifest["tileset_pages"].size() == 26);
    REQUIRE(manifest["tile_definitions"].size() == 1);
    REQUIRE(manifest["tile_definitions"][0]["page_id"] == "A");
    REQUIRE(manifest["tile_definitions"][0]["autotile"] == true);
    REQUIRE(manifest["tile_definitions"][0]["animated"] == true);
    REQUIRE(manifest["tiles"][0]["metadata"]["region_id"] == 7);
    REQUIRE(manifest["tiles"][0]["metadata"]["star_passability"] == true);

    workspace.Render({0.016f, 37});
    const auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_tiles.tile_page_count == 26);
    REQUIRE(snapshot.perspective_2d_tiles.tile_definition_count == 1);
    REQUIRE(snapshot.perspective_2d_tiles.autotile_count == 1);
    REQUIRE(snapshot.perspective_2d_tiles.animated_tile_count == 1);
    REQUIRE(snapshot.perspective_2d_tiles.collision_tile_count == 1);
    REQUIRE(snapshot.perspective_2d_tiles.star_passability_tile_count == 1);
    REQUIRE(snapshot.perspective_2d_tiles.latest_preview.success);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D product workflow analyzes playable package proof",
          "[editor][spatial][p2d_product]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_product_town";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_product_town", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.SetPerspectiveProjectDatabaseReferences({
        {"actor", "hero", "Hero", "", 0, 0},
        {"item", "potion", "Potion", "", 0, 0},
        {"switch", "door_open", "Door Open", "", 0, 0},
        {"variable", "rank", "Rank", "", 0, 0},
        {"common_event", "common_unlock", "Unlock Door", "", 0, 0},
        {"map", "p2d_product_town", "Town", "", 0, 0},
        {"map", "castle", "Castle", "", 0, 0},
        {"transfer", "town_to_castle", "Town To Castle", "castle", 2, 7},
        {"asset", "asset.overworld.grass", "Grass", "", 0, 0},
    }));
    REQUIRE(workspace.SetPerspectiveProjectStartingParty({"hero"}));
    REQUIRE(workspace.SetPerspectiveProjectSaveLoadState(true, "slot_1"));

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    workspace.SetPerspectiveTilePaletteOptions({
        {"grass", "Grass", "overworld", "grass", "asset.overworld.grass",
         "content/tiles/grass.png", "field", "content/tiles/grass.preview.png"},
    });
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTilePaletteOption("grass"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_gate", "Gate", "confirm_interact", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_gate", "main", "Main", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "main", "show_text", "Welcome."));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "main", "change_switch", "door_open=true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "main", "change_variable", "rank+=1"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "main", "change_gold", "+25"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "main", "change_item", "potion:+1"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "main", "call_common_event", "common_unlock"));
    REQUIRE(workspace.AddPerspectiveEventPageConditionalBranch("ev_gate", "main", "switch", "door_open", "equals", "true"));
    REQUIRE(workspace.AddPerspectiveEventPageBranchCommand("ev_gate", "main", 6, true, "transfer_player", "castle:2,7"));

    REQUIRE(workspace.RecordPerspectiveReleaseAssetGate(1, 1, 2, 2).success);
    const auto save = workspace.SavePerspectiveMapDraft();
    REQUIRE(save.success);
    const auto runtime = workspace.ExecutePerspectiveRuntimeEvent("ev_gate");
    REQUIRE(runtime.success);
    const auto playtest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE(playtest.success);
    const auto export_result = workspace.ExportPerspectiveMap();
    REQUIRE(export_result.success);

    const auto report = Perspective2DProductWorkflow::Analyze(save.serialized_document_json,
                                                              playtest.serialized_runtime_manifest_json,
                                                              export_result.serialized_package_manifest_json);
    REQUIRE(report.ready);
    REQUIRE(report.map_id == "p2d_product_town");
    REQUIRE(report.draft_layer_count == 2);
    REQUIRE(report.draft_tile_count == 1);
    REQUIRE(report.draft_event_count == 1);
    REQUIRE(report.runtime_layer_count == 2);
    REQUIRE(report.runtime_tile_count == 1);
    REQUIRE(report.runtime_event_count == 1);
    REQUIRE(report.export_package_file_count >= 4);
    REQUIRE(report.package_signature_present);
    REQUIRE(report.transfer_edge_count == 1);
    REQUIRE(report.supported_command_count >= 8);
    REQUIRE(report.unsupported_command_count == 0);
    REQUIRE(report.blockers.empty());

    workspace.Render({0.016f, 39});
    const auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_product.ready);
    REQUIRE(snapshot.perspective_2d_product.map_id == "p2d_product_town");
    REQUIRE(snapshot.perspective_2d_product.transfer_edge_count == 1);
    REQUIRE(snapshot.perspective_2d_product.export_package_file_count >= 4);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D connects maps to project database references",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "town";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("town", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.SetPerspectiveProjectDatabaseReferences({
        {"actor", "hero", "Hero", "", 0, 0},
        {"item", "potion", "Potion", "", 0, 0},
        {"switch", "door_open", "Door Open", "", 0, 0},
        {"variable", "rank", "Rank", "", 0, 0},
        {"common_event", "common_unlock", "Unlock Door", "", 0, 0},
        {"map", "town", "Town", "", 0, 0},
        {"map", "castle", "Castle", "", 0, 0},
        {"transfer", "town_to_castle", "Town To Castle", "castle", 2, 7},
        {"encounter", "slime_field", "Slime Field", "town", 4, 4},
        {"asset", "asset.overworld.grass_a", "Grass A", "", 0, 0},
    }));
    REQUIRE(workspace.SetPerspectiveProjectStartingParty({"hero"}));
    REQUIRE(workspace.SetPerspectiveProjectSaveLoadState(true, "slot_1"));

    REQUIRE(workspace.AddPerspectiveLayer("ground", "Ground", "tile"));
    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    workspace.SetPerspectiveTilePaletteOptions({
        {"grass_01", "Grass A", "overworld", "grass_a", "asset.overworld.grass_a",
         "content/tiles/grass_a.png", "field", "content/tiles/grass_a.preview.png"},
    });
    REQUIRE(workspace.SelectPerspectiveLayer("ground"));
    REQUIRE(workspace.SelectPerspectiveTilePaletteOption("grass_01"));
    REQUIRE(workspace.PaintPerspectiveTileFromScreen(80.0f, 80.0f));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_db", "Database Event", "confirm_interact", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_db", "main", "Main", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_db", "main", "change_item", "potion:+1"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_db", "main", "change_switch", "door_open=true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_db", "main", "change_variable", "rank+=1"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_db", "main", "call_common_event", "common_unlock"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_db", "main", "transfer_player", "castle:2,7"));

    const auto integration = workspace.ValidatePerspectiveProjectIntegration();
    REQUIRE(integration.success);
    REQUIRE(integration.actor_count == 1);
    REQUIRE(integration.item_count == 1);
    REQUIRE(integration.switch_count == 1);
    REQUIRE(integration.variable_count == 1);
    REQUIRE(integration.common_event_count == 1);
    REQUIRE(integration.map_count == 2);
    REQUIRE(integration.transfer_count == 1);
    REQUIRE(integration.encounter_count == 1);
    REQUIRE(integration.asset_count == 1);
    REQUIRE(integration.starting_party_count == 1);
    REQUIRE(integration.save_load_enabled);
    REQUIRE(integration.diagnostics.empty());

    const auto playtest = workspace.RunPerspectiveMapPlaytest();
    REQUIRE(playtest.success);
    const auto manifest = nlohmann::json::parse(playtest.serialized_runtime_manifest_json);
    REQUIRE(manifest["project_database"]["actors"].size() == 1);
    REQUIRE(manifest["project_database"]["items"].size() == 1);
    REQUIRE(manifest["project_database"]["switches"].size() == 1);
    REQUIRE(manifest["project_database"]["variables"].size() == 1);
    REQUIRE(manifest["project_database"]["common_events"].size() == 1);
    REQUIRE(manifest["project_database"]["maps"].size() == 2);
    REQUIRE(manifest["project_database"]["starting_party"][0] == "hero");
    REQUIRE(manifest["project_database"]["save_load"]["enabled"] == true);
    REQUIRE(manifest["project_database"]["save_load"]["profile_id"] == "slot_1");
    REQUIRE(manifest["project_database"]["transfers"][0]["target_map_id"] == "castle");
    REQUIRE(manifest["project_database"]["encounters"][0]["id"] == "slime_field");
    REQUIRE(manifest["project_database"]["assets"][0]["id"] == "asset.overworld.grass_a");

    workspace.Render({0.016f, 38});
    const auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_project_database.ready);
    REQUIRE(snapshot.perspective_2d_project_database.actor_count == 1);
    REQUIRE(snapshot.perspective_2d_project_database.map_count == 2);
    REQUIRE(snapshot.perspective_2d_project_database.save_load_enabled);
}

TEST_CASE("Spatial Editor Tooling Integration - Perspective 2D edits event pages conditions and commands",
          "[editor][spatial][p2d_depth]") {
    SpatialMapOverlay overlay;
    overlay.mapId = "p2d_event_page_editing";
    overlay.elevation.width = 8;
    overlay.elevation.height = 8;
    overlay.elevation.levels.resize(64, 0);
    urpg::scene::MapScene map("p2d_event_page_editing", 8, 8);
    SpatialAuthoringWorkspace workspace;
    workspace.SetTargets(&map, &overlay);

    PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 160.0f;
    projection.viewportHeight = 160.0f;
    projection.cameraCenterX = 4.0f;
    projection.cameraCenterZ = 4.0f;
    projection.worldUnitsPerPixel = 0.1f;
    workspace.SetProjectionSettings(projection);

    REQUIRE(workspace.AddPerspectiveLayer("events", "Events", "event"));
    REQUIRE(workspace.SelectPerspectiveLayer("events"));
    REQUIRE(workspace.AddPerspectiveEventFromScreen("ev_gate", "Castle Gate", "confirm_interact", 80.0f, 80.0f));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_gate", "closed", "Closed", "confirm_interact"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "closed", "show_text", "Closed."));
    REQUIRE(workspace.AddPerspectiveEventPage("ev_gate", "open", "Open", "player_touch"));
    REQUIRE(workspace.AddPerspectiveEventPageCondition("ev_gate", "open", "switch", "gate_open", "true"));
    REQUIRE(workspace.AddPerspectiveEventPageCommand("ev_gate", "open", "transfer_player", "castle:2,7"));

    REQUIRE(workspace.DuplicatePerspectiveEventPage("ev_gate", "open", "locked", "Locked"));
    REQUIRE(workspace.UpdatePerspectiveEventPageCondition("ev_gate", "locked", 0, "self_switch", "A", "on"));
    REQUIRE(workspace.UpdatePerspectiveEventPageCommand("ev_gate", "locked", 0, "show_text", "Locked tight."));
    REQUIRE(workspace.MovePerspectiveEventPage("ev_gate", "locked", 0));
    REQUIRE(workspace.SelectPerspectiveEventPage("ev_gate", "locked"));

    workspace.Render({0.016f, 20});
    auto snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_events[0].page_count == 3);
    REQUIRE(snapshot.perspective_2d_events[0].selected_page_id == "locked");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].page_id == "locked");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].order == 0);
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].conditions[0].type == "self_switch");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].commands[0].argument == "Locked tight.");

    REQUIRE(workspace.RemovePerspectiveEventPageCondition("ev_gate", "locked", 0));
    REQUIRE(workspace.RemovePerspectiveEventPageCommand("ev_gate", "locked", 0));
    REQUIRE(workspace.DeletePerspectiveEventPage("ev_gate", "closed"));
    REQUIRE_FALSE(workspace.DeletePerspectiveEventPage("ev_gate", "missing"));

    workspace.Render({0.016f, 21});
    snapshot = workspace.lastRenderSnapshot();
    REQUIRE(snapshot.perspective_2d_events[0].page_count == 2);
    REQUIRE(snapshot.perspective_2d_events[0].selected_page_id == "locked");
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].condition_count == 0);
    REQUIRE(snapshot.perspective_2d_events[0].pages[0].command_count == 0);
    REQUIRE(snapshot.perspective_2d_events[0].pages[1].page_id == "open");
    REQUIRE(snapshot.perspective_2d_events[0].pages[1].order == 1);

    const auto save = workspace.SavePerspectiveMapDraft();
    REQUIRE(save.success);
    const auto savedJson = nlohmann::json::parse(save.serialized_document_json);
    REQUIRE(savedJson["events"][0]["selected_page_id"] == "locked");
    REQUIRE(savedJson["events"][0]["pages"].size() == 2);
    REQUIRE(savedJson["events"][0]["pages"][0]["page_id"] == "locked");
    REQUIRE(savedJson["events"][0]["pages"][0]["conditions"].empty());
    REQUIRE(savedJson["events"][0]["pages"][0]["commands"].empty());
}
