#include "engine/core/render/dungeon3d_world.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

namespace urpg::render {

namespace {

Dungeon3DMaterial materialFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("wall_texture", ""),
            json.value("floor_texture", ""),
            json.value("ceiling_texture", "")};
}

nlohmann::json materialToJson(const Dungeon3DMaterial& material) {
    return {{"id", material.id},
            {"wall_texture", material.wall_texture},
            {"floor_texture", material.floor_texture},
            {"ceiling_texture", material.ceiling_texture}};
}

Dungeon3DCell cellFromJson(const nlohmann::json& json) {
    return {json.value("tile_id", ""),
            json.value("material_id", ""),
            json.value("event_id", ""),
            json.value("blocking", false),
            json.value("dark_zone", false),
            json.value("stairs_up", false),
            json.value("stairs_down", false)};
}

nlohmann::json cellToJson(const Dungeon3DCell& cell) {
    return {{"tile_id", cell.tile_id},
            {"material_id", cell.material_id},
            {"event_id", cell.event_id},
            {"blocking", cell.blocking},
            {"dark_zone", cell.dark_zone},
            {"stairs_up", cell.stairs_up},
            {"stairs_down", cell.stairs_down}};
}

RaycastRenderer::AuthoringAdapter buildAdapter(const Dungeon3DWorldDocument& document) {
    RaycastRenderer::AuthoringAdapter adapter;
    adapter.mapId = document.map_id;
    adapter.width = document.width;
    adapter.height = document.height;
    adapter.blockingCells.assign(static_cast<size_t>(std::max(0, document.width * document.height)), 0);
    for (size_t i = 0; i < document.cells.size() && i < adapter.blockingCells.size(); ++i) {
        adapter.blockingCells[i] = document.cells[i].blocking ? 1 : 0;
    }
    return adapter;
}

bool is3DMode(const std::string& mode) {
    return mode == "3d" || mode == "spatial";
}

std::string commandForMode(const Dungeon3DWorldDocument& document) {
    return is3DMode(document.mode) ? "switch_to_3d:" + document.map_id : "switch_to_2d:" + document.map_id;
}

} // namespace

std::vector<Dungeon3DDiagnostic> Dungeon3DWorldDocument::validate() const {
    std::vector<Dungeon3DDiagnostic> diagnostics;
    if (schema_version != "urpg.dungeon3d_world.v1") {
        diagnostics.push_back({"invalid_schema_version", "3D dungeon world document has an unsupported schema version."});
    }
    if (id.empty()) {
        diagnostics.push_back({"missing_id", "3D dungeon world document is missing an id."});
    }
    if (map_id.empty()) {
        diagnostics.push_back({"missing_map_id", "3D dungeon world document is missing a map id."});
    }
    if (width <= 0 || height <= 0) {
        diagnostics.push_back({"invalid_dimensions", "3D dungeon world map dimensions must be positive."});
    }
    if (screen_width <= 0 || screen_height <= 0) {
        diagnostics.push_back({"invalid_screen_size", "3D dungeon preview screen size must be positive."});
    }
    const auto expected_cells = static_cast<size_t>(std::max(0, width * height));
    if (cells.size() != expected_cells) {
        diagnostics.push_back({"cell_count_mismatch", "3D dungeon world cell count does not match width times height."});
    }
    if (!is3DMode(mode) && mode != "2d") {
        diagnostics.push_back({"invalid_mode", "3D dungeon world mode must be 2d or 3d."});
    }

    const auto spawn_x = static_cast<int32_t>(std::floor(camera.pos_x));
    const auto spawn_y = static_cast<int32_t>(std::floor(camera.pos_y));
    if (spawn_x < 0 || spawn_y < 0 || spawn_x >= width || spawn_y >= height) {
        diagnostics.push_back({"camera_out_of_bounds", "3D dungeon camera starts outside the authored map."});
    } else {
        const auto index = static_cast<size_t>(spawn_y * width + spawn_x);
        if (index < cells.size() && cells[index].blocking) {
            diagnostics.push_back({"camera_inside_blocking_cell", "3D dungeon camera starts inside a blocking cell."});
        }
    }

    std::set<std::string> material_ids;
    for (const auto& [id_key, material] : materials) {
        if (id_key.empty() || material.id.empty()) {
            diagnostics.push_back({"missing_material_id", "3D dungeon material is missing an id."});
        }
        if (material.wall_texture.empty() || material.floor_texture.empty() || material.ceiling_texture.empty()) {
            diagnostics.push_back({"incomplete_material", "3D dungeon material must define wall, floor, and ceiling textures."});
        }
        material_ids.insert(id_key);
    }
    for (const auto& cell : cells) {
        if (!cell.material_id.empty() && !material_ids.contains(cell.material_id)) {
            diagnostics.push_back({"unknown_cell_material", "3D dungeon cell references an unknown material: " + cell.material_id});
            break;
        }
    }
    return diagnostics;
}

Dungeon3DPreview Dungeon3DWorldDocument::preview() const {
    Dungeon3DPreview result;
    result.mode = mode;
    result.diagnostics = validate();
    result.runtime_commands.push_back(commandForMode(*this));

    const auto adapter = buildAdapter(*this);
    if (is3DMode(mode) && result.diagnostics.empty()) {
        RaycastRenderer::Config config;
        config.screenWidth = screen_width;
        config.screenHeight = screen_height;
        config.presentationMode = presentation::PresentationMode::Spatial;

        RaycastRenderer::Camera ray_camera{camera.pos_x, camera.pos_y, camera.dir_x, camera.dir_y, camera.plane_x, camera.plane_y};
        result.columns = RaycastRenderer::castFrame(ray_camera, config, [&](int32_t x, int32_t y) {
            return adapter.isBlocking(x, y);
        });
        result.runtime_commands.push_back("raycast_frame:" + map_id);
    }

    if (minimap_enabled && width > 0 && height > 0) {
        std::set<size_t> discovered;
        const auto current_x = static_cast<int32_t>(std::floor(camera.pos_x));
        const auto current_y = static_cast<int32_t>(std::floor(camera.pos_y));
        if (current_x >= 0 && current_y >= 0 && current_x < width && current_y < height) {
            discovered.insert(static_cast<size_t>(current_y * width + current_x));
        }
        if (auto_mapping) {
            for (const auto& column : result.columns) {
                if (column.mapX >= 0 && column.mapY >= 0 && column.mapX < width && column.mapY < height) {
                    discovered.insert(static_cast<size_t>(column.mapY * width + column.mapX));
                }
            }
        }
        for (int32_t y = 0; y < height; ++y) {
            for (int32_t x = 0; x < width; ++x) {
                const auto index = static_cast<size_t>(y * width + x);
                if (index >= cells.size()) {
                    continue;
                }
                result.minimap_tiles.push_back(
                    {x, y, cells[index].blocking, discovered.contains(index), x == current_x && y == current_y, cells[index].event_id});
            }
        }
        result.runtime_commands.push_back(auto_mapping ? "update_automap:" + map_id : "render_minimap:" + map_id);
    }

    return result;
}

Dungeon3DPreview Dungeon3DWorldDocument::switchMode(std::string next_mode) {
    mode = std::move(next_mode);
    return preview();
}

nlohmann::json Dungeon3DWorldDocument::toJson() const {
    nlohmann::json json{{"schema_version", schema_version},
                        {"id", id},
                        {"map_id", map_id},
                        {"width", width},
                        {"height", height},
                        {"screen_width", screen_width},
                        {"screen_height", screen_height},
                        {"minimap_enabled", minimap_enabled},
                        {"auto_mapping", auto_mapping},
                        {"mode", mode},
                        {"camera",
                         {{"pos_x", camera.pos_x},
                          {"pos_y", camera.pos_y},
                          {"dir_x", camera.dir_x},
                          {"dir_y", camera.dir_y},
                          {"plane_x", camera.plane_x},
                          {"plane_y", camera.plane_y}}}};
    json["materials"] = nlohmann::json::array();
    for (const auto& [_, material] : materials) {
        json["materials"].push_back(materialToJson(material));
    }
    json["cells"] = nlohmann::json::array();
    for (const auto& cell : cells) {
        json["cells"].push_back(cellToJson(cell));
    }
    return json;
}

Dungeon3DWorldDocument Dungeon3DWorldDocument::fromJson(const nlohmann::json& json) {
    Dungeon3DWorldDocument document;
    document.schema_version = json.value("schema_version", "urpg.dungeon3d_world.v1");
    document.id = json.value("id", "");
    document.map_id = json.value("map_id", "");
    document.width = json.value("width", 0);
    document.height = json.value("height", 0);
    document.screen_width = json.value("screen_width", 160);
    document.screen_height = json.value("screen_height", 120);
    document.minimap_enabled = json.value("minimap_enabled", true);
    document.auto_mapping = json.value("auto_mapping", true);
    document.mode = json.value("mode", "3d");
    if (json.contains("camera")) {
        const auto& camera_json = json.at("camera");
        document.camera.pos_x = camera_json.value("pos_x", 1.5f);
        document.camera.pos_y = camera_json.value("pos_y", 1.5f);
        document.camera.dir_x = camera_json.value("dir_x", 1.0f);
        document.camera.dir_y = camera_json.value("dir_y", 0.0f);
        document.camera.plane_x = camera_json.value("plane_x", 0.0f);
        document.camera.plane_y = camera_json.value("plane_y", 0.66f);
    }
    for (const auto& material_json : json.value("materials", nlohmann::json::array())) {
        auto material = materialFromJson(material_json);
        document.materials[material.id] = std::move(material);
    }
    for (const auto& cell_json : json.value("cells", nlohmann::json::array())) {
        document.cells.push_back(cellFromJson(cell_json));
    }
    return document;
}

nlohmann::json dungeon3DPreviewToJson(const Dungeon3DPreview& preview) {
    nlohmann::json json{{"mode", preview.mode}, {"runtime_commands", preview.runtime_commands}};
    json["columns"] = nlohmann::json::array();
    for (const auto& column : preview.columns) {
        json["columns"].push_back({{"x", column.x},
                                   {"map_x", column.mapX},
                                   {"map_y", column.mapY},
                                   {"side", column.side},
                                   {"wall_dist", column.wallDist}});
    }
    json["minimap_tiles"] = nlohmann::json::array();
    for (const auto& tile : preview.minimap_tiles) {
        json["minimap_tiles"].push_back({{"x", tile.x},
                                         {"y", tile.y},
                                         {"blocking", tile.blocking},
                                         {"discovered", tile.discovered},
                                         {"current", tile.current},
                                         {"event_id", tile.event_id}});
    }
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}});
    }
    return json;
}

} // namespace urpg::render
