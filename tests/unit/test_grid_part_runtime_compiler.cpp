#include "engine/core/map/grid_part_runtime_compiler.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

using namespace urpg::map;

namespace {

PlacedPartInstance makePart(std::string instanceId, std::string partId, GridPartCategory category, int32_t x,
                            int32_t y) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.category = category;
    part.grid_x = x;
    part.grid_y = y;
    return part;
}

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category, int32_t tileId = 0) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.category = category;
    definition.default_layer = category == GridPartCategory::Tile ? GridPartLayer::Terrain : GridPartLayer::Object;
    definition.tile_id = tileId;
    definition.asset_id = definition.part_id + ".asset";
    definition.footprint.width = 1;
    definition.footprint.height = 1;
    return definition;
}

GridPartCatalog makeCompilerCatalog() {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("tile.grass", GridPartCategory::Tile, 7)));

    auto wall = makeDefinition("wall.stone", GridPartCategory::Wall, 9);
    wall.default_layer = GridPartLayer::Collision;
    wall.collision_policy = GridPartCollisionPolicy::Solid;
    wall.footprint.blocks_navigation = true;
    REQUIRE(catalog.addDefinition(wall));

    auto door = makeDefinition("door.locked", GridPartCategory::Door);
    door.collision_policy = GridPartCollisionPolicy::Solid;
    door.footprint.blocks_navigation = true;
    REQUIRE(catalog.addDefinition(door));

    REQUIRE(catalog.addDefinition(makeDefinition("enemy.slime", GridPartCategory::Enemy)));
    REQUIRE(catalog.addDefinition(makeDefinition("hazard.spikes", GridPartCategory::Hazard)));
    REQUIRE(catalog.addDefinition(makeDefinition("trigger.zone", GridPartCategory::Trigger)));
    REQUIRE(catalog.addDefinition(makeDefinition("cutscene.zone", GridPartCategory::CutsceneZone)));
    REQUIRE(catalog.addDefinition(makeDefinition("treasure.chest", GridPartCategory::TreasureChest)));
    return catalog;
}

} // namespace

TEST_CASE("Grid part runtime compiler creates terrain and collision layers", "[grid_part][compiler]") {
    const auto catalog = makeCompilerCatalog();
    GridPartDocument document("map001", 4, 4);
    REQUIRE(document.placePart(makePart("map001:tile.grass:1:1", "tile.grass", GridPartCategory::Tile, 1, 1)));
    REQUIRE(document.placePart(makePart("map001:wall.stone:2:1", "wall.stone", GridPartCategory::Wall, 2, 1)));

    const auto result = CompileGridPartRuntime(document, catalog);

    REQUIRE(result.ok);
    REQUIRE(result.tile_document.width() == 4);
    REQUIRE(result.tile_document.height() == 4);
    REQUIRE(result.tile_document.tileAt("terrain", 1, 1).value() == 7);
    REQUIRE(result.tile_document.tileAt("terrain", 2, 1).value() == 9);
    REQUIRE(result.tile_document.tileAt("collision", 2, 1).value() == 1);
    REQUIRE(result.tile_document.tileAt("collision", 1, 1).value() == 0);
}

TEST_CASE("Grid part runtime compiler emits props and solid object collision", "[grid_part][compiler]") {
    const auto catalog = makeCompilerCatalog();
    GridPartDocument document("map001", 4, 4);
    auto chest = makePart("map001:treasure.chest:1:1", "treasure.chest", GridPartCategory::TreasureChest, 1, 1);
    chest.rot_y = 90.0f;
    chest.scale = 1.25f;
    REQUIRE(document.placePart(chest));
    REQUIRE(document.placePart(makePart("map001:door.locked:2:2", "door.locked", GridPartCategory::Door, 2, 2)));

    const auto result = CompileGridPartRuntime(document, catalog);

    REQUIRE(result.ok);
    REQUIRE(result.spatial_overlay.mapId == "map001");
    REQUIRE(result.spatial_overlay.props.size() == 2);
    REQUIRE(result.spatial_overlay.props[0].instanceId == "map001:door.locked:2:2");
    REQUIRE(result.spatial_overlay.props[0].assetId == "door.locked.asset");
    REQUIRE(result.spatial_overlay.props[0].posX == 2.0f);
    REQUIRE(result.spatial_overlay.props[0].posZ == 2.0f);
    REQUIRE(result.spatial_overlay.props[1].instanceId == "map001:treasure.chest:1:1");
    REQUIRE(result.spatial_overlay.props[1].assetId == "treasure.chest.asset");
    REQUIRE(result.spatial_overlay.props[1].rotY == 90.0f);
    REQUIRE(result.spatial_overlay.props[1].scale == 1.25f);
    REQUIRE(result.tile_document.tileAt("collision", 2, 2).value() == 1);
}

TEST_CASE("Grid part runtime compiler emits spawn table entries", "[grid_part][compiler]") {
    const auto catalog = makeCompilerCatalog();
    GridPartDocument document("map001", 4, 4);
    auto enemy = makePart("map001:enemy.slime:2:3", "enemy.slime", GridPartCategory::Enemy, 2, 3);
    enemy.properties["enemyId"] = "slime";
    enemy.properties["cooldownSeconds"] = "12";
    enemy.properties["persistent"] = "true";
    REQUIRE(document.placePart(enemy));

    const auto result = CompileGridPartRuntime(document, catalog);

    REQUIRE(result.ok);
    REQUIRE(result.spawn_table.id == "map001:grid_parts");
    REQUIRE(result.spawn_table.entries.size() == 1);
    REQUIRE(result.spawn_table.entries[0].id == "map001:enemy.slime:2:3");
    REQUIRE(result.spawn_table.entries[0].enemy_id == "slime");
    REQUIRE(result.spawn_table.entries[0].x == 2);
    REQUIRE(result.spawn_table.entries[0].y == 3);
    REQUIRE(result.spawn_table.entries[0].cooldown_seconds == 12);
    REQUIRE(result.spawn_table.entries[0].persistent);
}

TEST_CASE("Grid part runtime compiler emits region rules for hazards, triggers, and cutscenes",
          "[grid_part][compiler]") {
    const auto catalog = makeCompilerCatalog();
    GridPartDocument document("map001", 8, 8);
    auto hazard = makePart("map001:hazard.spikes:1:2", "hazard.spikes", GridPartCategory::Hazard, 1, 2);
    hazard.properties["hazard"] = "spikes";
    auto trigger = makePart("map001:trigger.zone:3:2", "trigger.zone", GridPartCategory::Trigger, 3, 2);
    trigger.properties["eventId"] = "open_gate";
    auto cutscene = makePart("map001:cutscene.zone:5:2", "cutscene.zone", GridPartCategory::CutsceneZone, 5, 2);
    cutscene.properties["eventId"] = "intro_scene";

    REQUIRE(document.placePart(hazard));
    REQUIRE(document.placePart(trigger));
    REQUIRE(document.placePart(cutscene));

    const auto result = CompileGridPartRuntime(document, catalog);

    REQUIRE(result.ok);
    REQUIRE(result.region_rules.size() == 3);
    REQUIRE(result.region_rules[0].id == "map001:cutscene.zone:5:2");
    REQUIRE(result.region_rules[0].event_id == "intro_scene");
    REQUIRE(result.region_rules[1].id == "map001:hazard.spikes:1:2");
    REQUIRE(result.region_rules[1].hazard == "spikes");
    REQUIRE(result.region_rules[2].id == "map001:trigger.zone:3:2");
    REQUIRE(result.region_rules[2].event_id == "open_gate");
}

TEST_CASE("Grid part runtime compiler reports validator diagnostics", "[grid_part][compiler]") {
    GridPartCatalog catalog;
    GridPartDocument document("map001", 4, 4);
    REQUIRE(document.placePart(makePart("map001:missing:1:1", "missing.part", GridPartCategory::Prop, 1, 1)));

    const auto result = CompileGridPartRuntime(document, catalog);

    REQUIRE_FALSE(result.ok);
    REQUIRE_FALSE(result.diagnostics.empty());
    REQUIRE(result.diagnostics[0].code == "part_definition_missing");
}

TEST_CASE("Grid part runtime compiler can compile only dirty chunks", "[grid_part][compiler][chunks]") {
    const auto catalog = makeCompilerCatalog();
    GridPartDocument document("map001", 8, 8, 4);
    REQUIRE(document.placePart(makePart("map001:tile.grass:1:1", "tile.grass", GridPartCategory::Tile, 1, 1)));
    REQUIRE(document.placePart(makePart("map001:wall.stone:5:5", "wall.stone", GridPartCategory::Wall, 5, 5)));
    document.clearDirtyChunks();

    REQUIRE(document.movePart("map001:wall.stone:5:5", 6, 5));

    const auto result = CompileGridPartRuntimeForDirtyChunks(document, catalog);

    REQUIRE(result.ok);
    REQUIRE(result.compiled_chunks.size() == 1);
    REQUIRE(result.compiled_chunks[0].chunk_x == 1);
    REQUIRE(result.compiled_chunks[0].chunk_y == 1);
    REQUIRE(result.compiled_instance_ids.size() == 1);
    REQUIRE(result.compiled_instance_ids[0] == "map001:wall.stone:5:5");
    REQUIRE(result.tile_document.tileAt("terrain", 6, 5).value() == 9);
    REQUIRE(result.tile_document.tileAt("collision", 6, 5).value() == 1);
    REQUIRE(result.tile_document.tileAt("terrain", 1, 1).value() == 0);
}

TEST_CASE("Grid part runtime compiler preserves dirty deletion chunks", "[grid_part][compiler][chunks]") {
    const auto catalog = makeCompilerCatalog();
    GridPartDocument document("map001", 8, 8, 4);
    REQUIRE(document.placePart(makePart("map001:tile.grass:5:5", "tile.grass", GridPartCategory::Tile, 5, 5)));
    document.clearDirtyChunks();
    REQUIRE(document.removePart("map001:tile.grass:5:5"));

    const auto result = CompileGridPartRuntimeForDirtyChunks(document, catalog);

    REQUIRE(result.ok);
    REQUIRE(result.compiled_chunks.size() == 1);
    REQUIRE(result.compiled_chunks[0].chunk_x == 1);
    REQUIRE(result.compiled_chunks[0].chunk_y == 1);
    REQUIRE(result.compiled_instance_ids.empty());
}

TEST_CASE("Grid part runtime compiler normalizes requested chunk filters", "[grid_part][compiler][chunks]") {
    const auto catalog = makeCompilerCatalog();
    GridPartDocument document("map001", 8, 8, 4);
    REQUIRE(document.placePart(makePart("map001:wall.stone:5:5", "wall.stone", GridPartCategory::Wall, 5, 5)));

    const auto result = CompileGridPartRuntimeForChunks(document, catalog, {{1, 1}, {1, 1}, {-1, 0}, {3, 3}});

    REQUIRE(result.ok);
    REQUIRE(result.partial_compile);
    REQUIRE(result.compiled_chunks.size() == 1);
    REQUIRE(result.compiled_chunks[0].chunk_x == 1);
    REQUIRE(result.compiled_chunks[0].chunk_y == 1);
    REQUIRE(result.compiled_instance_ids.size() == 1);
    REQUIRE(result.compiled_instance_ids[0] == "map001:wall.stone:5:5");
}
