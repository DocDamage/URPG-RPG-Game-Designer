#include "engine/core/map/grid_part_catalog.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <fstream>
#include <string>
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
