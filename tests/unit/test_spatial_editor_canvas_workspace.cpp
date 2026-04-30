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
    REQUIRE(snapshot.toolbar.actions.size() == 6);
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
