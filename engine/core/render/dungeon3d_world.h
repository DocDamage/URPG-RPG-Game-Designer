#pragma once

#include "engine/core/render/raycast_renderer.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <set>
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

struct Dungeon3DFloor {
    std::string id;
    std::string display_name;
    int32_t depth = 0;
    std::string ambient_sound;
};

struct Dungeon3DMapMarker {
    std::string id;
    std::string label;
    std::string type;
    int32_t x = 0;
    int32_t y = 0;
    std::string floor_id;
    bool visible = true;
    bool completed = false;
};

struct Dungeon3DMapNote {
    std::string id;
    std::string text;
    int32_t x = 0;
    int32_t y = 0;
    std::string floor_id;
};

struct Dungeon3DEncounterZone {
    std::string id;
    std::string encounter_tag;
    int32_t x = 0;
    int32_t y = 0;
    int32_t radius = 0;
    int32_t weight = 1;
    std::string floor_id;
    bool repeatable = false;
};

struct Dungeon3DLockLink {
    std::string id;
    std::string source_event_id;
    std::string target_door_id;
    std::string required_item;
    bool opens_on_interact = true;
};

struct Dungeon3DTrap {
    std::string id;
    int32_t x = 0;
    int32_t y = 0;
    std::string floor_id;
    std::string effect_id;
    int32_t damage = 0;
    bool repeatable = false;
    std::string disarm_item;
    bool armed = true;
};

struct Dungeon3DAudioZone {
    std::string id;
    int32_t x = 0;
    int32_t y = 0;
    int32_t radius = 0;
    std::string floor_id;
    std::string ambient_sound;
    std::string reverb_preset;
};

struct Dungeon3DAtmosphere {
    std::string id;
    std::string floor_id;
    std::string weather;
    std::string particle_preset;
    float light_multiplier = 1.0f;
};

struct Dungeon3DPatrolWaypoint {
    int32_t x = 0;
    int32_t y = 0;
};

struct Dungeon3DPatrolRoute {
    std::string id;
    std::string enemy_id;
    std::string floor_id;
    std::vector<Dungeon3DPatrolWaypoint> waypoints;
    int32_t vision_range = 0;
    int32_t vision_width = 0;
    bool active = true;
};

struct Dungeon3DHidingSpot {
    std::string id;
    int32_t x = 0;
    int32_t y = 0;
    std::string floor_id;
    std::string required_item;
};

struct Dungeon3DCell {
    std::string tile_id;
    std::string material_id;
    std::string event_id;
    std::string door_id;
    std::string required_item;
    std::string encounter_tag;
    std::string floor_transfer;
    bool blocking = false;
    bool dark_zone = false;
    bool stairs_up = false;
    bool stairs_down = false;
    bool secret = false;
    bool locked = false;
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
    std::string marker_id;
    std::string marker_type;
    std::string note_id;
    std::string trap_id;
    std::string audio_zone_id;
    std::string patrol_id;
    std::string hiding_spot_id;
    bool in_patrol_vision = false;
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
    bool secret = false;
    bool locked = false;
    bool can_open = false;
    bool can_transfer = false;
    std::string event_id;
    std::string material_id;
    std::string door_id;
    std::string required_item;
    std::string encounter_tag;
    std::string floor_transfer;
};

struct Dungeon3DNavigationResult {
    bool moved = false;
    bool blocked = false;
    bool encounter_triggered = false;
    bool trap_triggered = false;
    bool audio_zone_entered = false;
    bool patrol_alerted = false;
    std::string command;
    std::string triggered_id;
    Dungeon3DDiagnostic diagnostic;
};

struct Dungeon3DInteractionResult {
    bool handled = false;
    bool opened_door = false;
    bool revealed_secret = false;
    bool transferred_floor = false;
    bool activated_switch = false;
    bool disarmed_trap = false;
    bool entered_hiding_spot = false;
    std::string command;
    std::string target_id;
    Dungeon3DDiagnostic diagnostic;
};

struct Dungeon3DSessionState {
    std::set<std::string> inventory;
    std::set<std::string> discovered_cells;
    std::set<std::string> opened_doors;
    std::set<std::string> revealed_secrets;
    std::set<std::string> triggered_encounters;
    std::set<std::string> completed_markers;
    std::set<std::string> disabled_traps;
    std::set<std::string> activated_switches;
    std::set<std::string> alerted_patrols;
    std::map<std::string, int32_t> patrol_indices;
    std::string current_hiding_spot;
    std::string current_floor_id;
    std::vector<std::string> event_log;
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
    int32_t door_count = 0;
    int32_t secret_count = 0;
    int32_t encounter_cell_count = 0;
    int32_t marker_count = 0;
    int32_t visible_marker_count = 0;
    int32_t objective_count = 0;
    int32_t completed_objective_count = 0;
    int32_t note_count = 0;
    int32_t encounter_zone_count = 0;
    int32_t lock_link_count = 0;
    int32_t trap_count = 0;
    int32_t armed_trap_count = 0;
    int32_t audio_zone_count = 0;
    int32_t patrol_count = 0;
    int32_t active_patrol_count = 0;
    int32_t alerted_patrol_count = 0;
    int32_t hiding_spot_count = 0;
    int32_t opened_door_count = 0;
    int32_t revealed_secret_count = 0;
    bool player_hidden = false;
    float floor_completion = 0.0f;
    float average_wall_distance = 0.0f;
    float current_light_multiplier = 1.0f;
    std::string active_ambient_sound;
    std::string active_reverb_preset;
    std::string active_weather;
    std::string active_particles;
    std::string nearest_patrol_id;
    std::string current_hiding_spot;
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
    Dungeon3DSessionState session;
    std::vector<Dungeon3DFloor> floors;
    std::vector<Dungeon3DMapMarker> markers;
    std::vector<Dungeon3DMapNote> notes;
    std::vector<Dungeon3DEncounterZone> encounter_zones;
    std::vector<Dungeon3DLockLink> lock_links;
    std::vector<Dungeon3DTrap> traps;
    std::vector<Dungeon3DAudioZone> audio_zones;
    std::vector<Dungeon3DAtmosphere> atmospheres;
    std::vector<Dungeon3DPatrolRoute> patrol_routes;
    std::vector<Dungeon3DHidingSpot> hiding_spots;
    std::map<std::string, Dungeon3DMaterial> materials;
    std::vector<Dungeon3DCell> cells;

    [[nodiscard]] std::vector<Dungeon3DDiagnostic> validate() const;
    [[nodiscard]] Dungeon3DPreview preview() const;
    [[nodiscard]] Dungeon3DPreview switchMode(std::string next_mode);
    [[nodiscard]] Dungeon3DNavigationResult moveForward(float distance = 1.0f);
    [[nodiscard]] Dungeon3DNavigationResult strafe(float distance);
    [[nodiscard]] Dungeon3DInteractionResult interactFacing();
    void addInventoryItem(std::string item_id);
    bool completeMarker(std::string marker_id);
    bool disarmTrap(std::string trap_id);
    bool enterHidingSpot(std::string hiding_spot_id);
    bool leaveHidingSpot();
    bool advancePatrol(std::string patrol_id);
    void rotate(float radians);
    [[nodiscard]] nlohmann::json toJson() const;

    static Dungeon3DWorldDocument fromJson(const nlohmann::json& json);
};

nlohmann::json dungeon3DPreviewToJson(const Dungeon3DPreview& preview);

} // namespace urpg::render
