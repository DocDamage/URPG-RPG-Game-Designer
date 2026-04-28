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

} // namespace

TEST_CASE("3D dungeon world converts 2D map data into runtime raycast preview", "[dungeon3d][wysiwyg]") {
    const auto document = urpg::render::Dungeon3DWorldDocument::fromJson(loadFixture("dungeon3d_world"));

    REQUIRE(document.id == "dungeon3d.demo");
    REQUIRE(document.map_id == "ancient_crypt");
    REQUIRE(document.width == 5);
    REQUIRE(document.height == 5);
    REQUIRE(document.cells.size() == 25);
    REQUIRE(document.floors.size() == 2);
    REQUIRE(document.markers.size() == 3);
    REQUIRE(document.notes.size() == 1);
    REQUIRE(document.encounter_zones.size() == 1);
    REQUIRE(document.lock_links.size() == 1);
    REQUIRE(document.traps.size() == 1);
    REQUIRE(document.audio_zones.size() == 1);
    REQUIRE(document.atmospheres.size() == 2);
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
    REQUIRE(preview.marker_count == 3);
    REQUIRE(preview.visible_marker_count == 3);
    REQUIRE(preview.objective_count == 1);
    REQUIRE(preview.completed_objective_count == 0);
    REQUIRE(preview.note_count == 1);
    REQUIRE(preview.encounter_zone_count == 1);
    REQUIRE(preview.lock_link_count == 1);
    REQUIRE(preview.trap_count == 1);
    REQUIRE(preview.armed_trap_count == 1);
    REQUIRE(preview.audio_zone_count == 1);
    REQUIRE(preview.floor_completion == 0.0f);
    REQUIRE(preview.current_light_multiplier == 0.85f);
    REQUIRE(preview.active_ambient_sound == "ambience_crypt_echo");
    REQUIRE(preview.active_reverb_preset == "stone_room");
    REQUIRE(preview.active_weather == "dust_motes");
    REQUIRE(preview.active_particles == "slow_ash");
    REQUIRE(preview.facing_interaction.has_value());
    REQUIRE(preview.facing_interaction->event_id.empty());
    REQUIRE(containsCommand(preview.runtime_commands, "switch_to_3d:ancient_crypt"));
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
    REQUIRE(panel.snapshot().marker_count == 3);
    REQUIRE(panel.snapshot().visible_marker_count == 3);
    REQUIRE(panel.snapshot().objective_count == 1);
    REQUIRE(panel.snapshot().completed_objective_count == 0);
    REQUIRE(panel.snapshot().note_count == 1);
    REQUIRE(panel.snapshot().encounter_zone_count == 1);
    REQUIRE(panel.snapshot().lock_link_count == 1);
    REQUIRE(panel.snapshot().trap_count == 1);
    REQUIRE(panel.snapshot().armed_trap_count == 1);
    REQUIRE(panel.snapshot().audio_zone_count == 1);
    REQUIRE(panel.snapshot().active_ambient_sound == "ambience_crypt_echo");
    REQUIRE(panel.snapshot().active_reverb_preset == "stone_room");
    REQUIRE(panel.snapshot().active_weather == "dust_motes");
    REQUIRE(panel.snapshot().active_particles == "slow_ash");
    REQUIRE(panel.snapshot().current_light_multiplier == 0.85f);
    REQUIRE(panel.snapshot().average_wall_distance > 0.0f);
    REQUIRE(panel.snapshot().runtime_command_count == 6);
    REQUIRE(panel.saveProjectData() == document.toJson());
    REQUIRE(urpg::render::dungeon3DPreviewToJson(panel.preview())["columns"].size() == 64);

    panel.setMode("2d");
    REQUIRE(panel.snapshot().mode == "2d");
    REQUIRE(panel.snapshot().raycast_column_count == 0);
    REQUIRE(panel.snapshot().runtime_command_count == 5);
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

    REQUIRE(preview.marker_count == 3);
    REQUIRE(preview.visible_marker_count == 3);
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

TEST_CASE("3D dungeon world is release registered with promoted spatial authoring", "[dungeon3d][wysiwyg]") {
    const auto* dungeon = urpg::editor::findEditorPanelRegistryEntry("3d_dungeon_world");
    REQUIRE(dungeon != nullptr);
    REQUIRE(dungeon->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);

    const auto* spatial = urpg::editor::findEditorPanelRegistryEntry("spatial_authoring");
    REQUIRE(spatial != nullptr);
    REQUIRE(spatial->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);
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
