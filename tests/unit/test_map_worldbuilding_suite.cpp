#include "editor/spatial/procedural_map_panel.h"
#include "editor/spatial/region_rules_panel.h"
#include "editor/spatial/terrain_brush_panel.h"
#include "engine/core/map/spawn_table.h"
#include "engine/core/map/tactical_grid_preview.h"
#include "engine/core/map/tile_layer_document.h"

#include <catch2/catch_test_macros.hpp>

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

    REQUIRE(panel.snapshot().diagnostic_count == 1);
    REQUIRE(panel.diagnostics()[0].code == "region_movement_conflict");
    REQUIRE(panel.diagnostics()[0].x == 2);
    REQUIRE(panel.diagnostics()[0].y == 2);
    REQUIRE(panel.diagnostics()[0].target == "mud:ice");
}

TEST_CASE("Procedural generator reports unsatisfied constraints and returns editable document", "[map][worldbuilding][ffs06]") {
    urpg::editor::ProceduralMapPanel panel;
    panel.generate({"tiny_boss", "dungeon", 4, 4, 7, true, true, true});

    REQUIRE(panel.snapshot().has_result);
    REQUIRE(panel.snapshot().width == 4);
    REQUIRE(panel.snapshot().height == 4);
    REQUIRE(panel.snapshot().layer_count == 2);
    REQUIRE(panel.snapshot().diagnostic_count == 1);
    REQUIRE(panel.result().diagnostics[0].code == "unsatisfied_constraints");

    auto doc = panel.result().document;
    REQUIRE(doc.setTile("terrain", 1, 1, 42));
    REQUIRE(doc.tileAt("terrain", 1, 1).value() == 42);
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
