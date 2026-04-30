#include "editor/spatial/spatial_authoring_workspace.h"

#include <algorithm>
#include <cmath>

namespace urpg::editor {

const char* SpatialAuthoringWorkspace::modeName(ToolMode mode) {
    switch (mode) {
    case ToolMode::Composite:
        return "composite";
    case ToolMode::Elevation:
        return "elevation";
    case ToolMode::Props:
        return "props";
    case ToolMode::Abilities:
        return "abilities";
    case ToolMode::Parts:
        return "parts";
    }

    return "composite";
}

void SpatialAuthoringWorkspace::syncPanelVisibility() {
    const bool show_all = active_mode_ == ToolMode::Composite;
    elevation_panel_.SetVisible(show_all || active_mode_ == ToolMode::Elevation);
    prop_panel_.SetVisible(show_all || active_mode_ == ToolMode::Props);
    grid_part_palette_panel_.SetVisible(show_all || active_mode_ == ToolMode::Parts);
    grid_part_placement_panel_.SetVisible(show_all || active_mode_ == ToolMode::Parts);
    grid_part_inspector_panel_.SetVisible(show_all || active_mode_ == ToolMode::Parts);
    binding_panel_.SetVisible(show_all || active_mode_ == ToolMode::Abilities);
    canvas_panel_.SetVisible(true);
}

void SpatialAuthoringWorkspace::Render(const urpg::FrameContext& context) {
    if (!m_visible) {
        return;
    }

    syncPanelVisibility();
    elevation_panel_.Render(context);
    prop_panel_.Render(context);
    grid_part_palette_panel_.Render(context);
    grid_part_placement_panel_.Render(context);
    grid_part_inspector_panel_.Render(context);
    binding_panel_.Render(context);
    canvas_panel_.Render(context);
    captureRenderSnapshot();
}

void SpatialAuthoringWorkspace::SetTargets(urpg::scene::MapScene* scene,
                                           urpg::presentation::SpatialMapOverlay* overlay) {
    m_target_scene = scene;
    m_target_overlay = overlay;
    elevation_panel_.SetTarget(overlay);
    prop_panel_.SetTarget(overlay);
    binding_panel_.SetTarget(scene);
    binding_panel_.SetSpatialTarget(overlay);
    canvas_panel_.SetSpatialTarget(overlay);
    canvas_panel_.SetBindingPanel(&binding_panel_);
    canvas_panel_.SetActiveMode(modeName(active_mode_));
    grid_part_placement_panel_.SetTargets(grid_part_document_, grid_part_catalog_, overlay);
    syncPanelVisibility();
    captureRenderSnapshot();
}

void SpatialAuthoringWorkspace::SetGridPartTargets(urpg::map::GridPartDocument* document,
                                                   const urpg::map::GridPartCatalog* catalog) {
    grid_part_document_ = document;
    grid_part_catalog_ = catalog;
    grid_part_palette_panel_.SetCatalog(catalog);
    grid_part_placement_panel_.SetTargets(document, catalog, m_target_overlay);
    grid_part_inspector_panel_.SetTargets(document, catalog);
    syncPanelVisibility();
    captureRenderSnapshot();
}

bool SpatialAuthoringWorkspace::SetProjectRoot(const std::string& root_path) {
    const bool configured = binding_panel_.SetProjectRoot(root_path);
    captureRenderSnapshot();
    return configured;
}

void SpatialAuthoringWorkspace::SetProjectionSettings(const PropPlacementPanel::ScreenProjectionSettings& settings) {
    projection_settings_ = settings;
    canvas_panel_.SetProjectionSettings(settings);
    grid_part_placement_panel_.SetProjectionSettings(settings);
    captureRenderSnapshot();
}

void SpatialAuthoringWorkspace::SetAvailableTriggers(std::vector<std::string> trigger_ids) {
    canvas_panel_.SetAvailableTriggers(std::move(trigger_ids));
    captureRenderSnapshot();
}

void SpatialAuthoringWorkspace::SetActiveMode(ToolMode mode) {
    if (active_mode_ == mode) {
        return;
    }

    active_mode_ = mode;
    canvas_panel_.SetActiveMode(modeName(active_mode_));
    syncPanelVisibility();
    captureRenderSnapshot();
}

bool SpatialAuthoringWorkspace::ActivateToolbarAction(const std::string& action_id) {
    if (action_id == "composite") {
        SetActiveMode(ToolMode::Composite);
        return true;
    }
    if (action_id == "elevation") {
        SetActiveMode(ToolMode::Elevation);
        return true;
    }
    if (action_id == "props") {
        SetActiveMode(ToolMode::Props);
        return true;
    }
    if (action_id == "abilities") {
        SetActiveMode(ToolMode::Abilities);
        return true;
    }
    if (action_id == "parts") {
        SetActiveMode(ToolMode::Parts);
        return true;
    }
    if (action_id == "resolve_conflict") {
        const auto& canvas_snapshot = canvas_panel_.lastRenderSnapshot();
        if (canvas_snapshot.conflicts.empty()) {
            return false;
        }
        const bool resolved = canvas_panel_.ResolveConflictWithSuggestion(0);
        captureRenderSnapshot();
        return resolved;
    }

    return false;
}

bool SpatialAuthoringWorkspace::ActivateCanvasAction(const std::string& action_id) {
    if (action_id.empty()) {
        return false;
    }
    if (ActivateToolbarAction(action_id)) {
        return true;
    }

    const auto& canvas_snapshot = canvas_panel_.lastRenderSnapshot();
    const auto conflict_chip =
        std::find_if(canvas_snapshot.conflict_action_chips.begin(), canvas_snapshot.conflict_action_chips.end(),
                     [&](const auto& chip) { return chip.action_id == action_id; });
    if (conflict_chip == canvas_snapshot.conflict_action_chips.end()) {
        return false;
    }

    const size_t index = conflict_chip->conflict_index;
    bool handled = false;
    if (action_id == "resolve_conflict") {
        handled = canvas_panel_.ResolveConflictWithSuggestion(index);
    } else if (action_id.rfind("conflict:", 0) == 0) {
        const auto last_separator = action_id.rfind(':');
        const std::string operation =
            last_separator == std::string::npos ? std::string{} : action_id.substr(last_separator + 1);
        if (operation == "keep_primary") {
            handled = canvas_panel_.ResolveConflictByRemovingSecondary(index);
        } else if (operation == "keep_secondary") {
            handled = canvas_panel_.ResolveConflictByRemovingPrimary(index);
        } else if (operation == "swap") {
            handled = canvas_panel_.SwapConflictTriggers(index);
        } else if (operation == "replace_secondary") {
            handled = canvas_panel_.ReplaceSecondaryWithPrimaryAsset(index);
        }
    }

    if (handled) {
        captureRenderSnapshot();
    }
    return handled;
}

bool SpatialAuthoringWorkspace::RouteCanvasPrimaryAction(float screen_x, float screen_y) {
    switch (active_mode_) {
    case ToolMode::Composite:
    case ToolMode::Abilities: {
        const bool handled = canvas_panel_.ClickAtScreen(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    case ToolMode::Elevation: {
        if (m_target_overlay == nullptr) {
            return false;
        }
        float world_x = 0.0f;
        float world_y = 0.0f;
        float world_z = 0.0f;
        if (!PropPlacementPanel::TryProjectScreenToGround(*m_target_overlay, screen_x, screen_y, projection_settings_,
                                                          world_x, world_y, world_z)) {
            return false;
        }
        const int tile_x = static_cast<int>(std::floor(world_x));
        const int tile_y = static_cast<int>(std::floor(world_z));
        if (tile_x < 0 || tile_y < 0) {
            return false;
        }
        const int8_t target_level =
            static_cast<int8_t>(std::lround(elevation_panel_.lastRenderSnapshot().brush_height));
        elevation_panel_.ApplyBrush(static_cast<uint32_t>(tile_x), static_cast<uint32_t>(tile_y), target_level);
        captureRenderSnapshot();
        return true;
    }
    case ToolMode::Props: {
        const auto selected_asset_id = prop_panel_.lastRenderSnapshot().selected_asset_id;
        if (selected_asset_id.empty()) {
            return false;
        }
        const bool placed = prop_panel_.AddPropFromScreen(selected_asset_id, screen_x, screen_y, projection_settings_);
        captureRenderSnapshot();
        return placed;
    }
    case ToolMode::Parts: {
        const bool placed = grid_part_placement_panel_.PlaceSelectedPartFromScreen(screen_x, screen_y);
        if (placed && grid_part_document_ != nullptr) {
            const auto& parts = grid_part_document_->parts();
            if (!parts.empty()) {
                (void)grid_part_inspector_panel_.SelectInstance(parts.back().instance_id);
            }
        }
        captureRenderSnapshot();
        return placed;
    }
    }

    return false;
}

bool SpatialAuthoringWorkspace::RouteCanvasSecondaryAction(float screen_x, float screen_y) {
    switch (active_mode_) {
    case ToolMode::Composite:
    case ToolMode::Abilities: {
        const bool handled = canvas_panel_.ApplySelectedAssetToSelection();
        captureRenderSnapshot();
        return handled;
    }
    case ToolMode::Elevation:
        return RouteCanvasPrimaryAction(screen_x, screen_y);
    case ToolMode::Props: {
        const bool handled = canvas_panel_.ClickAtScreen(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    case ToolMode::Parts: {
        const bool undone = grid_part_placement_panel_.Undo();
        captureRenderSnapshot();
        return undone;
    }
    }

    return false;
}

bool SpatialAuthoringWorkspace::RouteCanvasHover(float screen_x, float screen_y) {
    switch (active_mode_) {
    case ToolMode::Composite:
    case ToolMode::Abilities: {
        const bool handled = canvas_panel_.HoverAtScreen(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    case ToolMode::Elevation:
    case ToolMode::Props: {
        const bool handled = canvas_panel_.HoverAtScreen(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    case ToolMode::Parts: {
        const bool handled = grid_part_placement_panel_.HoverSelectedPartFromScreen(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    }

    return false;
}

bool SpatialAuthoringWorkspace::SelectGridPart(const std::string& part_id) {
    const bool palette_selected = grid_part_palette_panel_.SelectPart(part_id);
    const bool placement_selected = grid_part_placement_panel_.SetSelectedPartId(part_id);
    captureRenderSnapshot();
    return palette_selected && placement_selected;
}

void SpatialAuthoringWorkspace::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.has_target_scene = (m_target_scene != nullptr);
    last_render_snapshot_.has_target_overlay = (m_target_overlay != nullptr);
    if (!last_render_snapshot_.has_target_scene && !last_render_snapshot_.has_target_overlay) {
        last_render_snapshot_.status = "disabled";
        last_render_snapshot_.message = "No spatial authoring targets are bound.";
        last_render_snapshot_.remediation =
            "Bind a MapScene and SpatialMapOverlay before using spatial authoring tools.";
    } else if (!last_render_snapshot_.has_target_scene || !last_render_snapshot_.has_target_overlay) {
        last_render_snapshot_.status = "error";
        last_render_snapshot_.message = "Spatial authoring is partially bound.";
        last_render_snapshot_.remediation =
            "Bind both MapScene and SpatialMapOverlay so ability, elevation, and prop tools can operate together.";
    } else {
        last_render_snapshot_.status = "ready";
        last_render_snapshot_.message = "Spatial authoring targets are ready.";
        last_render_snapshot_.remediation = "";
    }
    last_render_snapshot_.elevation = elevation_panel_.lastRenderSnapshot();
    last_render_snapshot_.props = prop_panel_.lastRenderSnapshot();
    last_render_snapshot_.parts_palette = grid_part_palette_panel_.lastRenderSnapshot();
    last_render_snapshot_.parts_placement = grid_part_placement_panel_.lastRenderSnapshot();
    last_render_snapshot_.parts_inspector = grid_part_inspector_panel_.lastRenderSnapshot();
    last_render_snapshot_.bindings = binding_panel_.lastRenderSnapshot();
    last_render_snapshot_.canvas = canvas_panel_.lastRenderSnapshot();
    last_render_snapshot_.toolbar.active_mode = modeName(active_mode_);
    last_render_snapshot_.toolbar.selected_trigger_id = last_render_snapshot_.canvas.selection.trigger_id;
    last_render_snapshot_.toolbar.selected_ability_id = last_render_snapshot_.bindings.selected_ability_id;
    last_render_snapshot_.toolbar.selected_prop_asset_id = last_render_snapshot_.props.selected_asset_id;
    last_render_snapshot_.toolbar.placement_tile_x = last_render_snapshot_.bindings.placement.tile_x;
    last_render_snapshot_.toolbar.placement_tile_y = last_render_snapshot_.bindings.placement.tile_y;
    last_render_snapshot_.toolbar.has_conflicts = last_render_snapshot_.canvas.has_conflicts;
    last_render_snapshot_.toolbar.conflict_count = last_render_snapshot_.canvas.conflict_count;
    last_render_snapshot_.toolbar.can_apply_suggested_conflict_resolution = last_render_snapshot_.canvas.has_conflicts;
    last_render_snapshot_.toolbar.actions = {
        {"composite", "Composite", active_mode_ == ToolMode::Composite, true},
        {"elevation", "Elevation", active_mode_ == ToolMode::Elevation, last_render_snapshot_.has_target_overlay},
        {"props", "Props", active_mode_ == ToolMode::Props, last_render_snapshot_.has_target_overlay},
        {"abilities", "Abilities", active_mode_ == ToolMode::Abilities, last_render_snapshot_.has_target_scene},
        {"parts", "Parts", active_mode_ == ToolMode::Parts,
         last_render_snapshot_.parts_placement.has_document && last_render_snapshot_.parts_placement.has_catalog &&
             last_render_snapshot_.parts_placement.has_spatial_overlay},
        {"resolve_conflict", "Resolve Conflict", false, last_render_snapshot_.canvas.has_conflicts},
    };
    canvas_panel_.SetActiveMode(last_render_snapshot_.toolbar.active_mode);
    last_render_snapshot_.canvas = canvas_panel_.lastRenderSnapshot();
}

} // namespace urpg::editor
