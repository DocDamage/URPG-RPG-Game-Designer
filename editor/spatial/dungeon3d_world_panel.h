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
    size_t door_count = 0;
    size_t secret_count = 0;
    size_t encounter_cell_count = 0;
    size_t marker_count = 0;
    size_t visible_marker_count = 0;
    size_t objective_count = 0;
    size_t completed_objective_count = 0;
    size_t note_count = 0;
    size_t encounter_zone_count = 0;
    size_t lock_link_count = 0;
    size_t trap_count = 0;
    size_t armed_trap_count = 0;
    size_t audio_zone_count = 0;
    size_t patrol_count = 0;
    size_t active_patrol_count = 0;
    size_t alerted_patrol_count = 0;
    size_t hiding_spot_count = 0;
    size_t puzzle_device_count = 0;
    size_t active_puzzle_device_count = 0;
    size_t solved_puzzle_count = 0;
    size_t camera_rail_cue_count = 0;
    size_t room_template_count = 0;
    size_t placed_room_template_count = 0;
    size_t boss_arena_count = 0;
    size_t active_boss_arena_count = 0;
    size_t defeated_boss_arena_count = 0;
    size_t opened_door_count = 0;
    size_t revealed_secret_count = 0;
    size_t runtime_command_count = 0;
    size_t diagnostic_count = 0;
    float camera_x = 0.0f;
    float camera_y = 0.0f;
    float average_wall_distance = 0.0f;
    float floor_completion = 0.0f;
    float current_light_multiplier = 1.0f;
    float camera_fov = 66.0f;
    float camera_head_bob = 0.0f;
    float camera_shake = 0.0f;
    std::string active_ambient_sound;
    std::string active_reverb_preset;
    std::string active_weather;
    std::string active_particles;
    std::string nearest_patrol_id;
    std::string current_hiding_spot;
    std::string active_camera_rail_id;
    std::string facing_event_id;
    std::string facing_material_id;
    std::string facing_door_id;
    std::string facing_required_item;
    std::string facing_floor_transfer;
    bool facing_secret = false;
    bool facing_locked = false;
    bool facing_can_open = false;
    bool facing_can_transfer = false;
    bool facing_blocked = false;
    bool player_hidden = false;
    std::string current_floor_id;
    std::string last_event_log_entry;
    std::string status_message = "Load a 3D dungeon world document before rendering this panel.";
};

class Dungeon3DWorldPanel {
public:
    void loadDocument(urpg::render::Dungeon3DWorldDocument document);
    void setMode(std::string mode);
    urpg::render::Dungeon3DNavigationResult moveForward(float distance = 1.0f);
    urpg::render::Dungeon3DNavigationResult strafe(float distance);
    urpg::render::Dungeon3DInteractionResult interactFacing();
    void addInventoryItem(std::string item_id);
    bool completeMarker(std::string marker_id);
    bool disarmTrap(std::string trap_id);
    bool enterHidingSpot(std::string hiding_spot_id);
    bool leaveHidingSpot();
    bool advancePatrol(std::string patrol_id);
    bool activatePuzzle(std::string puzzle_id);
    bool playCameraRail(std::string rail_id);
    bool placeRoomTemplate(std::string template_id);
    bool startBossArena(std::string arena_id);
    bool defeatBossArena(std::string arena_id);
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
