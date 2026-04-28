#include "engine/core/render/dungeon3d_world.h"

#include <algorithm>
#include <cmath>
#include <limits>
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

Dungeon3DEncounterZone encounterZoneFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("encounter_tag", ""),
            json.value("x", 0),
            json.value("y", 0),
            json.value("radius", 0),
            json.value("weight", 1),
            json.value("floor_id", ""),
            json.value("repeatable", false)};
}

Dungeon3DLockLink lockLinkFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("source_event_id", ""),
            json.value("target_door_id", ""),
            json.value("required_item", ""),
            json.value("opens_on_interact", true)};
}

Dungeon3DTrap trapFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("x", 0),
            json.value("y", 0),
            json.value("floor_id", ""),
            json.value("effect_id", ""),
            json.value("damage", 0),
            json.value("repeatable", false),
            json.value("disarm_item", ""),
            json.value("armed", true)};
}

Dungeon3DAudioZone audioZoneFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("x", 0),
            json.value("y", 0),
            json.value("radius", 0),
            json.value("floor_id", ""),
            json.value("ambient_sound", ""),
            json.value("reverb_preset", "")};
}

Dungeon3DAtmosphere atmosphereFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("floor_id", ""),
            json.value("weather", ""),
            json.value("particle_preset", ""),
            json.value("light_multiplier", 1.0f)};
}

Dungeon3DPatrolWaypoint patrolWaypointFromJson(const nlohmann::json& json) {
    return {json.value("x", 0), json.value("y", 0)};
}

Dungeon3DPatrolRoute patrolRouteFromJson(const nlohmann::json& json) {
    Dungeon3DPatrolRoute route;
    route.id = json.value("id", "");
    route.enemy_id = json.value("enemy_id", "");
    route.floor_id = json.value("floor_id", "");
    route.vision_range = json.value("vision_range", 0);
    route.vision_width = json.value("vision_width", 0);
    route.active = json.value("active", true);
    for (const auto& waypoint_json : json.value("waypoints", nlohmann::json::array())) {
        route.waypoints.push_back(patrolWaypointFromJson(waypoint_json));
    }
    return route;
}

Dungeon3DHidingSpot hidingSpotFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("x", 0),
            json.value("y", 0),
            json.value("floor_id", ""),
            json.value("required_item", "")};
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

nlohmann::json encounterZoneToJson(const Dungeon3DEncounterZone& zone) {
    return {{"id", zone.id},
            {"encounter_tag", zone.encounter_tag},
            {"x", zone.x},
            {"y", zone.y},
            {"radius", zone.radius},
            {"weight", zone.weight},
            {"floor_id", zone.floor_id},
            {"repeatable", zone.repeatable}};
}

nlohmann::json lockLinkToJson(const Dungeon3DLockLink& link) {
    return {{"id", link.id},
            {"source_event_id", link.source_event_id},
            {"target_door_id", link.target_door_id},
            {"required_item", link.required_item},
            {"opens_on_interact", link.opens_on_interact}};
}

nlohmann::json trapToJson(const Dungeon3DTrap& trap) {
    return {{"id", trap.id},
            {"x", trap.x},
            {"y", trap.y},
            {"floor_id", trap.floor_id},
            {"effect_id", trap.effect_id},
            {"damage", trap.damage},
            {"repeatable", trap.repeatable},
            {"disarm_item", trap.disarm_item},
            {"armed", trap.armed}};
}

nlohmann::json audioZoneToJson(const Dungeon3DAudioZone& zone) {
    return {{"id", zone.id},
            {"x", zone.x},
            {"y", zone.y},
            {"radius", zone.radius},
            {"floor_id", zone.floor_id},
            {"ambient_sound", zone.ambient_sound},
            {"reverb_preset", zone.reverb_preset}};
}

nlohmann::json atmosphereToJson(const Dungeon3DAtmosphere& atmosphere) {
    return {{"id", atmosphere.id},
            {"floor_id", atmosphere.floor_id},
            {"weather", atmosphere.weather},
            {"particle_preset", atmosphere.particle_preset},
            {"light_multiplier", atmosphere.light_multiplier}};
}

nlohmann::json patrolWaypointToJson(const Dungeon3DPatrolWaypoint& waypoint) {
    return {{"x", waypoint.x}, {"y", waypoint.y}};
}

nlohmann::json patrolRouteToJson(const Dungeon3DPatrolRoute& route) {
    nlohmann::json json{{"id", route.id},
                        {"enemy_id", route.enemy_id},
                        {"floor_id", route.floor_id},
                        {"vision_range", route.vision_range},
                        {"vision_width", route.vision_width},
                        {"active", route.active}};
    json["waypoints"] = nlohmann::json::array();
    for (const auto& waypoint : route.waypoints) {
        json["waypoints"].push_back(patrolWaypointToJson(waypoint));
    }
    return json;
}

nlohmann::json hidingSpotToJson(const Dungeon3DHidingSpot& spot) {
    return {{"id", spot.id},
            {"x", spot.x},
            {"y", spot.y},
            {"floor_id", spot.floor_id},
            {"required_item", spot.required_item}};
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
            {"disabled_traps", session.disabled_traps},
            {"activated_switches", session.activated_switches},
            {"alerted_patrols", session.alerted_patrols},
            {"patrol_indices", session.patrol_indices},
            {"current_hiding_spot", session.current_hiding_spot},
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
    session.disabled_traps = json.value("disabled_traps", std::set<std::string>{});
    session.activated_switches = json.value("activated_switches", std::set<std::string>{});
    session.alerted_patrols = json.value("alerted_patrols", std::set<std::string>{});
    session.patrol_indices = json.value("patrol_indices", std::map<std::string, int32_t>{});
    session.current_hiding_spot = json.value("current_hiding_spot", "");
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

bool matchesCurrentFloor(const Dungeon3DWorldDocument& document, const std::string& floor_id) {
    return floor_id.empty() || document.session.current_floor_id.empty() || floor_id == document.session.current_floor_id;
}

int32_t squaredDistance(int32_t ax, int32_t ay, int32_t bx, int32_t by) {
    const auto dx = ax - bx;
    const auto dy = ay - by;
    return dx * dx + dy * dy;
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

const Dungeon3DEncounterZone* encounterZoneAt(const Dungeon3DWorldDocument& document, int32_t x, int32_t y) {
    const Dungeon3DEncounterZone* best = nullptr;
    int32_t best_weight = -1;
    for (const auto& zone : document.encounter_zones) {
        const auto radius = std::max(zone.radius, 0);
        if (!matchesCurrentFloor(document, zone.floor_id) || zone.encounter_tag.empty()) {
            continue;
        }
        if (squaredDistance(x, y, zone.x, zone.y) <= radius * radius && zone.weight > best_weight) {
            best = &zone;
            best_weight = zone.weight;
        }
    }
    return best;
}

const Dungeon3DTrap* trapAt(const Dungeon3DWorldDocument& document, int32_t x, int32_t y) {
    const auto it = std::find_if(document.traps.begin(), document.traps.end(), [&](const auto& trap) {
        return trap.x == x && trap.y == y && matchesCurrentFloor(document, trap.floor_id);
    });
    return it == document.traps.end() ? nullptr : &(*it);
}

const Dungeon3DAudioZone* audioZoneAt(const Dungeon3DWorldDocument& document, int32_t x, int32_t y) {
    const Dungeon3DAudioZone* best = nullptr;
    int32_t best_distance = std::numeric_limits<int32_t>::max();
    for (const auto& zone : document.audio_zones) {
        const auto radius = std::max(zone.radius, 0);
        if (!matchesCurrentFloor(document, zone.floor_id)) {
            continue;
        }
        const auto distance = squaredDistance(x, y, zone.x, zone.y);
        if (distance <= radius * radius && distance < best_distance) {
            best = &zone;
            best_distance = distance;
        }
    }
    return best;
}

int32_t patrolIndexFor(const Dungeon3DWorldDocument& document, const Dungeon3DPatrolRoute& route) {
    if (route.waypoints.empty()) {
        return 0;
    }
    const auto it = document.session.patrol_indices.find(route.id);
    const auto raw_index = it == document.session.patrol_indices.end() ? 0 : it->second;
    return std::clamp(raw_index, 0, static_cast<int32_t>(route.waypoints.size() - 1));
}

Dungeon3DPatrolWaypoint patrolPositionFor(const Dungeon3DWorldDocument& document, const Dungeon3DPatrolRoute& route) {
    if (route.waypoints.empty()) {
        return {};
    }
    return route.waypoints[static_cast<size_t>(patrolIndexFor(document, route))];
}

const Dungeon3DHidingSpot* hidingSpotAt(const Dungeon3DWorldDocument& document, int32_t x, int32_t y) {
    const auto it = std::find_if(document.hiding_spots.begin(), document.hiding_spots.end(), [&](const auto& spot) {
        return spot.x == x && spot.y == y && matchesCurrentFloor(document, spot.floor_id);
    });
    return it == document.hiding_spots.end() ? nullptr : &(*it);
}

bool patrolSeesTile(const Dungeon3DWorldDocument& document, const Dungeon3DPatrolRoute& route, int32_t x, int32_t y) {
    if (!route.active || route.waypoints.empty() || !matchesCurrentFloor(document, route.floor_id)) {
        return false;
    }
    const auto position = patrolPositionFor(document, route);
    const auto manhattan = std::abs(position.x - x) + std::abs(position.y - y);
    const auto width = std::max(route.vision_width, 0);
    return manhattan <= route.vision_range && std::abs(position.x - x) <= route.vision_range &&
           std::abs(position.y - y) <= std::max(route.vision_range, width);
}

const Dungeon3DPatrolRoute* visiblePatrolForPlayer(const Dungeon3DWorldDocument& document, int32_t x, int32_t y) {
    if (!document.session.current_hiding_spot.empty()) {
        return nullptr;
    }
    for (const auto& route : document.patrol_routes) {
        if (patrolSeesTile(document, route, x, y)) {
            return &route;
        }
    }
    return nullptr;
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
    document.session.current_hiding_spot.clear();
    document.session.discovered_cells.insert(cellKey(tile_x, tile_y));
    result.moved = true;
    result.command = "move_camera:" + document.map_id;
    if (!cell->encounter_tag.empty() && !document.session.triggered_encounters.contains(cell->encounter_tag)) {
        document.session.triggered_encounters.insert(cell->encounter_tag);
        document.session.event_log.push_back("encounter:" + cell->encounter_tag);
        result.encounter_triggered = true;
        result.command = "trigger_encounter:" + cell->encounter_tag;
        result.triggered_id = cell->encounter_tag;
    } else if (const auto* zone = encounterZoneAt(document, tile_x, tile_y);
               zone != nullptr && (zone->repeatable || !document.session.triggered_encounters.contains(zone->id))) {
        document.session.triggered_encounters.insert(zone->id);
        document.session.event_log.push_back("encounter_zone:" + zone->id + ":" + zone->encounter_tag);
        result.encounter_triggered = true;
        result.command = "trigger_encounter_zone:" + zone->id;
        result.triggered_id = zone->id;
    }
    if (const auto* trap = trapAt(document, tile_x, tile_y);
        trap != nullptr && trap->armed && !document.session.disabled_traps.contains(trap->id)) {
        document.session.event_log.push_back("trigger_trap:" + trap->id + ":" + trap->effect_id);
        result.trap_triggered = true;
        result.command = "trigger_trap:" + trap->id;
        result.triggered_id = trap->id;
        if (!trap->repeatable) {
            document.session.disabled_traps.insert(trap->id);
        }
    }
    if (const auto* zone = audioZoneAt(document, tile_x, tile_y); zone != nullptr) {
        document.session.event_log.push_back("enter_audio_zone:" + zone->id);
        result.audio_zone_entered = true;
        if (!result.trap_triggered && !result.encounter_triggered) {
            result.command = "enter_audio_zone:" + zone->id;
            result.triggered_id = zone->id;
        }
    }
    if (const auto* patrol = visiblePatrolForPlayer(document, tile_x, tile_y); patrol != nullptr) {
        document.session.alerted_patrols.insert(patrol->id);
        document.session.event_log.push_back("patrol_alert:" + patrol->id);
        result.patrol_alerted = true;
        if (!result.trap_triggered && !result.encounter_triggered && !result.audio_zone_entered) {
            result.command = "patrol_alert:" + patrol->id;
            result.triggered_id = patrol->id;
        }
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
    std::set<std::string> encounter_zone_ids;
    for (const auto& zone : encounter_zones) {
        if (zone.id.empty()) {
            diagnostics.push_back({"missing_encounter_zone_id", "3D dungeon encounter zone is missing an id."});
        } else if (!encounter_zone_ids.insert(zone.id).second) {
            diagnostics.push_back({"duplicate_encounter_zone_id", "3D dungeon encounter zone id is duplicated: " + zone.id});
        }
        if (zone.encounter_tag.empty()) {
            diagnostics.push_back({"missing_encounter_zone_tag", "3D dungeon encounter zone is missing an encounter tag: " + zone.id});
        }
        if (!inBounds(*this, zone.x, zone.y)) {
            diagnostics.push_back({"encounter_zone_out_of_bounds", "3D dungeon encounter zone is outside the authored map: " + zone.id});
        }
        if (zone.radius < 0 || zone.weight <= 0) {
            diagnostics.push_back({"invalid_encounter_zone_shape", "3D dungeon encounter zone radius and weight must be positive: " + zone.id});
        }
        if (!zone.floor_id.empty() && !floor_ids.empty() && !floor_ids.contains(zone.floor_id)) {
            diagnostics.push_back({"unknown_encounter_zone_floor", "3D dungeon encounter zone references an unknown floor: " + zone.id});
        }
    }
    std::set<std::string> trap_ids;
    for (const auto& trap : traps) {
        if (trap.id.empty()) {
            diagnostics.push_back({"missing_trap_id", "3D dungeon trap is missing an id."});
        } else if (!trap_ids.insert(trap.id).second) {
            diagnostics.push_back({"duplicate_trap_id", "3D dungeon trap id is duplicated: " + trap.id});
        }
        if (!inBounds(*this, trap.x, trap.y)) {
            diagnostics.push_back({"trap_out_of_bounds", "3D dungeon trap is outside the authored map: " + trap.id});
        }
        if (trap.effect_id.empty()) {
            diagnostics.push_back({"missing_trap_effect", "3D dungeon trap is missing an effect id: " + trap.id});
        }
        if (trap.damage < 0) {
            diagnostics.push_back({"invalid_trap_damage", "3D dungeon trap damage cannot be negative: " + trap.id});
        }
        if (!trap.floor_id.empty() && !floor_ids.empty() && !floor_ids.contains(trap.floor_id)) {
            diagnostics.push_back({"unknown_trap_floor", "3D dungeon trap references an unknown floor: " + trap.id});
        }
    }
    std::set<std::string> audio_zone_ids;
    for (const auto& zone : audio_zones) {
        if (zone.id.empty()) {
            diagnostics.push_back({"missing_audio_zone_id", "3D dungeon audio zone is missing an id."});
        } else if (!audio_zone_ids.insert(zone.id).second) {
            diagnostics.push_back({"duplicate_audio_zone_id", "3D dungeon audio zone id is duplicated: " + zone.id});
        }
        if (!inBounds(*this, zone.x, zone.y)) {
            diagnostics.push_back({"audio_zone_out_of_bounds", "3D dungeon audio zone is outside the authored map: " + zone.id});
        }
        if (zone.radius < 0) {
            diagnostics.push_back({"invalid_audio_zone_radius", "3D dungeon audio zone radius cannot be negative: " + zone.id});
        }
        if (zone.ambient_sound.empty() && zone.reverb_preset.empty()) {
            diagnostics.push_back({"empty_audio_zone", "3D dungeon audio zone must define ambient sound or reverb: " + zone.id});
        }
        if (!zone.floor_id.empty() && !floor_ids.empty() && !floor_ids.contains(zone.floor_id)) {
            diagnostics.push_back({"unknown_audio_zone_floor", "3D dungeon audio zone references an unknown floor: " + zone.id});
        }
    }
    std::set<std::string> atmosphere_ids;
    for (const auto& atmosphere : atmospheres) {
        if (atmosphere.id.empty()) {
            diagnostics.push_back({"missing_atmosphere_id", "3D dungeon atmosphere is missing an id."});
        } else if (!atmosphere_ids.insert(atmosphere.id).second) {
            diagnostics.push_back({"duplicate_atmosphere_id", "3D dungeon atmosphere id is duplicated: " + atmosphere.id});
        }
        if (!atmosphere.floor_id.empty() && !floor_ids.empty() && !floor_ids.contains(atmosphere.floor_id)) {
            diagnostics.push_back({"unknown_atmosphere_floor", "3D dungeon atmosphere references an unknown floor: " + atmosphere.id});
        }
        if (atmosphere.light_multiplier < 0.0f) {
            diagnostics.push_back({"invalid_atmosphere_light", "3D dungeon atmosphere light multiplier cannot be negative: " + atmosphere.id});
        }
    }
    std::set<std::string> patrol_ids;
    for (const auto& route : patrol_routes) {
        if (route.id.empty()) {
            diagnostics.push_back({"missing_patrol_id", "3D dungeon patrol route is missing an id."});
        } else if (!patrol_ids.insert(route.id).second) {
            diagnostics.push_back({"duplicate_patrol_id", "3D dungeon patrol route id is duplicated: " + route.id});
        }
        if (route.enemy_id.empty()) {
            diagnostics.push_back({"missing_patrol_enemy", "3D dungeon patrol route is missing an enemy id: " + route.id});
        }
        if (route.waypoints.empty()) {
            diagnostics.push_back({"empty_patrol_route", "3D dungeon patrol route must have at least one waypoint: " + route.id});
        }
        if (route.vision_range < 0 || route.vision_width < 0) {
            diagnostics.push_back({"invalid_patrol_vision", "3D dungeon patrol vision range and width cannot be negative: " + route.id});
        }
        if (!route.floor_id.empty() && !floor_ids.empty() && !floor_ids.contains(route.floor_id)) {
            diagnostics.push_back({"unknown_patrol_floor", "3D dungeon patrol route references an unknown floor: " + route.id});
        }
        for (const auto& waypoint : route.waypoints) {
            if (!inBounds(*this, waypoint.x, waypoint.y)) {
                diagnostics.push_back({"patrol_waypoint_out_of_bounds",
                                       "3D dungeon patrol route has a waypoint outside the authored map: " + route.id});
                break;
            }
        }
    }
    std::set<std::string> hiding_spot_ids;
    for (const auto& spot : hiding_spots) {
        if (spot.id.empty()) {
            diagnostics.push_back({"missing_hiding_spot_id", "3D dungeon hiding spot is missing an id."});
        } else if (!hiding_spot_ids.insert(spot.id).second) {
            diagnostics.push_back({"duplicate_hiding_spot_id", "3D dungeon hiding spot id is duplicated: " + spot.id});
        }
        if (!inBounds(*this, spot.x, spot.y)) {
            diagnostics.push_back({"hiding_spot_out_of_bounds", "3D dungeon hiding spot is outside the authored map: " + spot.id});
        }
        if (!spot.floor_id.empty() && !floor_ids.empty() && !floor_ids.contains(spot.floor_id)) {
            diagnostics.push_back({"unknown_hiding_spot_floor", "3D dungeon hiding spot references an unknown floor: " + spot.id});
        }
    }
    std::set<std::string> door_ids;
    std::set<std::string> event_ids;
    for (const auto& cell : cells) {
        if (!cell.material_id.empty() && !material_ids.contains(cell.material_id)) {
            diagnostics.push_back({"unknown_cell_material", "3D dungeon cell references an unknown material: " + cell.material_id});
            break;
        }
        if (!cell.door_id.empty()) {
            door_ids.insert(cell.door_id);
        }
        if (!cell.event_id.empty()) {
            event_ids.insert(cell.event_id);
        }
    }
    std::set<std::string> lock_link_ids;
    for (const auto& link : lock_links) {
        if (link.id.empty()) {
            diagnostics.push_back({"missing_lock_link_id", "3D dungeon lock link is missing an id."});
        } else if (!lock_link_ids.insert(link.id).second) {
            diagnostics.push_back({"duplicate_lock_link_id", "3D dungeon lock link id is duplicated: " + link.id});
        }
        if (link.source_event_id.empty() || !event_ids.contains(link.source_event_id)) {
            diagnostics.push_back({"unknown_lock_link_source", "3D dungeon lock link references an unknown source event: " + link.id});
        }
        if (link.target_door_id.empty() || !door_ids.contains(link.target_door_id)) {
            diagnostics.push_back({"unknown_lock_link_target", "3D dungeon lock link references an unknown target door: " + link.id});
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
    result.encounter_zone_count = static_cast<int32_t>(encounter_zones.size());
    result.lock_link_count = static_cast<int32_t>(lock_links.size());
    result.trap_count = static_cast<int32_t>(traps.size());
    result.audio_zone_count = static_cast<int32_t>(audio_zones.size());
    result.patrol_count = static_cast<int32_t>(patrol_routes.size());
    result.hiding_spot_count = static_cast<int32_t>(hiding_spots.size());
    for (const auto& trap : traps) {
        if (trap.armed && !session.disabled_traps.contains(trap.id)) {
            ++result.armed_trap_count;
        }
    }
    for (const auto& route : patrol_routes) {
        if (route.active) {
            ++result.active_patrol_count;
        }
    }
    result.alerted_patrol_count = static_cast<int32_t>(session.alerted_patrols.size());
    result.player_hidden = !session.current_hiding_spot.empty();
    result.current_hiding_spot = session.current_hiding_spot;
    if (result.objective_count > 0) {
        result.floor_completion =
            static_cast<float>(result.completed_objective_count) / static_cast<float>(result.objective_count);
    }
    for (const auto& atmosphere : atmospheres) {
        if (matchesCurrentFloor(*this, atmosphere.floor_id)) {
            result.current_light_multiplier = atmosphere.light_multiplier;
            result.active_weather = atmosphere.weather;
            result.active_particles = atmosphere.particle_preset;
            break;
        }
    }
    const auto current_x = static_cast<int32_t>(std::floor(camera.pos_x));
    const auto current_y = static_cast<int32_t>(std::floor(camera.pos_y));
    if (const auto* zone = audioZoneAt(*this, current_x, current_y); zone != nullptr) {
        result.active_ambient_sound = zone->ambient_sound;
        result.active_reverb_preset = zone->reverb_preset;
    }
    if (const auto* patrol = visiblePatrolForPlayer(*this, current_x, current_y); patrol != nullptr) {
        result.nearest_patrol_id = patrol->id;
    }
    if (result.active_ambient_sound.empty()) {
        const auto floor_it = std::find_if(floors.begin(), floors.end(), [&](const auto& floor) {
            return floor.id == session.current_floor_id;
        });
        if (floor_it != floors.end()) {
            result.active_ambient_sound = floor_it->ambient_sound;
        }
    }
    if (!result.active_ambient_sound.empty()) {
        result.runtime_commands.push_back("set_ambient:" + result.active_ambient_sound);
    }
    if (!result.active_weather.empty()) {
        result.runtime_commands.push_back("set_weather:" + result.active_weather);
    }
    if (!result.active_particles.empty()) {
        result.runtime_commands.push_back("set_particles:" + result.active_particles);
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
                                      std::clamp(side_shade * material_light * result.current_light_multiplier, 0.0f, 2.0f),
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
                std::string trap_id;
                for (const auto& trap : traps) {
                    if (trap.x == x && trap.y == y && trap.armed && !session.disabled_traps.contains(trap.id) &&
                        matchesCurrentFloor(*this, trap.floor_id)) {
                        trap_id = trap.id;
                        break;
                    }
                }
                std::string audio_zone_id;
                for (const auto& zone : audio_zones) {
                    const auto radius = std::max(zone.radius, 0);
                    if (matchesCurrentFloor(*this, zone.floor_id) && squaredDistance(x, y, zone.x, zone.y) <= radius * radius) {
                        audio_zone_id = zone.id;
                        break;
                    }
                }
                std::string patrol_id;
                bool in_patrol_vision = false;
                for (const auto& route : patrol_routes) {
                    const auto position = patrolPositionFor(*this, route);
                    if (route.active && matchesCurrentFloor(*this, route.floor_id) && position.x == x && position.y == y) {
                        patrol_id = route.id;
                    }
                    in_patrol_vision = in_patrol_vision || patrolSeesTile(*this, route, x, y);
                }
                std::string hiding_spot_id;
                for (const auto& spot : hiding_spots) {
                    if (spot.x == x && spot.y == y && matchesCurrentFloor(*this, spot.floor_id)) {
                        hiding_spot_id = spot.id;
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
                     trap_id,
                     audio_zone_id,
                     patrol_id,
                     hiding_spot_id,
                     in_patrol_vision,
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
    if (const auto* trap = trapAt(*this, interaction->x, interaction->y);
        trap != nullptr && trap->armed && !session.disabled_traps.contains(trap->id)) {
        if (!trap->disarm_item.empty() && !session.inventory.contains(trap->disarm_item)) {
            result.command = "trap_requires_item:" + trap->id;
            result.target_id = trap->id;
            result.diagnostic = {"missing_trap_disarm_item", "3D dungeon trap requires item: " + trap->disarm_item};
            return result;
        }
        session.disabled_traps.insert(trap->id);
        session.event_log.push_back("disarm_trap:" + trap->id);
        result.handled = true;
        result.disarmed_trap = true;
        result.command = "disarm_trap:" + trap->id;
        result.target_id = trap->id;
        return result;
    }
    if (const auto* spot = hidingSpotAt(*this, interaction->x, interaction->y); spot != nullptr) {
        if (!spot->required_item.empty() && !session.inventory.contains(spot->required_item)) {
            result.command = "hiding_spot_requires_item:" + spot->id;
            result.target_id = spot->id;
            result.diagnostic = {"missing_hiding_spot_item", "3D dungeon hiding spot requires item: " + spot->required_item};
            return result;
        }
        session.current_hiding_spot = spot->id;
        session.event_log.push_back("enter_hiding_spot:" + spot->id);
        result.handled = true;
        result.entered_hiding_spot = true;
        result.command = "enter_hiding_spot:" + spot->id;
        result.target_id = spot->id;
        return result;
    }
    if (!interaction->event_id.empty()) {
        const auto link_it = std::find_if(lock_links.begin(), lock_links.end(), [&](const auto& link) {
            return link.opens_on_interact && link.source_event_id == interaction->event_id;
        });
        if (link_it != lock_links.end()) {
            if (!link_it->required_item.empty() && !session.inventory.contains(link_it->required_item)) {
                result.command = "switch_requires_item:" + link_it->id;
                result.target_id = link_it->target_door_id;
                result.diagnostic = {"missing_switch_required_item",
                                     "3D dungeon switch requires item: " + link_it->required_item};
                return result;
            }
            session.activated_switches.insert(link_it->id);
            session.opened_doors.insert(link_it->target_door_id);
            session.event_log.push_back("activate_switch:" + link_it->id + ":" + link_it->target_door_id);
            result.handled = true;
            result.activated_switch = true;
            result.opened_door = true;
            result.command = "activate_switch:" + link_it->id;
            result.target_id = link_it->target_door_id;
            return result;
        }
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

bool Dungeon3DWorldDocument::disarmTrap(std::string trap_id) {
    if (trap_id.empty()) {
        return false;
    }
    const auto found = std::find_if(traps.begin(), traps.end(), [&](const auto& trap) {
        return trap.id == trap_id;
    });
    if (found == traps.end()) {
        return false;
    }
    session.disabled_traps.insert(trap_id);
    session.event_log.push_back("disarm_trap:" + trap_id);
    return true;
}

bool Dungeon3DWorldDocument::enterHidingSpot(std::string hiding_spot_id) {
    if (hiding_spot_id.empty()) {
        return false;
    }
    const auto found = std::find_if(hiding_spots.begin(), hiding_spots.end(), [&](const auto& spot) {
        return spot.id == hiding_spot_id;
    });
    if (found == hiding_spots.end()) {
        return false;
    }
    if (!found->required_item.empty() && !session.inventory.contains(found->required_item)) {
        return false;
    }
    session.current_hiding_spot = hiding_spot_id;
    session.event_log.push_back("enter_hiding_spot:" + hiding_spot_id);
    return true;
}

bool Dungeon3DWorldDocument::leaveHidingSpot() {
    if (session.current_hiding_spot.empty()) {
        return false;
    }
    session.event_log.push_back("leave_hiding_spot:" + session.current_hiding_spot);
    session.current_hiding_spot.clear();
    return true;
}

bool Dungeon3DWorldDocument::advancePatrol(std::string patrol_id) {
    const auto found = std::find_if(patrol_routes.begin(), patrol_routes.end(), [&](const auto& route) {
        return route.id == patrol_id;
    });
    if (found == patrol_routes.end() || found->waypoints.empty()) {
        return false;
    }
    auto& index = session.patrol_indices[patrol_id];
    index = (index + 1) % static_cast<int32_t>(found->waypoints.size());
    session.event_log.push_back("advance_patrol:" + patrol_id + ":" + std::to_string(index));
    const auto player_x = static_cast<int32_t>(std::floor(camera.pos_x));
    const auto player_y = static_cast<int32_t>(std::floor(camera.pos_y));
    if (patrolSeesTile(*this, *found, player_x, player_y) && session.current_hiding_spot.empty()) {
        session.alerted_patrols.insert(patrol_id);
        session.event_log.push_back("patrol_alert:" + patrol_id);
    }
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
    json["encounter_zones"] = nlohmann::json::array();
    for (const auto& zone : encounter_zones) {
        json["encounter_zones"].push_back(encounterZoneToJson(zone));
    }
    json["lock_links"] = nlohmann::json::array();
    for (const auto& link : lock_links) {
        json["lock_links"].push_back(lockLinkToJson(link));
    }
    json["traps"] = nlohmann::json::array();
    for (const auto& trap : traps) {
        json["traps"].push_back(trapToJson(trap));
    }
    json["audio_zones"] = nlohmann::json::array();
    for (const auto& zone : audio_zones) {
        json["audio_zones"].push_back(audioZoneToJson(zone));
    }
    json["atmospheres"] = nlohmann::json::array();
    for (const auto& atmosphere : atmospheres) {
        json["atmospheres"].push_back(atmosphereToJson(atmosphere));
    }
    json["patrol_routes"] = nlohmann::json::array();
    for (const auto& route : patrol_routes) {
        json["patrol_routes"].push_back(patrolRouteToJson(route));
    }
    json["hiding_spots"] = nlohmann::json::array();
    for (const auto& spot : hiding_spots) {
        json["hiding_spots"].push_back(hidingSpotToJson(spot));
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
    for (const auto& zone_json : json.value("encounter_zones", nlohmann::json::array())) {
        document.encounter_zones.push_back(encounterZoneFromJson(zone_json));
    }
    for (const auto& link_json : json.value("lock_links", nlohmann::json::array())) {
        document.lock_links.push_back(lockLinkFromJson(link_json));
    }
    for (const auto& trap_json : json.value("traps", nlohmann::json::array())) {
        document.traps.push_back(trapFromJson(trap_json));
    }
    for (const auto& zone_json : json.value("audio_zones", nlohmann::json::array())) {
        document.audio_zones.push_back(audioZoneFromJson(zone_json));
    }
    for (const auto& atmosphere_json : json.value("atmospheres", nlohmann::json::array())) {
        document.atmospheres.push_back(atmosphereFromJson(atmosphere_json));
    }
    for (const auto& route_json : json.value("patrol_routes", nlohmann::json::array())) {
        document.patrol_routes.push_back(patrolRouteFromJson(route_json));
    }
    for (const auto& spot_json : json.value("hiding_spots", nlohmann::json::array())) {
        document.hiding_spots.push_back(hidingSpotFromJson(spot_json));
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
                                         {"trap_id", tile.trap_id},
                                         {"audio_zone_id", tile.audio_zone_id},
                                         {"patrol_id", tile.patrol_id},
                                         {"hiding_spot_id", tile.hiding_spot_id},
                                         {"in_patrol_vision", tile.in_patrol_vision},
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
    json["encounter_zone_count"] = preview.encounter_zone_count;
    json["lock_link_count"] = preview.lock_link_count;
    json["trap_count"] = preview.trap_count;
    json["armed_trap_count"] = preview.armed_trap_count;
    json["audio_zone_count"] = preview.audio_zone_count;
    json["patrol_count"] = preview.patrol_count;
    json["active_patrol_count"] = preview.active_patrol_count;
    json["alerted_patrol_count"] = preview.alerted_patrol_count;
    json["hiding_spot_count"] = preview.hiding_spot_count;
    json["opened_door_count"] = preview.opened_door_count;
    json["revealed_secret_count"] = preview.revealed_secret_count;
    json["player_hidden"] = preview.player_hidden;
    json["floor_completion"] = preview.floor_completion;
    json["average_wall_distance"] = preview.average_wall_distance;
    json["current_light_multiplier"] = preview.current_light_multiplier;
    json["active_ambient_sound"] = preview.active_ambient_sound;
    json["active_reverb_preset"] = preview.active_reverb_preset;
    json["active_weather"] = preview.active_weather;
    json["active_particles"] = preview.active_particles;
    json["nearest_patrol_id"] = preview.nearest_patrol_id;
    json["current_hiding_spot"] = preview.current_hiding_spot;
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}});
    }
    return json;
}

} // namespace urpg::render
