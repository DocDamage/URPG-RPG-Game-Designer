#pragma once

#include "engine/core/map/map_environment_preview.h"

#include <cstddef>
#include <string>

namespace urpg::editor {

struct MapEnvironmentPreviewPanelSnapshot {
    bool visible = true;
    bool rendered = false;
    bool disabled = true;
    int32_t selected_x = 0;
    int32_t selected_y = 0;
    std::string map_id;
    std::string region_id;
    std::string weather;
    size_t runtime_command_count = 0;
    size_t runtime_overlay_command_count = 0;
    size_t visible_tile_layer_count = 0;
    size_t collision_tile_count = 0;
    size_t region_overlay_count = 0;
    size_t tactical_reachable_count = 0;
    size_t spawn_entry_count = 0;
    size_t diagnostic_count = 0;
    float overlay_density = 0.0f;
    bool selected_tile_blocked = false;
    std::string ux_focus_lane = "map_layers";
    std::string selected_tile_summary;
    std::string primary_action = "Load a map environment preview.";
    std::string saved_project_json;
    std::string status_message = "Load a map environment preview before rendering this panel.";
};

class MapEnvironmentPreviewPanel {
public:
    void loadDocument(urpg::map::MapEnvironmentPreviewDocument document);
    void selectTile(int32_t x, int32_t y);
    void render();

    const MapEnvironmentPreviewPanelSnapshot& snapshot() const { return snapshot_; }
    const urpg::map::MapEnvironmentPreviewResult& preview() const { return preview_; }
    bool hasRenderedFrame() const { return snapshot_.rendered; }

private:
    void refreshPreview();

    urpg::map::MapEnvironmentPreviewDocument document_;
    urpg::map::MapEnvironmentPreviewResult preview_;
    MapEnvironmentPreviewPanelSnapshot snapshot_{};
    bool loaded_ = false;
};

} // namespace urpg::editor
