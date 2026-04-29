#include "editor/spatial/map_environment_preview_panel.h"
#include "editor/spatial/procedural_map_panel.h"
#include "editor/spatial/region_rules_panel.h"
#include "editor/spatial/terrain_brush_panel.h"
#include "engine/core/map/map_environment_preview.h"
#include "engine/core/map/spawn_table.h"
#include "engine/core/map/tactical_grid_preview.h"
#include "engine/core/map/tile_layer_document.h"
#include "engine/core/presentation/presentation_runtime.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path mapWorldbuildingRepoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

nlohmann::json loadMapWorldbuildingJson(const std::filesystem::path& path) {
    std::ifstream stream(path);
    REQUIRE(stream.is_open());
    nlohmann::json json;
    stream >> json;
    return json;
}

size_t countPresentationCommand(const urpg::presentation::PresentationFrameIntent& intent,
                                urpg::presentation::PresentationCommand::Type type) {
    return static_cast<size_t>(std::count_if(intent.commands.begin(), intent.commands.end(), [type](const auto& cmd) {
        return cmd.type == type;
    }));
}

} // namespace

TEST_CASE("Terrain brush output is deterministic for the same seed", "[map][worldbuilding][ffs06]") {
    urpg::map::TerrainBrush brush;
    brush.mode = urpg::map::TerrainBrushMode::RandomWeighted;
    brush.width = 2;
    brush.height = 2;
    brush.weighted_tiles = {{4, 1}, {9, 3}};

    urpg::editor::TerrainBrushPanel panel_a;
    urpg::editor::TerrainBrushPanel panel_b;
    panel_a.preview(brush, 3, 4, 42);
    panel_b.preview(brush, 3, 4, 42);

    REQUIRE(panel_a.points().size() == 4);
    REQUIRE(panel_a.points().size() == panel_b.points().size());
    for (size_t i = 0; i < panel_a.points().size(); ++i) {
        REQUIRE(panel_a.points()[i].x == panel_b.points()[i].x);
        REQUIRE(panel_a.points()[i].y == panel_b.points()[i].y);
        REQUIRE(panel_a.points()[i].tile_id == panel_b.points()[i].tile_id);
    }
}

TEST_CASE("Locked tile layer rejects edits and navigation validator catches blocked cells", "[map][worldbuilding][ffs06]") {
    urpg::map::TileLayerDocument doc(3, 3);
    doc.addLayer({"terrain", true, true, false, true, 0, {}});
    doc.addLayer({"collision", true, false, true, false, 1, {}});
    doc.addLayer({"nav", true, false, false, true, 2, {}});

    REQUIRE_FALSE(doc.setTile("terrain", 1, 1, 7));
    REQUIRE(doc.setTile("collision", 1, 1, 1));
    REQUIRE(doc.setTile("nav", 1, 1, 1));

    const auto diagnostics = doc.validateNavigation();
    REQUIRE(diagnostics.size() == 1);
    REQUIRE(diagnostics[0].code == "navigation_on_blocked_cell");
    REQUIRE(diagnostics[0].x == 1);
    REQUIRE(diagnostics[0].y == 1);
}

TEST_CASE("Region conflict validator emits exact cell and rule conflict", "[map][worldbuilding][ffs06]") {
    urpg::editor::RegionRulesPanel panel;
    panel.loadRules({
        {"mud", 0, 0, 3, 3, "", "", "rain", "", "slow", ""},
        {"ice", 2, 2, 3, 3, "", "", "snow", "", "slide", ""},
    });
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().diagnostic_count == 1);
    REQUIRE(panel.snapshot().status_message == "Region rules have diagnostics.");
    REQUIRE(panel.diagnostics()[0].code == "region_movement_conflict");
    REQUIRE(panel.diagnostics()[0].x == 2);
    REQUIRE(panel.diagnostics()[0].y == 2);
    REQUIRE(panel.diagnostics()[0].target == "mud:ice");
}

TEST_CASE("Region rules panel renders an explicit disabled empty state", "[map][worldbuilding][editor][spatial]") {
    urpg::editor::RegionRulesPanel panel;

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().rule_count == 0);
    REQUIRE(panel.snapshot().diagnostic_count == 0);
    REQUIRE(panel.snapshot().status_message == "Load region rules before previewing this panel.");
}

TEST_CASE("Map environment preview turns region rules into live runtime lighting and weather",
          "[map][worldbuilding][environment][wysiwyg]") {
    const auto document = urpg::map::MapEnvironmentPreviewDocument::fromRegionRules(
        "forest_storm_pass",
        12,
        8,
        {
            {"rain_path", 2, 1, 4, 3, "", "rain_loop", "rain", "", "normal", ""},
            {"night_clearing", 7, 2, 3, 3, "", "crickets", "fog", "dark", "normal", ""},
        });

    urpg::editor::MapEnvironmentPreviewPanel panel;
    panel.loadDocument(document);
    panel.selectTile(3, 2);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().map_id == "forest_storm_pass");
    REQUIRE(panel.snapshot().region_id == "rain_path");
    REQUIRE(panel.snapshot().weather == "rain");
    REQUIRE(panel.snapshot().diagnostic_count == 0);
    REQUIRE(panel.snapshot().status_message == "Map environment preview is ready.");
    REQUIRE(panel.snapshot().ux_focus_lane == "lighting_weather");
    REQUIRE(panel.snapshot().selected_tile_summary == "rain_path:rain");
    REQUIRE(panel.snapshot().overlay_density == 0.0f);
    REQUIRE_FALSE(panel.snapshot().saved_project_json.empty());
    REQUIRE(panel.snapshot().region_overlay_count == 2);

    const auto& intent = panel.preview().runtime_intent;
    REQUIRE(countPresentationCommand(intent, urpg::presentation::PresentationCommand::Type::SetFog) == 1);
    REQUIRE(countPresentationCommand(intent, urpg::presentation::PresentationCommand::Type::SetPostFX) == 1);
    REQUIRE(countPresentationCommand(intent, urpg::presentation::PresentationCommand::Type::SetLight) == 2);
    REQUIRE(intent.resolvedFogProfiles.size() == 1);
    REQUIRE(intent.resolvedFogProfiles[0].density > 0.006f);
}

TEST_CASE("Map environment preview fixture is saved project data and executes in presentation runtime",
          "[map][worldbuilding][environment][wysiwyg]") {
    const auto json = loadMapWorldbuildingJson(
        mapWorldbuildingRepoRoot() / "content" / "fixtures" / "map_environment_preview_fixture.json");
    const auto document = urpg::map::MapEnvironmentPreviewDocument::fromJson(json);

    REQUIRE(document.map_id == "forest_weather_lab");
    REQUIRE(document.regions.size() == 2);
    REQUIRE(document.tile_layers.size() == 2);
    REQUIRE(document.parallax_layers.size() == 2);
    REQUIRE(document.smoke_emitters.size() == 1);
    REQUIRE(document.terrain_mesh_rules.size() == 1);
    REQUIRE(document.edge_scroll_camera.enabled);
    REQUIRE(document.tactical_overlay.enabled);
    REQUIRE(document.spawn_table.entries.size() == 2);
    REQUIRE(document.validate().empty());

    const auto saved = document.toJson();
    REQUIRE(saved["schema"] == "urpg.map_environment_preview.v1");
    REQUIRE(saved["tile_layers"].size() == 2);
    REQUIRE(saved["regions"].size() == 2);
    REQUIRE(saved["parallax_layers"].size() == 2);
    REQUIRE(saved["smoke_emitters"].size() == 1);
    REQUIRE(saved["terrain_mesh_rules"].size() == 1);
    REQUIRE(saved["edge_scroll_camera"]["enabled"].get<bool>());
    REQUIRE(saved["tactical_overlay"]["enabled"].get<bool>());
    REQUIRE(saved["spawn_table"]["entries"].size() == 2);

    const auto result = urpg::map::PreviewMapEnvironment(document, 8, 3);
    REQUIRE(result.region != nullptr);
    REQUIRE(result.region->id == "night_clearing");
    REQUIRE(result.diagnostics.empty());
    REQUIRE(result.visible_tile_layer_count == 2);
    REQUIRE(result.collision_tile_count == 2);
    REQUIRE(result.region_overlay_count == 2);
    REQUIRE(result.parallax_layer_count == 2);
    REQUIRE(result.smoke_emitter_count == 1);
    REQUIRE(result.terrain_mesh_rule_count == 1);
    REQUIRE(result.edge_scroll_enabled);
    REQUIRE(result.tactical_reachable_count > 0);
    REQUIRE(result.spawn_entry_count == 2);
    REQUIRE_FALSE(result.selected_tile_blocked);
    REQUIRE(result.runtime_overlay_commands.size() == 8);
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "render_tile_layers:forest_weather_lab:2") != result.runtime_overlay_commands.end());
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "render_spawn_overlay:forest_weather_spawns:2") != result.runtime_overlay_commands.end());
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "render_parallax_layers:forest_weather_lab:2") != result.runtime_overlay_commands.end());
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "render_region_smoke:forest_weather_lab:1") != result.runtime_overlay_commands.end());
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "render_terrain_mesh_rules:forest_weather_lab:1") != result.runtime_overlay_commands.end());
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "camera_edge_scroll:forest_weather_lab:32") != result.runtime_overlay_commands.end());
    REQUIRE(countPresentationCommand(result.runtime_intent, urpg::presentation::PresentationCommand::Type::SetFog) == 1);
    REQUIRE(countPresentationCommand(result.runtime_intent, urpg::presentation::PresentationCommand::Type::SetPostFX) == 1);

    urpg::editor::MapEnvironmentPreviewPanel panel;
    panel.loadDocument(document);
    panel.selectTile(8, 3);
    panel.render();

    REQUIRE(panel.snapshot().visible_tile_layer_count == 2);
    REQUIRE(panel.snapshot().runtime_overlay_command_count == 8);
    REQUIRE(panel.snapshot().parallax_layer_count == 2);
    REQUIRE(panel.snapshot().smoke_emitter_count == 1);
    REQUIRE(panel.snapshot().terrain_mesh_rule_count == 1);
    REQUIRE(panel.snapshot().edge_scroll_enabled);
    REQUIRE(panel.snapshot().tactical_reachable_count == result.tactical_reachable_count);
    REQUIRE(panel.snapshot().spawn_entry_count == 2);
    REQUIRE(panel.snapshot().ux_focus_lane == "spawns");
    REQUIRE(panel.snapshot().primary_action.find("spawn-table") != std::string::npos);
}

TEST_CASE("Map environment preview absorbs parallax smoke terrain and edge-scroll camera settings",
          "[map][worldbuilding][environment][native-plugin-absorption]") {
    urpg::map::MapEnvironmentPreviewDocument document;
    document.map_id = "plugin_absorption_map";
    document.width = 4;
    document.height = 4;
    document.base_weather = "clear";
    document.parallax_layers = {
        {"clouds", "parallax/clouds.png", "screen", "add", 0.6f, 0.1f, 0.0f, 0, 3},
    };
    document.smoke_emitters = {
        {"smoke_region", 2, 5, 1.25f, 160, "#ffffff", true, 0.1f, -0.2f},
    };
    document.terrain_mesh_rules = {
        {"hill_region", 3, 2.0f, 0.25f, true},
    };
    document.edge_scroll_camera = {true, true, true, 8.0f, 24, "Space"};

    const auto result = urpg::map::PreviewMapEnvironment(document, 1, 1);
    REQUIRE(result.diagnostics.empty());
    REQUIRE(result.parallax_layer_count == 1);
    REQUIRE(result.smoke_emitter_count == 1);
    REQUIRE(result.terrain_mesh_rule_count == 1);
    REQUIRE(result.edge_scroll_enabled);
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "render_parallax_layers:plugin_absorption_map:1") != result.runtime_overlay_commands.end());
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "render_region_smoke:plugin_absorption_map:1") != result.runtime_overlay_commands.end());
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "render_terrain_mesh_rules:plugin_absorption_map:1") != result.runtime_overlay_commands.end());
    REQUIRE(std::find(result.runtime_overlay_commands.begin(),
                      result.runtime_overlay_commands.end(),
                      "camera_edge_scroll:plugin_absorption_map:24") != result.runtime_overlay_commands.end());

    urpg::editor::MapEnvironmentPreviewPanel panel;
    panel.loadDocument(document);
    panel.selectTile(1, 1);
    panel.render();
    REQUIRE(panel.snapshot().parallax_layer_count == 1);
    REQUIRE(panel.snapshot().smoke_emitter_count == 1);
    REQUIRE(panel.snapshot().terrain_mesh_rule_count == 1);
    REQUIRE(panel.snapshot().edge_scroll_enabled);
}

TEST_CASE("Map environment preview diagnostics block false completion claims",
          "[map][worldbuilding][environment][wysiwyg]") {
    urpg::map::MapEnvironmentPreviewDocument document;
    document.map_id = "broken_weather";
    document.width = 4;
    document.height = 4;
    document.base_weather = "clear";
    document.regions = {
        {"storm", 1, 1, 4, 2, "storm", "day", {}, {}, {}},
        {"storm", 2, 2, 2, 2, "acid", "day", {}, {}, {}},
    };
    document.tile_layers = {
        {"collision", true, false, true, false, 0, {1}},
    };
    document.tactical_overlay = {true, 0, 0, 2};
    document.spawn_table = {"broken_spawns", {{"blocked_spawn", "slime", 0, 0, 10, true}}};
    document.regions[0].light.range = 0.0f;
    document.regions[0].fog.endDist = 0.0f;
    document.regions[0].fog.startDist = 1.0f;

    urpg::editor::MapEnvironmentPreviewPanel panel;
    panel.loadDocument(document);
    panel.selectTile(9, 9);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().diagnostic_count >= 5);
    REQUIRE(panel.snapshot().ux_focus_lane == "diagnostics");
    REQUIRE(panel.snapshot().status_message == "Map environment preview has diagnostics.");

    const auto& diagnostics = panel.preview().diagnostics;
    const auto hasCode = [&diagnostics](const std::string& code) {
        return std::any_of(diagnostics.begin(), diagnostics.end(), [&code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
    };
    REQUIRE(hasCode("region_out_of_bounds"));
    REQUIRE(hasCode("duplicate_region_id"));
    REQUIRE(hasCode("unknown_weather"));
    REQUIRE(hasCode("invalid_light_profile"));
    REQUIRE(hasCode("invalid_fog_profile"));
    REQUIRE(hasCode("tile_layer_size_mismatch"));
    REQUIRE(hasCode("tactical_origin_blocked"));
    REQUIRE(hasCode("spawn_on_blocked_tile"));
    REQUIRE(hasCode("preview_tile_out_of_bounds"));
}

TEST_CASE("Procedural generator reports unsatisfied constraints and returns editable document", "[map][worldbuilding][ffs06]") {
    urpg::editor::ProceduralMapPanel panel;
    panel.generate({"tiny_boss", "dungeon", 4, 4, 7, true, true, true});
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().has_result);
    REQUIRE(panel.snapshot().width == 4);
    REQUIRE(panel.snapshot().height == 4);
    REQUIRE(panel.snapshot().layer_count == 2);
    REQUIRE(panel.snapshot().diagnostic_count == 1);
    REQUIRE(panel.snapshot().status_message == "Procedural map preview has diagnostics.");
    REQUIRE(panel.result().diagnostics[0].code == "unsatisfied_constraints");

    auto doc = panel.result().document;
    REQUIRE(doc.setTile("terrain", 1, 1, 42));
    REQUIRE(doc.tileAt("terrain", 1, 1).value() == 42);
}

TEST_CASE("Procedural map panel renders an explicit disabled empty state", "[map][worldbuilding][editor][spatial]") {
    urpg::editor::ProceduralMapPanel panel;

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().disabled);
    REQUIRE_FALSE(panel.snapshot().has_result);
    REQUIRE(panel.snapshot().width == 0);
    REQUIRE(panel.snapshot().height == 0);
    REQUIRE(panel.snapshot().status_message == "Generate a procedural map profile before previewing this panel.");
}

TEST_CASE("Tactical range preview respects blocked cells", "[map][worldbuilding][ffs06]") {
    const auto reachable = urpg::map::PreviewTacticalMoveRange(
        5,
        5,
        {2, 2},
        2,
        {{2, 1}, {3, 2}}
    );

    REQUIRE(reachable.contains({2, 2}));
    REQUIRE_FALSE(reachable.contains({2, 1}));
    REQUIRE_FALSE(reachable.contains({3, 2}));
    REQUIRE(reachable.contains({1, 2}));
}

TEST_CASE("Spawn persistence round-trips through save JSON and validates missing enemies", "[map][worldbuilding][ffs06]") {
    urpg::map::SpawnTable table;
    table.id = "forest_spawns";
    table.entries = {
        {"slime_spawn", "slime", 2, 3, 30, true},
        {"ghost_spawn", "ghost", 4, 5, 60, false},
    };

    const auto json = urpg::map::SpawnTableToJson(table);
    const auto restored = urpg::map::SpawnTableFromJson(json);
    REQUIRE(restored.id == table.id);
    REQUIRE(restored.entries.size() == 2);
    REQUIRE(restored.entries[0].persistent);

    const auto diagnostics = urpg::map::ValidateSpawnTable(restored, {"slime"});
    REQUIRE(diagnostics.size() == 1);
    REQUIRE(diagnostics[0].code == "missing_spawn_enemy");
    REQUIRE(diagnostics[0].target == "ghost");
}
