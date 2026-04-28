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
    REQUIRE(document.materials.size() == 2);
    REQUIRE(document.validate().empty());

    const auto preview = document.preview();
    REQUIRE(preview.diagnostics.empty());
    REQUIRE(preview.columns.size() == 64);
    REQUIRE(preview.minimap_tiles.size() == 25);
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
    REQUIRE(panel.snapshot().runtime_command_count == 3);
    REQUIRE(panel.saveProjectData() == document.toJson());
    REQUIRE(urpg::render::dungeon3DPreviewToJson(panel.preview())["columns"].size() == 64);

    panel.setMode("2d");
    REQUIRE(panel.snapshot().mode == "2d");
    REQUIRE(panel.snapshot().raycast_column_count == 0);
    REQUIRE(panel.snapshot().runtime_command_count == 2);
    REQUIRE(containsCommand(panel.preview().runtime_commands, "switch_to_2d:ancient_crypt"));

    const auto switched = document.switchMode("3d");
    REQUIRE(document.mode == "3d");
    REQUIRE(switched.columns.size() == 64);
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
    document.cells.push_back({"floor", "missing_material", "", false, false, false, false});
    document.materials["stone"] = {"stone", "", "floor.png", "ceiling.png"};

    const auto diagnostics = document.validate();
    REQUIRE(diagnostics.size() >= 3);

    const auto preview = document.preview();
    REQUIRE_FALSE(preview.diagnostics.empty());
    REQUIRE(preview.columns.empty());
}
