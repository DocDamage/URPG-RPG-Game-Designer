#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_catalog_loader.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace urpg::map;

namespace {

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category = GridPartCategory::Prop,
                                  std::vector<GridPartRuleset> rulesets = {GridPartRuleset::TopDownJRPG}) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.description = "Test part definition";
    definition.category = category;
    definition.supported_rulesets = std::move(rulesets);
    definition.tags = {"test"};
    return definition;
}

nlohmann::json loadJson(const std::string& relativePath) {
    std::ifstream stream(std::string(URPG_SOURCE_DIR) + "/" + relativePath);
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

std::filesystem::path uniqueTempDirectoryPath(std::string_view stem) {
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (std::string(stem) + "_" + std::to_string(ticks));
}

void writeTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}

} // namespace

TEST_CASE("GridPartCatalog adds and finds definitions", "[grid_part][catalog]") {
    GridPartCatalog catalog;
    auto definition = makeDefinition("prop.crate");

    REQUIRE(catalog.addDefinition(definition));

    REQUIRE(catalog.size() == 1);
    const auto* found = catalog.find("prop.crate");
    REQUIRE(found != nullptr);
    REQUIRE(found->part_id == "prop.crate");
}

TEST_CASE("GridPartCatalog rejects missing and duplicate part IDs", "[grid_part][catalog]") {
    GridPartCatalog catalog;

    REQUIRE_FALSE(catalog.addDefinition(makeDefinition("")));
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));
    REQUIRE_FALSE(catalog.addDefinition(makeDefinition("prop.crate")));
    REQUIRE(catalog.size() == 1);
}

TEST_CASE("GridPartCatalog returns definitions in deterministic part ID order", "[grid_part][catalog]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.z")));
    REQUIRE(catalog.addDefinition(makeDefinition("enemy.slime", GridPartCategory::Enemy)));
    REQUIRE(catalog.addDefinition(makeDefinition("door.basic", GridPartCategory::Door)));

    const auto definitions = catalog.allDefinitions();

    REQUIRE(definitions.size() == 3);
    REQUIRE(definitions[0].part_id == "door.basic");
    REQUIRE(definitions[1].part_id == "enemy.slime");
    REQUIRE(definitions[2].part_id == "prop.z");
}

TEST_CASE("GridPartCatalog filters by category and ruleset", "[grid_part][catalog]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    REQUIRE(catalog.addDefinition(makeDefinition("enemy.slime", GridPartCategory::Enemy,
                                                 {GridPartRuleset::TopDownJRPG, GridPartRuleset::DungeonRoomBuilder})));
    REQUIRE(catalog.addDefinition(
        makeDefinition("platform.grass", GridPartCategory::Platform, {GridPartRuleset::SideScrollerAction})));

    const auto enemies = catalog.filterByCategory(GridPartCategory::Enemy);
    REQUIRE(enemies.size() == 1);
    REQUIRE(enemies.front().part_id == "enemy.slime");

    const auto dungeonParts = catalog.filterByRuleset(GridPartRuleset::DungeonRoomBuilder);
    REQUIRE(dungeonParts.size() == 1);
    REQUIRE(dungeonParts.front().part_id == "enemy.slime");
}

TEST_CASE("GridPartCatalog searches IDs, names, descriptions, and tags", "[grid_part][catalog]") {
    GridPartCatalog catalog;
    auto chest = makeDefinition("treasure.basic", GridPartCategory::TreasureChest);
    chest.display_name = "Wooden Chest";
    chest.description = "Contains a small reward";
    chest.tags = {"loot", "interactable"};
    REQUIRE(catalog.addDefinition(chest));
    REQUIRE(catalog.addDefinition(makeDefinition("save.crystal", GridPartCategory::SavePoint)));

    REQUIRE(catalog.search("chest").front().part_id == "treasure.basic");
    REQUIRE(catalog.search("reward").front().part_id == "treasure.basic");
    REQUIRE(catalog.search("loot").front().part_id == "treasure.basic");
    REQUIRE(catalog.search("missing").empty());
}

TEST_CASE("Grid part catalog schema and base JRPG content are parseable", "[grid_part][catalog][schema]") {
    const auto schema = loadJson("content/schemas/grid_part_catalog.schema.json");
    REQUIRE(schema["title"] == "Grid Part Catalog");
    REQUIRE(schema["required"].size() >= 2);

    const auto catalog = loadJson("content/part_catalogs/base_jrpg_parts.json");
    REQUIRE(catalog["schemaVersion"] == 1);
    REQUIRE(catalog["catalogId"] == "base_jrpg_parts");
    REQUIRE(catalog["parts"].is_array());
    REQUIRE(catalog["parts"].size() >= 8);

    GridPartCatalog runtimeCatalog;
    for (const auto& part : catalog["parts"]) {
        REQUIRE(part.contains("partId"));
        REQUIRE(part.contains("category"));
        REQUIRE(part.contains("defaultLayer"));
        REQUIRE(runtimeCatalog.addDefinition(makeDefinition(part["partId"].get<std::string>())));
    }
    REQUIRE(runtimeCatalog.find("enemy.slime") != nullptr);
    REQUIRE(runtimeCatalog.find("treasure.chest.basic") != nullptr);
}

TEST_CASE("Grid part catalog loader preserves gameplay asset slice metadata", "[grid_part][catalog][assets]") {
    const auto tempRoot = uniqueTempDirectoryPath("urpg_grid_part_asset_slice");
    const auto catalogPath = tempRoot / "content" / "part_catalogs" / "asset_slice_parts.json";
    writeTextFile(catalogPath, R"({
  "schemaVersion": 1,
  "catalogId": "asset_slice_parts",
  "displayName": "Asset Slice Parts",
  "parts": [
    {
      "partId": "asset.tile.grunge.000",
      "displayName": "Grunge Tile 000",
      "category": "Tile",
      "defaultLayer": "Terrain",
      "collisionPolicy": "None",
      "supportedRulesets": ["TopDownJRPG"],
      "footprint": {"width": 1, "height": 1},
      "assetId": "src012.grunge_tileset.000",
      "tileId": 1000,
      "previewPath": "content/assets/gameplay/src012/grunge_tileset/tile_000.png",
      "sourceImagePath": "imports/normalized/src012_cc0_tiles_vfx/tilesets/grunge_tileset.png",
      "atlasRect": {"x": 0, "y": 0, "width": 16, "height": 16}
    }
  ]
})");

    GridPartCatalog catalog;
    std::string error;
    REQUIRE(LoadGridPartCatalogFromFile(catalogPath, catalog, &error));
    REQUIRE(error.empty());

    const auto* definition = catalog.find("asset.tile.grunge.000");
    REQUIRE(definition != nullptr);
    REQUIRE(definition->asset_id == "src012.grunge_tileset.000");
    REQUIRE(definition->preview_path == "content/assets/gameplay/src012/grunge_tileset/tile_000.png");
    REQUIRE(definition->source_image_path == "imports/normalized/src012_cc0_tiles_vfx/tilesets/grunge_tileset.png");
    REQUIRE(definition->atlas_rect.x == 0);
    REQUIRE(definition->atlas_rect.y == 0);
    REQUIRE(definition->atlas_rect.width == 16);
    REQUIRE(definition->atlas_rect.height == 16);

    std::filesystem::remove_all(tempRoot);
}

TEST_CASE("Grid part catalog loader merges included generated asset catalogs", "[grid_part][catalog][assets]") {
    const auto tempRoot = uniqueTempDirectoryPath("urpg_grid_part_catalog_include");
    writeTextFile(tempRoot / "content" / "part_catalogs" / "base.json", R"({
  "schemaVersion": 1,
  "catalogId": "base",
  "displayName": "Base",
  "includes": ["generated/tiles.json"],
  "parts": [
    {
      "partId": "spawn.player",
      "displayName": "Player Spawn",
      "category": "Trigger",
      "defaultLayer": "Object",
      "collisionPolicy": "TriggerOnly",
      "footprint": {"width": 1, "height": 1}
    }
  ]
})");
    writeTextFile(tempRoot / "content" / "part_catalogs" / "generated" / "tiles.json", R"({
  "schemaVersion": 1,
  "catalogId": "generated_tiles",
  "displayName": "Generated Tiles",
  "parts": [
    {
      "partId": "asset.tile.grunge.001",
      "displayName": "Grunge Tile 001",
      "category": "Tile",
      "defaultLayer": "Terrain",
      "collisionPolicy": "None",
      "footprint": {"width": 1, "height": 1},
      "previewPath": "content/assets/gameplay/src012/grunge_tileset/tile_001.png",
      "sourceImagePath": "imports/normalized/src012_cc0_tiles_vfx/tilesets/grunge_tileset.png",
      "atlasRect": {"x": 16, "y": 0, "width": 16, "height": 16}
    }
  ]
})");

    GridPartCatalog catalog;
    std::string error;
    REQUIRE(LoadGridPartCatalogFromFile(tempRoot / "content" / "part_catalogs" / "base.json", catalog, &error));
    REQUIRE(error.empty());
    REQUIRE(catalog.find("spawn.player") != nullptr);
    REQUIRE(catalog.find("asset.tile.grunge.001") != nullptr);
    REQUIRE(catalog.size() == 2);

    std::filesystem::remove_all(tempRoot);
}

TEST_CASE("Grid part catalog scope switches between starter and full library without retaining inactive rows",
          "[grid_part][catalog][scope]") {
    const auto tempRoot = uniqueTempDirectoryPath("urpg_grid_part_catalog_scope");
    writeTextFile(tempRoot / "content" / "part_catalogs" / "starter.json", R"({
  "schemaVersion": 1,
  "catalogId": "starter",
  "displayName": "Starter",
  "parts": [
    {
      "partId": "starter.spawn",
      "displayName": "Starter Spawn",
      "category": "Trigger",
      "defaultLayer": "Object",
      "collisionPolicy": "TriggerOnly",
      "footprint": {"width": 1, "height": 1}
    }
  ]
})");
    writeTextFile(tempRoot / "content" / "part_catalogs" / "full.json", R"({
  "schemaVersion": 1,
  "catalogId": "full",
  "displayName": "Full",
  "parts": [
    {
      "partId": "full.castle.wall",
      "displayName": "Full Castle Wall",
      "category": "Wall",
      "defaultLayer": "Collision",
      "collisionPolicy": "Solid",
      "footprint": {"width": 1, "height": 1}
    }
  ]
})");

    GridPartCatalogScope starterScope;
    std::string error;
    REQUIRE(LoadGridPartCatalogScopeFromProject(tempRoot, {"content/part_catalogs/starter.json"}, starterScope, &error));
    REQUIRE(error.empty());
    REQUIRE(starterScope.catalog.size() == 1);
    REQUIRE(starterScope.catalog.find("starter.spawn") != nullptr);
    REQUIRE(starterScope.catalog.find("full.castle.wall") == nullptr);
    REQUIRE(starterScope.full_library_active == false);
    REQUIRE(starterScope.active_catalog_paths == std::vector<std::filesystem::path>{"content/part_catalogs/starter.json"});

    GridPartCatalogScope fullScope;
    REQUIRE(LoadGridPartCatalogScopeFromProject(tempRoot, {"content/part_catalogs/full.json"}, fullScope, &error));
    REQUIRE(error.empty());
    REQUIRE(fullScope.catalog.size() == 1);
    REQUIRE(fullScope.catalog.find("starter.spawn") == nullptr);
    REQUIRE(fullScope.catalog.find("full.castle.wall") != nullptr);
    REQUIRE(fullScope.full_library_active == true);

    GridPartCatalogScope restoredStarterScope;
    REQUIRE(LoadGridPartCatalogScopeFromProject(tempRoot, {"content/part_catalogs/starter.json"}, restoredStarterScope,
                                                &error));
    REQUIRE(error.empty());
    REQUIRE(restoredStarterScope.catalog.find("starter.spawn") != nullptr);
    REQUIRE(restoredStarterScope.catalog.find("full.castle.wall") == nullptr);
    REQUIRE(restoredStarterScope.full_library_active == false);

    std::filesystem::remove_all(tempRoot);
}
