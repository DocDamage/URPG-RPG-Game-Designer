#include "editor/spatial/dungeon3d_world_panel.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

void Dungeon3DWorldPanel::loadDocument(urpg::render::Dungeon3DWorldDocument document) {
    document_ = std::move(document);
    loaded_ = true;
    refresh();
}

void Dungeon3DWorldPanel::setMode(std::string mode) {
    document_.mode = std::move(mode);
    if (loaded_) {
        refresh();
    }
}

urpg::render::Dungeon3DNavigationResult Dungeon3DWorldPanel::moveForward(float distance) {
    auto result = document_.moveForward(distance);
    if (loaded_) {
        refresh();
    }
    return result;
}

urpg::render::Dungeon3DNavigationResult Dungeon3DWorldPanel::strafe(float distance) {
    auto result = document_.strafe(distance);
    if (loaded_) {
        refresh();
    }
    return result;
}

urpg::render::Dungeon3DInteractionResult Dungeon3DWorldPanel::interactFacing() {
    auto result = document_.interactFacing();
    if (loaded_) {
        refresh();
    }
    return result;
}

void Dungeon3DWorldPanel::addInventoryItem(std::string item_id) {
    document_.addInventoryItem(std::move(item_id));
    if (loaded_) {
        refresh();
    }
}

bool Dungeon3DWorldPanel::completeMarker(std::string marker_id) {
    const bool completed = document_.completeMarker(std::move(marker_id));
    if (loaded_) {
        refresh();
    }
    return completed;
}

bool Dungeon3DWorldPanel::disarmTrap(std::string trap_id) {
    const bool disarmed = document_.disarmTrap(std::move(trap_id));
    if (loaded_) {
        refresh();
    }
    return disarmed;
}

bool Dungeon3DWorldPanel::enterHidingSpot(std::string hiding_spot_id) {
    const bool entered = document_.enterHidingSpot(std::move(hiding_spot_id));
    if (loaded_) {
        refresh();
    }
    return entered;
}

bool Dungeon3DWorldPanel::leaveHidingSpot() {
    const bool left = document_.leaveHidingSpot();
    if (loaded_) {
        refresh();
    }
    return left;
}

bool Dungeon3DWorldPanel::advancePatrol(std::string patrol_id) {
    const bool advanced = document_.advancePatrol(std::move(patrol_id));
    if (loaded_) {
        refresh();
    }
    return advanced;
}

bool Dungeon3DWorldPanel::activatePuzzle(std::string puzzle_id) {
    const bool activated = document_.activatePuzzle(std::move(puzzle_id));
    if (loaded_) {
        refresh();
    }
    return activated;
}

void Dungeon3DWorldPanel::rotate(float radians) {
    document_.rotate(radians);
    if (loaded_) {
        refresh();
    }
}

void Dungeon3DWorldPanel::render() {
    snapshot_.visible = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load a 3D dungeon world document before rendering this panel.";
        return;
    }
    refresh();
}

nlohmann::json Dungeon3DWorldPanel::saveProjectData() const {
    return document_.toJson();
}

void Dungeon3DWorldPanel::refresh() {
    preview_ = document_.preview();
    snapshot_.disabled = false;
    snapshot_.map_id = document_.map_id;
    snapshot_.mode = document_.mode;
    snapshot_.raycast_column_count = preview_.columns.size();
    snapshot_.minimap_tile_count = preview_.minimap_tiles.size();
    snapshot_.discovered_tile_count = static_cast<size_t>(
        std::count_if(preview_.minimap_tiles.begin(), preview_.minimap_tiles.end(), [](const auto& tile) {
            return tile.discovered;
        }));
    snapshot_.visible_minimap_tile_count = static_cast<size_t>(
        std::count_if(preview_.minimap_tiles.begin(), preview_.minimap_tiles.end(), [](const auto& tile) {
            return tile.visible;
        }));
    snapshot_.blocking_cell_count = static_cast<size_t>(preview_.blocking_cell_count);
    snapshot_.event_cell_count = static_cast<size_t>(preview_.event_cell_count);
    snapshot_.door_count = static_cast<size_t>(preview_.door_count);
    snapshot_.secret_count = static_cast<size_t>(preview_.secret_count);
    snapshot_.encounter_cell_count = static_cast<size_t>(preview_.encounter_cell_count);
    snapshot_.marker_count = static_cast<size_t>(preview_.marker_count);
    snapshot_.visible_marker_count = static_cast<size_t>(preview_.visible_marker_count);
    snapshot_.objective_count = static_cast<size_t>(preview_.objective_count);
    snapshot_.completed_objective_count = static_cast<size_t>(preview_.completed_objective_count);
    snapshot_.note_count = static_cast<size_t>(preview_.note_count);
    snapshot_.encounter_zone_count = static_cast<size_t>(preview_.encounter_zone_count);
    snapshot_.lock_link_count = static_cast<size_t>(preview_.lock_link_count);
    snapshot_.trap_count = static_cast<size_t>(preview_.trap_count);
    snapshot_.armed_trap_count = static_cast<size_t>(preview_.armed_trap_count);
    snapshot_.audio_zone_count = static_cast<size_t>(preview_.audio_zone_count);
    snapshot_.patrol_count = static_cast<size_t>(preview_.patrol_count);
    snapshot_.active_patrol_count = static_cast<size_t>(preview_.active_patrol_count);
    snapshot_.alerted_patrol_count = static_cast<size_t>(preview_.alerted_patrol_count);
    snapshot_.hiding_spot_count = static_cast<size_t>(preview_.hiding_spot_count);
    snapshot_.puzzle_device_count = static_cast<size_t>(preview_.puzzle_device_count);
    snapshot_.active_puzzle_device_count = static_cast<size_t>(preview_.active_puzzle_device_count);
    snapshot_.solved_puzzle_count = static_cast<size_t>(preview_.solved_puzzle_count);
    snapshot_.opened_door_count = static_cast<size_t>(preview_.opened_door_count);
    snapshot_.revealed_secret_count = static_cast<size_t>(preview_.revealed_secret_count);
    snapshot_.runtime_command_count = preview_.runtime_commands.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
    snapshot_.camera_x = document_.camera.pos_x;
    snapshot_.camera_y = document_.camera.pos_y;
    snapshot_.average_wall_distance = preview_.average_wall_distance;
    snapshot_.floor_completion = preview_.floor_completion;
    snapshot_.current_light_multiplier = preview_.current_light_multiplier;
    snapshot_.active_ambient_sound = preview_.active_ambient_sound;
    snapshot_.active_reverb_preset = preview_.active_reverb_preset;
    snapshot_.active_weather = preview_.active_weather;
    snapshot_.active_particles = preview_.active_particles;
    snapshot_.nearest_patrol_id = preview_.nearest_patrol_id;
    snapshot_.current_hiding_spot = preview_.current_hiding_spot;
    snapshot_.facing_event_id = preview_.facing_interaction ? preview_.facing_interaction->event_id : "";
    snapshot_.facing_material_id = preview_.facing_interaction ? preview_.facing_interaction->material_id : "";
    snapshot_.facing_door_id = preview_.facing_interaction ? preview_.facing_interaction->door_id : "";
    snapshot_.facing_required_item = preview_.facing_interaction ? preview_.facing_interaction->required_item : "";
    snapshot_.facing_floor_transfer = preview_.facing_interaction ? preview_.facing_interaction->floor_transfer : "";
    snapshot_.facing_secret = preview_.facing_interaction ? preview_.facing_interaction->secret : false;
    snapshot_.facing_locked = preview_.facing_interaction ? preview_.facing_interaction->locked : false;
    snapshot_.facing_can_open = preview_.facing_interaction ? preview_.facing_interaction->can_open : false;
    snapshot_.facing_can_transfer = preview_.facing_interaction ? preview_.facing_interaction->can_transfer : false;
    snapshot_.facing_blocked = preview_.facing_interaction ? preview_.facing_interaction->blocking : false;
    snapshot_.player_hidden = preview_.player_hidden;
    snapshot_.current_floor_id = document_.session.current_floor_id;
    snapshot_.last_event_log_entry = document_.session.event_log.empty() ? "" : document_.session.event_log.back();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "3D dungeon world preview is ready." : "3D dungeon world has diagnostics.";
}

} // namespace urpg::editor
