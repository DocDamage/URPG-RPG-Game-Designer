#include "editor/spatial/spatial_ability_canvas_panel.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace urpg::editor {

namespace {

bool RegionContains(const MapAbilityBindingPanel::RegionOverlayEntry& region, int tile_x, int tile_y) {
    return tile_x >= region.min_x &&
           tile_x <= region.max_x &&
           tile_y >= region.min_y &&
           tile_y <= region.max_y;
}

int RegionArea(const MapAbilityBindingPanel::RegionOverlayEntry& region) {
    return (region.max_x - region.min_x + 1) * (region.max_y - region.min_y + 1);
}

bool RegionsOverlap(const MapAbilityBindingPanel::RegionOverlayEntry& lhs,
                    const MapAbilityBindingPanel::RegionOverlayEntry& rhs) {
    return lhs.min_x <= rhs.max_x &&
           lhs.max_x >= rhs.min_x &&
           lhs.min_y <= rhs.max_y &&
           lhs.max_y >= rhs.min_y;
}

std::vector<SpatialAbilityCanvasPanel::TriggerMenuEntry> BuildTriggerMenu(
    SpatialAbilityCanvasPanel::SelectionKind kind,
    const std::vector<std::string>& trigger_ids,
    const std::string& selected_trigger_id) {
    std::vector<SpatialAbilityCanvasPanel::TriggerMenuEntry> menu;
    menu.reserve(trigger_ids.size());
    for (const auto& trigger_id : trigger_ids) {
        bool allowed = true;
        bool recommended = false;
        if (kind == SpatialAbilityCanvasPanel::SelectionKind::Tile) {
            allowed = trigger_id == "confirm_interact" || trigger_id == "touch_interact";
            recommended = trigger_id == "confirm_interact";
        } else if (kind == SpatialAbilityCanvasPanel::SelectionKind::Prop) {
            allowed = trigger_id == "confirm_interact" || trigger_id == "touch_interact" || trigger_id == "inspect_prop";
            recommended = trigger_id == "inspect_prop";
        } else if (kind == SpatialAbilityCanvasPanel::SelectionKind::Region) {
            allowed = trigger_id == "confirm_interact" || trigger_id == "touch_interact" || trigger_id == "enter_region";
            recommended = trigger_id == "enter_region";
        } else {
            recommended = trigger_id == "confirm_interact";
        }

        menu.push_back({trigger_id, trigger_id, recommended, trigger_id == selected_trigger_id, allowed});
    }
    return menu;
}

std::vector<SpatialAbilityCanvasPanel::ModeBadge> BuildModeBadges(
    const std::string& active_mode,
    bool has_target_overlay,
    bool has_target_scene) {
    return {
        {"composite", "Composite", active_mode == "composite", true},
        {"elevation", "Elevation", active_mode == "elevation", has_target_overlay},
        {"props", "Props", active_mode == "props", has_target_overlay},
        {"abilities", "Abilities", active_mode == "abilities", has_target_scene},
    };
}

std::vector<SpatialAbilityCanvasPanel::HoverAffordance> BuildHoverAffordances(
    SpatialAbilityCanvasPanel::SelectionKind kind,
    bool hover_active,
    const std::string& active_mode) {
    if (!hover_active) {
        return {};
    }

    switch (kind) {
    case SpatialAbilityCanvasPanel::SelectionKind::Tile:
        return {
            {"abilities", "Bind Ability", "Switch to interaction authoring on this hovered tile.", "tile", active_mode == "abilities", true},
            {"elevation", "Raise Terrain", "Switch to elevation editing for this hovered tile.", "tile", active_mode == "elevation", true},
        };
    case SpatialAbilityCanvasPanel::SelectionKind::Prop:
        return {
            {"props", "Edit Prop", "Switch to prop placement and selection for this hovered prop.", "prop", active_mode == "props", true},
            {"abilities", "Bind Ability", "Switch to interaction authoring on this hovered prop.", "prop", active_mode == "abilities", true},
        };
    case SpatialAbilityCanvasPanel::SelectionKind::Region:
        return {
            {"abilities", "Edit Region", "Switch to interaction authoring for this hovered region.", "region", active_mode == "abilities", true},
            {"elevation", "Shape Area", "Switch to terrain editing around this hovered region.", "region", active_mode == "elevation", true},
        };
    case SpatialAbilityCanvasPanel::SelectionKind::None:
        return {};
    }

    return {};
}

std::vector<SpatialAbilityCanvasPanel::ConflictActionChip> BuildConflictActionChips(
    const std::vector<SpatialAbilityCanvasPanel::ConflictWarning>& conflicts) {
    std::vector<SpatialAbilityCanvasPanel::ConflictActionChip> chips;
    for (size_t i = 0; i < conflicts.size(); ++i) {
        const auto& conflict = conflicts[i];
        chips.push_back({"resolve_conflict", "Resolve", conflict.kind, conflict.severity, i, true, true});
        if (conflict.can_keep_primary) {
            chips.push_back({"conflict:" + std::to_string(i) + ":keep_primary",
                             "Keep Primary",
                             conflict.kind,
                             conflict.severity,
                             i,
                             conflict.severity == "blocking",
                             true});
        }
        if (conflict.can_keep_secondary) {
            chips.push_back({"conflict:" + std::to_string(i) + ":keep_secondary",
                             "Keep Secondary",
                             conflict.kind,
                             conflict.severity,
                             i,
                             false,
                             true});
        }
        if (conflict.can_swap_triggers) {
            chips.push_back({"conflict:" + std::to_string(i) + ":swap",
                             "Swap",
                             conflict.kind,
                             conflict.severity,
                             i,
                             conflict.severity != "blocking",
                             true});
        }
        if (conflict.can_replace_secondary) {
            chips.push_back({"conflict:" + std::to_string(i) + ":replace_secondary",
                             "Replace",
                             conflict.kind,
                             conflict.severity,
                             i,
                             false,
                             true});
        }
    }
    return chips;
}

std::optional<size_t> FindNearestPropIndex(const urpg::presentation::SpatialMapOverlay& overlay,
                                           float world_x,
                                           float world_z,
                                           float max_distance) {
    std::optional<size_t> best_index;
    float best_distance_sq = max_distance * max_distance;
    for (size_t i = 0; i < overlay.props.size(); ++i) {
        const auto& prop = overlay.props[i];
        const float dx = prop.posX - world_x;
        const float dz = prop.posZ - world_z;
        const float distance_sq = dx * dx + dz * dz;
        if (distance_sq <= best_distance_sq) {
            best_distance_sq = distance_sq;
            best_index = i;
        }
    }
    return best_index;
}

} // namespace

void SpatialAbilityCanvasPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible) {
        return;
    }

    captureRenderSnapshot();
}

void SpatialAbilityCanvasPanel::SetSpatialTarget(urpg::presentation::SpatialMapOverlay* overlay) {
    m_target_overlay = overlay;
    captureRenderSnapshot();
}

void SpatialAbilityCanvasPanel::SetBindingPanel(MapAbilityBindingPanel* panel) {
    m_binding_panel = panel;
    captureRenderSnapshot();
}

void SpatialAbilityCanvasPanel::SetProjectionSettings(const PropPlacementPanel::ScreenProjectionSettings& settings) {
    m_projection_settings = settings;
    captureRenderSnapshot();
}

void SpatialAbilityCanvasPanel::SetAvailableTriggers(std::vector<std::string> trigger_ids) {
    if (trigger_ids.empty()) {
        trigger_ids = {"confirm_interact", "touch_interact", "inspect_prop", "enter_region"};
    }
    m_available_triggers = std::move(trigger_ids);
    captureRenderSnapshot();
}

void SpatialAbilityCanvasPanel::SetActiveMode(const std::string& active_mode) {
    if (m_active_mode == active_mode || active_mode.empty()) {
        return;
    }
    m_active_mode = active_mode;
    captureRenderSnapshot();
}

namespace {

bool ProjectToTile(const urpg::presentation::SpatialMapOverlay* overlay,
                   const urpg::editor::PropPlacementPanel::ScreenProjectionSettings& settings,
                   float screen_x,
                   float screen_y,
                   int& tile_x,
                   int& tile_y) {
    if (overlay == nullptr) {
        return false;
    }
    float world_x = 0.0f;
    float world_y = 0.0f;
    float world_z = 0.0f;
    if (!urpg::editor::PropPlacementPanel::TryProjectScreenToGround(
            *overlay,
            screen_x,
            screen_y,
            settings,
            world_x,
            world_y,
            world_z)) {
        return false;
    }

    tile_x = static_cast<int>(std::floor(world_x));
    tile_y = static_cast<int>(std::floor(world_z));
    return true;
}

}

bool SpatialAbilityCanvasPanel::HoverAtScreen(float screen_x, float screen_y) {
    if (m_target_overlay == nullptr || m_binding_panel == nullptr) {
        return false;
    }

    int tile_x = -1;
    int tile_y = -1;
    if (!ProjectToTile(m_target_overlay, m_projection_settings, screen_x, screen_y, tile_x, tile_y)) {
        m_hover_preview = {};
        captureRenderSnapshot();
        return false;
    }

    float world_x = 0.0f;
    float world_y = 0.0f;
    float world_z = 0.0f;
    if (!PropPlacementPanel::TryProjectScreenToGround(
            *m_target_overlay,
            screen_x,
            screen_y,
            m_projection_settings,
            world_x,
            world_y,
            world_z)) {
        m_hover_preview = {};
        captureRenderSnapshot();
        return false;
    }

    const auto& snapshot = m_binding_panel->lastRenderSnapshot();
    m_hover_preview = {};
    m_hover_preview.active = true;
    m_hover_preview.trigger_id = !m_selection.trigger_id.empty() ? m_selection.trigger_id : snapshot.selected_trigger_id;
    m_hover_preview.ability_id = snapshot.selected_ability_id;

    switch (m_selection.kind) {
    case SelectionKind::Prop: {
        m_hover_preview.kind = SelectionKind::Prop;
        if (const auto nearest_prop_index = FindNearestPropIndex(*m_target_overlay, world_x, world_z, 1.0f);
            nearest_prop_index.has_value()) {
            const auto& prop = m_target_overlay->props[*nearest_prop_index];
            m_hover_preview.prop_asset_id = prop.assetId;
            m_hover_preview.tile_x = static_cast<int>(std::floor(prop.posX));
            m_hover_preview.tile_y = static_cast<int>(std::floor(prop.posZ));
        }
        break;
    }
    case SelectionKind::Region:
        m_hover_preview.kind = SelectionKind::Region;
        m_hover_preview.region_min_x = std::min(m_selection.region_min_x, tile_x);
        m_hover_preview.region_min_y = std::min(m_selection.region_min_y, tile_y);
        m_hover_preview.region_max_x = std::max(m_selection.region_max_x, tile_x);
        m_hover_preview.region_max_y = std::max(m_selection.region_max_y, tile_y);
        break;
    case SelectionKind::Tile:
    case SelectionKind::None:
        m_hover_preview.kind = SelectionKind::Tile;
        m_hover_preview.tile_x = tile_x;
        m_hover_preview.tile_y = tile_y;
        break;
    }

    captureRenderSnapshot();
    return true;
}

bool SpatialAbilityCanvasPanel::ClickAtScreen(float screen_x, float screen_y) {
    if (m_target_overlay == nullptr || m_binding_panel == nullptr) {
        return false;
    }

    int tile_x = -1;
    int tile_y = -1;
    if (!ProjectToTile(m_target_overlay, m_projection_settings, screen_x, screen_y, tile_x, tile_y)) {
        return false;
    }
    float world_x = 0.0f;
    float world_y = 0.0f;
    float world_z = 0.0f;
    const bool projected = PropPlacementPanel::TryProjectScreenToGround(
        *m_target_overlay,
        screen_x,
        screen_y,
        m_projection_settings,
        world_x,
        world_y,
        world_z);
    if (!projected) {
        return false;
    }
    const auto binding_snapshot = m_binding_panel->lastRenderSnapshot();

    if (const auto nearest_prop_index = FindNearestPropIndex(*m_target_overlay, world_x, world_z, 1.0f);
        nearest_prop_index.has_value()) {
        m_binding_panel->SelectPropHandle(*nearest_prop_index);
        const auto& prop = m_target_overlay->props[*nearest_prop_index];
        m_selection.kind = SelectionKind::Prop;
        m_selection.prop_asset_id = prop.assetId;
        m_selection.tile_x = static_cast<int>(std::floor(prop.posX));
        m_selection.tile_y = static_cast<int>(std::floor(prop.posZ));
        m_selection.source = "binding";
        m_selection.trigger_id = binding_snapshot.selected_trigger_id;
        m_selection.ability_id.clear();
        captureRenderSnapshot();
        return true;
    }

    for (const auto& tile : binding_snapshot.tile_overlays) {
        if (tile.tile_x == tile_x && tile.tile_y == tile_y) {
            m_binding_panel->SetPlacementTile(tile_x, tile_y);
            m_selection.kind = SelectionKind::Tile;
            m_selection.tile_x = tile_x;
            m_selection.tile_y = tile_y;
            m_selection.source = tile.source;
            m_selection.trigger_id = tile.trigger_id;
            m_selection.ability_id = tile.ability_id;
            captureRenderSnapshot();
            return true;
        }
    }

    const MapAbilityBindingPanel::RegionOverlayEntry* best_region = nullptr;
    int best_area = std::numeric_limits<int>::max();
    for (const auto& region : binding_snapshot.region_overlays) {
        if (!RegionContains(region, tile_x, tile_y)) {
            continue;
        }
        const int area = RegionArea(region);
        if (area < best_area) {
            best_area = area;
            best_region = &region;
        }
    }

    if (best_region != nullptr) {
        m_binding_panel->SetActiveRegionBounds(
            best_region->min_x,
            best_region->min_y,
            best_region->max_x,
            best_region->max_y);
        m_selection.kind = SelectionKind::Region;
        m_selection.region_min_x = best_region->min_x;
        m_selection.region_min_y = best_region->min_y;
        m_selection.region_max_x = best_region->max_x;
        m_selection.region_max_y = best_region->max_y;
        m_selection.source = best_region->source;
        m_selection.trigger_id = best_region->trigger_id;
        m_selection.ability_id = best_region->ability_id;
        captureRenderSnapshot();
        return true;
    }

    m_binding_panel->SetPlacementTile(tile_x, tile_y);
    m_selection.kind = SelectionKind::Tile;
    m_selection.tile_x = tile_x;
    m_selection.tile_y = tile_y;
    m_selection.prop_asset_id.clear();
    m_selection.region_min_x = -1;
    m_selection.region_min_y = -1;
    m_selection.region_max_x = -1;
    m_selection.region_max_y = -1;
    m_selection.source = "canvas";
    m_selection.trigger_id = binding_snapshot.selected_trigger_id;
    m_selection.ability_id.clear();
    captureRenderSnapshot();
    return true;
}

bool SpatialAbilityCanvasPanel::DragSelectionToScreen(float screen_x, float screen_y) {
    if (m_target_overlay == nullptr || m_binding_panel == nullptr) {
        return false;
    }

    int tile_x = -1;
    int tile_y = -1;
    if (!ProjectToTile(m_target_overlay, m_projection_settings, screen_x, screen_y, tile_x, tile_y)) {
        return false;
    }

    bool moved = false;
    switch (m_selection.kind) {
    case SelectionKind::Tile:
        moved = m_binding_panel->MoveTileBinding(m_selection.tile_x, m_selection.tile_y, tile_x, tile_y);
        if (moved) {
            m_selection.tile_x = tile_x;
            m_selection.tile_y = tile_y;
        }
        break;
    case SelectionKind::Prop: {
        if (m_binding_panel->SelectPropFromScreen(screen_x, screen_y, m_projection_settings, 1.0f)) {
            const auto snapshot = m_binding_panel->lastRenderSnapshot();
            std::string next_prop_asset_id;
            int next_tile_x = tile_x;
            int next_tile_y = tile_y;
            for (const auto& handle : snapshot.prop_handles) {
                if (handle.selected) {
                    next_prop_asset_id = handle.asset_id;
                    next_tile_x = handle.tile_x;
                    next_tile_y = handle.tile_y;
                    break;
                }
            }
            moved = m_binding_panel->MovePropBinding(m_selection.prop_asset_id, next_prop_asset_id);
            if (moved) {
                m_selection.prop_asset_id = next_prop_asset_id;
                m_selection.tile_x = next_tile_x;
                m_selection.tile_y = next_tile_y;
            }
        }
        break;
    }
    case SelectionKind::Region:
        moved = false;
        break;
    case SelectionKind::None:
        moved = false;
        break;
    }

    captureRenderSnapshot();
    return moved;
}

bool SpatialAbilityCanvasPanel::ResizeSelectedRegionToScreen(float screen_x, float screen_y) {
    if (m_selection.kind != SelectionKind::Region || m_target_overlay == nullptr || m_binding_panel == nullptr) {
        return false;
    }

    int tile_x = -1;
    int tile_y = -1;
    if (!ProjectToTile(m_target_overlay, m_projection_settings, screen_x, screen_y, tile_x, tile_y)) {
        return false;
    }

    const bool resized = m_binding_panel->ResizeRegionBinding(
        m_selection.region_min_x,
        m_selection.region_min_y,
        m_selection.region_max_x,
        m_selection.region_max_y,
        m_selection.region_min_x,
        m_selection.region_min_y,
        tile_x,
        tile_y);
    if (resized) {
        m_selection.region_min_x = std::min(m_selection.region_min_x, tile_x);
        m_selection.region_min_y = std::min(m_selection.region_min_y, tile_y);
        m_selection.region_max_x = std::max(m_selection.region_max_x, tile_x);
        m_selection.region_max_y = std::max(m_selection.region_max_y, tile_y);
    }
    captureRenderSnapshot();
    return resized;
}

bool SpatialAbilityCanvasPanel::SetSelectionTriggerId(const std::string& trigger_id) {
    if (m_binding_panel == nullptr || trigger_id.empty() || m_selection.kind == SelectionKind::None ||
        m_selection.trigger_id == trigger_id) {
        return false;
    }

    bool switched = false;
    switch (m_selection.kind) {
    case SelectionKind::Tile:
        switched = m_binding_panel->SwitchTileBindingTrigger(
            m_selection.tile_x,
            m_selection.tile_y,
            m_selection.trigger_id,
            trigger_id);
        break;
    case SelectionKind::Prop:
        switched = m_binding_panel->SwitchPropBindingTrigger(
            m_selection.prop_asset_id,
            m_selection.trigger_id,
            trigger_id);
        break;
    case SelectionKind::Region:
        switched = m_binding_panel->SwitchRegionBindingTrigger(
            m_selection.region_min_x,
            m_selection.region_min_y,
            m_selection.region_max_x,
            m_selection.region_max_y,
            m_selection.trigger_id,
            trigger_id);
        break;
    case SelectionKind::None:
        switched = false;
        break;
    }

    if (switched) {
        m_selection.trigger_id = trigger_id;
        if (m_hover_preview.active) {
            m_hover_preview.trigger_id = trigger_id;
        }
    }
    captureRenderSnapshot();
    return switched;
}

bool SpatialAbilityCanvasPanel::ResolveConflictByRemovingSecondary(size_t index) {
    if (m_binding_panel == nullptr || index >= last_render_snapshot_.conflicts.size()) {
        return false;
    }

    const auto& conflict = last_render_snapshot_.conflicts[index];
    bool removed = false;
    if (conflict.other_kind == "tile") {
        removed = m_binding_panel->RemoveTileBinding(
            conflict.other_tile_x,
            conflict.other_tile_y,
            conflict.other_trigger_id);
    } else if (conflict.other_kind == "prop") {
        removed = m_binding_panel->RemovePropBinding(
            conflict.other_prop_asset_id,
            conflict.other_trigger_id);
    } else if (conflict.other_kind == "region") {
        removed = m_binding_panel->RemoveRegionBinding(
            conflict.other_region_min_x,
            conflict.other_region_min_y,
            conflict.other_region_max_x,
            conflict.other_region_max_y,
            conflict.other_trigger_id);
    }

    captureRenderSnapshot();
    return removed;
}

bool SpatialAbilityCanvasPanel::ResolveConflictByRemovingPrimary(size_t index) {
    if (m_binding_panel == nullptr || index >= last_render_snapshot_.conflicts.size()) {
        return false;
    }

    const auto& conflict = last_render_snapshot_.conflicts[index];
    bool removed = false;
    if (conflict.primary_kind == "tile") {
        removed = m_binding_panel->RemoveTileBinding(conflict.tile_x, conflict.tile_y, conflict.primary_trigger_id);
    } else if (conflict.primary_kind == "prop") {
        removed = m_binding_panel->RemovePropBinding(conflict.prop_asset_id, conflict.primary_trigger_id);
    } else if (conflict.primary_kind == "region") {
        removed = m_binding_panel->RemoveRegionBinding(
            conflict.region_min_x,
            conflict.region_min_y,
            conflict.region_max_x,
            conflict.region_max_y,
            conflict.primary_trigger_id);
    }

    captureRenderSnapshot();
    return removed;
}

bool SpatialAbilityCanvasPanel::SwapConflictTriggers(size_t index) {
    if (m_binding_panel == nullptr || index >= last_render_snapshot_.conflicts.size()) {
        return false;
    }

    const auto& conflict = last_render_snapshot_.conflicts[index];
    if (!conflict.can_swap_triggers) {
        return false;
    }

    bool swapped = false;
    if (conflict.primary_kind == "tile" && conflict.secondary_kind == "tile") {
        swapped = m_binding_panel->SwapTileBindingTriggers(
            conflict.tile_x,
            conflict.tile_y,
            conflict.primary_trigger_id,
            conflict.secondary_trigger_id);
    } else if (conflict.primary_kind == "prop" && conflict.secondary_kind == "prop") {
        swapped = m_binding_panel->SwapPropBindingTriggers(
            conflict.prop_asset_id,
            conflict.primary_trigger_id,
            conflict.secondary_trigger_id);
    } else if (conflict.primary_kind == "region" && conflict.secondary_kind == "region") {
        swapped = m_binding_panel->SwapRegionBindingTriggersBetween(
            conflict.region_min_x,
            conflict.region_min_y,
            conflict.region_max_x,
            conflict.region_max_y,
            conflict.primary_trigger_id,
            conflict.other_region_min_x,
            conflict.other_region_min_y,
            conflict.other_region_max_x,
            conflict.other_region_max_y,
            conflict.secondary_trigger_id);
    }

    captureRenderSnapshot();
    return swapped;
}

bool SpatialAbilityCanvasPanel::ReplaceSecondaryWithPrimaryAsset(size_t index) {
    if (m_binding_panel == nullptr || index >= last_render_snapshot_.conflicts.size()) {
        return false;
    }

    const auto& conflict = last_render_snapshot_.conflicts[index];
    if (!conflict.can_replace_secondary || conflict.primary_asset_path.empty()) {
        return false;
    }

    bool replaced = false;
    if (conflict.secondary_kind == "tile") {
        replaced = m_binding_panel->ReplaceTileBindingAsset(
            conflict.other_tile_x,
            conflict.other_tile_y,
            conflict.secondary_trigger_id,
            conflict.primary_asset_path);
    } else if (conflict.secondary_kind == "prop") {
        replaced = m_binding_panel->ReplacePropBindingAsset(
            conflict.other_prop_asset_id,
            conflict.secondary_trigger_id,
            conflict.primary_asset_path);
    } else if (conflict.secondary_kind == "region") {
        replaced = m_binding_panel->ReplaceRegionBindingAsset(
            conflict.other_region_min_x,
            conflict.other_region_min_y,
            conflict.other_region_max_x,
            conflict.other_region_max_y,
            conflict.secondary_trigger_id,
            conflict.primary_asset_path);
    }

    captureRenderSnapshot();
    return replaced;
}

bool SpatialAbilityCanvasPanel::ResolveConflictWithSuggestion(size_t index) {
    if (index >= last_render_snapshot_.conflicts.size()) {
        return false;
    }

    const auto& conflict = last_render_snapshot_.conflicts[index];
    if (conflict.severity == "blocking") {
        return ResolveConflictByRemovingSecondary(index);
    }
    if (conflict.can_swap_triggers && SwapConflictTriggers(index)) {
        return true;
    }
    if (conflict.can_replace_secondary && ReplaceSecondaryWithPrimaryAsset(index)) {
        return true;
    }
    return ResolveConflictByRemovingSecondary(index);
}

bool SpatialAbilityCanvasPanel::ApplySelectedAssetToSelection() {
    if (m_binding_panel == nullptr) {
        return false;
    }

    bool applied = false;
    switch (m_selection.kind) {
    case SelectionKind::Tile:
        applied = m_binding_panel->BindSelectedAbilityToPlacementTile();
        break;
    case SelectionKind::Prop:
        applied = m_binding_panel->BindSelectedAbilityToSelectedProp();
        break;
    case SelectionKind::Region:
        applied = m_binding_panel->BindSelectedAbilityToPaintedRegion();
        break;
    case SelectionKind::None:
        applied = false;
        break;
    }

    captureRenderSnapshot();
    return applied;
}

void SpatialAbilityCanvasPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.has_target_overlay = (m_target_overlay != nullptr);
    last_render_snapshot_.has_binding_panel = (m_binding_panel != nullptr);
    last_render_snapshot_.active_mode = m_active_mode;
    last_render_snapshot_.selection = m_selection;
    last_render_snapshot_.hover_preview = m_hover_preview;
    last_render_snapshot_.available_triggers = m_available_triggers;
    last_render_snapshot_.mode_badges =
        BuildModeBadges(m_active_mode, last_render_snapshot_.has_target_overlay, last_render_snapshot_.has_binding_panel);
    last_render_snapshot_.mode_badge_count = last_render_snapshot_.mode_badges.size();
    last_render_snapshot_.selection_trigger_menu =
        BuildTriggerMenu(m_selection.kind, m_available_triggers, m_selection.trigger_id);
    last_render_snapshot_.hover_affordances =
        BuildHoverAffordances(last_render_snapshot_.hover_preview.kind,
                              last_render_snapshot_.hover_preview.active,
                              m_active_mode);
    last_render_snapshot_.hover_affordance_count = last_render_snapshot_.hover_affordances.size();

    if (m_binding_panel == nullptr) {
        return;
    }

    const auto& binding_snapshot = m_binding_panel->lastRenderSnapshot();
    last_render_snapshot_.selected_ability_id = binding_snapshot.selected_ability_id;
    last_render_snapshot_.selected_trigger_id = binding_snapshot.selected_trigger_id;
    last_render_snapshot_.prop_handles = binding_snapshot.prop_handles;
    last_render_snapshot_.tile_overlays = binding_snapshot.tile_overlays;
    last_render_snapshot_.region_overlays = binding_snapshot.region_overlays;
    last_render_snapshot_.prop_handle_count = binding_snapshot.prop_handle_count;
    last_render_snapshot_.tile_overlay_count = binding_snapshot.tile_overlay_count;
    last_render_snapshot_.region_overlay_count = binding_snapshot.region_overlay_count;

    for (const auto& tile : last_render_snapshot_.tile_overlays) {
        InlineBadge badge;
        badge.label = tile.ability_id + " @" + tile.trigger_id;
        badge.kind = "tile";
        badge.trigger_id = tile.trigger_id;
        badge.ability_id = tile.ability_id;
        badge.tile_x = tile.tile_x;
        badge.tile_y = tile.tile_y;
        badge.selected = tile.selected;
        last_render_snapshot_.badges.push_back(std::move(badge));
    }
    for (const auto& handle : last_render_snapshot_.prop_handles) {
        if (!handle.has_binding) {
            continue;
        }
        for (const auto& binding : binding_snapshot.bindings) {
            if (binding.scope == "prop" && binding.prop_asset_id == handle.asset_id) {
                InlineBadge badge;
                badge.label = binding.ability_id + " @" + binding.trigger_id;
                badge.kind = "prop";
                badge.trigger_id = binding.trigger_id;
                badge.ability_id = binding.ability_id;
                badge.tile_x = handle.tile_x;
                badge.tile_y = handle.tile_y;
                badge.prop_asset_id = handle.asset_id;
                badge.selected = handle.selected;
                last_render_snapshot_.badges.push_back(std::move(badge));
            }
        }
    }
    for (const auto& region : last_render_snapshot_.region_overlays) {
        InlineBadge badge;
        badge.label = region.ability_id + " @" + region.trigger_id;
        badge.kind = "region";
        badge.trigger_id = region.trigger_id;
        badge.ability_id = region.ability_id;
        badge.region_min_x = region.min_x;
        badge.region_min_y = region.min_y;
        badge.region_max_x = region.max_x;
        badge.region_max_y = region.max_y;
        badge.selected = region.selected;
        last_render_snapshot_.badges.push_back(std::move(badge));
    }
    last_render_snapshot_.badge_count = last_render_snapshot_.badges.size();

    for (size_t i = 0; i < binding_snapshot.bindings.size(); ++i) {
        const auto& lhs = binding_snapshot.bindings[i];
        for (size_t j = i + 1; j < binding_snapshot.bindings.size(); ++j) {
            const auto& rhs = binding_snapshot.bindings[j];
            if (lhs.scope == "tile" && rhs.scope == "tile" &&
                lhs.tile_x == rhs.tile_x && lhs.tile_y == rhs.tile_y) {
                ConflictWarning warning;
                const bool same_trigger = lhs.trigger_id == rhs.trigger_id;
                warning.kind = same_trigger ? "tile_overlap" : "tile_multi_trigger";
                warning.message = same_trigger
                                      ? "Multiple tile bindings compete on the same tile and trigger."
                                      : "Multiple tile bindings share a tile across different triggers.";
                warning.severity = same_trigger ? "blocking" : "warning";
                warning.policy = same_trigger ? "exclusive_target" : "shared_target";
                warning.primary_kind = "tile";
                warning.primary_trigger_id = lhs.trigger_id;
                warning.primary_ability_id = lhs.ability_id;
                warning.primary_asset_path = lhs.asset_path;
                warning.secondary_kind = "tile";
                warning.secondary_trigger_id = rhs.trigger_id;
                warning.secondary_ability_id = rhs.ability_id;
                warning.secondary_asset_path = rhs.asset_path;
                warning.can_keep_primary = true;
                warning.can_keep_secondary = true;
                warning.can_swap_triggers = !same_trigger;
                warning.can_replace_secondary = true;
                warning.trigger_id = lhs.trigger_id + "|" + rhs.trigger_id;
                warning.ability_id = lhs.ability_id + "|" + rhs.ability_id;
                warning.tile_x = lhs.tile_x;
                warning.tile_y = lhs.tile_y;
                warning.other_kind = "tile";
                warning.other_trigger_id = rhs.trigger_id;
                warning.other_ability_id = rhs.ability_id;
                warning.other_tile_x = rhs.tile_x;
                warning.other_tile_y = rhs.tile_y;
                last_render_snapshot_.conflicts.push_back(std::move(warning));
            }
            if (lhs.scope == "prop" && rhs.scope == "prop" &&
                lhs.prop_asset_id == rhs.prop_asset_id) {
                ConflictWarning warning;
                const bool same_trigger = lhs.trigger_id == rhs.trigger_id;
                warning.kind = same_trigger ? "prop_overlap" : "prop_multi_trigger";
                warning.message = same_trigger
                                      ? "Multiple prop bindings compete on the same prop and trigger."
                                      : "Multiple prop bindings share a prop across different triggers.";
                warning.severity = same_trigger ? "blocking" : "warning";
                warning.policy = same_trigger ? "exclusive_target" : "shared_target";
                warning.primary_kind = "prop";
                warning.primary_trigger_id = lhs.trigger_id;
                warning.primary_ability_id = lhs.ability_id;
                warning.primary_asset_path = lhs.asset_path;
                warning.secondary_kind = "prop";
                warning.secondary_trigger_id = rhs.trigger_id;
                warning.secondary_ability_id = rhs.ability_id;
                warning.secondary_asset_path = rhs.asset_path;
                warning.can_keep_primary = true;
                warning.can_keep_secondary = true;
                warning.can_swap_triggers = !same_trigger;
                warning.can_replace_secondary = true;
                warning.trigger_id = lhs.trigger_id + "|" + rhs.trigger_id;
                warning.ability_id = lhs.ability_id + "|" + rhs.ability_id;
                warning.prop_asset_id = lhs.prop_asset_id;
                warning.other_kind = "prop";
                warning.other_trigger_id = rhs.trigger_id;
                warning.other_ability_id = rhs.ability_id;
                warning.other_prop_asset_id = rhs.prop_asset_id;
                last_render_snapshot_.conflicts.push_back(std::move(warning));
            }
            if (lhs.scope == "region" && rhs.scope == "region") {
                MapAbilityBindingPanel::RegionOverlayEntry lhs_region{lhs.region_min_x, lhs.region_min_y, lhs.region_max_x,
                                                                     lhs.region_max_y, lhs.trigger_id, lhs.ability_id,
                                                                     "binding", false, false};
                MapAbilityBindingPanel::RegionOverlayEntry rhs_region{rhs.region_min_x, rhs.region_min_y, rhs.region_max_x,
                                                                     rhs.region_max_y, rhs.trigger_id, rhs.ability_id,
                                                                     "binding", false, false};
                if (RegionsOverlap(lhs_region, rhs_region)) {
                    ConflictWarning warning;
                    const bool same_trigger = lhs.trigger_id == rhs.trigger_id;
                    warning.kind = same_trigger ? "region_overlap" : "region_layered";
                    warning.message = same_trigger
                                          ? "Multiple region bindings overlap on the same trigger."
                                          : "Multiple region bindings overlap across different triggers.";
                    warning.severity = same_trigger ? "blocking" : "warning";
                    warning.policy = same_trigger ? "exclusive_target" : "layered_region";
                    warning.primary_kind = "region";
                    warning.primary_trigger_id = lhs.trigger_id;
                    warning.primary_ability_id = lhs.ability_id;
                    warning.primary_asset_path = lhs.asset_path;
                    warning.secondary_kind = "region";
                    warning.secondary_trigger_id = rhs.trigger_id;
                    warning.secondary_ability_id = rhs.ability_id;
                    warning.secondary_asset_path = rhs.asset_path;
                    warning.can_keep_primary = true;
                    warning.can_keep_secondary = true;
                    warning.can_swap_triggers = !same_trigger;
                    warning.can_replace_secondary = true;
                    warning.trigger_id = lhs.trigger_id + "|" + rhs.trigger_id;
                    warning.ability_id = lhs.ability_id + "|" + rhs.ability_id;
                    warning.region_min_x = lhs.region_min_x;
                    warning.region_min_y = lhs.region_min_y;
                    warning.region_max_x = lhs.region_max_x;
                    warning.region_max_y = lhs.region_max_y;
                    warning.other_kind = "region";
                    warning.other_trigger_id = rhs.trigger_id;
                    warning.other_ability_id = rhs.ability_id;
                    warning.other_region_min_x = rhs.region_min_x;
                    warning.other_region_min_y = rhs.region_min_y;
                    warning.other_region_max_x = rhs.region_max_x;
                    warning.other_region_max_y = rhs.region_max_y;
                    last_render_snapshot_.conflicts.push_back(std::move(warning));
                }
            }
        }
    }

    auto preview_conflicts = [&](const HoverPreview& preview) {
        if (!preview.active) {
            return false;
        }
        for (const auto& binding : binding_snapshot.bindings) {
            if (preview.kind == SelectionKind::Tile &&
                binding.scope == "tile" &&
                binding.tile_x == preview.tile_x &&
                binding.tile_y == preview.tile_y &&
                binding.trigger_id == preview.trigger_id &&
                !(m_selection.kind == SelectionKind::Tile &&
                  binding.tile_x == m_selection.tile_x &&
                  binding.tile_y == m_selection.tile_y &&
                  binding.trigger_id == m_selection.trigger_id)) {
                return true;
            }
            if (preview.kind == SelectionKind::Prop &&
                binding.scope == "prop" &&
                binding.prop_asset_id == preview.prop_asset_id &&
                binding.trigger_id == preview.trigger_id &&
                !(m_selection.kind == SelectionKind::Prop &&
                  binding.prop_asset_id == m_selection.prop_asset_id &&
                  binding.trigger_id == m_selection.trigger_id)) {
                return true;
            }
            if (preview.kind == SelectionKind::Region && binding.scope == "region") {
                MapAbilityBindingPanel::RegionOverlayEntry preview_region{preview.region_min_x,
                                                                         preview.region_min_y,
                                                                         preview.region_max_x,
                                                                         preview.region_max_y,
                                                                         preview.trigger_id,
                                                                         preview.ability_id,
                                                                         "hover",
                                                                         false,
                                                                         true};
                MapAbilityBindingPanel::RegionOverlayEntry bound_region{binding.region_min_x,
                                                                       binding.region_min_y,
                                                                       binding.region_max_x,
                                                                       binding.region_max_y,
                                                                       binding.trigger_id,
                                                                       binding.ability_id,
                                                                       "binding",
                                                                       false,
                                                                       false};
                if (RegionsOverlap(preview_region, bound_region) &&
                    binding.trigger_id == preview.trigger_id &&
                    !(m_selection.kind == SelectionKind::Region &&
                      binding.region_min_x == m_selection.region_min_x &&
                      binding.region_min_y == m_selection.region_min_y &&
                      binding.region_max_x == m_selection.region_max_x &&
                      binding.region_max_y == m_selection.region_max_y &&
                      binding.trigger_id == m_selection.trigger_id)) {
                    return true;
                }
            }
        }
        return false;
    };
    last_render_snapshot_.hover_preview.would_conflict = preview_conflicts(last_render_snapshot_.hover_preview);
    last_render_snapshot_.conflict_count = last_render_snapshot_.conflicts.size();
    last_render_snapshot_.has_conflicts = !last_render_snapshot_.conflicts.empty();
    last_render_snapshot_.conflict_action_chips = BuildConflictActionChips(last_render_snapshot_.conflicts);
    last_render_snapshot_.conflict_action_chip_count = last_render_snapshot_.conflict_action_chips.size();
    last_render_snapshot_.selection_trigger_menu =
        BuildTriggerMenu(m_selection.kind, m_available_triggers, m_selection.trigger_id);

    switch (m_selection.kind) {
    case SelectionKind::Tile:
        for (auto& tile : last_render_snapshot_.tile_overlays) {
            tile.selected = (tile.tile_x == m_selection.tile_x && tile.tile_y == m_selection.tile_y);
        }
        break;
    case SelectionKind::Prop:
        for (auto& handle : last_render_snapshot_.prop_handles) {
            handle.selected = (handle.asset_id == m_selection.prop_asset_id);
        }
        break;
    case SelectionKind::Region:
        for (auto& region : last_render_snapshot_.region_overlays) {
            region.selected =
                region.min_x == m_selection.region_min_x &&
                region.min_y == m_selection.region_min_y &&
                region.max_x == m_selection.region_max_x &&
                region.max_y == m_selection.region_max_y;
        }
        break;
    case SelectionKind::None:
        break;
    }
}

} // namespace urpg::editor
