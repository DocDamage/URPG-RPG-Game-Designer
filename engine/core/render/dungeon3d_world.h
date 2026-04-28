#pragma once

#include "engine/core/render/raycast_renderer.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace urpg::render {

struct Dungeon3DDiagnostic {
    std::string code;
    std::string message;
};

struct Dungeon3DMaterial {
    std::string id;
    std::string wall_texture;
    std::string floor_texture;
    std::string ceiling_texture;
    float light = 1.0f;
    std::string footstep_sound;
};

struct Dungeon3DCell {
    std::string tile_id;
    std::string material_id;
    std::string event_id;
    bool blocking = false;
    bool dark_zone = false;
    bool stairs_up = false;
    bool stairs_down = false;
};

struct Dungeon3DCamera {
    float pos_x = 1.5f;
    float pos_y = 1.5f;
    float dir_x = 1.0f;
    float dir_y = 0.0f;
    float plane_x = 0.0f;
    float plane_y = 0.66f;
};

struct Dungeon3DColumn {
    int32_t x = 0;
    int32_t map_x = 0;
    int32_t map_y = 0;
    int32_t side = 0;
    float wall_dist = 0.0f;
    std::string material_id;
    std::string wall_texture;
    float shade = 1.0f;
    int32_t projected_wall_height = 0;
};

struct Dungeon3DMinimapTile {
    int32_t x = 0;
    int32_t y = 0;
    bool blocking = false;
    bool discovered = false;
    bool current = false;
    bool visible = false;
    std::string event_id;
};

struct Dungeon3DInteraction {
    int32_t x = 0;
    int32_t y = 0;
    bool blocking = false;
    bool has_event = false;
    bool stairs_up = false;
    bool stairs_down = false;
    bool dark_zone = false;
    std::string event_id;
    std::string material_id;
};

struct Dungeon3DNavigationResult {
    bool moved = false;
    bool blocked = false;
    std::string command;
    Dungeon3DDiagnostic diagnostic;
};

struct Dungeon3DPreview {
    std::string mode;
    std::vector<Dungeon3DColumn> columns;
    std::vector<Dungeon3DMinimapTile> minimap_tiles;
    std::vector<std::string> runtime_commands;
    std::vector<Dungeon3DDiagnostic> diagnostics;
    std::optional<Dungeon3DInteraction> facing_interaction;
    int32_t blocking_cell_count = 0;
    int32_t event_cell_count = 0;
    float average_wall_distance = 0.0f;
};

class Dungeon3DWorldDocument {
public:
    std::string schema_version = "urpg.dungeon3d_world.v1";
    std::string id;
    std::string map_id;
    int32_t width = 0;
    int32_t height = 0;
    int32_t screen_width = 160;
    int32_t screen_height = 120;
    bool minimap_enabled = true;
    bool auto_mapping = true;
    std::string mode = "3d";
    Dungeon3DCamera camera;
    std::map<std::string, Dungeon3DMaterial> materials;
    std::vector<Dungeon3DCell> cells;

    [[nodiscard]] std::vector<Dungeon3DDiagnostic> validate() const;
    [[nodiscard]] Dungeon3DPreview preview() const;
    [[nodiscard]] Dungeon3DPreview switchMode(std::string next_mode);
    [[nodiscard]] Dungeon3DNavigationResult moveForward(float distance = 1.0f);
    [[nodiscard]] Dungeon3DNavigationResult strafe(float distance);
    void rotate(float radians);
    [[nodiscard]] nlohmann::json toJson() const;

    static Dungeon3DWorldDocument fromJson(const nlohmann::json& json);
};

nlohmann::json dungeon3DPreviewToJson(const Dungeon3DPreview& preview);

} // namespace urpg::render
