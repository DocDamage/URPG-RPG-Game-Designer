#include "engine/core/map/grid_part_dependency_graph.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

using namespace urpg::map;

namespace {

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category = GridPartCategory::Prop) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.description = "Dependency graph test part";
    definition.category = category;
    definition.default_layer = GridPartLayer::Object;
    return definition;
}

PlacedPartInstance makePart(std::string instanceId, std::string partId,
                            GridPartCategory category = GridPartCategory::Prop) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.category = category;
    part.layer = GridPartLayer::Object;
    part.grid_x = 1;
    part.grid_y = 1;
    return part;
}

} // namespace

TEST_CASE("Grid part dependency graph collects catalog asset prefab and tileset dependencies",
          "[grid_part][dependency_graph]") {
    GridPartCatalog catalog;
    auto definition = makeDefinition("prop.crate");
    definition.asset_id = "asset.crate";
    definition.prefab_path = "prefabs/crate.json";
    definition.tile_id = 42;
    REQUIRE(catalog.addDefinition(definition));

    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate")));

    const auto dependencies = CollectGridPartDependencies(document, catalog);

    REQUIRE(dependencies.size() == 3);
    REQUIRE(dependencies[0].type == GridPartDependencyType::Asset);
    REQUIRE(dependencies[0].id == "asset.crate");
    REQUIRE(dependencies[1].type == GridPartDependencyType::Tileset);
    REQUIRE(dependencies[1].id == "tile:42");
    REQUIRE(dependencies[2].type == GridPartDependencyType::Prefab);
    REQUIRE(dependencies[2].id == "prefabs/crate.json");
}

TEST_CASE("Grid part dependency graph collects authored property dependencies", "[grid_part][dependency_graph]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("npc.vendor", GridPartCategory::Npc)));

    auto npc = makePart("map001:npc.vendor:2:2", "npc.vendor", GridPartCategory::Npc);
    npc.properties["npcId"] = "npc.vendor";
    npc.properties["dialogueId"] = "dialogue.shop_intro";
    npc.properties["shopTableId"] = "shop.basic";
    npc.properties["questId"] = "quest.first_trade";
    npc.properties["abilityId"] = "ability.inspect_shop";
    npc.properties["audioId"] = "audio.shop_theme";
    npc.properties["animationId"] = "anim.wave";
    npc.properties["scriptId"] = "script.vendor_idle";

    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(npc));

    const auto dependencies = CollectGridPartDependencies(document, catalog);

    REQUIRE(dependencies.size() == 8);
    REQUIRE(dependencies[0].type == GridPartDependencyType::Npc);
    REQUIRE(dependencies[0].id == "npc.vendor");
    REQUIRE(dependencies[1].type == GridPartDependencyType::Dialogue);
    REQUIRE(dependencies[1].id == "dialogue.shop_intro");
    REQUIRE(dependencies[2].type == GridPartDependencyType::ShopTable);
    REQUIRE(dependencies[2].id == "shop.basic");
    REQUIRE(dependencies[3].type == GridPartDependencyType::Quest);
    REQUIRE(dependencies[3].id == "quest.first_trade");
    REQUIRE(dependencies[4].type == GridPartDependencyType::Ability);
    REQUIRE(dependencies[4].id == "ability.inspect_shop");
    REQUIRE(dependencies[5].type == GridPartDependencyType::Audio);
    REQUIRE(dependencies[5].id == "audio.shop_theme");
    REQUIRE(dependencies[6].type == GridPartDependencyType::Animation);
    REQUIRE(dependencies[6].id == "anim.wave");
    REQUIRE(dependencies[7].type == GridPartDependencyType::Script);
    REQUIRE(dependencies[7].id == "script.vendor_idle");
}

TEST_CASE("Grid part dependency graph uses instance properties over catalog defaults",
          "[grid_part][dependency_graph]") {
    GridPartCatalog catalog;
    auto definition = makeDefinition("enemy.slime", GridPartCategory::Enemy);
    definition.default_properties["enemyId"] = "enemy.default_slime";
    definition.default_properties["lootTableId"] = "loot.default";
    REQUIRE(catalog.addDefinition(definition));

    auto enemy = makePart("map001:enemy.slime:3:3", "enemy.slime", GridPartCategory::Enemy);
    enemy.properties["enemyId"] = "enemy.red_slime";

    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(enemy));

    const auto dependencies = CollectGridPartDependencies(document, catalog);

    REQUIRE(dependencies.size() == 2);
    REQUIRE(dependencies[0].type == GridPartDependencyType::Enemy);
    REQUIRE(dependencies[0].id == "enemy.red_slime");
    REQUIRE(dependencies[1].type == GridPartDependencyType::LootTable);
    REQUIRE(dependencies[1].id == "loot.default");
}

TEST_CASE("Grid part dependency graph de-duplicates and sorts dependencies deterministically",
          "[grid_part][dependency_graph]") {
    GridPartCatalog catalog;
    auto first = makeDefinition("prop.a");
    first.asset_id = "asset.shared";
    REQUIRE(catalog.addDefinition(first));
    auto second = makeDefinition("prop.b");
    second.asset_id = "asset.shared";
    REQUIRE(catalog.addDefinition(second));

    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.b:2:1", "prop.b")));
    REQUIRE(document.placePart(makePart("map001:prop.a:1:1", "prop.a")));

    const auto dependencies = CollectGridPartDependencies(document, catalog);

    REQUIRE(dependencies.size() == 2);
    REQUIRE(dependencies[0].id == "asset.shared");
    REQUIRE(dependencies[0].source_instance_id == "map001:prop.a:1:1");
    REQUIRE(dependencies[1].id == "asset.shared");
    REQUIRE(dependencies[1].source_instance_id == "map001:prop.b:2:1");
}

TEST_CASE("Grid part dependency graph ignores missing catalog definitions", "[grid_part][dependency_graph]") {
    GridPartCatalog catalog;
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:missing:1:1", "missing.part")));

    const auto dependencies = CollectGridPartDependencies(document, catalog);

    REQUIRE(dependencies.empty());
}
