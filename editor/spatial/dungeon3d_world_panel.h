#pragma once

#include "engine/core/render/dungeon3d_world.h"

#include <nlohmann/json.hpp>

#include <string>

namespace urpg::editor {

struct Dungeon3DWorldPanelSnapshot {
    bool visible = true;
    bool disabled = true;
    std::string map_id;
    std::string mode;
    size_t raycast_column_count = 0;
    size_t minimap_tile_count = 0;
    size_t discovered_tile_count = 0;
    size_t visible_minimap_tile_count = 0;
    size_t blocking_cell_count = 0;
    size_t event_cell_count = 0;
    size_t runtime_command_count = 0;
    size_t diagnostic_count = 0;
    float camera_x = 0.0f;
    float camera_y = 0.0f;
    float average_wall_distance = 0.0f;
    std::string facing_event_id;
    std::string facing_material_id;
    bool facing_blocked = false;
    std::string status_message = "Load a 3D dungeon world document before rendering this panel.";
};

class Dungeon3DWorldPanel {
public:
    void loadDocument(urpg::render::Dungeon3DWorldDocument document);
    void setMode(std::string mode);
    urpg::render::Dungeon3DNavigationResult moveForward(float distance = 1.0f);
    urpg::render::Dungeon3DNavigationResult strafe(float distance);
    void rotate(float radians);
    void render();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const urpg::render::Dungeon3DPreview& preview() const { return preview_; }
    [[nodiscard]] const Dungeon3DWorldPanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    urpg::render::Dungeon3DWorldDocument document_;
    urpg::render::Dungeon3DPreview preview_;
    Dungeon3DWorldPanelSnapshot snapshot_;
    bool loaded_ = false;
};

} // namespace urpg::editor
