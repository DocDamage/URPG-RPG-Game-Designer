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
            json.value("ceiling_texture", ""),
            json.value("light", 1.0f),
            json.value("footstep_sound", "")};
}

Dungeon3DFloor floorFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("display_name", ""),
            json.value("depth", 0),
            json.value("ambient_sound", "")};
}

Dungeon3DMapMarker markerFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("label", ""),
            json.value("type", ""),
            json.value("x", 0),
            json.value("y", 0),
            json.value("floor_id", ""),
            json.value("visible", true),
            json.value("completed", false)};
}

Dungeon3DMapNote noteFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("text", ""),
            json.value("x", 0),
            json.value("y", 0),
            json.value("floor_id", "")};
}

nlohmann::json materialToJson(const Dungeon3DMaterial& material) {
    return {{"id", material.id},
            {"wall_texture", material.wall_texture},
            {"floor_texture", material.floor_texture},
            {"ceiling_texture", material.ceiling_texture},
            {"light", material.light},
            {"footstep_sound", material.footstep_sound}};
}

nlohmann::json floorToJson(const Dungeon3DFloor& floor) {
    return {{"id", floor.id},
            {"display_name", floor.display_name},
            {"depth", floor.depth},
            {"ambient_sound", floor.ambient_sound}};
}

nlohmann::json markerToJson(const Dungeon3DMapMarker& marker) {
    return {{"id", marker.id},
            {"label", marker.label},
            {"type", marker.type},
            {"x", marker.x},
            {"y", marker.y},
            {"floor_id", marker.floor_id},
            {"visible", marker.visible},
            {"completed", marker.completed}};
}

nlohmann::json noteToJson(const Dungeon3DMapNote& note) {
    return {{"id", note.id},
            {"text", note.text},
            {"x", note.x},
            {"y", note.y},
            {"floor_id", note.floor_id}};
}

Dungeon3DCell cellFromJson(const nlohmann::json& json) {
    return {json.value("tile_id", ""),
            json.value("material_id", ""),
            json.value("event_id", ""),
            json.value("door_id", ""),
            json.value("required_item", ""),
            json.value("encounter_tag", ""),
            json.value("floor_transfer", ""),
            json.value("blocking", false),
            json.value("dark_zone", false),
            json.value("stairs_up", false),
            json.value("stairs_down", false),
            json.value("secret", false),
            json.value("locked", false)};
}

nlohmann::json cellToJson(const Dungeon3DCell& cell) {
    return {{"tile_id", cell.tile_id},
            {"material_id", cell.material_id},
            {"event_id", cell.event_id},
            {"door_id", cell.door_id},
            {"required_item", cell.required_item},
            {"encounter_tag", cell.encounter_tag},
            {"floor_transfer", cell.floor_transfer},
            {"blocking", cell.blocking},
            {"dark_zone", cell.dark_zone},
            {"stairs_up", cell.stairs_up},
            {"stairs_down", cell.stairs_down},
            {"secret", cell.secret},
            {"locked", cell.locked}};
}

std::string cellKey(int32_t x, int32_t y) {
    return std::to_string(x) + "," + std::to_string(y);
}

nlohmann::json sessionToJson(const Dungeon3DSessionState& session) {
    return {{"inventory", session.inventory},
            {"discovered_cells", session.discovered_cells},
            {"opened_doors", session.opened_doors},
            {"revealed_secrets", session.revealed_secrets},
            {"triggered_encounters", session.triggered_encounters},
            {"completed_markers", session.completed_markers},
            {"current_floor_id", session.current_floor_id},
            {"event_log", session.event_log}};
}

Dungeon3DSessionState sessionFromJson(const nlohmann::json& json) {
    Dungeon3DSessionState session;
    session.inventory = json.value("inventory", std::set<std::string>{});
    session.discovered_cells = json.value("discovered_cells", std::set<std::string>{});
    session.opened_doors = json.value("opened_doors", std::set<std::string>{});
    session.revealed_secrets = json.value("revealed_secrets", std::set<std::string>{});
    session.triggered_encounters = json.value("triggered_encounters", std::set<std::string>{});
    session.completed_markers = json.value("completed_markers", std::set<std::string>{});
    session.current_floor_id = json.value("current_floor_id", "");
    session.event_log = json.value("event_log", std::vector<std::string>{});
    return session;
}

RaycastRenderer::AuthoringAdapter buildAdapter(const Dungeon3DWorldDocument& document) {
    RaycastRenderer::AuthoringAdapter adapter;
    adapter.mapId = document.map_id;
    adapter.width = document.width;
    adapter.height = document.height;
    adapter.blockingCells.assign(static_cast<size_t>(std::max(0, document.width * document.height)), 0);
    for (size_t i = 0; i < document.cells.size() && i < adapter.blockingCells.size(); ++i) {
        const auto& cell = document.cells[i];
        const bool opened = !cell.door_id.empty() && document.session.opened_doors.contains(cell.door_id);
        const bool revealed = cell.secret && document.session.revealed_secrets.contains(cellKey(
                                        static_cast<int32_t>(i % static_cast<size_t>(std::max(document.width, 1))),
                                        static_cast<int32_t>(i / static_cast<size_t>(std::max(document.width, 1)))));
        adapter.blockingCells[i] = (cell.blocking && !opened && !revealed) ? 1 : 0;
    }
    return adapter;
}

bool is3DMode(const std::string& mode) {
    return mode == "3d" || mode == "spatial";
}

std::string commandForMode(const Dungeon3DWorldDocument& document) {
    return is3DMode(document.mode) ? "switch_to_3d:" + document.map_id : "switch_to_2d:" + document.map_id;
}

size_t cellIndex(const Dungeon3DWorldDocument& document, int32_t x, int32_t y) {
    return static_cast<size_t>(y * document.width + x);
}

bool inBounds(const Dungeon3DWorldDocument& document, int32_t x, int32_t y) {
    return x >= 0 && y >= 0 && x < document.width && y < document.height;
}

const Dungeon3DCell* cellAt(const Dungeon3DWorldDocument& document, int32_t x, int32_t y) {
    if (!inBounds(document, x, y)) {
        return nullptr;
    }
    const auto index = cellIndex(document, x, y);
    return index < document.cells.size() ? &document.cells[index] : nullptr;
}

bool isPassable(const Dungeon3DWorldDocument& document, int32_t x, int32_t y) {
    const auto* cell = cellAt(document, x, y);
    if (cell == nullptr) {
        return false;
    }
    if (!cell->door_id.empty() && document.session.opened_doors.contains(cell->door_id)) {
        return true;
    }
    if (cell->secret && document.session.revealed_secrets.contains(cellKey(x, y))) {
        return true;
    }
    return !cell->blocking;
}

std::optional<Dungeon3DInteraction> interactionAhead(const Dungeon3DWorldDocument& document) {
    const auto target_x = static_cast<int32_t>(std::floor(document.camera.pos_x + document.camera.dir_x));
    const auto target_y = static_cast<int32_t>(std::floor(document.camera.pos_y + document.camera.dir_y));
    const auto* cell = cellAt(document, target_x, target_y);
    if (cell == nullptr) {
        return std::nullopt;
    }
    const bool opened = !cell->door_id.empty() && document.session.opened_doors.contains(cell->door_id);
    const bool has_key = cell->required_item.empty() || document.session.inventory.contains(cell->required_item);
    return Dungeon3DInteraction{target_x,
                                target_y,
                                !isPassable(document, target_x, target_y),
                                !cell->event_id.empty(),
                                cell->stairs_up,
                                cell->stairs_down,
                                cell->dark_zone,
                                cell->secret,
                                cell->locked && !opened,
                                !cell->door_id.empty() && !opened && has_key,
                                !cell->floor_transfer.empty(),
                                cell->event_id,
                                cell->material_id,
                                cell->door_id,
                                cell->required_item,
                                cell->encounter_tag,
                                cell->floor_transfer};
}

Dungeon3DNavigationResult tryMove(Dungeon3DWorldDocument& document, float delta_x, float delta_y) {
    Dungeon3DNavigationResult result;
    const auto next_x = document.camera.pos_x + delta_x;
    const auto next_y = document.camera.pos_y + delta_y;
    const auto tile_x = static_cast<int32_t>(std::floor(next_x));
    const auto tile_y = static_cast<int32_t>(std::floor(next_y));
    const auto* cell = cellAt(document, tile_x, tile_y);
    if (cell == nullptr) {
        result.blocked = true;
        result.command = "blocked:out_of_bounds";
        result.diagnostic = {"navigation_out_of_bounds", "3D dungeon movement would leave the authored map."};
        return result;
    }
    if (!isPassable(document, tile_x, tile_y)) {
        result.blocked = true;
        result.command = "blocked:wall";
        result.diagnostic = {"navigation_blocked", "3D dungeon movement hit a blocking cell."};
        return result;
    }
    document.camera.pos_x = next_x;
    document.camera.pos_y = next_y;
    document.session.discovered_cells.insert(cellKey(tile_x, tile_y));
    result.moved = true;
    result.command = "move_camera:" + document.map_id;
    if (!cell->encounter_tag.empty() && !document.session.triggered_encounters.contains(cell->encounter_tag)) {
        document.session.triggered_encounters.insert(cell->encounter_tag);
        document.session.event_log.push_back("encounter:" + cell->encounter_tag);
        result.encounter_triggered = true;
        result.command = "trigger_encounter:" + cell->encounter_tag;
    }
    return result;
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
        if (material.light < 0.0f) {
            diagnostics.push_back({"invalid_material_light", "3D dungeon material light multiplier cannot be negative."});
        }
        material_ids.insert(id_key);
    }
    std::set<std::string> floor_ids;
    for (const auto& floor : floors) {
        if (floor.id.empty()) {
            diagnostics.push_back({"missing_floor_id", "3D dungeon floor is missing an id."});
        }
        floor_ids.insert(floor.id);
    }
    if (!session.current_floor_id.empty() && !floor_ids.empty() && !floor_ids.contains(session.current_floor_id)) {
        diagnostics.push_back({"unknown_current_floor", "3D dungeon session current floor is not authored."});
    }
    std::set<std::string> marker_ids;
    for (const auto& marker : markers) {
        if (marker.id.empty()) {
            diagnostics.push_back({"missing_marker_id", "3D dungeon map marker is missing an id."});
        } else if (!marker_ids.insert(marker.id).second) {
            diagnostics.push_back({"duplicate_marker_id", "3D dungeon map marker id is duplicated: " + marker.id});
        }
        if (!inBounds(*this, marker.x, marker.y)) {
            diagnostics.push_back({"marker_out_of_bounds", "3D dungeon map marker is outside the authored map: " + marker.id});
        }
        if (!marker.floor_id.empty() && !floor_ids.empty() && !floor_ids.contains(marker.floor_id)) {
            diagnostics.push_back({"unknown_marker_floor", "3D dungeon marker references an unknown floor: " + marker.id});
        }
    }
    for (const auto& note : notes) {
        if (note.id.empty()) {
            diagnostics.push_back({"missing_note_id", "3D dungeon map note is missing an id."});
        }
        if (!inBounds(*this, note.x, note.y)) {
            diagnostics.push_back({"note_out_of_bounds", "3D dungeon map note is outside the authored map: " + note.id});
        }
        if (!note.floor_id.empty() && !floor_ids.empty() && !floor_ids.contains(note.floor_id)) {
            diagnostics.push_back({"unknown_note_floor", "3D dungeon note references an unknown floor: " + note.id});
        }
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
    for (const auto& cell : cells) {
        if (cell.blocking) {
            ++result.blocking_cell_count;
        }
        if (!cell.event_id.empty()) {
            ++result.event_cell_count;
        }
        if (!cell.door_id.empty()) {
            ++result.door_count;
        }
        if (cell.secret) {
            ++result.secret_count;
        }
        if (!cell.encounter_tag.empty()) {
            ++result.encounter_cell_count;
        }
    }
    result.opened_door_count = static_cast<int32_t>(session.opened_doors.size());
    result.revealed_secret_count = static_cast<int32_t>(session.revealed_secrets.size());
    for (const auto& marker : markers) {
        ++result.marker_count;
        if (marker.visible) {
            ++result.visible_marker_count;
        }
        if (marker.type == "objective") {
            ++result.objective_count;
            if (marker.completed || session.completed_markers.contains(marker.id)) {
                ++result.completed_objective_count;
            }
        }
    }
    result.note_count = static_cast<int32_t>(notes.size());
    if (result.objective_count > 0) {
        result.floor_completion =
            static_cast<float>(result.completed_objective_count) / static_cast<float>(result.objective_count);
    }
    result.facing_interaction = interactionAhead(*this);

    const auto adapter = buildAdapter(*this);
    if (is3DMode(mode) && result.diagnostics.empty()) {
        RaycastRenderer::Config config;
        config.screenWidth = screen_width;
        config.screenHeight = screen_height;
        config.presentationMode = presentation::PresentationMode::Spatial;

        RaycastRenderer::Camera ray_camera{camera.pos_x, camera.pos_y, camera.dir_x, camera.dir_y, camera.plane_x, camera.plane_y};
        const auto casts = RaycastRenderer::castFrame(ray_camera, config, [&](int32_t x, int32_t y) {
            return adapter.isBlocking(x, y);
        });
        float distance_total = 0.0f;
        for (const auto& cast : casts) {
            const auto* cell = cellAt(*this, cast.mapX, cast.mapY);
            const auto material_id = cell != nullptr ? cell->material_id : std::string{};
            const auto material_it = materials.find(material_id);
            const float side_shade = cast.side == 0 ? 1.0f : 0.78f;
            const float material_light = material_it != materials.end() ? material_it->second.light : 1.0f;
            const auto wall_texture = material_it != materials.end() ? material_it->second.wall_texture : std::string{};
            const auto wall_height = static_cast<int32_t>(
                std::clamp(config.screenHeight / std::max(cast.wallDist, 0.001f), 1.0f, static_cast<float>(config.screenHeight)));
            result.columns.push_back({cast.x,
                                      cast.mapX,
                                      cast.mapY,
                                      cast.side,
                                      cast.wallDist,
                                      material_id,
                                      wall_texture,
                                      std::clamp(side_shade * material_light, 0.0f, 2.0f),
                                      wall_height});
            distance_total += cast.wallDist;
        }
        if (!result.columns.empty()) {
            result.average_wall_distance = distance_total / static_cast<float>(result.columns.size());
        }
        result.runtime_commands.push_back("raycast_frame:" + map_id);
    }

    if (minimap_enabled && width > 0 && height > 0) {
        std::set<size_t> discovered;
        const auto current_x = static_cast<int32_t>(std::floor(camera.pos_x));
        const auto current_y = static_cast<int32_t>(std::floor(camera.pos_y));
        if (current_x >= 0 && current_y >= 0 && current_x < width && current_y < height) {
            discovered.insert(static_cast<size_t>(current_y * width + current_x));
        }
        for (const auto& key : session.discovered_cells) {
            const auto comma = key.find(',');
            if (comma == std::string::npos) {
                continue;
            }
            const auto x = std::stoi(key.substr(0, comma));
            const auto y = std::stoi(key.substr(comma + 1));
            if (inBounds(*this, x, y)) {
                discovered.insert(cellIndex(*this, x, y));
            }
        }
        if (auto_mapping) {
            for (const auto& column : result.columns) {
                if (column.map_x >= 0 && column.map_y >= 0 && column.map_x < width && column.map_y < height) {
                    discovered.insert(static_cast<size_t>(column.map_y * width + column.map_x));
                }
            }
        }
        for (int32_t y = 0; y < height; ++y) {
            for (int32_t x = 0; x < width; ++x) {
                const auto index = static_cast<size_t>(y * width + x);
                if (index >= cells.size()) {
                    continue;
                }
                const bool visible = discovered.contains(index) || (std::abs(x - current_x) <= 1 && std::abs(y - current_y) <= 1);
                std::string marker_id;
                std::string marker_type;
                for (const auto& marker : markers) {
                    if (marker.visible && marker.x == x && marker.y == y &&
                        (marker.floor_id.empty() || marker.floor_id == session.current_floor_id)) {
                        marker_id = marker.id;
                        marker_type = marker.type;
                        break;
                    }
                }
                std::string note_id;
                for (const auto& note : notes) {
                    if (note.x == x && note.y == y && (note.floor_id.empty() || note.floor_id == session.current_floor_id)) {
                        note_id = note.id;
                        break;
                    }
                }
                result.minimap_tiles.push_back(
                    {x,
                     y,
                     cells[index].blocking,
                     discovered.contains(index),
                     x == current_x && y == current_y,
                     visible,
                     marker_id,
                     marker_type,
                     note_id,
                     cells[index].event_id});
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

Dungeon3DNavigationResult Dungeon3DWorldDocument::moveForward(float distance) {
    return tryMove(*this, camera.dir_x * distance, camera.dir_y * distance);
}

Dungeon3DNavigationResult Dungeon3DWorldDocument::strafe(float distance) {
    return tryMove(*this, camera.plane_x * distance, camera.plane_y * distance);
}

Dungeon3DInteractionResult Dungeon3DWorldDocument::interactFacing() {
    Dungeon3DInteractionResult result;
    const auto interaction = interactionAhead(*this);
    if (!interaction.has_value()) {
        result.diagnostic = {"interaction_out_of_bounds", "There is no authored 3D dungeon cell in front of the camera."};
        return result;
    }
    const auto key = cellKey(interaction->x, interaction->y);
    if (!interaction->door_id.empty() && !session.opened_doors.contains(interaction->door_id)) {
        if (!interaction->required_item.empty() && !session.inventory.contains(interaction->required_item)) {
            result.command = "locked_door:" + interaction->door_id;
            result.diagnostic = {"missing_required_item", "3D dungeon door requires item: " + interaction->required_item};
            return result;
        }
        session.opened_doors.insert(interaction->door_id);
        session.event_log.push_back("open_door:" + interaction->door_id);
        result.handled = true;
        result.opened_door = true;
        result.command = "open_door:" + interaction->door_id;
        return result;
    }
    if (interaction->secret && !session.revealed_secrets.contains(key)) {
        session.revealed_secrets.insert(key);
        session.event_log.push_back("reveal_secret:" + key);
        result.handled = true;
        result.revealed_secret = true;
        result.command = "reveal_secret:" + key;
        return result;
    }
    if (!interaction->floor_transfer.empty()) {
        session.current_floor_id = interaction->floor_transfer;
        session.event_log.push_back("transfer_floor:" + interaction->floor_transfer);
        result.handled = true;
        result.transferred_floor = true;
        result.command = "transfer_floor:" + interaction->floor_transfer;
        return result;
    }
    if (!interaction->event_id.empty()) {
        session.event_log.push_back("activate_event:" + interaction->event_id);
        result.handled = true;
        result.command = "activate_event:" + interaction->event_id;
        return result;
    }
    result.command = "inspect_cell:" + key;
    result.handled = true;
    return result;
}

void Dungeon3DWorldDocument::addInventoryItem(std::string item_id) {
    if (!item_id.empty()) {
        session.inventory.insert(std::move(item_id));
    }
}

bool Dungeon3DWorldDocument::completeMarker(std::string marker_id) {
    if (marker_id.empty()) {
        return false;
    }
    const auto found = std::find_if(markers.begin(), markers.end(), [&](const auto& marker) {
        return marker.id == marker_id;
    });
    if (found == markers.end()) {
        return false;
    }
    session.completed_markers.insert(marker_id);
    session.event_log.push_back("complete_marker:" + marker_id);
    return true;
}

void Dungeon3DWorldDocument::rotate(float radians) {
    const auto old_dir_x = camera.dir_x;
    camera.dir_x = camera.dir_x * std::cos(radians) - camera.dir_y * std::sin(radians);
    camera.dir_y = old_dir_x * std::sin(radians) + camera.dir_y * std::cos(radians);
    const auto old_plane_x = camera.plane_x;
    camera.plane_x = camera.plane_x * std::cos(radians) - camera.plane_y * std::sin(radians);
    camera.plane_y = old_plane_x * std::sin(radians) + camera.plane_y * std::cos(radians);
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
                          {"plane_y", camera.plane_y}}},
                        {"session", sessionToJson(session)}};
    json["floors"] = nlohmann::json::array();
    for (const auto& floor : floors) {
        json["floors"].push_back(floorToJson(floor));
    }
    json["markers"] = nlohmann::json::array();
    for (const auto& marker : markers) {
        json["markers"].push_back(markerToJson(marker));
    }
    json["notes"] = nlohmann::json::array();
    for (const auto& note : notes) {
        json["notes"].push_back(noteToJson(note));
    }
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
    if (json.contains("session")) {
        document.session = sessionFromJson(json.at("session"));
    }
    for (const auto& floor_json : json.value("floors", nlohmann::json::array())) {
        document.floors.push_back(floorFromJson(floor_json));
    }
    for (const auto& marker_json : json.value("markers", nlohmann::json::array())) {
        document.markers.push_back(markerFromJson(marker_json));
    }
    for (const auto& note_json : json.value("notes", nlohmann::json::array())) {
        document.notes.push_back(noteFromJson(note_json));
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
                                   {"map_x", column.map_x},
                                   {"map_y", column.map_y},
                                   {"side", column.side},
                                   {"wall_dist", column.wall_dist},
                                   {"material_id", column.material_id},
                                   {"wall_texture", column.wall_texture},
                                   {"shade", column.shade},
                                   {"projected_wall_height", column.projected_wall_height}});
    }
    json["minimap_tiles"] = nlohmann::json::array();
    for (const auto& tile : preview.minimap_tiles) {
        json["minimap_tiles"].push_back({{"x", tile.x},
                                         {"y", tile.y},
                                         {"blocking", tile.blocking},
                                         {"discovered", tile.discovered},
                                         {"current", tile.current},
                                         {"visible", tile.visible},
                                         {"marker_id", tile.marker_id},
                                         {"marker_type", tile.marker_type},
                                         {"note_id", tile.note_id},
                                         {"event_id", tile.event_id}});
    }
    if (preview.facing_interaction.has_value()) {
        const auto& interaction = *preview.facing_interaction;
        json["facing_interaction"] = {{"x", interaction.x},
                                      {"y", interaction.y},
                                      {"blocking", interaction.blocking},
                                      {"has_event", interaction.has_event},
                                      {"stairs_up", interaction.stairs_up},
                                      {"stairs_down", interaction.stairs_down},
                                      {"dark_zone", interaction.dark_zone},
                                      {"secret", interaction.secret},
                                      {"locked", interaction.locked},
                                      {"can_open", interaction.can_open},
                                      {"can_transfer", interaction.can_transfer},
                                      {"event_id", interaction.event_id},
                                      {"material_id", interaction.material_id},
                                      {"door_id", interaction.door_id},
                                      {"required_item", interaction.required_item},
                                      {"encounter_tag", interaction.encounter_tag},
                                      {"floor_transfer", interaction.floor_transfer}};
    }
    json["blocking_cell_count"] = preview.blocking_cell_count;
    json["event_cell_count"] = preview.event_cell_count;
    json["door_count"] = preview.door_count;
    json["secret_count"] = preview.secret_count;
    json["encounter_cell_count"] = preview.encounter_cell_count;
    json["marker_count"] = preview.marker_count;
    json["visible_marker_count"] = preview.visible_marker_count;
    json["objective_count"] = preview.objective_count;
    json["completed_objective_count"] = preview.completed_objective_count;
    json["note_count"] = preview.note_count;
    json["opened_door_count"] = preview.opened_door_count;
    json["revealed_secret_count"] = preview.revealed_secret_count;
    json["floor_completion"] = preview.floor_completion;
    json["average_wall_distance"] = preview.average_wall_distance;
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}});
    }
    return json;
}

} // namespace urpg::render
