#include "editor/spatial/map_environment_preview_panel.h"

#include <utility>

namespace urpg::editor {

void MapEnvironmentPreviewPanel::loadDocument(urpg::map::MapEnvironmentPreviewDocument document) {
    document_ = std::move(document);
    loaded_ = true;
    refreshPreview();
}

void MapEnvironmentPreviewPanel::selectTile(int32_t x, int32_t y) {
    snapshot_.selected_x = x;
    snapshot_.selected_y = y;
    if (loaded_) {
        refreshPreview();
    }
}

void MapEnvironmentPreviewPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load a map environment preview before rendering this panel.";
        return;
    }

    refreshPreview();
}

void MapEnvironmentPreviewPanel::refreshPreview() {
    preview_ = urpg::map::PreviewMapEnvironment(document_, snapshot_.selected_x, snapshot_.selected_y);
    snapshot_.disabled = false;
    snapshot_.map_id = document_.map_id;
    snapshot_.region_id = preview_.region ? preview_.region->id : "";
    snapshot_.weather = preview_.region ? preview_.region->weather : document_.base_weather;
    snapshot_.runtime_command_count = preview_.runtime_intent.commands.size();
    snapshot_.runtime_overlay_command_count = preview_.runtime_overlay_commands.size();
    snapshot_.visible_tile_layer_count = preview_.visible_tile_layer_count;
    snapshot_.collision_tile_count = preview_.collision_tile_count;
    snapshot_.region_overlay_count = preview_.region_overlay_count;
    snapshot_.tactical_reachable_count = preview_.tactical_reachable_count;
    snapshot_.spawn_entry_count = preview_.spawn_entry_count;
    snapshot_.diagnostic_count = preview_.diagnostics.size();
    snapshot_.selected_tile_blocked = preview_.selected_tile_blocked;
    const auto overlay_count = snapshot_.region_overlay_count + snapshot_.tactical_reachable_count +
                               snapshot_.spawn_entry_count;
    snapshot_.overlay_density = snapshot_.visible_tile_layer_count == 0
        ? 0.0f
        : static_cast<float>(overlay_count) / static_cast<float>(snapshot_.visible_tile_layer_count);
    snapshot_.selected_tile_summary = snapshot_.region_id.empty()
        ? "No region/weather rule on selected tile."
        : snapshot_.region_id + ":" + snapshot_.weather;
    if (snapshot_.diagnostic_count > 0) {
        snapshot_.ux_focus_lane = "diagnostics";
        snapshot_.primary_action = "Resolve map preview diagnostics before shipping this area.";
    } else if (snapshot_.selected_tile_blocked) {
        snapshot_.ux_focus_lane = "collision";
        snapshot_.primary_action = "Inspect passability and collision overlays for the selected tile.";
    } else if (snapshot_.spawn_entry_count > 0) {
        snapshot_.ux_focus_lane = "spawns";
        snapshot_.primary_action = "Tune spawn-table weights against the selected region preview.";
    } else {
        snapshot_.ux_focus_lane = "lighting_weather";
        snapshot_.primary_action = "Preview lighting, weather, and region overlays on the selected tile.";
    }
    snapshot_.saved_project_json = document_.toJson().dump();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Map environment preview is ready." : "Map environment preview has diagnostics.";
}

} // namespace urpg::editor
