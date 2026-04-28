#pragma once

#include "engine/core/render/raycast_renderer.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
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

struct Dungeon3DMinimapTile {
    int32_t x = 0;
    int32_t y = 0;
    bool blocking = false;
    bool discovered = false;
    bool current = false;
    std::string event_id;
};

struct Dungeon3DPreview {
    std::string mode;
    std::vector<RaycastRenderer::CastResult> columns;
    std::vector<Dungeon3DMinimapTile> minimap_tiles;
    std::vector<std::string> runtime_commands;
    std::vector<Dungeon3DDiagnostic> diagnostics;
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
    [[nodiscard]] nlohmann::json toJson() const;

    static Dungeon3DWorldDocument fromJson(const nlohmann::json& json);
};

nlohmann::json dungeon3DPreviewToJson(const Dungeon3DPreview& preview);

} // namespace urpg::render
