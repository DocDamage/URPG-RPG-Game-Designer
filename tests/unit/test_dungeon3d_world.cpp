#include "editor/spatial/dungeon3d_world_panel.h"
#include "engine/core/editor/editor_panel_registry.h"
#include "engine/core/render/dungeon3d_world.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace {

nlohmann::json loadFixture(const std::string& name) {
    const auto path = std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" / (name + "_fixture.json");
    std::ifstream stream(path);
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

bool containsCommand(const std::vector<std::string>& commands, const std::string& command) {
    return std::find(commands.begin(), commands.end(), command) != commands.end();
}

bool containsValue(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

} // namespace

TEST_CASE("3D dungeon world converts 2D map data into runtime raycast preview", "[dungeon3d][wysiwyg]") {
    const auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));

    REQUIRE(document.id == "dungeon3d.demo");
    REQUIRE(document.map_id == "ancient_crypt");
    REQUIRE(document.width == 5);
    REQUIRE(document.height == 5);
    REQUIRE(document.cells.size() == 25);
    REQUIRE(document.floors.size() == 2);
    REQUIRE(document.markers.size() == 4);
    REQUIRE(document.notes.size() == 1);
    REQUIRE(document.encounter_zones.size() == 1);
    REQUIRE(document.lock_links.size() == 1);
    REQUIRE(document.traps.size() == 1);
    REQUIRE(document.audio_zones.size() == 1);
    REQUIRE(document.atmospheres.size() == 2);
    REQUIRE(document.patrol_routes.size() == 1);
    REQUIRE(document.hiding_spots.size() == 1);
    REQUIRE(document.puzzle_devices.size() == 1);
    REQUIRE(document.camera_rails.size() == 1);
    REQUIRE(document.room_templates.size() == 1);
    REQUIRE(document.boss_arenas.size() == 1);
    REQUIRE(document.visual_verification_steps.size() == 2);
    REQUIRE(document.template_bindings.size() == 2);
    REQUIRE(document.camera_feel.fov == 72.0f);
    REQUIRE(document.materials.size() == 2);
    REQUIRE(document.validate().empty());

    const auto preview = document.preview();
    REQUIRE(preview.diagnostics.empty());
    REQUIRE(preview.columns.size() == 64);
    REQUIRE_FALSE(preview.columns.front().material_id.empty());
    REQUIRE_FALSE(preview.columns.front().wall_texture.empty());
    REQUIRE(preview.columns.front().projected_wall_height > 0);
    REQUIRE(preview.average_wall_distance > 0.0f);
    REQUIRE(preview.minimap_tiles.size() == 25);
    REQUIRE(preview.blocking_cell_count == 18);
    REQUIRE(preview.event_cell_count == 5);
    REQUIRE(preview.door_count == 1);
    REQUIRE(preview.secret_count == 1);
    REQUIRE(preview.encounter_cell_count == 1);
    REQUIRE(preview.marker_count == 4);
    REQUIRE(preview.visible_marker_count == 4);
    REQUIRE(preview.objective_count == 1);
    REQUIRE(preview.completed_objective_count == 0);
    REQUIRE(preview.note_count == 1);
    REQUIRE(preview.encounter_zone_count == 1);
    REQUIRE(preview.lock_link_count == 1);
    REQUIRE(preview.trap_count == 1);
    REQUIRE(preview.armed_trap_count == 1);
    REQUIRE(preview.audio_zone_count == 1);
    REQUIRE(preview.patrol_count == 1);
    REQUIRE(preview.active_patrol_count == 1);
    REQUIRE(preview.alerted_patrol_count == 0);
    REQUIRE(preview.hiding_spot_count == 1);
    REQUIRE(preview.puzzle_device_count == 1);
    REQUIRE(preview.active_puzzle_device_count == 1);
    REQUIRE(preview.solved_puzzle_count == 0);
    REQUIRE(preview.camera_rail_cue_count == 1);
    REQUIRE(preview.room_template_count == 1);
    REQUIRE(preview.placed_room_template_count == 0);
    REQUIRE(preview.boss_arena_count == 1);
    REQUIRE(preview.active_boss_arena_count == 0);
    REQUIRE(preview.defeated_boss_arena_count == 0);
    REQUIRE(preview.visual_authoring_layer_count == 13);
    REQUIRE(preview.verification_step_count == 2);
    REQUIRE(preview.required_verification_step_count == 2);
    REQUIRE(preview.passed_verification_step_count == 0);
    REQUIRE(preview.template_binding_count == 2);
    REQUIRE(preview.enabled_template_binding_count == 2);
    REQUIRE(preview.verification_completion == 0.0f);
    REQUIRE(containsValue(preview.visual_authoring_layers, "lighting_weather"));
    REQUIRE(containsValue(preview.visual_authoring_layers, "spawn_tables"));
    REQUIRE(containsValue(preview.visual_authoring_layers, "stealth"));
    REQUIRE(containsValue(preview.visual_authoring_layers, "boss_arenas"));
    REQUIRE(containsValue(preview.template_binding_ids, "2_5d_rpg"));
    REQUIRE(containsValue(preview.template_binding_ids, "metroidvania_lite"));
    REQUIRE(preview.camera_fov == 72.0f);
    REQUIRE(preview.camera_head_bob == 0.05f);
    REQUIRE_FALSE(preview.player_hidden);
    REQUIRE(preview.floor_completion == 0.0f);
    REQUIRE(preview.current_light_multiplier == 0.85f);
    REQUIRE(preview.active_ambient_sound == "ambience_crypt_echo");
    REQUIRE(preview.active_reverb_preset == "stone_room");
    REQUIRE(preview.active_weather == "dust_motes");
    REQUIRE(preview.active_particles == "slow_ash");
    REQUIRE(preview.nearest_patrol_id == "skeleton_guard_route");
    REQUIRE(preview.facing_interaction.has_value());
    REQUIRE(preview.facing_interaction->event_id.empty());
    REQUIRE(containsCommand(preview.runtime_commands, "switch_to_3d:ancient_crypt"));
    REQUIRE(containsCommand(preview.runtime_commands, "camera_feel:ancient_crypt"));
    REQUIRE(containsCommand(preview.runtime_commands, "preview_camera_rail:door_reveal_rail"));
    REQUIRE(containsCommand(preview.runtime_commands, "preview_room_template:crypt_guard_room"));
    REQUIRE(containsCommand(preview.runtime_commands, "preview_boss_arena:crypt_champion_arena"));
    REQUIRE(containsCommand(preview.runtime_commands, "bind_template_3d:2_5d_rpg"));
    REQUIRE(containsCommand(preview.runtime_commands, "bind_template_3d:metroidvania_lite"));
    REQUIRE(containsCommand(preview.runtime_commands, "raycast_frame:ancient_crypt"));
    REQUIRE(containsCommand(preview.runtime_commands, "update_automap:ancient_crypt"));
}

TEST_CASE("3D dungeon world supports 2D to 3D switching and WYSIWYG panel snapshots", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(document);
    panel.render();

    REQUIRE(panel.snapshot().map_id == "ancient_crypt");
    REQUIRE(panel.snapshot().mode == "3d");
    REQUIRE(panel.snapshot().raycast_column_count == 64);
    REQUIRE(panel.snapshot().minimap_tile_count == 25);
    REQUIRE(panel.snapshot().discovered_tile_count > 0);
    REQUIRE(panel.snapshot().visible_minimap_tile_count > 0);
    REQUIRE(panel.snapshot().blocking_cell_count == 18);
    REQUIRE(panel.snapshot().event_cell_count == 5);
    REQUIRE(panel.snapshot().door_count == 1);
    REQUIRE(panel.snapshot().secret_count == 1);
    REQUIRE(panel.snapshot().encounter_cell_count == 1);
    REQUIRE(panel.snapshot().marker_count == 4);
    REQUIRE(panel.snapshot().visible_marker_count == 4);
    REQUIRE(panel.snapshot().objective_count == 1);
    REQUIRE(panel.snapshot().completed_objective_count == 0);
    REQUIRE(panel.snapshot().note_count == 1);
    REQUIRE(panel.snapshot().encounter_zone_count == 1);
    REQUIRE(panel.snapshot().lock_link_count == 1);
    REQUIRE(panel.snapshot().trap_count == 1);
    REQUIRE(panel.snapshot().armed_trap_count == 1);
    REQUIRE(panel.snapshot().audio_zone_count == 1);
    REQUIRE(panel.snapshot().patrol_count == 1);
    REQUIRE(panel.snapshot().active_patrol_count == 1);
    REQUIRE(panel.snapshot().alerted_patrol_count == 0);
    REQUIRE(panel.snapshot().hiding_spot_count == 1);
    REQUIRE(panel.snapshot().puzzle_device_count == 1);
    REQUIRE(panel.snapshot().active_puzzle_device_count == 1);
    REQUIRE(panel.snapshot().solved_puzzle_count == 0);
    REQUIRE(panel.snapshot().camera_rail_cue_count == 1);
    REQUIRE(panel.snapshot().room_template_count == 1);
    REQUIRE(panel.snapshot().placed_room_template_count == 0);
    REQUIRE(panel.snapshot().boss_arena_count == 1);
    REQUIRE(panel.snapshot().active_boss_arena_count == 0);
    REQUIRE(panel.snapshot().defeated_boss_arena_count == 0);
    REQUIRE(panel.snapshot().visual_authoring_layer_count == 13);
    REQUIRE(panel.snapshot().verification_step_count == 2);
    REQUIRE(panel.snapshot().required_verification_step_count == 2);
    REQUIRE(panel.snapshot().passed_verification_step_count == 0);
    REQUIRE(panel.snapshot().template_binding_count == 2);
    REQUIRE(panel.snapshot().enabled_template_binding_count == 2);
    REQUIRE(panel.snapshot().verification_completion == 0.0f);
    REQUIRE(containsValue(panel.snapshot().visual_authoring_layers, "lighting_weather"));
    REQUIRE(containsValue(panel.snapshot().template_binding_ids, "2_5d_rpg"));
    REQUIRE(panel.snapshot().camera_fov == 72.0f);
    REQUIRE(panel.snapshot().camera_head_bob == 0.05f);
    REQUIRE(panel.snapshot().active_ambient_sound == "ambience_crypt_echo");
    REQUIRE(panel.snapshot().active_reverb_preset == "stone_room");
    REQUIRE(panel.snapshot().active_weather == "dust_motes");
    REQUIRE(panel.snapshot().active_particles == "slow_ash");
    REQUIRE(panel.snapshot().current_light_multiplier == 0.85f);
    REQUIRE(panel.snapshot().nearest_patrol_id == "skeleton_guard_route");
    REQUIRE_FALSE(panel.snapshot().player_hidden);
    REQUIRE(panel.snapshot().average_wall_distance > 0.0f);
    REQUIRE(panel.snapshot().runtime_command_count == 12);
    REQUIRE(panel.saveProjectData() == document.toJson());
    REQUIRE(urpg::render::dungeon3DPreviewToJson(panel.preview())["columns"].size() == 64);

    panel.setMode("2d");
    REQUIRE(panel.snapshot().mode == "2d");
    REQUIRE(panel.snapshot().raycast_column_count == 0);
    REQUIRE(panel.snapshot().runtime_command_count == 11);
    REQUIRE(containsCommand(panel.preview().runtime_commands, "switch_to_2d:ancient_crypt"));

    const auto switched = document.switchMode("3d");
    REQUIRE(document.mode == "3d");
    REQUIRE(switched.columns.size() == 64);
}

TEST_CASE("3D dungeon world supports movement collision and facing interactions", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));

    auto moved = document.moveForward(0.5f);
    REQUIRE(moved.moved);
    REQUIRE_FALSE(moved.blocked);
    REQUIRE(moved.encounter_triggered);
    REQUIRE(moved.command == "trigger_encounter:crypt_slime");
    REQUIRE(document.camera.pos_x > 2.5f);

    moved = document.moveForward(1.0f);
    REQUIRE(moved.blocked);
    REQUIRE(moved.command == "blocked:wall");
    REQUIRE(moved.diagnostic.code == "navigation_blocked");

    document.rotate(3.14159265f);
    auto preview = document.preview();
    REQUIRE(preview.facing_interaction.has_value());
    REQUIRE(preview.facing_interaction->event_id == "spawn");

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(document);
    panel.render();
    REQUIRE(panel.snapshot().facing_event_id == "spawn");
    REQUIRE_FALSE(panel.snapshot().facing_material_id.empty());

    const auto sidestep = panel.strafe(0.5f);
    REQUIRE(sidestep.moved);
    REQUIRE(panel.snapshot().camera_y != document.camera.pos_y);
}

TEST_CASE("3D dungeon world resolves doors secrets floor transfers and session automap", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));

    document.camera.pos_x = 2.5f;
    document.camera.pos_y = 3.5f;
    document.camera.dir_x = 1.0f;
    document.camera.dir_y = 0.0f;
    document.camera.plane_x = 0.0f;
    document.camera.plane_y = 0.66f;

    auto preview = document.preview();
    REQUIRE(preview.facing_interaction.has_value());
    REQUIRE(preview.facing_interaction->door_id == "crypt_door");
    REQUIRE(preview.facing_interaction->locked);
    REQUIRE_FALSE(preview.facing_interaction->can_open);

    auto interaction = document.interactFacing();
    REQUIRE_FALSE(interaction.handled);
    REQUIRE(interaction.command == "locked_door:crypt_door");
    REQUIRE(interaction.diagnostic.code == "missing_required_item");

    document.addInventoryItem("crypt_key");
    preview = document.preview();
    REQUIRE(preview.facing_interaction->can_open);

    interaction = document.interactFacing();
    REQUIRE(interaction.handled);
    REQUIRE(interaction.opened_door);
    REQUIRE(interaction.command == "open_door:crypt_door");

    preview = document.preview();
    REQUIRE(preview.opened_door_count == 1);
    REQUIRE_FALSE(preview.facing_interaction->blocking);

    interaction = document.interactFacing();
    REQUIRE(interaction.transferred_floor);
    REQUIRE(interaction.command == "transfer_floor:crypt_b2");
    REQUIRE(document.session.current_floor_id == "crypt_b2");

    document.camera.pos_x = 2.5f;
    document.camera.pos_y = 2.5f;
    document.camera.dir_x = -1.0f;
    document.camera.dir_y = 0.0f;
    document.camera.plane_x = 0.0f;
    document.camera.plane_y = -0.66f;

    preview = document.preview();
    REQUIRE(preview.facing_interaction.has_value());
    REQUIRE(preview.facing_interaction->secret);
    REQUIRE(preview.facing_interaction->blocking);

    interaction = document.interactFacing();
    REQUIRE(interaction.revealed_secret);
    REQUIRE(interaction.command == "reveal_secret:1,2");

    const auto moved = document.moveForward(1.0f);
    REQUIRE(moved.moved);
    REQUIRE_FALSE(moved.blocked);
    REQUIRE(document.session.discovered_cells.contains("1,2"));

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(document);
    panel.render();
    REQUIRE(panel.snapshot().opened_door_count == 1);
    REQUIRE(panel.snapshot().revealed_secret_count == 1);
    REQUIRE(panel.snapshot().current_floor_id == "crypt_b2");
    REQUIRE_FALSE(panel.snapshot().last_event_log_entry.empty());
}

TEST_CASE("3D dungeon world tracks floor objectives markers and notes", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));
    auto preview = document.preview();

    REQUIRE(preview.marker_count == 4);
    REQUIRE(preview.visible_marker_count == 4);
    REQUIRE(preview.objective_count == 1);
    REQUIRE(preview.completed_objective_count == 0);
    REQUIRE(preview.note_count == 1);

    const auto json = urpg::render::dungeon3DPreviewToJson(preview);
    bool found_marker = false;
    bool found_note = false;
    for (const auto& tile : json.at("minimap_tiles")) {
        found_marker = found_marker || !tile.value("marker_id", std::string{}).empty();
        found_note = found_note || !tile.value("note_id", std::string{}).empty();
    }
    REQUIRE(found_marker);
    REQUIRE(found_note);

    REQUIRE(document.completeMarker("obj_unlock_door"));
    preview = document.preview();
    REQUIRE(preview.completed_objective_count == 1);
    REQUIRE(preview.floor_completion == 1.0f);

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(document);
    REQUIRE(panel.snapshot().completed_objective_count == 1);
    REQUIRE(panel.snapshot().floor_completion == 1.0f);
    REQUIRE(panel.snapshot().last_event_log_entry == "complete_marker:obj_unlock_door");
    REQUIRE_FALSE(panel.completeMarker("missing_marker"));
}

TEST_CASE("3D dungeon world runs authored encounter zones traps audio and switch locks", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));

    document.camera.pos_x = 1.5f;
    document.camera.pos_y = 2.5f;
    document.camera.dir_x = 0.0f;
    document.camera.dir_y = 1.0f;
    document.camera.plane_x = -0.66f;
    document.camera.plane_y = 0.0f;

    auto moved = document.moveForward(1.0f);
    REQUIRE(moved.moved);
    REQUIRE(moved.encounter_triggered);
    REQUIRE(moved.trap_triggered);
    REQUIRE(moved.command == "trigger_trap:poison_plate");
    REQUIRE(document.session.triggered_encounters.contains("zone_bone_pile"));
    REQUIRE(document.session.disabled_traps.contains("poison_plate"));

    auto preview = document.preview();
    REQUIRE(preview.armed_trap_count == 0);
    REQUIRE(preview.encounter_zone_count == 1);
    REQUIRE(preview.trap_count == 1);

    const auto json = urpg::render::dungeon3DPreviewToJson(preview);
    REQUIRE(json["trap_count"] == 1);
    REQUIRE(json["armed_trap_count"] == 0);
    REQUIRE(json["active_weather"] == "dust_motes");

    auto switchDocument = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));
    switchDocument.camera.pos_x = 2.5f;
    switchDocument.camera.pos_y = 2.5f;
    switchDocument.camera.dir_x = 0.0f;
    switchDocument.camera.dir_y = -1.0f;
    switchDocument.camera.plane_x = 0.66f;
    switchDocument.camera.plane_y = 0.0f;

    auto interaction = switchDocument.interactFacing();
    REQUIRE(interaction.handled);
    REQUIRE(interaction.activated_switch);
    REQUIRE(interaction.opened_door);
    REQUIRE(interaction.command == "activate_switch:torch_switch_opens_door");
    REQUIRE(interaction.target_id == "crypt_door");
    REQUIRE(switchDocument.session.activated_switches.contains("torch_switch_opens_door"));
    REQUIRE(switchDocument.session.opened_doors.contains("crypt_door"));

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world")));
    REQUIRE(panel.snapshot().armed_trap_count == 1);
    REQUIRE(panel.disarmTrap("poison_plate"));
    REQUIRE(panel.snapshot().armed_trap_count == 0);
    REQUIRE(panel.snapshot().last_event_log_entry == "disarm_trap:poison_plate");
}

TEST_CASE("3D dungeon world previews patrol stealth and hiding spot authoring", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));
    auto preview = document.preview();

    REQUIRE(preview.patrol_count == 1);
    REQUIRE(preview.active_patrol_count == 1);
    REQUIRE(preview.hiding_spot_count == 1);
    REQUIRE(preview.nearest_patrol_id == "skeleton_guard_route");
    REQUIRE_FALSE(preview.player_hidden);

    const auto json = urpg::render::dungeon3DPreviewToJson(preview);
    bool found_patrol = false;
    bool found_hiding_spot = false;
    bool found_vision = false;
    for (const auto& tile : json.at("minimap_tiles")) {
        found_patrol = found_patrol || tile.value("patrol_id", std::string{}) == "skeleton_guard_route";
        found_hiding_spot = found_hiding_spot || tile.value("hiding_spot_id", std::string{}) == "shadow_urn";
        found_vision = found_vision || tile.value("in_patrol_vision", false);
    }
    REQUIRE(found_patrol);
    REQUIRE(found_hiding_spot);
    REQUIRE(found_vision);

    REQUIRE(document.advancePatrol("skeleton_guard_route"));
    preview = document.preview();
    REQUIRE(preview.alerted_patrol_count == 1);
    REQUIRE(document.session.alerted_patrols.contains("skeleton_guard_route"));
    REQUIRE(document.session.event_log.back() == "patrol_alert:skeleton_guard_route");

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world")));
    REQUIRE(panel.enterHidingSpot("shadow_urn"));
    REQUIRE(panel.snapshot().player_hidden);
    REQUIRE(panel.snapshot().current_hiding_spot == "shadow_urn");
    REQUIRE(panel.advancePatrol("skeleton_guard_route"));
    REQUIRE(panel.snapshot().alerted_patrol_count == 0);
    REQUIRE(panel.leaveHidingSpot());
    REQUIRE_FALSE(panel.snapshot().player_hidden);
    REQUIRE(panel.snapshot().last_event_log_entry == "leave_hiding_spot:shadow_urn");
}

TEST_CASE("3D dungeon world activates authored puzzle devices and target links", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));
    auto preview = document.preview();

    REQUIRE(preview.puzzle_device_count == 1);
    REQUIRE(preview.active_puzzle_device_count == 1);
    REQUIRE(preview.solved_puzzle_count == 0);

    const auto json = urpg::render::dungeon3DPreviewToJson(preview);
    bool found_puzzle = false;
    for (const auto& tile : json.at("minimap_tiles")) {
        const bool is_plate = tile.value("puzzle_id", std::string{}) == "pressure_plate_door" &&
                              tile.value("puzzle_type", std::string{}) == "pressure_plate";
        found_puzzle = found_puzzle || is_plate;
    }
    REQUIRE(found_puzzle);

    document.camera.pos_x = 2.5f;
    document.camera.pos_y = 2.5f;
    document.camera.dir_x = 1.0f;
    document.camera.dir_y = 0.0f;
    document.camera.plane_x = 0.0f;
    document.camera.plane_y = 0.66f;

    auto interaction = document.interactFacing();
    REQUIRE(interaction.handled);
    REQUIRE(interaction.activated_puzzle);
    REQUIRE(interaction.opened_door);
    REQUIRE(interaction.command == "activate_puzzle:pressure_plate_door");
    REQUIRE(interaction.target_id == "crypt_door");
    REQUIRE(document.session.activated_puzzles.contains("pressure_plate_door"));
    REQUIRE(document.session.opened_doors.contains("crypt_door"));

    preview = document.preview();
    REQUIRE(preview.solved_puzzle_count == 1);
    REQUIRE(preview.opened_door_count == 1);

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world")));
    REQUIRE(panel.activatePuzzle("pressure_plate_door"));
    REQUIRE(panel.snapshot().solved_puzzle_count == 1);
    REQUIRE(panel.snapshot().opened_door_count == 1);
    REQUIRE(panel.snapshot().last_event_log_entry == "activate_puzzle:pressure_plate_door");
    REQUIRE_FALSE(panel.activatePuzzle("missing_puzzle"));
}

TEST_CASE("3D dungeon world previews camera feel and plays cutscene rails", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));
    auto preview = document.preview();

    REQUIRE(preview.camera_rail_cue_count == 1);
    REQUIRE(preview.camera_fov == 72.0f);
    REQUIRE(preview.camera_head_bob == 0.05f);
    REQUIRE(preview.camera_shake == 0.0f);
    REQUIRE(containsCommand(preview.runtime_commands, "camera_feel:ancient_crypt"));
    REQUIRE(containsCommand(preview.runtime_commands, "preview_camera_rail:door_reveal_rail"));

    REQUIRE(document.playCameraRail("door_reveal_rail"));
    REQUIRE(document.camera.pos_x == 2.5f);
    REQUIRE(document.camera.pos_y == 3.5f);
    REQUIRE(document.session.current_camera_rail == "door_reveal_rail");

    preview = document.preview();
    REQUIRE(preview.active_camera_rail_id == "door_reveal_rail");

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world")));
    REQUIRE(panel.playCameraRail("door_reveal_rail"));
    REQUIRE(panel.snapshot().camera_y == 3.5f);
    REQUIRE(panel.snapshot().active_camera_rail_id == "door_reveal_rail");
    REQUIRE(panel.snapshot().last_event_log_entry == "play_camera_rail:door_reveal_rail");
    REQUIRE_FALSE(panel.playCameraRail("missing_rail"));
}

TEST_CASE("3D dungeon world places room templates and resolves boss arenas", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));
    auto preview = document.preview();

    REQUIRE(preview.room_template_count == 1);
    REQUIRE(preview.placed_room_template_count == 0);
    REQUIRE(preview.boss_arena_count == 1);
    REQUIRE(preview.active_boss_arena_count == 0);
    REQUIRE(preview.defeated_boss_arena_count == 0);
    REQUIRE(containsCommand(preview.runtime_commands, "preview_room_template:crypt_guard_room"));
    REQUIRE(containsCommand(preview.runtime_commands, "preview_boss_arena:crypt_champion_arena"));

    REQUIRE(document.placeRoomTemplate("crypt_guard_room"));
    preview = document.preview();
    REQUIRE(preview.placed_room_template_count == 1);
    REQUIRE_FALSE(document.placeRoomTemplate("crypt_guard_room"));

    REQUIRE(document.startBossArena("crypt_champion_arena"));
    preview = document.preview();
    REQUIRE(preview.active_boss_arena_count == 1);
    REQUIRE(document.session.triggered_encounters.contains("crypt_champion"));
    REQUIRE_FALSE(document.session.opened_doors.contains("crypt_door"));

    REQUIRE(document.defeatBossArena("crypt_champion_arena"));
    preview = document.preview();
    REQUIRE(preview.active_boss_arena_count == 0);
    REQUIRE(preview.defeated_boss_arena_count == 1);
    REQUIRE(document.session.opened_doors.contains("crypt_door"));
    REQUIRE(document.session.completed_markers.contains("boss_reward"));

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world")));
    REQUIRE(panel.placeRoomTemplate("crypt_guard_room"));
    REQUIRE(panel.snapshot().placed_room_template_count == 1);
    REQUIRE(panel.startBossArena("crypt_champion_arena"));
    REQUIRE(panel.snapshot().active_boss_arena_count == 1);
    REQUIRE(panel.defeatBossArena("crypt_champion_arena"));
    REQUIRE(panel.snapshot().defeated_boss_arena_count == 1);
    REQUIRE(panel.snapshot().last_event_log_entry == "defeat_boss_arena:crypt_champion_arena");
    REQUIRE_FALSE(panel.startBossArena("missing_arena"));
}

TEST_CASE("3D dungeon world persists visual verification and template bindings", "[dungeon3d][wysiwyg]") {
    auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));
    auto preview = document.preview();

    REQUIRE(preview.verification_step_count == 2);
    REQUIRE(preview.passed_verification_step_count == 0);
    REQUIRE(preview.template_binding_count == 2);
    REQUIRE(containsCommand(preview.runtime_commands, "bind_template_3d:2_5d_rpg"));

    REQUIRE(document.markVisualVerification("runtime_preview_match", true, "captures/dungeon3d/runtime-preview-match.png"));
    preview = document.preview();
    REQUIRE(preview.passed_verification_step_count == 1);
    REQUIRE(preview.verification_completion == 0.5f);
    REQUIRE(document.visual_verification_steps.front().evidence == "captures/dungeon3d/runtime-preview-match.png");
    REQUIRE(document.session.event_log.back() == "mark_visual_verification:runtime_preview_match:passed");

    const auto json = document.toJson();
    REQUIRE(json.at("visual_verification_steps").at(0).at("passed").get<bool>());
    REQUIRE(json.at("template_bindings").size() == 2);

    auto roundtrip = urpg::render::Dungeon3DWorldDocument::fromJson(json);
    REQUIRE(roundtrip.visual_verification_steps.front().passed);
    REQUIRE(roundtrip.visual_verification_steps.front().evidence == "captures/dungeon3d/runtime-preview-match.png");
    REQUIRE(roundtrip.template_bindings.front().template_id == "2_5d_rpg");

    urpg::editor::Dungeon3DWorldPanel panel;
    panel.loadDocument(urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world")));
    REQUIRE(panel.markVisualVerification("manual_walkthrough", true, "manual-review:2026-04-28"));
    REQUIRE(panel.snapshot().passed_verification_step_count == 1);
    REQUIRE(panel.snapshot().verification_completion == 0.5f);
    REQUIRE(panel.snapshot().last_event_log_entry == "mark_visual_verification:manual_walkthrough:passed");
    REQUIRE_FALSE(panel.markVisualVerification("missing_step", true, "manual-review:missing"));

    auto invalid = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));
    invalid.visual_verification_steps.push_back(invalid.visual_verification_steps.front());
    invalid.template_bindings.push_back(invalid.template_bindings.front());
    const auto diagnostics = invalid.validate();
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "duplicate_visual_verification_id";
    }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "duplicate_template_binding_id";
    }));
}

TEST_CASE("3D dungeon world is release registered with native level building", "[dungeon3d][wysiwyg]") {
    const auto* dungeon = urpg::editor::findEditorPanelRegistryEntry("3d_dungeon_world");
    REQUIRE(dungeon != nullptr);
    REQUIRE(dungeon->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);

    const auto* levelBuilder = urpg::editor::findEditorPanelRegistryEntry("level_builder");
    REQUIRE(levelBuilder != nullptr);
    REQUIRE(levelBuilder->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);

    const auto* spatial = urpg::editor::findEditorPanelRegistryEntry("spatial_authoring");
    REQUIRE(spatial != nullptr);
    REQUIRE(spatial->exposure == urpg::editor::EditorPanelExposure::Nested);
}

TEST_CASE("3D dungeon world reports broken authoring diagnostics", "[dungeon3d][wysiwyg]") {
    urpg::render::Dungeon3DWorldDocument document;
    document.id = "broken";
    document.map_id = "bad_map";
    document.width = 2;
    document.height = 2;
    document.cells.push_back({"floor", "missing_material", "", "", "", "", "", false, false, false, false, false, false});
    document.materials["stone"] = {"stone", "", "floor.png", "ceiling.png", -1.0f, ""};

    const auto diagnostics = document.validate();
    REQUIRE(diagnostics.size() >= 3);

    const auto preview = document.preview();
    REQUIRE_FALSE(preview.diagnostics.empty());
    REQUIRE(preview.columns.empty());
}
