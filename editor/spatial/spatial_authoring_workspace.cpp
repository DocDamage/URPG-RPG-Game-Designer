#include "editor/spatial/spatial_authoring_workspace.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <nlohmann/json.hpp>

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
    case ToolMode::Worldbuilding:
        return "worldbuilding";
    case ToolMode::Tiles:
        return "tiles";
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
    if (active_mode_ == ToolMode::Composite || active_mode_ == ToolMode::Worldbuilding) {
        terrain_brush_panel_.render();
        region_rules_panel_.render();
        procedural_map_panel_.render();
        environment_preview_panel_.render();
    }
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

void SpatialAuthoringWorkspace::PreviewTerrainBrush(const urpg::map::TerrainBrush& brush,
                                                    int32_t x,
                                                    int32_t y,
                                                    uint32_t seed) {
    terrain_brush_panel_.preview(brush, x, y, seed);
    captureRenderSnapshot();
}

void SpatialAuthoringWorkspace::LoadRegionRules(std::vector<urpg::map::MapRegionRule> rules) {
    region_rules_panel_.loadRules(std::move(rules));
    captureRenderSnapshot();
}

void SpatialAuthoringWorkspace::GenerateProceduralMap(const urpg::map::ProceduralMapProfile& profile) {
    procedural_map_panel_.generate(profile);
    captureRenderSnapshot();
}

void SpatialAuthoringWorkspace::LoadEnvironmentPreview(urpg::map::MapEnvironmentPreviewDocument document) {
    environment_preview_panel_.loadDocument(std::move(document));
    captureRenderSnapshot();
}

void SpatialAuthoringWorkspace::SelectEnvironmentTile(int32_t x, int32_t y) {
    environment_preview_panel_.selectTile(x, y);
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
    if (action_id == "worldbuilding") {
        SetActiveMode(ToolMode::Worldbuilding);
        return true;
    }
    if (action_id == "tiles") {
        SetActiveMode(ToolMode::Tiles);
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
    case ToolMode::Worldbuilding: {
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
        (void)world_y;
        environment_preview_panel_.selectTile(static_cast<int32_t>(std::floor(world_x)),
                                              static_cast<int32_t>(std::floor(world_z)));
        captureRenderSnapshot();
        return true;
    }
    case ToolMode::Tiles:
        return PaintPerspectiveTileFromScreen(screen_x, screen_y);
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
    case ToolMode::Worldbuilding:
        captureRenderSnapshot();
        return false;
    case ToolMode::Tiles:
        captureRenderSnapshot();
        return false;
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
    case ToolMode::Worldbuilding:
        return RouteCanvasPrimaryAction(screen_x, screen_y);
    case ToolMode::Tiles: {
        const bool handled = canvas_panel_.HoverAtScreen(screen_x, screen_y);
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

void SpatialAuthoringWorkspace::markPerspectiveDirty() {
    perspective_has_unsaved_changes_ = true;
    perspective_playtest_ready_ = false;
}

bool SpatialAuthoringWorkspace::projectScreenToTile(float screen_x, float screen_y, int32_t& out_tile_x,
                                                    int32_t& out_tile_y) const {
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
    (void)world_y;
    out_tile_x = static_cast<int32_t>(std::floor(world_x));
    out_tile_y = static_cast<int32_t>(std::floor(world_z));
    return out_tile_x >= 0 && out_tile_y >= 0;
}

bool SpatialAuthoringWorkspace::AddPerspectiveLayer(const std::string& layer_id,
                                                    const std::string& label,
                                                    const std::string& kind) {
    if (layer_id.empty() || label.empty() || kind.empty()) {
        return false;
    }
    const auto existing =
        std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                     [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
    if (existing != perspective_layers_.end()) {
        return false;
    }

    PerspectiveLayer layer;
    layer.id = layer_id;
    layer.label = label;
    layer.kind = kind;
    layer.order = static_cast<int>(perspective_layers_.size());
    perspective_layers_.push_back(std::move(layer));
    if (selected_perspective_layer_id_.empty()) {
        selected_perspective_layer_id_ = layer_id;
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SelectPerspectiveLayer(const std::string& layer_id) {
    const auto existing =
        std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                     [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
    if (existing == perspective_layers_.end()) {
        return false;
    }
    selected_perspective_layer_id_ = layer_id;
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SetPerspectiveLayerVisible(const std::string& layer_id, bool visible) {
    auto existing = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                 [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
    if (existing == perspective_layers_.end()) {
        return false;
    }
    existing->visible = visible;
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SetPerspectiveLayerLocked(const std::string& layer_id, bool locked) {
    auto existing = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                 [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
    if (existing == perspective_layers_.end()) {
        return false;
    }
    existing->locked = locked;
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::MovePerspectiveLayer(const std::string& layer_id, int new_order) {
    auto existing = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                 [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
    if (existing == perspective_layers_.end() || new_order < 0) {
        return false;
    }
    existing->order = new_order;
    std::stable_sort(perspective_layers_.begin(), perspective_layers_.end(),
                     [](const PerspectiveLayer& lhs, const PerspectiveLayer& rhs) { return lhs.order < rhs.order; });
    for (size_t i = 0; i < perspective_layers_.size(); ++i) {
        perspective_layers_[i].order = static_cast<int>(i);
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::DeletePerspectiveLayer(const std::string& layer_id) {
    if (layer_id.empty()) {
        return false;
    }
    auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                              [&](const PerspectiveLayer& candidate) { return candidate.id == layer_id; });
    if (layer == perspective_layers_.end()) {
        return false;
    }

    perspective_layers_.erase(layer);
    perspective_tiles_.erase(std::remove_if(perspective_tiles_.begin(), perspective_tiles_.end(),
                                            [&](const PerspectiveTilePaint& tile) {
                                                return tile.layer_id == layer_id;
                                            }),
                             perspective_tiles_.end());
    perspective_events_.erase(std::remove_if(perspective_events_.begin(), perspective_events_.end(),
                                             [&](const PerspectiveEvent& event) {
                                                 return event.layer_id == layer_id;
                                             }),
                              perspective_events_.end());
    for (size_t i = 0; i < perspective_layers_.size(); ++i) {
        perspective_layers_[i].order = static_cast<int>(i);
    }
    if (selected_perspective_layer_id_ == layer_id) {
        selected_perspective_layer_id_ = perspective_layers_.empty() ? std::string{} : perspective_layers_.front().id;
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::DuplicatePerspectiveLayer(const std::string& source_layer_id,
                                                          const std::string& new_layer_id,
                                                          const std::string& new_label) {
    if (source_layer_id.empty() || new_layer_id.empty() || new_label.empty()) {
        return false;
    }
    const auto source = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                     [&](const PerspectiveLayer& layer) { return layer.id == source_layer_id; });
    const auto existing = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                       [&](const PerspectiveLayer& layer) { return layer.id == new_layer_id; });
    if (source == perspective_layers_.end() || existing != perspective_layers_.end()) {
        return false;
    }

    PerspectiveLayer copy = *source;
    copy.id = new_layer_id;
    copy.label = new_label;
    copy.locked = false;
    copy.order = static_cast<int>(perspective_layers_.size());
    perspective_layers_.push_back(std::move(copy));
    const auto original_tile_count = perspective_tiles_.size();
    for (size_t i = 0; i < original_tile_count; ++i) {
        if (perspective_tiles_[i].layer_id == source_layer_id) {
            PerspectiveTilePaint tile = perspective_tiles_[i];
            tile.layer_id = new_layer_id;
            perspective_tiles_.push_back(std::move(tile));
        }
    }
    const auto original_event_count = perspective_events_.size();
    for (size_t i = 0; i < original_event_count; ++i) {
        if (perspective_events_[i].layer_id == source_layer_id) {
            PerspectiveEvent event = perspective_events_[i];
            event.layer_id = new_layer_id;
            event.event_id += "_copy";
            perspective_events_.push_back(std::move(event));
        }
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::ClearPerspectiveLayer(const std::string& layer_id) {
    const auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                    [&](const PerspectiveLayer& candidate) { return candidate.id == layer_id; });
    if (layer == perspective_layers_.end() || layer->locked) {
        return false;
    }
    perspective_tiles_.erase(std::remove_if(perspective_tiles_.begin(), perspective_tiles_.end(),
                                            [&](const PerspectiveTilePaint& tile) {
                                                return tile.layer_id == layer_id;
                                            }),
                             perspective_tiles_.end());
    perspective_events_.erase(std::remove_if(perspective_events_.begin(), perspective_events_.end(),
                                             [&](const PerspectiveEvent& event) {
                                                 return event.layer_id == layer_id;
                                             }),
                              perspective_events_.end());
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

void SpatialAuthoringWorkspace::SetPerspectiveTilePaletteOptions(std::vector<Perspective2DPaletteOption> options) {
    perspective_tile_palette_options_ = std::move(options);
    const auto selected = std::find_if(perspective_tile_palette_options_.begin(),
                                       perspective_tile_palette_options_.end(),
                                       [&](const Perspective2DPaletteOption& option) {
                                           return option.option_id == selected_palette_option_id_;
                                       });
    if (selected == perspective_tile_palette_options_.end()) {
        selected_palette_option_id_.clear();
    }
    captureRenderSnapshot();
}

bool SpatialAuthoringWorkspace::SelectPerspectiveTilePaletteOption(const std::string& option_id) {
    const auto option = std::find_if(perspective_tile_palette_options_.begin(),
                                     perspective_tile_palette_options_.end(),
                                     [&](const Perspective2DPaletteOption& candidate) {
                                         return candidate.option_id == option_id;
                                     });
    if (option == perspective_tile_palette_options_.end() || option->tileset_id.empty() ||
        option->tile_id.empty()) {
        return false;
    }
    selected_palette_option_id_ = option->option_id;
    selected_tileset_id_ = option->tileset_id;
    selected_tile_id_ = option->tile_id;
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SelectPerspectiveTile(const std::string& tileset_id, const std::string& tile_id) {
    if (tileset_id.empty() || tile_id.empty()) {
        return false;
    }
    selected_palette_option_id_.clear();
    selected_tileset_id_ = tileset_id;
    selected_tile_id_ = tile_id;
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SetPerspectiveBrushSize(int brush_size) {
    if (brush_size < 1 || brush_size > 16) {
        return false;
    }
    perspective_brush_size_ = brush_size;
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::PaintPerspectiveTileFromScreen(float screen_x, float screen_y) {
    if (selected_perspective_layer_id_.empty() || selected_tileset_id_.empty() || selected_tile_id_.empty()) {
        return false;
    }
    auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                              [&](const PerspectiveLayer& candidate) {
                                  return candidate.id == selected_perspective_layer_id_;
                              });
    if (layer == perspective_layers_.end() || layer->kind != "tile" || layer->locked || !layer->visible) {
        return false;
    }

    int32_t tile_x = 0;
    int32_t tile_y = 0;
    if (!projectScreenToTile(screen_x, screen_y, tile_x, tile_y)) {
        return false;
    }

    for (int offset_y = 0; offset_y < perspective_brush_size_; ++offset_y) {
        for (int offset_x = 0; offset_x < perspective_brush_size_; ++offset_x) {
            const int32_t painted_x = tile_x + offset_x;
            const int32_t painted_y = tile_y + offset_y;
            if (painted_x < 0 || painted_y < 0 ||
                painted_x >= static_cast<int32_t>(m_target_overlay->elevation.width) ||
                painted_y >= static_cast<int32_t>(m_target_overlay->elevation.height)) {
                continue;
            }
            auto existing = std::find_if(perspective_tiles_.begin(), perspective_tiles_.end(),
                                         [&](const PerspectiveTilePaint& paint) {
                                             return paint.layer_id == selected_perspective_layer_id_ &&
                                                    paint.tile_x == painted_x && paint.tile_y == painted_y;
                                         });
            if (existing == perspective_tiles_.end()) {
                perspective_tiles_.push_back({selected_perspective_layer_id_, selected_tileset_id_, selected_tile_id_,
                                              painted_x, painted_y});
            } else {
                existing->tileset_id = selected_tileset_id_;
                existing->tile_id = selected_tile_id_;
            }
        }
    }

    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::ErasePerspectiveTileFromScreen(float screen_x, float screen_y) {
    if (selected_perspective_layer_id_.empty()) {
        return false;
    }
    const auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                    [&](const PerspectiveLayer& candidate) {
                                        return candidate.id == selected_perspective_layer_id_;
                                    });
    if (layer == perspective_layers_.end() || layer->kind != "tile" || layer->locked || !layer->visible) {
        return false;
    }

    int32_t tile_x = 0;
    int32_t tile_y = 0;
    if (!projectScreenToTile(screen_x, screen_y, tile_x, tile_y)) {
        return false;
    }

    const auto old_size = perspective_tiles_.size();
    perspective_tiles_.erase(std::remove_if(perspective_tiles_.begin(), perspective_tiles_.end(),
                                            [&](const PerspectiveTilePaint& paint) {
                                                if (paint.layer_id != selected_perspective_layer_id_) {
                                                    return false;
                                                }
                                                return paint.tile_x >= tile_x &&
                                                       paint.tile_x < tile_x + perspective_brush_size_ &&
                                                       paint.tile_y >= tile_y &&
                                                       paint.tile_y < tile_y + perspective_brush_size_;
                                            }),
                             perspective_tiles_.end());
    if (perspective_tiles_.size() == old_size) {
        return false;
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::AddPerspectiveEventFromScreen(const std::string& event_id,
                                                              const std::string& label,
                                                              const std::string& trigger_id,
                                                              float screen_x,
                                                              float screen_y) {
    if (event_id.empty() || label.empty() || trigger_id.empty() || selected_perspective_layer_id_.empty()) {
        return false;
    }
    auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                              [&](const PerspectiveLayer& candidate) {
                                  return candidate.id == selected_perspective_layer_id_;
                              });
    if (layer == perspective_layers_.end() || (layer->kind != "event" && layer->kind != "object")) {
        return false;
    }

    int32_t tile_x = 0;
    int32_t tile_y = 0;
    if (!projectScreenToTile(screen_x, screen_y, tile_x, tile_y)) {
        return false;
    }

    auto existing = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                                 [&](const PerspectiveEvent& event) { return event.event_id == event_id; });
    if (existing == perspective_events_.end()) {
        PerspectiveEvent event;
        event.event_id = event_id;
        event.label = label;
        event.trigger_id = trigger_id;
        event.layer_id = selected_perspective_layer_id_;
        event.tile_x = tile_x;
        event.tile_y = tile_y;
        perspective_events_.push_back(std::move(event));
    } else {
        existing->label = label;
        existing->trigger_id = trigger_id;
        existing->layer_id = selected_perspective_layer_id_;
        existing->tile_x = tile_x;
        existing->tile_y = tile_y;
    }

    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::MovePerspectiveEventFromScreen(const std::string& event_id,
                                                               float screen_x,
                                                               float screen_y) {
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    const auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                    [&](const PerspectiveLayer& candidate) {
                                        return candidate.id == event->layer_id;
                                    });
    if (layer == perspective_layers_.end() || layer->locked || !layer->visible) {
        return false;
    }

    int32_t tile_x = 0;
    int32_t tile_y = 0;
    if (!projectScreenToTile(screen_x, screen_y, tile_x, tile_y)) {
        return false;
    }
    event->tile_x = tile_x;
    event->tile_y = tile_y;
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::AddPerspectiveEventCommand(const std::string& event_id,
                                                           const std::string& command_code,
                                                           const std::string& argument) {
    if (event_id.empty() || command_code.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    event->commands.push_back({command_code, argument});
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::UpdatePerspectiveEventCommand(const std::string& event_id,
                                                              size_t command_index,
                                                              const std::string& command_code,
                                                              const std::string& argument) {
    if (event_id.empty() || command_code.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end() || command_index >= event->commands.size()) {
        return false;
    }
    event->commands[command_index] = {command_code, argument};
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::RemovePerspectiveEventCommand(const std::string& event_id, size_t command_index) {
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end() || command_index >= event->commands.size()) {
        return false;
    }
    event->commands.erase(event->commands.begin() + static_cast<std::ptrdiff_t>(command_index));
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

std::string SpatialAuthoringWorkspace::serializePerspectiveMapDraft() const {
    nlohmann::json json;
    json["document_kind"] = "urpg.perspective_2d.map";
    json["version"] = 1;
    json["map_id"] = m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    json["width"] = m_target_overlay != nullptr ? m_target_overlay->elevation.width : 0;
    json["height"] = m_target_overlay != nullptr ? m_target_overlay->elevation.height : 0;
    json["selected_layer_id"] = selected_perspective_layer_id_;
    json["selected_palette_option_id"] = selected_palette_option_id_;
    json["selected_tileset_id"] = selected_tileset_id_;
    json["selected_tile_id"] = selected_tile_id_;
    json["layers"] = nlohmann::json::array();
    json["tiles"] = nlohmann::json::array();
    json["events"] = nlohmann::json::array();

    for (const auto& layer : perspective_layers_) {
        json["layers"].push_back({{"id", layer.id},
                                  {"label", layer.label},
                                  {"kind", layer.kind},
                                  {"visible", layer.visible},
                                  {"locked", layer.locked},
                                  {"order", layer.order}});
    }
    for (const auto& tile : perspective_tiles_) {
        json["tiles"].push_back({{"layer_id", tile.layer_id},
                                 {"tileset_id", tile.tileset_id},
                                 {"tile_id", tile.tile_id},
                                 {"x", tile.tile_x},
                                 {"y", tile.tile_y}});
    }
    for (const auto& event : perspective_events_) {
        nlohmann::json commands = nlohmann::json::array();
        for (const auto& command : event.commands) {
            commands.push_back({{"code", command.code}, {"argument", command.argument}});
        }
        json["events"].push_back({{"event_id", event.event_id},
                                  {"label", event.label},
                                  {"trigger_id", event.trigger_id},
                                  {"layer_id", event.layer_id},
                                  {"x", event.tile_x},
                                  {"y", event.tile_y},
                                  {"commands", std::move(commands)}});
    }

    return json.dump(2);
}

SpatialAuthoringWorkspace::Perspective2DDraftResult SpatialAuthoringWorkspace::SavePerspectiveMapDraft() {
    Perspective2DDraftResult result;
    result.command_id = "save_perspective_2d_map_draft";
    result.map_id = m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    result.layer_count = perspective_layers_.size();
    result.painted_tile_count = perspective_tiles_.size();
    result.event_count = perspective_events_.size();
    if (m_target_overlay == nullptr) {
        result.message = "Bind a SpatialMapOverlay before saving the Perspective 2D draft.";
        result.blocker_codes.push_back("p2d_overlay_unbound");
        last_perspective_save_result_ = result;
        captureRenderSnapshot();
        return last_perspective_save_result_;
    }

    result.success = true;
    result.message = "Perspective 2D map draft saved.";
    result.serialized_document_json = serializePerspectiveMapDraft();
    perspective_has_unsaved_changes_ = false;
    last_perspective_save_result_ = result;
    captureRenderSnapshot();
    return last_perspective_save_result_;
}

SpatialAuthoringWorkspace::Perspective2DDraftResult
SpatialAuthoringWorkspace::LoadPerspectiveMapDraft(const std::string& serialized_document_json) {
    Perspective2DDraftResult result;
    result.command_id = "load_perspective_2d_map_draft";

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(serialized_document_json);
    } catch (const nlohmann::json::exception&) {
        result.message = "Perspective 2D map draft JSON could not be parsed.";
        result.blocker_codes.push_back("p2d_draft_json_parse_failed");
        last_perspective_load_result_ = result;
        captureRenderSnapshot();
        return last_perspective_load_result_;
    }

    if (json.value("document_kind", "") != "urpg.perspective_2d.map") {
        result.message = "Perspective 2D map draft JSON has the wrong document kind.";
        result.blocker_codes.push_back("p2d_draft_kind_invalid");
        last_perspective_load_result_ = result;
        captureRenderSnapshot();
        return last_perspective_load_result_;
    }

    perspective_layers_.clear();
    perspective_tiles_.clear();
    perspective_events_.clear();
    selected_perspective_layer_id_ = json.value("selected_layer_id", "");
    selected_palette_option_id_ = json.value("selected_palette_option_id", "");
    selected_tileset_id_ = json.value("selected_tileset_id", "");
    selected_tile_id_ = json.value("selected_tile_id", "");

    for (const auto& layer_json : json.value("layers", nlohmann::json::array())) {
        PerspectiveLayer layer;
        layer.id = layer_json.value("id", "");
        layer.label = layer_json.value("label", layer.id);
        layer.kind = layer_json.value("kind", "tile");
        layer.visible = layer_json.value("visible", true);
        layer.locked = layer_json.value("locked", false);
        layer.order = layer_json.value("order", static_cast<int>(perspective_layers_.size()));
        if (!layer.id.empty()) {
            perspective_layers_.push_back(std::move(layer));
        }
    }
    std::stable_sort(perspective_layers_.begin(), perspective_layers_.end(),
                     [](const PerspectiveLayer& lhs, const PerspectiveLayer& rhs) { return lhs.order < rhs.order; });
    for (size_t i = 0; i < perspective_layers_.size(); ++i) {
        perspective_layers_[i].order = static_cast<int>(i);
    }
    if (selected_perspective_layer_id_.empty() && !perspective_layers_.empty()) {
        selected_perspective_layer_id_ = perspective_layers_.front().id;
    }

    for (const auto& tile_json : json.value("tiles", nlohmann::json::array())) {
        PerspectiveTilePaint tile;
        tile.layer_id = tile_json.value("layer_id", "");
        tile.tileset_id = tile_json.value("tileset_id", "");
        tile.tile_id = tile_json.value("tile_id", "");
        tile.tile_x = tile_json.value("x", 0);
        tile.tile_y = tile_json.value("y", 0);
        if (!tile.layer_id.empty() && !tile.tileset_id.empty() && !tile.tile_id.empty()) {
            perspective_tiles_.push_back(std::move(tile));
        }
    }
    for (const auto& event_json : json.value("events", nlohmann::json::array())) {
        PerspectiveEvent event;
        event.event_id = event_json.value("event_id", "");
        event.label = event_json.value("label", event.event_id);
        event.trigger_id = event_json.value("trigger_id", "confirm_interact");
        event.layer_id = event_json.value("layer_id", "");
        event.tile_x = event_json.value("x", 0);
        event.tile_y = event_json.value("y", 0);
        for (const auto& command_json : event_json.value("commands", nlohmann::json::array())) {
            PerspectiveEvent::Command command;
            command.code = command_json.value("code", "");
            command.argument = command_json.value("argument", "");
            if (!command.code.empty()) {
                event.commands.push_back(std::move(command));
            }
        }
        if (!event.event_id.empty() && !event.layer_id.empty()) {
            perspective_events_.push_back(std::move(event));
        }
    }

    result.success = true;
    result.message = "Perspective 2D map draft loaded.";
    result.map_id = json.value("map_id", "");
    result.layer_count = perspective_layers_.size();
    result.painted_tile_count = perspective_tiles_.size();
    result.event_count = perspective_events_.size();
    result.serialized_document_json = serializePerspectiveMapDraft();
    perspective_has_unsaved_changes_ = false;
    perspective_playtest_ready_ = false;
    last_perspective_load_result_ = result;
    captureRenderSnapshot();
    return last_perspective_load_result_;
}

std::vector<std::string> SpatialAuthoringWorkspace::validatePerspectiveMapForPlaytest() const {
    std::vector<std::string> blockers;
    if (m_target_scene == nullptr || m_target_overlay == nullptr) {
        blockers.push_back("p2d_targets_unbound");
        return blockers;
    }

    const auto layer_by_id = [&](const std::string& layer_id) -> const PerspectiveLayer* {
        const auto found = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                        [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
        return found == perspective_layers_.end() ? nullptr : &(*found);
    };

    const auto visible_tile_layer =
        std::find_if(perspective_layers_.begin(), perspective_layers_.end(), [](const PerspectiveLayer& layer) {
            return layer.kind == "tile" && layer.visible && !layer.locked;
        });
    if (visible_tile_layer == perspective_layers_.end()) {
        blockers.push_back("p2d_no_tile_layer");
        return blockers;
    }

    const bool has_painted_visible_tile = std::any_of(
        perspective_tiles_.begin(), perspective_tiles_.end(), [&](const PerspectiveTilePaint& tile) {
            const PerspectiveLayer* layer = layer_by_id(tile.layer_id);
            return layer != nullptr && layer->kind == "tile" && layer->visible && !layer->locked;
        });
    if (!has_painted_visible_tile) {
        blockers.push_back("p2d_no_painted_tiles");
        return blockers;
    }

    for (const auto& tile : perspective_tiles_) {
        if (tile.tile_x < 0 || tile.tile_y < 0 ||
            tile.tile_x >= static_cast<int32_t>(m_target_overlay->elevation.width) ||
            tile.tile_y >= static_cast<int32_t>(m_target_overlay->elevation.height)) {
            blockers.push_back("p2d_tile_out_of_bounds");
            return blockers;
        }
    }

    for (const auto& event : perspective_events_) {
        const PerspectiveLayer* layer = layer_by_id(event.layer_id);
        if (layer == nullptr) {
            blockers.push_back("p2d_event_layer_missing");
            return blockers;
        }
        if (layer->locked) {
            blockers.push_back("p2d_event_layer_locked");
            return blockers;
        }
        if (!layer->visible) {
            blockers.push_back("p2d_event_layer_hidden");
            return blockers;
        }
        if (event.tile_x < 0 || event.tile_y < 0 ||
            event.tile_x >= static_cast<int32_t>(m_target_overlay->elevation.width) ||
            event.tile_y >= static_cast<int32_t>(m_target_overlay->elevation.height)) {
            blockers.push_back("p2d_event_out_of_bounds");
            return blockers;
        }
    }

    return blockers;
}

SpatialAuthoringWorkspace::Perspective2DPlaytestResult SpatialAuthoringWorkspace::RunPerspectiveMapPlaytest() {
    Perspective2DPlaytestResult result;
    result.map_id = m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    result.playtest_tile_count = perspective_tiles_.size();
    result.playtest_event_count = perspective_events_.size();
    result.blocker_codes = validatePerspectiveMapForPlaytest();
    if (!result.blocker_codes.empty()) {
        result.message = "Perspective 2D map playtest readiness is blocked.";
        perspective_playtest_ready_ = false;
        last_perspective_playtest_result_ = result;
        captureRenderSnapshot();
        return last_perspective_playtest_result_;
    }

    result.success = true;
    result.message = "Perspective 2D map playtest readiness passed.";
    perspective_playtest_ready_ = true;
    last_perspective_playtest_result_ = result;
    captureRenderSnapshot();
    return last_perspective_playtest_result_;
}

SpatialAuthoringWorkspace::Perspective2DExportResult SpatialAuthoringWorkspace::ExportPerspectiveMap() {
    Perspective2DExportResult result;
    result.map_id = m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    result.exported_tile_count = perspective_tiles_.size();
    result.exported_event_count = perspective_events_.size();
    result.blocker_codes = validatePerspectiveMapForPlaytest();
    if (!last_perspective_playtest_result_.success || !perspective_playtest_ready_) {
        result.blocker_codes.insert(result.blocker_codes.begin(), "p2d_playtest_required");
    }
    if (!result.blocker_codes.empty()) {
        result.message = "Perspective 2D map is not exportable.";
        last_perspective_export_result_ = result;
        captureRenderSnapshot();
        return last_perspective_export_result_;
    }

    result.success = true;
    result.message = "Perspective 2D map is exportable.";
    result.serialized_document_json = serializePerspectiveMapDraft();
    last_perspective_export_result_ = result;
    captureRenderSnapshot();
    return last_perspective_export_result_;
}

void SpatialAuthoringWorkspace::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.has_target_scene = (m_target_scene != nullptr);
    last_render_snapshot_.has_target_overlay = (m_target_overlay != nullptr);
    if (!last_render_snapshot_.has_target_scene && !last_render_snapshot_.has_target_overlay) {
        last_render_snapshot_.status = "disabled";
        last_render_snapshot_.message = "No Perspective 2D map targets are bound.";
        last_render_snapshot_.remediation =
            "Bind a MapScene and SpatialMapOverlay before using Perspective 2D tools.";
    } else if (!last_render_snapshot_.has_target_scene || !last_render_snapshot_.has_target_overlay) {
        last_render_snapshot_.status = "error";
        last_render_snapshot_.message = "Perspective 2D map editor is partially bound.";
        last_render_snapshot_.remediation =
            "Bind both MapScene and SpatialMapOverlay so ability, elevation, and prop tools can operate together.";
    } else {
        last_render_snapshot_.status = "ready";
        last_render_snapshot_.message = "Perspective 2D map editor is ready.";
        last_render_snapshot_.remediation = "";
    }
    last_render_snapshot_.elevation = elevation_panel_.lastRenderSnapshot();
    last_render_snapshot_.props = prop_panel_.lastRenderSnapshot();
    last_render_snapshot_.parts_palette = grid_part_palette_panel_.lastRenderSnapshot();
    last_render_snapshot_.parts_placement = grid_part_placement_panel_.lastRenderSnapshot();
    last_render_snapshot_.parts_inspector = grid_part_inspector_panel_.lastRenderSnapshot();
    last_render_snapshot_.worldbuilding_terrain = terrain_brush_panel_.snapshot();
    last_render_snapshot_.worldbuilding_regions = region_rules_panel_.snapshot();
    last_render_snapshot_.worldbuilding_procedural = procedural_map_panel_.snapshot();
    last_render_snapshot_.worldbuilding_environment = environment_preview_panel_.snapshot();
    last_render_snapshot_.bindings = binding_panel_.lastRenderSnapshot();
    last_render_snapshot_.canvas = canvas_panel_.lastRenderSnapshot();
    last_render_snapshot_.perspective_2d_palette.selected_option_id = selected_palette_option_id_;
    last_render_snapshot_.perspective_2d_palette.selected_tileset_id = selected_tileset_id_;
    last_render_snapshot_.perspective_2d_palette.selected_tile_id = selected_tile_id_;
    last_render_snapshot_.perspective_2d_palette.brush_size = perspective_brush_size_;
    last_render_snapshot_.perspective_2d_palette.has_selected_tile =
        !selected_tileset_id_.empty() && !selected_tile_id_.empty();
    last_render_snapshot_.perspective_2d_palette.tile_options.clear();
    for (const auto& option : perspective_tile_palette_options_) {
        Perspective2DPaletteOptionSnapshot option_snapshot;
        option_snapshot.option_id = option.option_id;
        option_snapshot.label = option.label;
        option_snapshot.tileset_id = option.tileset_id;
        option_snapshot.tile_id = option.tile_id;
        option_snapshot.asset_id = option.asset_id;
        option_snapshot.project_path = option.project_path;
        option_snapshot.selected = option.option_id == selected_palette_option_id_;
        last_render_snapshot_.perspective_2d_palette.tile_options.push_back(std::move(option_snapshot));
    }
    last_render_snapshot_.perspective_2d_palette.tile_option_count =
        last_render_snapshot_.perspective_2d_palette.tile_options.size();
    last_render_snapshot_.perspective_2d_layers.clear();
    for (const auto& layer : perspective_layers_) {
        Perspective2DLayerSnapshot layer_snapshot;
        layer_snapshot.id = layer.id;
        layer_snapshot.label = layer.label;
        layer_snapshot.kind = layer.kind;
        layer_snapshot.visible = layer.visible;
        layer_snapshot.locked = layer.locked;
        layer_snapshot.order = layer.order;
        layer_snapshot.selected = layer.id == selected_perspective_layer_id_;
        layer_snapshot.tile_count = static_cast<size_t>(std::count_if(
            perspective_tiles_.begin(), perspective_tiles_.end(),
            [&](const PerspectiveTilePaint& tile) { return tile.layer_id == layer.id; }));
        layer_snapshot.event_count = static_cast<size_t>(std::count_if(
            perspective_events_.begin(), perspective_events_.end(),
            [&](const PerspectiveEvent& event) { return event.layer_id == layer.id; }));
        last_render_snapshot_.perspective_2d_layers.push_back(std::move(layer_snapshot));
    }
    last_render_snapshot_.perspective_2d_events.clear();
    for (const auto& event : perspective_events_) {
        Perspective2DEventSnapshot event_snapshot;
        event_snapshot.event_id = event.event_id;
        event_snapshot.label = event.label;
        event_snapshot.trigger_id = event.trigger_id;
        event_snapshot.layer_id = event.layer_id;
        event_snapshot.tile_x = event.tile_x;
        event_snapshot.tile_y = event.tile_y;
        for (const auto& command : event.commands) {
            event_snapshot.commands.push_back({command.code, command.argument});
        }
        event_snapshot.command_count = event_snapshot.commands.size();
        const auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                        [&](const PerspectiveLayer& candidate) {
                                            return candidate.id == event.layer_id;
                                        });
        event_snapshot.visible_in_playtest =
            layer != perspective_layers_.end() && layer->visible && !layer->locked;
        last_render_snapshot_.perspective_2d_events.push_back(std::move(event_snapshot));
    }
    last_render_snapshot_.perspective_2d_project.map_id =
        m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    last_render_snapshot_.perspective_2d_project.width =
        m_target_overlay != nullptr ? m_target_overlay->elevation.width : 0;
    last_render_snapshot_.perspective_2d_project.height =
        m_target_overlay != nullptr ? m_target_overlay->elevation.height : 0;
    last_render_snapshot_.perspective_2d_project.selected_layer_id = selected_perspective_layer_id_;
    last_render_snapshot_.perspective_2d_project.layer_count = perspective_layers_.size();
    last_render_snapshot_.perspective_2d_project.painted_tile_count = perspective_tiles_.size();
    last_render_snapshot_.perspective_2d_project.event_count = perspective_events_.size();
    last_render_snapshot_.perspective_2d_project.has_unsaved_changes = perspective_has_unsaved_changes_;
    last_render_snapshot_.perspective_2d_project.can_save = m_target_overlay != nullptr;
    last_render_snapshot_.perspective_2d_project.diagnostics = validatePerspectiveMapForPlaytest();
    last_render_snapshot_.perspective_2d_project.can_playtest =
        last_render_snapshot_.perspective_2d_project.diagnostics.empty();
    last_render_snapshot_.perspective_2d_project.can_export =
        last_perspective_playtest_result_.success && perspective_playtest_ready_ &&
        last_render_snapshot_.perspective_2d_project.diagnostics.empty();
    last_render_snapshot_.last_perspective_2d_save = last_perspective_save_result_;
    last_render_snapshot_.last_perspective_2d_load = last_perspective_load_result_;
    last_render_snapshot_.last_perspective_2d_playtest = last_perspective_playtest_result_;
    last_render_snapshot_.last_perspective_2d_export = last_perspective_export_result_;
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
        {"composite", "Compose", active_mode_ == ToolMode::Composite, true},
        {"elevation", "Elevation", active_mode_ == ToolMode::Elevation, last_render_snapshot_.has_target_overlay},
        {"props", "Props", active_mode_ == ToolMode::Props, last_render_snapshot_.has_target_overlay},
        {"abilities", "Abilities", active_mode_ == ToolMode::Abilities, last_render_snapshot_.has_target_scene},
        {"parts", "Parts", active_mode_ == ToolMode::Parts,
         last_render_snapshot_.parts_placement.has_document && last_render_snapshot_.parts_placement.has_catalog &&
             last_render_snapshot_.parts_placement.has_spatial_overlay},
        {"worldbuilding", "Worldbuilding", active_mode_ == ToolMode::Worldbuilding,
         last_render_snapshot_.has_target_overlay},
        {"tiles", "Tiles", active_mode_ == ToolMode::Tiles,
         last_render_snapshot_.has_target_overlay && !selected_perspective_layer_id_.empty()},
        {"resolve_conflict", "Resolve Conflict", false, last_render_snapshot_.canvas.has_conflicts},
    };
    canvas_panel_.SetActiveMode(last_render_snapshot_.toolbar.active_mode);
    last_render_snapshot_.canvas = canvas_panel_.lastRenderSnapshot();
}

} // namespace urpg::editor
