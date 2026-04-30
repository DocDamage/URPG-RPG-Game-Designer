#include "engine/core/map/grid_part_runtime_compiler.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/scene/map_scene.h"

#include <catch2/catch_test_macros.hpp>

#include <limits>

using namespace urpg::map;

namespace {

GridPartDefinition makeWallDefinition() {
    GridPartDefinition definition;
    definition.part_id = "wall.stone";
    definition.display_name = "Stone Wall";
    definition.category = GridPartCategory::Wall;
    definition.default_layer = GridPartLayer::Collision;
    definition.collision_policy = GridPartCollisionPolicy::Solid;
    definition.tile_id = 4;
    definition.footprint.blocks_navigation = true;
    return definition;
}

} // namespace

TEST_CASE("Grid part runtime compiler applies collision to MapScene", "[grid_part][compiler][map_scene]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeWallDefinition()));

    GridPartDocument document("map001", 4, 4);
    PlacedPartInstance wall;
    wall.instance_id = "map001:wall.stone:1:1";
    wall.part_id = "wall.stone";
    wall.category = GridPartCategory::Wall;
    wall.layer = GridPartLayer::Collision;
    wall.grid_x = 1;
    wall.grid_y = 1;
    REQUIRE(document.placePart(wall));

    const auto result = CompileGridPartRuntime(document, catalog);
    REQUIRE(result.ok);

    urpg::scene::MapScene scene("map001", 4, 4);
    REQUIRE_FALSE(scene.checkCollision(1, 1));

    REQUIRE(ApplyGridPartRuntimeToMapScene(result, scene));

    REQUIRE(scene.checkCollision(1, 1));
    REQUIRE_FALSE(scene.checkCollision(0, 0));
}

TEST_CASE("Grid part runtime compiler refuses partial chunk result as full MapScene apply",
          "[grid_part][compiler][map_scene]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeWallDefinition()));

    GridPartDocument document("map001", 4, 4, 2);
    PlacedPartInstance wall;
    wall.instance_id = "map001:wall.stone:3:3";
    wall.part_id = "wall.stone";
    wall.category = GridPartCategory::Wall;
    wall.layer = GridPartLayer::Collision;
    wall.grid_x = 3;
    wall.grid_y = 3;
    REQUIRE(document.placePart(wall));

    const auto result = CompileGridPartRuntimeForChunks(document, catalog, {{1, 1}});
    REQUIRE(result.ok);

    urpg::scene::MapScene scene("map001", 4, 4);
    scene.setTile(0, 0, 9, false);

    REQUIRE_FALSE(ApplyGridPartRuntimeToMapScene(result, scene));
    REQUIRE(scene.checkCollision(0, 0));
    REQUIRE_FALSE(scene.checkCollision(3, 3));
}

TEST_CASE("Grid part runtime compiler clamps oversized tile ids before MapScene apply",
          "[grid_part][compiler][map_scene][robustness]") {
    GridPartDefinition definition = makeWallDefinition();
    definition.part_id = "tile.high";
    definition.category = GridPartCategory::Tile;
    definition.default_layer = GridPartLayer::Terrain;
    definition.collision_policy = GridPartCollisionPolicy::None;
    definition.tile_id = static_cast<int32_t>(std::numeric_limits<uint16_t>::max()) + 7;
    definition.footprint.blocks_navigation = false;

    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(definition));

    GridPartDocument document("map001", 1, 1);
    PlacedPartInstance tile;
    tile.instance_id = "map001:tile.high:0:0";
    tile.part_id = "tile.high";
    tile.category = GridPartCategory::Tile;
    tile.layer = GridPartLayer::Terrain;
    REQUIRE(document.placePart(tile));

    const auto result = CompileGridPartRuntime(document, catalog);
    REQUIRE(result.ok);

    urpg::scene::MapScene scene("map001", 1, 1);
    REQUIRE(ApplyGridPartRuntimeToMapScene(result, scene));

    auto& renderLayer = urpg::RenderLayer::getInstance();
    renderLayer.flush();
    scene.onUpdate(0.0f);
    const auto& frame = renderLayer.getFrameCommands();
    REQUIRE(frame.size() >= 1);
    REQUIRE(frame[0].type == urpg::RenderCmdType::Tile);
    const auto* tileCommand = frame[0].tryGet<urpg::TileRenderData>();
    REQUIRE(tileCommand != nullptr);
    REQUIRE(tileCommand->tileIndex == std::numeric_limits<uint16_t>::max());
}
