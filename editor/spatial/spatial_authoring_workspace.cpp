#include "editor/spatial/spatial_authoring_workspace.h"

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
    }

    return "composite";
}

void SpatialAuthoringWorkspace::syncPanelVisibility() {
    const bool show_all = active_mode_ == ToolMode::Composite;
    elevation_panel_.SetVisible(show_all || active_mode_ == ToolMode::Elevation);
    prop_panel_.SetVisible(show_all || active_mode_ == ToolMode::Props);
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
        if (!PropPlacementPanel::TryProjectScreenToGround(
                *m_target_overlay,
                screen_x,
                screen_y,
                projection_settings_,
                world_x,
                world_y,
                world_z)) {
            return false;
        }
        const int tile_x = static_cast<int>(std::floor(world_x));
        const int tile_y = static_cast<int>(std::floor(world_z));
        if (tile_x < 0 || tile_y < 0) {
            return false;
        }
        const int8_t target_level = static_cast<int8_t>(std::lround(elevation_panel_.lastRenderSnapshot().brush_height));
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
    }

    return false;
}

void SpatialAuthoringWorkspace::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.has_target_scene = (m_target_scene != nullptr);
    last_render_snapshot_.has_target_overlay = (m_target_overlay != nullptr);
    last_render_snapshot_.elevation = elevation_panel_.lastRenderSnapshot();
    last_render_snapshot_.props = prop_panel_.lastRenderSnapshot();
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
        {"resolve_conflict", "Resolve Conflict", false, last_render_snapshot_.canvas.has_conflicts},
    };
}

} // namespace urpg::editor
