#include "editor/spatial/spatial_authoring_workspace.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>

namespace urpg::editor {

namespace {

std::string lowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

bool containsCaseInsensitive(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) {
        return true;
    }
    return lowerCopy(haystack).find(lowerCopy(needle)) != std::string::npos;
}

bool containsString(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

std::string trimCopy(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char character) {
        return std::isspace(character) != 0;
    }).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

std::vector<std::string> splitString(const std::string& value, char delimiter) {
    std::vector<std::string> pieces;
    std::stringstream stream(value);
    std::string piece;
    while (std::getline(stream, piece, delimiter)) {
        const std::string trimmed = trimCopy(piece);
        if (!trimmed.empty()) {
            pieces.push_back(trimmed);
        }
    }
    return pieces;
}

bool parseDouble(const std::string& value, double& out_value) {
    char* end = nullptr;
    out_value = std::strtod(value.c_str(), &end);
    return end != value.c_str() && end != nullptr && *end == '\0';
}

bool parseInt(const std::string& value, int& out_value) {
    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (end == value.c_str() || end == nullptr || *end != '\0') {
        return false;
    }
    out_value = static_cast<int>(parsed);
    return true;
}

bool conditionValueMatches(const std::string& actual_value,
                           const std::string& comparison,
                           const std::string& expected_value) {
    if (comparison == "equals" || comparison.empty()) {
        return actual_value == expected_value;
    }
    if (comparison == "not_equals") {
        return actual_value != expected_value;
    }

    double actual_number = 0.0;
    double expected_number = 0.0;
    if (!parseDouble(actual_value, actual_number) || !parseDouble(expected_value, expected_number)) {
        return false;
    }
    if (comparison == "greater_equal") {
        return actual_number >= expected_number;
    }
    if (comparison == "greater_than") {
        return actual_number > expected_number;
    }
    if (comparison == "less_equal") {
        return actual_number <= expected_number;
    }
    if (comparison == "less_than") {
        return actual_number < expected_number;
    }
    return false;
}

std::string stableContentHash(const std::string& content) {
    uint64_t hash = 14695981039346656037ull;
    for (const unsigned char character : content) {
        hash ^= static_cast<uint64_t>(character);
        hash *= 1099511628211ull;
    }
    std::ostringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(16) << hash;
    return stream.str();
}

} // namespace

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
        selected_perspective_layer_ids_ = {layer_id};
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
    selected_perspective_layer_ids_ = {layer_id};
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SelectPerspectiveLayers(const std::vector<std::string>& layer_ids) {
    if (layer_ids.empty()) {
        return false;
    }
    std::vector<std::string> valid_layer_ids;
    for (const auto& layer_id : layer_ids) {
        if (layer_id.empty() || containsString(valid_layer_ids, layer_id)) {
            continue;
        }
        const auto existing = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                           [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
        if (existing == perspective_layers_.end()) {
            return false;
        }
        valid_layer_ids.push_back(layer_id);
    }
    if (valid_layer_ids.empty()) {
        return false;
    }
    selected_perspective_layer_ids_ = std::move(valid_layer_ids);
    if (!containsString(selected_perspective_layer_ids_, selected_perspective_layer_id_)) {
        selected_perspective_layer_id_ = selected_perspective_layer_ids_.front();
    }
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SetSelectedPerspectiveLayersVisible(bool visible) {
    if (selected_perspective_layer_ids_.empty()) {
        return false;
    }
    bool changed = false;
    for (auto& layer : perspective_layers_) {
        if (containsString(selected_perspective_layer_ids_, layer.id)) {
            layer.visible = visible;
            changed = true;
        }
    }
    if (!changed) {
        return false;
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SetSelectedPerspectiveLayersLocked(bool locked) {
    if (selected_perspective_layer_ids_.empty()) {
        return false;
    }
    bool changed = false;
    for (auto& layer : perspective_layers_) {
        if (containsString(selected_perspective_layer_ids_, layer.id)) {
            layer.locked = locked;
            changed = true;
        }
    }
    if (!changed) {
        return false;
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::DuplicateSelectedPerspectiveLayers(const std::string& id_suffix) {
    if (selected_perspective_layer_ids_.empty() || id_suffix.empty()) {
        return false;
    }
    std::vector<std::string> new_layer_ids;
    for (const auto& layer_id : selected_perspective_layer_ids_) {
        const auto source = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                         [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
        if (source == perspective_layers_.end()) {
            return false;
        }
        const std::string new_layer_id = layer_id + id_suffix;
        const auto existing = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                           [&](const PerspectiveLayer& layer) {
                                               return layer.id == new_layer_id;
                                           });
        if (existing != perspective_layers_.end()) {
            return false;
        }
    }

    const std::vector<std::string> source_layer_ids = selected_perspective_layer_ids_;
    for (const auto& layer_id : source_layer_ids) {
        const auto source = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                         [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
        PerspectiveLayer copy = *source;
        copy.id = layer_id + id_suffix;
        copy.label += " Copy";
        copy.locked = false;
        copy.order = static_cast<int>(perspective_layers_.size());
        new_layer_ids.push_back(copy.id);
        perspective_layers_.push_back(std::move(copy));

        const auto original_tile_count = perspective_tiles_.size();
        for (size_t i = 0; i < original_tile_count; ++i) {
            if (perspective_tiles_[i].layer_id == layer_id) {
                PerspectiveTilePaint tile = perspective_tiles_[i];
                tile.layer_id = new_layer_ids.back();
                perspective_tiles_.push_back(std::move(tile));
            }
        }
        const auto original_event_count = perspective_events_.size();
        for (size_t i = 0; i < original_event_count; ++i) {
            if (perspective_events_[i].layer_id == layer_id) {
                PerspectiveEvent event = perspective_events_[i];
                event.layer_id = new_layer_ids.back();
                event.event_id += id_suffix;
                perspective_events_.push_back(std::move(event));
            }
        }
    }
    selected_perspective_layer_ids_ = std::move(new_layer_ids);
    selected_perspective_layer_id_ = selected_perspective_layer_ids_.front();
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::DeleteSelectedPerspectiveLayers() {
    if (selected_perspective_layer_ids_.empty()) {
        return false;
    }
    bool removed = false;
    for (const auto& layer_id : selected_perspective_layer_ids_) {
        auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                  [&](const PerspectiveLayer& candidate) { return candidate.id == layer_id; });
        if (layer == perspective_layers_.end()) {
            continue;
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
        removed = true;
    }
    if (!removed) {
        return false;
    }
    for (size_t i = 0; i < perspective_layers_.size(); ++i) {
        perspective_layers_[i].order = static_cast<int>(i);
    }
    selected_perspective_layer_id_ = perspective_layers_.empty() ? std::string{} : perspective_layers_.front().id;
    selected_perspective_layer_ids_.clear();
    if (!selected_perspective_layer_id_.empty()) {
        selected_perspective_layer_ids_.push_back(selected_perspective_layer_id_);
    }
    markPerspectiveDirty();
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
    selected_perspective_layer_ids_.erase(
        std::remove(selected_perspective_layer_ids_.begin(), selected_perspective_layer_ids_.end(), layer_id),
        selected_perspective_layer_ids_.end());
    if (selected_perspective_layer_ids_.empty() && !selected_perspective_layer_id_.empty()) {
        selected_perspective_layer_ids_.push_back(selected_perspective_layer_id_);
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

void SpatialAuthoringWorkspace::SetPerspectiveTilePaletteFilter(const std::string& search_text,
                                                                const std::string& tileset_id,
                                                                const std::string& category_id) {
    palette_search_text_ = search_text;
    palette_filter_tileset_id_ = tileset_id;
    palette_filter_category_id_ = category_id;
    captureRenderSnapshot();
}

void SpatialAuthoringWorkspace::ClearPerspectiveTilePaletteFilter() {
    palette_search_text_.clear();
    palette_filter_tileset_id_.clear();
    palette_filter_category_id_.clear();
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
    if (!event->selected_page_id.empty()) {
        auto page = std::find_if(event->pages.begin(), event->pages.end(),
                                 [&](const PerspectiveEvent::Page& candidate) {
                                     return candidate.page_id == event->selected_page_id;
                                 });
        if (page != event->pages.end()) {
            page->commands.push_back({command_code, argument});
            markPerspectiveDirty();
            captureRenderSnapshot();
            return true;
        }
    }
    event->commands.push_back({command_code, argument});
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::AddPerspectiveEventPage(const std::string& event_id,
                                                        const std::string& page_id,
                                                        const std::string& label,
                                                        const std::string& trigger_id) {
    if (event_id.empty() || page_id.empty() || label.empty() || trigger_id.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    const auto duplicate = std::find_if(event->pages.begin(), event->pages.end(),
                                        [&](const PerspectiveEvent::Page& candidate) {
                                            return candidate.page_id == page_id;
                                        });
    if (duplicate != event->pages.end()) {
        return false;
    }

    PerspectiveEvent::Page page;
    page.page_id = page_id;
    page.label = label;
    page.trigger_id = trigger_id;
    page.order = static_cast<int>(event->pages.size());
    event->pages.push_back(std::move(page));
    if (event->selected_page_id.empty()) {
        event->selected_page_id = page_id;
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SelectPerspectiveEventPage(const std::string& event_id,
                                                           const std::string& page_id) {
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    const auto page = std::find_if(event->pages.begin(), event->pages.end(),
                                   [&](const PerspectiveEvent::Page& candidate) {
                                       return candidate.page_id == page_id;
                                   });
    if (page == event->pages.end()) {
        return false;
    }
    event->selected_page_id = page_id;
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::MovePerspectiveEventPage(const std::string& event_id,
                                                         const std::string& page_id,
                                                         int new_order) {
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end() || new_order < 0 ||
        new_order >= static_cast<int>(event->pages.size())) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end()) {
        return false;
    }
    PerspectiveEvent::Page moved_page = std::move(*page);
    event->pages.erase(page);
    event->pages.insert(event->pages.begin() + new_order, std::move(moved_page));
    for (size_t i = 0; i < event->pages.size(); ++i) {
        event->pages[i].order = static_cast<int>(i);
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::DuplicatePerspectiveEventPage(const std::string& event_id,
                                                              const std::string& source_page_id,
                                                              const std::string& new_page_id,
                                                              const std::string& new_label) {
    if (event_id.empty() || source_page_id.empty() || new_page_id.empty() || new_label.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    const auto source_page = std::find_if(event->pages.begin(), event->pages.end(),
                                          [&](const PerspectiveEvent::Page& candidate) {
                                              return candidate.page_id == source_page_id;
                                          });
    const auto duplicate = std::find_if(event->pages.begin(), event->pages.end(),
                                        [&](const PerspectiveEvent::Page& candidate) {
                                            return candidate.page_id == new_page_id;
                                        });
    if (source_page == event->pages.end() || duplicate != event->pages.end()) {
        return false;
    }
    PerspectiveEvent::Page copied_page = *source_page;
    copied_page.page_id = new_page_id;
    copied_page.label = new_label;
    copied_page.order = static_cast<int>(event->pages.size());
    event->pages.push_back(std::move(copied_page));
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::DeletePerspectiveEventPage(const std::string& event_id,
                                                           const std::string& page_id) {
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end()) {
        return false;
    }
    const bool removed_selected_page = event->selected_page_id == page_id;
    event->pages.erase(page);
    for (size_t i = 0; i < event->pages.size(); ++i) {
        event->pages[i].order = static_cast<int>(i);
    }
    if (removed_selected_page) {
        event->selected_page_id = event->pages.empty() ? std::string{} : event->pages.front().page_id;
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::AddPerspectiveEventPageCondition(const std::string& event_id,
                                                                 const std::string& page_id,
                                                                 const std::string& condition_type,
                                                                 const std::string& key,
                                                                 const std::string& value) {
    return AddPerspectiveEventPageConditionRule(event_id, page_id, condition_type, key, "equals", value);
}

bool SpatialAuthoringWorkspace::AddPerspectiveEventPageConditionRule(const std::string& event_id,
                                                                     const std::string& page_id,
                                                                     const std::string& condition_type,
                                                                     const std::string& key,
                                                                     const std::string& comparison,
                                                                     const std::string& value) {
    if (event_id.empty() || page_id.empty() || condition_type.empty() || key.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end()) {
        return false;
    }
    auto existing = std::find_if(page->conditions.begin(), page->conditions.end(),
                                 [&](const PerspectiveEvent::Condition& condition) {
                                     return condition.type == condition_type && condition.key == key &&
                                            condition.comparison == comparison;
                                 });
    if (existing == page->conditions.end()) {
        page->conditions.push_back({condition_type, key, comparison.empty() ? "equals" : comparison, value});
    } else {
        existing->value = value;
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::UpdatePerspectiveEventPageCondition(const std::string& event_id,
                                                                    const std::string& page_id,
                                                                    size_t condition_index,
                                                                    const std::string& condition_type,
                                                                    const std::string& key,
                                                                    const std::string& value) {
    if (condition_type.empty() || key.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end() || condition_index >= page->conditions.size()) {
        return false;
    }
    page->conditions[condition_index] = {condition_type, key, "equals", value};
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::RemovePerspectiveEventPageCondition(const std::string& event_id,
                                                                    const std::string& page_id,
                                                                    size_t condition_index) {
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end() || condition_index >= page->conditions.size()) {
        return false;
    }
    page->conditions.erase(page->conditions.begin() + static_cast<std::ptrdiff_t>(condition_index));
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::AddPerspectiveEventPageCommand(const std::string& event_id,
                                                               const std::string& page_id,
                                                               const std::string& command_code,
                                                               const std::string& argument) {
    if (event_id.empty() || page_id.empty() || command_code.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end()) {
        return false;
    }
    page->commands.push_back({command_code, argument});
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::AddPerspectiveEventPageConditionalBranch(const std::string& event_id,
                                                                         const std::string& page_id,
                                                                         const std::string& condition_type,
                                                                         const std::string& key,
                                                                         const std::string& comparison,
                                                                         const std::string& value) {
    if (event_id.empty() || page_id.empty() || condition_type.empty() || key.empty() || comparison.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end()) {
        return false;
    }
    PerspectiveEvent::Command command;
    command.code = "conditional_branch";
    command.condition_type = condition_type;
    command.condition_key = key;
    command.condition_comparison = comparison;
    command.condition_value = value;
    page->commands.push_back(std::move(command));
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::AddPerspectiveEventPageBranchCommand(const std::string& event_id,
                                                                     const std::string& page_id,
                                                                     size_t branch_command_index,
                                                                     bool when_true,
                                                                     const std::string& command_code,
                                                                     const std::string& argument) {
    if (event_id.empty() || page_id.empty() || command_code.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end() || branch_command_index >= page->commands.size()) {
        return false;
    }
    auto& branch = page->commands[branch_command_index];
    if (branch.code != "conditional_branch") {
        return false;
    }
    PerspectiveEvent::Command child;
    child.code = command_code;
    child.argument = argument;
    if (when_true) {
        branch.true_commands.push_back(std::move(child));
    } else {
        branch.false_commands.push_back(std::move(child));
    }
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::UpdatePerspectiveEventPageCommand(const std::string& event_id,
                                                                  const std::string& page_id,
                                                                  size_t command_index,
                                                                  const std::string& command_code,
                                                                  const std::string& argument) {
    if (command_code.empty()) {
        return false;
    }
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end() || command_index >= page->commands.size()) {
        return false;
    }
    page->commands[command_index] = {command_code, argument};
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::RemovePerspectiveEventPageCommand(const std::string& event_id,
                                                                  const std::string& page_id,
                                                                  size_t command_index) {
    auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                              [&](const PerspectiveEvent& candidate) { return candidate.event_id == event_id; });
    if (event == perspective_events_.end()) {
        return false;
    }
    auto page = std::find_if(event->pages.begin(), event->pages.end(),
                             [&](const PerspectiveEvent::Page& candidate) {
                                 return candidate.page_id == page_id;
                             });
    if (page == event->pages.end() || command_index >= page->commands.size()) {
        return false;
    }
    page->commands.erase(page->commands.begin() + static_cast<std::ptrdiff_t>(command_index));
    markPerspectiveDirty();
    captureRenderSnapshot();
    return true;
}

bool SpatialAuthoringWorkspace::SetPerspectiveEventConditionValue(const std::string& condition_type,
                                                                  const std::string& key,
                                                                  const std::string& value) {
    if (condition_type.empty() || key.empty()) {
        return false;
    }
    auto existing = std::find_if(perspective_event_condition_values_.begin(),
                                 perspective_event_condition_values_.end(),
                                 [&](const PerspectiveEventConditionValue& condition_value) {
                                     return condition_value.type == condition_type && condition_value.key == key;
                                 });
    if (existing == perspective_event_condition_values_.end()) {
        perspective_event_condition_values_.push_back({condition_type, key, value});
    } else {
        existing->value = value;
    }
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
        if (event != perspective_events_.end() && !event->selected_page_id.empty()) {
            auto page = std::find_if(event->pages.begin(), event->pages.end(),
                                     [&](const PerspectiveEvent::Page& candidate) {
                                         return candidate.page_id == event->selected_page_id;
                                     });
            if (page != event->pages.end() && command_index < page->commands.size()) {
                page->commands[command_index] = {command_code, argument};
                markPerspectiveDirty();
                captureRenderSnapshot();
                return true;
            }
        }
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
        if (event != perspective_events_.end() && !event->selected_page_id.empty()) {
            auto page = std::find_if(event->pages.begin(), event->pages.end(),
                                     [&](const PerspectiveEvent::Page& candidate) {
                                         return candidate.page_id == event->selected_page_id;
                                     });
            if (page != event->pages.end() && command_index < page->commands.size()) {
                page->commands.erase(page->commands.begin() + static_cast<std::ptrdiff_t>(command_index));
                markPerspectiveDirty();
                captureRenderSnapshot();
                return true;
            }
        }
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

    const std::function<nlohmann::json(const std::vector<PerspectiveEvent::Command>&)> serialize_commands =
        [&](const std::vector<PerspectiveEvent::Command>& commands) {
            nlohmann::json command_json = nlohmann::json::array();
            for (const auto& command : commands) {
                nlohmann::json item = {{"code", command.code}, {"argument", command.argument}};
                if (command.code == "conditional_branch") {
                    item["condition"] = {{"type", command.condition_type},
                                         {"key", command.condition_key},
                                         {"comparison", command.condition_comparison},
                                         {"value", command.condition_value}};
                    item["true_commands"] = serialize_commands(command.true_commands);
                    item["false_commands"] = serialize_commands(command.false_commands);
                }
                command_json.push_back(std::move(item));
            }
            return command_json;
        };

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
        nlohmann::json pages = nlohmann::json::array();
        for (const auto& page : event.pages) {
            nlohmann::json conditions = nlohmann::json::array();
            for (const auto& condition : page.conditions) {
                conditions.push_back({{"type", condition.type},
                                      {"key", condition.key},
                                      {"comparison", condition.comparison},
                                      {"value", condition.value}});
            }
            pages.push_back({{"page_id", page.page_id},
                             {"label", page.label},
                             {"trigger_id", page.trigger_id},
                             {"order", page.order},
                             {"conditions", std::move(conditions)},
                             {"commands", serialize_commands(page.commands)}});
        }
        json["events"].push_back({{"event_id", event.event_id},
                                  {"label", event.label},
                                  {"trigger_id", event.trigger_id},
                                  {"layer_id", event.layer_id},
                                  {"x", event.tile_x},
                                  {"y", event.tile_y},
                                  {"selected_page_id", event.selected_page_id},
                                  {"commands", serialize_commands(event.commands)},
                                  {"pages", std::move(pages)}});
    }

    return json.dump(2);
}

std::string SpatialAuthoringWorkspace::serializePerspectiveRuntimeManifest(size_t& out_layer_count,
                                                                           size_t& out_tile_count,
                                                                           size_t& out_event_count) const {
    out_layer_count = 0;
    out_tile_count = 0;
    out_event_count = 0;

    const auto layer_by_id = [&](const std::string& layer_id) -> const PerspectiveLayer* {
        const auto found = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                        [&](const PerspectiveLayer& layer) { return layer.id == layer_id; });
        return found == perspective_layers_.end() ? nullptr : &(*found);
    };
    const auto condition_matches = [&](const PerspectiveEvent::Condition& condition) {
        const auto value = std::find_if(perspective_event_condition_values_.begin(),
                                        perspective_event_condition_values_.end(),
                                        [&](const PerspectiveEventConditionValue& condition_value) {
                                            return condition_value.type == condition.type &&
                                                   condition_value.key == condition.key;
                                        });
        return value != perspective_event_condition_values_.end() &&
               conditionValueMatches(value->value, condition.comparison, condition.value);
    };
    const auto page_matches = [&](const PerspectiveEvent::Page& page) {
        return std::all_of(page.conditions.begin(), page.conditions.end(), condition_matches);
    };
    const auto active_page_for_event = [&](const PerspectiveEvent& event) -> const PerspectiveEvent::Page* {
        const PerspectiveEvent::Page* active_page = nullptr;
        for (const auto& page : event.pages) {
            if (page_matches(page)) {
                active_page = &page;
            }
        }
        if (active_page == nullptr && !event.pages.empty()) {
            active_page = &event.pages.front();
        }
        return active_page;
    };
    const std::function<nlohmann::json(const std::vector<PerspectiveEvent::Command>&)> serialize_commands =
        [&](const std::vector<PerspectiveEvent::Command>& commands) {
            nlohmann::json command_json = nlohmann::json::array();
            for (const auto& command : commands) {
                nlohmann::json item = {{"code", command.code}, {"argument", command.argument}};
                if (command.code == "conditional_branch") {
                    item["condition"] = {{"type", command.condition_type},
                                         {"key", command.condition_key},
                                         {"comparison", command.condition_comparison},
                                         {"value", command.condition_value}};
                    item["true_commands"] = serialize_commands(command.true_commands);
                    item["false_commands"] = serialize_commands(command.false_commands);
                }
                command_json.push_back(std::move(item));
            }
            return command_json;
        };

    nlohmann::json json;
    json["document_kind"] = "urpg.perspective_2d.runtime_manifest";
    json["version"] = 1;
    json["map_id"] = m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    json["width"] = m_target_overlay != nullptr ? m_target_overlay->elevation.width : 0;
    json["height"] = m_target_overlay != nullptr ? m_target_overlay->elevation.height : 0;
    json["layers"] = nlohmann::json::array();
    json["tiles"] = nlohmann::json::array();
    json["events"] = nlohmann::json::array();

    for (const auto& layer : perspective_layers_) {
        if (!layer.visible || layer.locked) {
            continue;
        }
        json["layers"].push_back({{"id", layer.id},
                                  {"label", layer.label},
                                  {"kind", layer.kind},
                                  {"order", layer.order}});
    }
    out_layer_count = json["layers"].size();

    for (const auto& tile : perspective_tiles_) {
        const PerspectiveLayer* layer = layer_by_id(tile.layer_id);
        if (layer == nullptr || layer->kind != "tile" || !layer->visible || layer->locked) {
            continue;
        }
        json["tiles"].push_back({{"layer_id", tile.layer_id},
                                 {"tileset_id", tile.tileset_id},
                                 {"tile_id", tile.tile_id},
                                 {"x", tile.tile_x},
                                 {"y", tile.tile_y}});
    }
    out_tile_count = json["tiles"].size();

    for (const auto& event : perspective_events_) {
        const PerspectiveLayer* layer = layer_by_id(event.layer_id);
        if (layer == nullptr || !layer->visible || layer->locked) {
            continue;
        }
        const PerspectiveEvent::Page* active_page = active_page_for_event(event);
        const auto& commands = active_page != nullptr ? active_page->commands : event.commands;
        json["events"].push_back({{"event_id", event.event_id},
                                  {"label", event.label},
                                  {"layer_id", event.layer_id},
                                  {"x", event.tile_x},
                                  {"y", event.tile_y},
                                  {"trigger_id", active_page != nullptr ? active_page->trigger_id : event.trigger_id},
                                  {"active_page_id", active_page != nullptr ? active_page->page_id : std::string{}},
                                  {"commands", serialize_commands(commands)}});
    }
    out_event_count = json["events"].size();

    return json.dump(2);
}

std::string
SpatialAuthoringWorkspace::serializePerspectiveExportPackageManifest(const Perspective2DExportResult& export_result) const {
    nlohmann::json json;
    json["document_kind"] = "urpg.perspective_2d.export_package";
    json["version"] = 1;
    json["map_id"] = export_result.map_id;
    json["draft_document_kind"] = "urpg.perspective_2d.map";
    json["runtime_manifest_kind"] = "urpg.perspective_2d.runtime_manifest";
    json["event_execution_trace_bundle_kind"] = "urpg.perspective_2d.event_execution_trace_bundle";
    json["runtime_layer_count"] = export_result.runtime_layer_count;
    json["runtime_tile_count"] = export_result.runtime_tile_count;
    json["runtime_event_count"] = export_result.runtime_event_count;
    json["runtime_event_execution_trace_count"] = export_result.runtime_event_execution_trace_count;
    json["package_signature"] = export_result.package_signature;
    json["files"] = nlohmann::json::array();
    for (const auto& file : export_result.package_files) {
        json["files"].push_back({{"path", file.path},
                                 {"kind", file.kind},
                                 {"byte_count", file.byte_count},
                                 {"content_hash", file.content_hash}});
    }
    json["release_asset_gate"] = {{"policy_state", last_perspective_release_asset_gate_result_.policy_state},
                                  {"success", last_perspective_release_asset_gate_result_.success},
                                  {"release_required_asset_count",
                                   last_perspective_release_asset_gate_result_.release_required_asset_count},
                                  {"verified_release_required_asset_count",
                                   last_perspective_release_asset_gate_result_.verified_release_required_asset_count},
                                  {"optional_lfs_asset_count",
                                   last_perspective_release_asset_gate_result_.optional_lfs_asset_count},
                                  {"optional_lfs_deferred_count",
                                   last_perspective_release_asset_gate_result_.optional_lfs_deferred_count}};
    return json.dump(2);
}

SpatialAuthoringWorkspace::Perspective2DEventExecutionResult
SpatialAuthoringWorkspace::buildPerspectiveEventExecutionTrace(const PerspectiveEvent& event) const {
    Perspective2DEventExecutionResult result;
    result.success = true;
    result.message = "Perspective 2D event execution preview is ready.";
    result.map_id = m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    result.event_id = event.event_id;

    const auto condition_matches = [&](const PerspectiveEvent::Condition& condition) {
        const auto value = std::find_if(perspective_event_condition_values_.begin(),
                                        perspective_event_condition_values_.end(),
                                        [&](const PerspectiveEventConditionValue& condition_value) {
                                            return condition_value.type == condition.type &&
                                                   condition_value.key == condition.key;
                                        });
        return value != perspective_event_condition_values_.end() &&
               conditionValueMatches(value->value, condition.comparison, condition.value);
    };
    const auto command_condition_matches = [&](const PerspectiveEvent::Command& command) {
        const auto value = std::find_if(perspective_event_condition_values_.begin(),
                                        perspective_event_condition_values_.end(),
                                        [&](const PerspectiveEventConditionValue& condition_value) {
                                            return condition_value.type == command.condition_type &&
                                                   condition_value.key == command.condition_key;
                                        });
        return value != perspective_event_condition_values_.end() &&
               conditionValueMatches(value->value, command.condition_comparison, command.condition_value);
    };
    const auto page_matches = [&](const PerspectiveEvent::Page& page) {
        return std::all_of(page.conditions.begin(), page.conditions.end(), condition_matches);
    };
    const auto active_page_for_event = [&](const PerspectiveEvent& candidate) -> const PerspectiveEvent::Page* {
        const PerspectiveEvent::Page* active_page = nullptr;
        for (const auto& page : candidate.pages) {
            if (page_matches(page)) {
                active_page = &page;
            }
        }
        if (active_page == nullptr && !candidate.pages.empty()) {
            active_page = &candidate.pages.front();
        }
        return active_page;
    };
    const std::function<void(const std::vector<PerspectiveEvent::Command>&, const std::string&)>
        append_execution_steps = [&](const std::vector<PerspectiveEvent::Command>& commands,
                                     const std::string& branch_path) {
            for (const auto& command : commands) {
                Perspective2DEventExecutionStep step;
                step.code = command.code;
                step.argument = command.argument;
                step.branch_path = branch_path;
                step.condition_type = command.condition_type;
                step.condition_key = command.condition_key;
                step.condition_comparison = command.condition_comparison;
                step.condition_value = command.condition_value;
                if (command.code == "conditional_branch") {
                    step.condition_matched = command_condition_matches(command);
                    result.executed_commands.push_back(std::move(step));
                    const bool matched = result.executed_commands.back().condition_matched;
                    const std::string child_path = branch_path == "root"
                                                       ? (matched ? "true" : "false")
                                                       : branch_path + (matched ? ".true" : ".false");
                    append_execution_steps(matched ? command.true_commands : command.false_commands, child_path);
                } else {
                    result.executed_commands.push_back(std::move(step));
                }
            }
        };

    const PerspectiveEvent::Page* active_page = active_page_for_event(event);
    if (active_page != nullptr) {
        result.active_page_id = active_page->page_id;
        result.trigger_id = active_page->trigger_id;
        append_execution_steps(active_page->commands, "root");
    } else {
        result.trigger_id = event.trigger_id;
        append_execution_steps(event.commands, "root");
    }
    result.executed_command_count = result.executed_commands.size();

    nlohmann::json trace_json;
    trace_json["document_kind"] = "urpg.perspective_2d.event_execution_trace";
    trace_json["version"] = 1;
    trace_json["map_id"] = result.map_id;
    trace_json["event_id"] = result.event_id;
    trace_json["active_page_id"] = result.active_page_id;
    trace_json["trigger_id"] = result.trigger_id;
    trace_json["executed_commands"] = nlohmann::json::array();
    for (const auto& step : result.executed_commands) {
        nlohmann::json step_json = {{"code", step.code},
                                    {"argument", step.argument},
                                    {"branch_path", step.branch_path}};
        if (step.code == "conditional_branch") {
            step_json["condition"] = {{"type", step.condition_type},
                                      {"key", step.condition_key},
                                      {"comparison", step.condition_comparison},
                                      {"value", step.condition_value},
                                      {"matched", step.condition_matched}};
        }
        trace_json["executed_commands"].push_back(std::move(step_json));
    }
    result.serialized_execution_trace_json = trace_json.dump(2);
    return result;
}

std::string SpatialAuthoringWorkspace::serializePerspectiveEventExecutionTraceBundle(size_t& out_trace_count) const {
    out_trace_count = 0;
    nlohmann::json json;
    json["document_kind"] = "urpg.perspective_2d.event_execution_trace_bundle";
    json["version"] = 1;
    json["map_id"] = m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    json["traces"] = nlohmann::json::array();
    for (const auto& event : perspective_events_) {
        const auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                        [&](const PerspectiveLayer& candidate) {
                                            return candidate.id == event.layer_id;
                                        });
        if (layer == perspective_layers_.end() || !layer->visible || layer->locked) {
            continue;
        }
        const auto trace = buildPerspectiveEventExecutionTrace(event);
        json["traces"].push_back(nlohmann::json::parse(trace.serialized_execution_trace_json));
    }
    out_trace_count = json["traces"].size();
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
    selected_perspective_layer_ids_.clear();
    if (!selected_perspective_layer_id_.empty()) {
        selected_perspective_layer_ids_.push_back(selected_perspective_layer_id_);
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
    const std::function<std::vector<PerspectiveEvent::Command>(const nlohmann::json&)> load_commands =
        [&](const nlohmann::json& commands_json) {
            std::vector<PerspectiveEvent::Command> commands;
            for (const auto& command_json : commands_json) {
                PerspectiveEvent::Command command;
                command.code = command_json.value("code", "");
                command.argument = command_json.value("argument", "");
                const auto condition_json = command_json.value("condition", nlohmann::json::object());
                command.condition_type = condition_json.value("type", "");
                command.condition_key = condition_json.value("key", "");
                command.condition_comparison = condition_json.value("comparison", "");
                command.condition_value = condition_json.value("value", "");
                command.true_commands = load_commands(command_json.value("true_commands", nlohmann::json::array()));
                command.false_commands = load_commands(command_json.value("false_commands", nlohmann::json::array()));
                if (!command.code.empty()) {
                    commands.push_back(std::move(command));
                }
            }
            return commands;
        };
    for (const auto& event_json : json.value("events", nlohmann::json::array())) {
        PerspectiveEvent event;
        event.event_id = event_json.value("event_id", "");
        event.label = event_json.value("label", event.event_id);
        event.trigger_id = event_json.value("trigger_id", "confirm_interact");
        event.layer_id = event_json.value("layer_id", "");
        event.tile_x = event_json.value("x", 0);
        event.tile_y = event_json.value("y", 0);
        event.selected_page_id = event_json.value("selected_page_id", "");
        event.commands = load_commands(event_json.value("commands", nlohmann::json::array()));
        for (const auto& page_json : event_json.value("pages", nlohmann::json::array())) {
            PerspectiveEvent::Page page;
            page.page_id = page_json.value("page_id", "");
            page.label = page_json.value("label", page.page_id);
            page.trigger_id = page_json.value("trigger_id", event.trigger_id);
            page.order = page_json.value("order", static_cast<int>(event.pages.size()));
            for (const auto& condition_json : page_json.value("conditions", nlohmann::json::array())) {
                PerspectiveEvent::Condition condition;
                condition.type = condition_json.value("type", "");
                condition.key = condition_json.value("key", "");
                condition.comparison = condition_json.value("comparison", "equals");
                condition.value = condition_json.value("value", "");
                if (!condition.type.empty() && !condition.key.empty()) {
                    page.conditions.push_back(std::move(condition));
                }
            }
            page.commands = load_commands(page_json.value("commands", nlohmann::json::array()));
            if (!page.page_id.empty()) {
                event.pages.push_back(std::move(page));
            }
        }
        std::stable_sort(event.pages.begin(), event.pages.end(),
                         [](const PerspectiveEvent::Page& lhs, const PerspectiveEvent::Page& rhs) {
                             return lhs.order < rhs.order;
                         });
        for (size_t i = 0; i < event.pages.size(); ++i) {
            event.pages[i].order = static_cast<int>(i);
        }
        if (event.selected_page_id.empty() && !event.pages.empty()) {
            event.selected_page_id = event.pages.front().page_id;
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
    result.serialized_runtime_manifest_json =
        serializePerspectiveRuntimeManifest(result.runtime_layer_count, result.runtime_tile_count, result.runtime_event_count);
    result.serialized_event_execution_traces_json =
        serializePerspectiveEventExecutionTraceBundle(result.runtime_event_execution_trace_count);
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
    result.serialized_runtime_manifest_json =
        serializePerspectiveRuntimeManifest(result.runtime_layer_count, result.runtime_tile_count, result.runtime_event_count);
    result.serialized_event_execution_traces_json =
        serializePerspectiveEventExecutionTraceBundle(result.runtime_event_execution_trace_count);
    const std::string map_id = result.map_id.empty() ? "perspective_2d_map" : result.map_id;
    const auto make_file = [](const std::string& path,
                              const std::string& kind,
                              const std::string& content) {
        Perspective2DPackageFile file;
        file.path = path;
        file.kind = kind;
        file.byte_count = content.size();
        file.content_hash = stableContentHash(content);
        return file;
    };
    result.package_files.push_back(
        make_file("maps/" + map_id + ".p2d.json", "draft_document", result.serialized_document_json));
    result.package_files.push_back(
        make_file("maps/" + map_id + ".runtime.json", "runtime_manifest", result.serialized_runtime_manifest_json));
    result.package_files.push_back(make_file("maps/" + map_id + ".event_traces.json",
                                             "event_execution_traces",
                                             result.serialized_event_execution_traces_json));
    result.package_signature = stableContentHash(result.package_files[0].content_hash + ":" +
                                                result.package_files[1].content_hash + ":" +
                                                result.package_files[2].content_hash + ":" +
                                                last_perspective_release_asset_gate_result_.policy_state);
    result.package_files.push_back({"package/" + map_id + ".package.json", "package_manifest", 0, "self"});
    result.serialized_package_manifest_json = serializePerspectiveExportPackageManifest(result);
    last_perspective_export_result_ = result;
    captureRenderSnapshot();
    return last_perspective_export_result_;
}

SpatialAuthoringWorkspace::Perspective2DEventExecutionResult
SpatialAuthoringWorkspace::PreviewPerspectiveEventExecution(const std::string& event_id) {
    Perspective2DEventExecutionResult result;
    result.map_id = m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    result.event_id = event_id;
    result.blocker_codes = validatePerspectiveMapForPlaytest();

    const auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                                    [&](const PerspectiveEvent& candidate) {
                                        return candidate.event_id == event_id;
                                    });
    if (event == perspective_events_.end()) {
        result.blocker_codes.push_back("p2d_event_missing");
    }
    if (!result.blocker_codes.empty()) {
        result.message = "Perspective 2D event execution preview is blocked.";
        last_perspective_event_execution_result_ = result;
        captureRenderSnapshot();
        return last_perspective_event_execution_result_;
    }

    result = buildPerspectiveEventExecutionTrace(*event);
    last_perspective_event_execution_result_ = result;
    captureRenderSnapshot();
    return last_perspective_event_execution_result_;
}

SpatialAuthoringWorkspace::Perspective2DRuntimeResult
SpatialAuthoringWorkspace::ExecutePerspectiveRuntimeEvent(const std::string& event_id) {
    Perspective2DRuntimeResult result;
    result.map_id = m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    result.event_id = event_id;
    result.switches = perspective_runtime_switches_;
    result.variables = perspective_runtime_variables_;
    result.self_switches = perspective_runtime_self_switches_;
    result.inventory = perspective_runtime_inventory_;
    result.gold = perspective_runtime_gold_;
    result.player_map_id =
        perspective_runtime_player_map_id_.empty() ? result.map_id : perspective_runtime_player_map_id_;
    result.player_tile_x = perspective_runtime_player_tile_x_;
    result.player_tile_y = perspective_runtime_player_tile_y_;
    result.blocker_codes = validatePerspectiveMapForPlaytest();

    const auto event = std::find_if(perspective_events_.begin(), perspective_events_.end(),
                                    [&](const PerspectiveEvent& candidate) {
                                        return candidate.event_id == event_id;
                                    });
    if (event == perspective_events_.end()) {
        result.blocker_codes.push_back("p2d_event_missing");
    }
    if (!result.blocker_codes.empty()) {
        result.message = "Perspective 2D runtime event execution is blocked.";
        last_perspective_runtime_result_ = result;
        captureRenderSnapshot();
        return last_perspective_runtime_result_;
    }

    const auto set_state_entry = [](std::vector<Perspective2DStateEntry>& entries,
                                    const std::string& key,
                                    const std::string& value) {
        const auto found = std::find_if(entries.begin(), entries.end(), [&](const Perspective2DStateEntry& entry) {
            return entry.key == key;
        });
        if (found != entries.end()) {
            found->value = value;
        } else {
            entries.push_back({key, value});
        }
    };
    const auto find_state_entry = [](const std::vector<Perspective2DStateEntry>& entries,
                                     const std::string& key,
                                     std::string& out_value) {
        const auto found = std::find_if(entries.begin(), entries.end(), [&](const Perspective2DStateEntry& entry) {
            return entry.key == key;
        });
        if (found == entries.end()) {
            return false;
        }
        out_value = found->value;
        return true;
    };
    const auto sync_condition_value = [&](const std::string& type,
                                          const std::string& key,
                                          const std::string& value) {
        const auto found = std::find_if(perspective_event_condition_values_.begin(),
                                        perspective_event_condition_values_.end(),
                                        [&](const PerspectiveEventConditionValue& condition_value) {
                                            return condition_value.type == type && condition_value.key == key;
                                        });
        if (found != perspective_event_condition_values_.end()) {
            found->value = value;
        } else {
            perspective_event_condition_values_.push_back({type, key, value});
        }
    };
    const auto condition_state_value = [&](const std::string& type,
                                           const std::string& key,
                                           std::string& out_value) {
        const std::string self_switch_key =
            type == "self_switch" && key.find(':') == std::string::npos ? event->event_id + ":" + key : key;
        if (type == "switch" && find_state_entry(result.switches, key, out_value)) {
            return true;
        }
        if (type == "variable" && find_state_entry(result.variables, key, out_value)) {
            return true;
        }
        if (type == "self_switch" && find_state_entry(result.self_switches, self_switch_key, out_value)) {
            return true;
        }
        const auto found = std::find_if(perspective_event_condition_values_.begin(),
                                        perspective_event_condition_values_.end(),
                                        [&](const PerspectiveEventConditionValue& condition_value) {
                                            return condition_value.type == type &&
                                                   (condition_value.key == key ||
                                                    condition_value.key == self_switch_key);
                                        });
        if (found == perspective_event_condition_values_.end()) {
            return false;
        }
        out_value = found->value;
        return true;
    };
    const auto page_condition_matches = [&](const PerspectiveEvent::Condition& condition) {
        std::string actual_value;
        return condition_state_value(condition.type, condition.key, actual_value) &&
               conditionValueMatches(actual_value, condition.comparison, condition.value);
    };
    const auto page_matches = [&](const PerspectiveEvent::Page& page) {
        return std::all_of(page.conditions.begin(), page.conditions.end(), page_condition_matches);
    };

    const PerspectiveEvent::Page* active_page = nullptr;
    for (const auto& page : event->pages) {
        if (page_matches(page)) {
            active_page = &page;
        }
    }
    if (active_page == nullptr && !event->pages.empty()) {
        active_page = &event->pages.front();
    }
    if (active_page != nullptr) {
        result.active_page_id = active_page->page_id;
        result.trigger_id = active_page->trigger_id;
    } else {
        result.trigger_id = event->trigger_id;
    }
    if (result.player_map_id.empty()) {
        result.player_map_id = result.map_id;
    }
    if (result.player_tile_x == 0 && result.player_tile_y == 0) {
        result.player_tile_x = event->tile_x;
        result.player_tile_y = event->tile_y;
    }

    const std::function<void(const std::vector<PerspectiveEvent::Command>&, const std::string&)>
        execute_commands = [&](const std::vector<PerspectiveEvent::Command>& commands,
                               const std::string& branch_path) {
            for (const auto& command : commands) {
                Perspective2DEventExecutionStep step;
                step.code = command.code;
                step.argument = command.argument;
                step.branch_path = branch_path;
                step.condition_type = command.condition_type;
                step.condition_key = command.condition_key;
                step.condition_comparison = command.condition_comparison;
                step.condition_value = command.condition_value;

                if (command.code == "conditional_branch") {
                    std::string actual_value;
                    step.condition_matched =
                        condition_state_value(command.condition_type, command.condition_key, actual_value) &&
                        conditionValueMatches(actual_value, command.condition_comparison, command.condition_value);
                    result.executed_commands.push_back(std::move(step));
                    const bool matched = result.executed_commands.back().condition_matched;
                    const std::string child_path = branch_path == "root"
                                                       ? (matched ? "true" : "false")
                                                       : branch_path + (matched ? ".true" : ".false");
                    execute_commands(matched ? command.true_commands : command.false_commands, child_path);
                    continue;
                }

                result.executed_commands.push_back(std::move(step));
                const std::string argument = trimCopy(command.argument);
                if (command.code == "show_text") {
                    result.messages.push_back(argument);
                } else if (command.code == "transfer_player") {
                    const auto map_split = argument.find(':');
                    const auto comma_split = argument.find(',', map_split == std::string::npos ? 0 : map_split + 1);
                    int tile_x = result.player_tile_x;
                    int tile_y = result.player_tile_y;
                    if (map_split != std::string::npos && comma_split != std::string::npos &&
                        parseInt(trimCopy(argument.substr(map_split + 1, comma_split - map_split - 1)), tile_x) &&
                        parseInt(trimCopy(argument.substr(comma_split + 1)), tile_y)) {
                        result.player_map_id = trimCopy(argument.substr(0, map_split));
                        result.player_tile_x = tile_x;
                        result.player_tile_y = tile_y;
                    }
                } else if (command.code == "change_switch") {
                    const auto equals = argument.find('=');
                    if (equals != std::string::npos) {
                        const std::string key = trimCopy(argument.substr(0, equals));
                        const std::string value = trimCopy(argument.substr(equals + 1));
                        set_state_entry(result.switches, key, value);
                        sync_condition_value("switch", key, value);
                    }
                } else if (command.code == "change_variable") {
                    std::string key;
                    std::string op;
                    std::string value_text;
                    size_t op_pos = argument.find("+=");
                    if (op_pos != std::string::npos) {
                        op = "+=";
                    } else {
                        op_pos = argument.find("-=");
                        if (op_pos != std::string::npos) {
                            op = "-=";
                        } else {
                            op_pos = argument.find('=');
                            if (op_pos != std::string::npos) {
                                op = "=";
                            }
                        }
                    }
                    if (!op.empty()) {
                        key = trimCopy(argument.substr(0, op_pos));
                        value_text = trimCopy(argument.substr(op_pos + op.size()));
                        int delta = 0;
                        if (parseInt(value_text, delta)) {
                            int current = 0;
                            std::string current_text;
                            if (find_state_entry(result.variables, key, current_text)) {
                                parseInt(current_text, current);
                            }
                            const int next_value = op == "+=" ? current + delta : (op == "-=" ? current - delta : delta);
                            const std::string serialized_value = std::to_string(next_value);
                            set_state_entry(result.variables, key, serialized_value);
                            sync_condition_value("variable", key, serialized_value);
                        }
                    }
                } else if (command.code == "change_self_switch") {
                    const auto equals = argument.find('=');
                    if (equals != std::string::npos) {
                        const std::string key = event->event_id + ":" + trimCopy(argument.substr(0, equals));
                        const std::string value = trimCopy(argument.substr(equals + 1));
                        set_state_entry(result.self_switches, key, value);
                        sync_condition_value("self_switch", key, value);
                    }
                } else if (command.code == "change_gold") {
                    int amount = 0;
                    if (parseInt(argument, amount)) {
                        result.gold = std::max(0, result.gold + amount);
                    }
                } else if (command.code == "change_item") {
                    const auto separator = argument.find(':');
                    if (separator != std::string::npos) {
                        const std::string key = trimCopy(argument.substr(0, separator));
                        int delta = 0;
                        if (parseInt(trimCopy(argument.substr(separator + 1)), delta)) {
                            int current = 0;
                            std::string current_text;
                            if (find_state_entry(result.inventory, key, current_text)) {
                                parseInt(current_text, current);
                            }
                            set_state_entry(result.inventory, key, std::to_string(std::max(0, current + delta)));
                        }
                    }
                } else if (command.code == "move_route") {
                    for (const auto& route_step : splitString(argument, ',')) {
                        result.movement_route_steps.push_back(route_step);
                        const std::string normalized = lowerCopy(route_step);
                        if (normalized == "left") {
                            --result.player_tile_x;
                        } else if (normalized == "right") {
                            ++result.player_tile_x;
                        } else if (normalized == "up") {
                            --result.player_tile_y;
                        } else if (normalized == "down") {
                            ++result.player_tile_y;
                        }
                    }
                } else if (command.code == "call_common_event") {
                    result.common_events.push_back(argument);
                }
            }
        };

    execute_commands(active_page != nullptr ? active_page->commands : event->commands, "root");
    result.executed_command_count = result.executed_commands.size();
    result.success = true;
    result.message = "Perspective 2D runtime event executed.";

    perspective_runtime_switches_ = result.switches;
    perspective_runtime_variables_ = result.variables;
    perspective_runtime_self_switches_ = result.self_switches;
    perspective_runtime_inventory_ = result.inventory;
    perspective_runtime_gold_ = result.gold;
    perspective_runtime_player_map_id_ = result.player_map_id;
    perspective_runtime_player_tile_x_ = result.player_tile_x;
    perspective_runtime_player_tile_y_ = result.player_tile_y;

    nlohmann::json runtime_json;
    runtime_json["document_kind"] = "urpg.perspective_2d.runtime_state";
    runtime_json["version"] = 1;
    runtime_json["map_id"] = result.map_id;
    runtime_json["event_id"] = result.event_id;
    runtime_json["active_page_id"] = result.active_page_id;
    runtime_json["trigger_id"] = result.trigger_id;
    runtime_json["messages"] = result.messages;
    runtime_json["movement_route_steps"] = result.movement_route_steps;
    runtime_json["common_events"] = result.common_events;
    runtime_json["gold"] = result.gold;
    runtime_json["player"] = {{"map_id", result.player_map_id},
                              {"tile_x", result.player_tile_x},
                              {"tile_y", result.player_tile_y}};
    const auto append_entries = [](const std::vector<Perspective2DStateEntry>& entries) {
        nlohmann::json json = nlohmann::json::array();
        for (const auto& entry : entries) {
            json.push_back({{"key", entry.key}, {"value", entry.value}});
        }
        return json;
    };
    runtime_json["switches"] = append_entries(result.switches);
    runtime_json["variables"] = append_entries(result.variables);
    runtime_json["self_switches"] = append_entries(result.self_switches);
    runtime_json["inventory"] = append_entries(result.inventory);
    runtime_json["executed_commands"] = nlohmann::json::array();
    for (const auto& step : result.executed_commands) {
        nlohmann::json step_json = {{"code", step.code},
                                    {"argument", step.argument},
                                    {"branch_path", step.branch_path}};
        if (step.code == "conditional_branch") {
            step_json["condition"] = {{"type", step.condition_type},
                                      {"key", step.condition_key},
                                      {"comparison", step.condition_comparison},
                                      {"value", step.condition_value},
                                      {"matched", step.condition_matched}};
        }
        runtime_json["executed_commands"].push_back(std::move(step_json));
    }
    result.serialized_runtime_state_json = runtime_json.dump(2);

    last_perspective_runtime_result_ = result;
    captureRenderSnapshot();
    return last_perspective_runtime_result_;
}

SpatialAuthoringWorkspace::Perspective2DReleaseAssetGateResult
SpatialAuthoringWorkspace::RecordPerspectiveReleaseAssetGate(size_t release_required_asset_count,
                                                             size_t verified_release_required_asset_count,
                                                             size_t optional_lfs_asset_count,
                                                             size_t optional_lfs_deferred_count) {
    Perspective2DReleaseAssetGateResult result;
    result.release_required_asset_count = release_required_asset_count;
    result.verified_release_required_asset_count = verified_release_required_asset_count;
    result.optional_lfs_asset_count = optional_lfs_asset_count;
    result.optional_lfs_deferred_count = optional_lfs_deferred_count;
    if (verified_release_required_asset_count < release_required_asset_count) {
        result.blocker_codes.push_back("p2d_release_required_assets_unverified");
        result.policy_state = "release_required_assets_blocked";
        result.message = "Perspective 2D release-required assets are not verified.";
    } else {
        result.success = true;
        result.policy_state = optional_lfs_asset_count > 0
                                  ? "bounded_release_required_verified_optional_lfs_deferred"
                                  : "bounded_release_required_verified";
        result.message = "Perspective 2D release-required assets are verified.";
    }
    last_perspective_release_asset_gate_result_ = result;
    captureRenderSnapshot();
    return last_perspective_release_asset_gate_result_;
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
    last_render_snapshot_.perspective_2d_palette.search_text = palette_search_text_;
    last_render_snapshot_.perspective_2d_palette.filtered_tileset_id = palette_filter_tileset_id_;
    last_render_snapshot_.perspective_2d_palette.filtered_category_id = palette_filter_category_id_;
    last_render_snapshot_.perspective_2d_palette.brush_size = perspective_brush_size_;
    last_render_snapshot_.perspective_2d_palette.has_selected_tile =
        !selected_tileset_id_.empty() && !selected_tile_id_.empty();
    last_render_snapshot_.perspective_2d_palette.tile_options.clear();
    last_render_snapshot_.perspective_2d_palette.tile_option_count = perspective_tile_palette_options_.size();
    last_render_snapshot_.perspective_2d_palette.selected_option_visible = selected_palette_option_id_.empty();
    for (const auto& option : perspective_tile_palette_options_) {
        const bool matches_search = palette_search_text_.empty() ||
                                    containsCaseInsensitive(option.option_id, palette_search_text_) ||
                                    containsCaseInsensitive(option.label, palette_search_text_) ||
                                    containsCaseInsensitive(option.tileset_id, palette_search_text_) ||
                                    containsCaseInsensitive(option.tile_id, palette_search_text_) ||
                                    containsCaseInsensitive(option.asset_id, palette_search_text_) ||
                                    containsCaseInsensitive(option.project_path, palette_search_text_) ||
                                    containsCaseInsensitive(option.category_id, palette_search_text_);
        const bool matches_tileset =
            palette_filter_tileset_id_.empty() || option.tileset_id == palette_filter_tileset_id_;
        const bool matches_category =
            palette_filter_category_id_.empty() || option.category_id == palette_filter_category_id_;
        if (!matches_search || !matches_tileset || !matches_category) {
            continue;
        }
        Perspective2DPaletteOptionSnapshot option_snapshot;
        option_snapshot.option_id = option.option_id;
        option_snapshot.label = option.label;
        option_snapshot.tileset_id = option.tileset_id;
        option_snapshot.tile_id = option.tile_id;
        option_snapshot.asset_id = option.asset_id;
        option_snapshot.project_path = option.project_path;
        option_snapshot.category_id = option.category_id;
        option_snapshot.thumbnail_path = option.thumbnail_path;
        option_snapshot.selected = option.option_id == selected_palette_option_id_;
        if (option_snapshot.selected) {
            last_render_snapshot_.perspective_2d_palette.selected_option_visible = true;
        }
        last_render_snapshot_.perspective_2d_palette.tile_options.push_back(std::move(option_snapshot));
    }
    last_render_snapshot_.perspective_2d_palette.visible_tile_option_count =
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
        layer_snapshot.selected_for_bulk_edit = containsString(selected_perspective_layer_ids_, layer.id);
        layer_snapshot.tile_count = static_cast<size_t>(std::count_if(
            perspective_tiles_.begin(), perspective_tiles_.end(),
            [&](const PerspectiveTilePaint& tile) { return tile.layer_id == layer.id; }));
        layer_snapshot.event_count = static_cast<size_t>(std::count_if(
            perspective_events_.begin(), perspective_events_.end(),
            [&](const PerspectiveEvent& event) { return event.layer_id == layer.id; }));
        last_render_snapshot_.perspective_2d_layers.push_back(std::move(layer_snapshot));
    }
    last_render_snapshot_.perspective_2d_events.clear();
    const auto condition_matches = [&](const PerspectiveEvent::Condition& condition) {
        const auto value = std::find_if(perspective_event_condition_values_.begin(),
                                        perspective_event_condition_values_.end(),
                                        [&](const PerspectiveEventConditionValue& condition_value) {
                                            return condition_value.type == condition.type &&
                                                   condition_value.key == condition.key;
                                        });
        return value != perspective_event_condition_values_.end() &&
               conditionValueMatches(value->value, condition.comparison, condition.value);
    };
    const auto page_matches = [&](const PerspectiveEvent::Page& page) {
        return std::all_of(page.conditions.begin(), page.conditions.end(), condition_matches);
    };
    const auto active_page_for_event = [&](const PerspectiveEvent& event) -> const PerspectiveEvent::Page* {
        const PerspectiveEvent::Page* active_page = nullptr;
        for (const auto& page : event.pages) {
            if (page_matches(page)) {
                active_page = &page;
            }
        }
        if (active_page == nullptr && !event.pages.empty()) {
            active_page = &event.pages.front();
        }
        return active_page;
    };
    const std::function<std::vector<Perspective2DEventSnapshot::CommandSnapshot>(
        const std::vector<PerspectiveEvent::Command>&)> command_snapshots =
        [&](const std::vector<PerspectiveEvent::Command>& commands) {
            std::vector<Perspective2DEventSnapshot::CommandSnapshot> snapshots;
            for (const auto& command : commands) {
                Perspective2DEventSnapshot::CommandSnapshot command_snapshot;
                command_snapshot.code = command.code;
                command_snapshot.argument = command.argument;
                command_snapshot.condition_type = command.condition_type;
                command_snapshot.condition_key = command.condition_key;
                command_snapshot.condition_comparison = command.condition_comparison;
                command_snapshot.condition_value = command.condition_value;
                command_snapshot.true_commands = command_snapshots(command.true_commands);
                command_snapshot.false_commands = command_snapshots(command.false_commands);
                command_snapshot.true_command_count = command_snapshot.true_commands.size();
                command_snapshot.false_command_count = command_snapshot.false_commands.size();
                snapshots.push_back(std::move(command_snapshot));
            }
            return snapshots;
        };
    for (const auto& event : perspective_events_) {
        Perspective2DEventSnapshot event_snapshot;
        event_snapshot.event_id = event.event_id;
        event_snapshot.label = event.label;
        event_snapshot.trigger_id = event.trigger_id;
        event_snapshot.layer_id = event.layer_id;
        event_snapshot.tile_x = event.tile_x;
        event_snapshot.tile_y = event.tile_y;
        event_snapshot.selected_page_id = event.selected_page_id;
        const PerspectiveEvent::Page* active_page = active_page_for_event(event);
        if (active_page != nullptr) {
            event_snapshot.active_page_id = active_page->page_id;
            event_snapshot.trigger_id = active_page->trigger_id;
        }
        const auto& active_commands = active_page != nullptr ? active_page->commands : event.commands;
        event_snapshot.commands = command_snapshots(active_commands);
        event_snapshot.command_count = event_snapshot.commands.size();
        for (const auto& page : event.pages) {
            Perspective2DEventSnapshot::PageSnapshot page_snapshot;
            page_snapshot.page_id = page.page_id;
            page_snapshot.label = page.label;
            page_snapshot.trigger_id = page.trigger_id;
            page_snapshot.order = page.order;
            page_snapshot.selected = page.page_id == event.selected_page_id;
            page_snapshot.active_in_playtest = active_page != nullptr && active_page->page_id == page.page_id;
            for (const auto& condition : page.conditions) {
                page_snapshot.conditions.push_back(
                    {condition.type, condition.key, condition.comparison, condition.value});
            }
            page_snapshot.condition_count = page_snapshot.conditions.size();
            page_snapshot.commands = command_snapshots(page.commands);
            page_snapshot.command_count = page_snapshot.commands.size();
            event_snapshot.pages.push_back(std::move(page_snapshot));
        }
        event_snapshot.page_count = event_snapshot.pages.size();
        const auto layer = std::find_if(perspective_layers_.begin(), perspective_layers_.end(),
                                        [&](const PerspectiveLayer& candidate) {
                                            return candidate.id == event.layer_id;
                                        });
        event_snapshot.visible_in_playtest =
            layer != perspective_layers_.end() && layer->visible && !layer->locked;
        last_render_snapshot_.perspective_2d_events.push_back(std::move(event_snapshot));
    }
    last_render_snapshot_.perspective_2d_ui.map_canvas_visible =
        last_render_snapshot_.visible && last_render_snapshot_.has_target_scene && last_render_snapshot_.has_target_overlay;
    last_render_snapshot_.perspective_2d_ui.layer_panel_visible = last_render_snapshot_.has_target_overlay;
    last_render_snapshot_.perspective_2d_ui.tile_palette_visible = last_render_snapshot_.has_target_overlay;
    last_render_snapshot_.perspective_2d_ui.event_page_tabs_visible = !perspective_events_.empty();
    last_render_snapshot_.perspective_2d_ui.condition_editor_visible = !perspective_events_.empty();
    last_render_snapshot_.perspective_2d_ui.command_list_visible = !perspective_events_.empty();
    last_render_snapshot_.perspective_2d_ui.command_picker_visible = last_render_snapshot_.has_target_overlay;
    last_render_snapshot_.perspective_2d_ui.playtest_controls_visible = last_render_snapshot_.has_target_overlay;
    last_render_snapshot_.perspective_2d_ui.export_controls_visible = last_render_snapshot_.has_target_overlay;
    last_render_snapshot_.perspective_2d_ui.visible_layer_count = static_cast<size_t>(std::count_if(
        perspective_layers_.begin(), perspective_layers_.end(), [](const PerspectiveLayer& layer) {
            return layer.visible;
        }));
    last_render_snapshot_.perspective_2d_ui.visible_event_count = static_cast<size_t>(std::count_if(
        last_render_snapshot_.perspective_2d_events.begin(),
        last_render_snapshot_.perspective_2d_events.end(),
        [](const Perspective2DEventSnapshot& event) {
            return event.visible_in_playtest;
        }));
    last_render_snapshot_.perspective_2d_ui.command_picker_options = {
        "show_text",
        "transfer_player",
        "change_switch",
        "change_variable",
        "change_self_switch",
        "change_gold",
        "change_item",
        "move_route",
        "call_common_event",
        "conditional_branch",
    };
    const auto has_branch_command = [&](const auto& self, const std::vector<PerspectiveEvent::Command>& commands) -> bool {
        return std::any_of(commands.begin(), commands.end(), [&](const PerspectiveEvent::Command& command) {
            return command.code == "conditional_branch" || self(self, command.true_commands) ||
                   self(self, command.false_commands);
        });
    };
    if (!perspective_events_.empty()) {
        const PerspectiveEvent& selected_event = perspective_events_.front();
        last_render_snapshot_.perspective_2d_ui.selected_event_id = selected_event.event_id;
        const PerspectiveEvent::Page* selected_page = nullptr;
        if (!selected_event.selected_page_id.empty()) {
            const auto found = std::find_if(selected_event.pages.begin(),
                                            selected_event.pages.end(),
                                            [&](const PerspectiveEvent::Page& page) {
                                                return page.page_id == selected_event.selected_page_id;
                                            });
            if (found != selected_event.pages.end()) {
                selected_page = &(*found);
            }
        }
        if (selected_page == nullptr && !selected_event.pages.empty()) {
            selected_page = &selected_event.pages.front();
        }
        if (selected_page != nullptr) {
            last_render_snapshot_.perspective_2d_ui.selected_page_id = selected_page->page_id;
            last_render_snapshot_.perspective_2d_ui.page_tab_count = selected_event.pages.size();
            last_render_snapshot_.perspective_2d_ui.command_row_count = selected_page->commands.size();
            last_render_snapshot_.perspective_2d_ui.branch_tree_visible =
                has_branch_command(has_branch_command, selected_page->commands);
        } else {
            last_render_snapshot_.perspective_2d_ui.command_row_count = selected_event.commands.size();
            last_render_snapshot_.perspective_2d_ui.branch_tree_visible =
                has_branch_command(has_branch_command, selected_event.commands);
        }
    }
    last_render_snapshot_.perspective_2d_project.map_id =
        m_target_overlay != nullptr ? m_target_overlay->mapId : std::string{};
    last_render_snapshot_.perspective_2d_project.width =
        m_target_overlay != nullptr ? m_target_overlay->elevation.width : 0;
    last_render_snapshot_.perspective_2d_project.height =
        m_target_overlay != nullptr ? m_target_overlay->elevation.height : 0;
    last_render_snapshot_.perspective_2d_project.selected_layer_id = selected_perspective_layer_id_;
    last_render_snapshot_.perspective_2d_project.selected_layer_count = selected_perspective_layer_ids_.size();
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
    last_render_snapshot_.perspective_2d_project.layer_workflow_ready =
        std::any_of(perspective_layers_.begin(), perspective_layers_.end(), [](const PerspectiveLayer& layer) {
            return layer.kind == "tile" && layer.visible && !layer.locked;
        }) && std::any_of(perspective_layers_.begin(), perspective_layers_.end(), [](const PerspectiveLayer& layer) {
            return layer.kind == "event" && layer.visible && !layer.locked;
        });
    last_render_snapshot_.perspective_2d_project.palette_workflow_ready =
        !selected_tileset_id_.empty() && !selected_tile_id_.empty();
    last_render_snapshot_.perspective_2d_project.event_workflow_ready =
        std::any_of(perspective_events_.begin(), perspective_events_.end(), [](const PerspectiveEvent& event) {
            return !event.pages.empty();
        });
    last_render_snapshot_.perspective_2d_project.playtest_workflow_ready =
        last_perspective_playtest_result_.success && last_perspective_playtest_result_.runtime_event_execution_trace_count ==
                                                       perspective_events_.size();
    last_render_snapshot_.perspective_2d_project.export_workflow_ready =
        last_perspective_export_result_.success && !last_perspective_export_result_.package_signature.empty();
    last_render_snapshot_.perspective_2d_project.release_asset_gate_ready =
        last_perspective_release_asset_gate_result_.success;
    last_render_snapshot_.perspective_2d_project.creator_workflow_ready =
        last_render_snapshot_.perspective_2d_project.layer_workflow_ready &&
        last_render_snapshot_.perspective_2d_project.palette_workflow_ready &&
        last_render_snapshot_.perspective_2d_project.event_workflow_ready &&
        last_render_snapshot_.perspective_2d_project.playtest_workflow_ready &&
        last_render_snapshot_.perspective_2d_project.export_workflow_ready &&
        last_render_snapshot_.perspective_2d_project.release_asset_gate_ready;
    if (last_render_snapshot_.perspective_2d_project.creator_workflow_ready) {
        last_render_snapshot_.perspective_2d_project.creator_next_step = "Ready to playtest and export.";
    } else if (!last_render_snapshot_.perspective_2d_project.layer_workflow_ready) {
        last_render_snapshot_.perspective_2d_project.creator_next_step = "Create visible unlocked tile and event layers.";
    } else if (!last_render_snapshot_.perspective_2d_project.palette_workflow_ready) {
        last_render_snapshot_.perspective_2d_project.creator_next_step = "Select a tile palette option.";
    } else if (!last_render_snapshot_.perspective_2d_project.event_workflow_ready) {
        last_render_snapshot_.perspective_2d_project.creator_next_step = "Add an event page.";
    } else if (!last_render_snapshot_.perspective_2d_project.release_asset_gate_ready) {
        last_render_snapshot_.perspective_2d_project.creator_next_step = "Record release asset gate evidence.";
    } else if (!last_render_snapshot_.perspective_2d_project.playtest_workflow_ready) {
        last_render_snapshot_.perspective_2d_project.creator_next_step = "Run Perspective 2D playtest.";
    } else {
        last_render_snapshot_.perspective_2d_project.creator_next_step = "Export Perspective 2D package.";
    }
    last_render_snapshot_.last_perspective_2d_save = last_perspective_save_result_;
    last_render_snapshot_.last_perspective_2d_load = last_perspective_load_result_;
    last_render_snapshot_.last_perspective_2d_playtest = last_perspective_playtest_result_;
    last_render_snapshot_.last_perspective_2d_export = last_perspective_export_result_;
    last_render_snapshot_.last_perspective_2d_event_execution = last_perspective_event_execution_result_;
    last_render_snapshot_.last_perspective_2d_runtime = last_perspective_runtime_result_;
    last_render_snapshot_.last_perspective_2d_release_asset_gate = last_perspective_release_asset_gate_result_;
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
