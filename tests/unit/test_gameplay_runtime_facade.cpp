#include "engine/core/gameplay/gameplay_runtime_facade.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("gameplay runtime facade delegates compact map npc event input and resource operations", "[gameplay][facade]") {
    urpg::map::GridPartDocument document("town", 4, 4);
    urpg::map::PlacedPartInstance chest;
    chest.instance_id = "town:chest";
    chest.part_id = "chest";
    chest.category = urpg::map::GridPartCategory::TreasureChest;
    chest.layer = urpg::map::GridPartLayer::Object;
    chest.grid_x = 1;
    chest.grid_y = 1;
    REQUIRE(document.placePart(chest));
    urpg::level::PathfindingGraph graph(4, 4);
    urpg::gameplay::GameplayRuntimeFacade facade;
    facade.bindMap(&document, &graph);
    urpg::npc::NpcRuntimeState merchant;
    merchant.npcId = "merchant";
    merchant.mapId = "town";
    REQUIRE(facade.registerNpc(merchant));
    REQUIRE(facade.queryMapAt(1, 1).objectIds == std::vector<std::string>{"town:chest"});
    REQUIRE(facade.moveNpc("merchant", {1, 0}).success);
    REQUIRE(facade.invokeEvent({"merchant_talk"}).success);
    REQUIRE(facade.bindInput("confirm", "interact").success);
    REQUIRE(facade.addResource("gold", 10).success);
    REQUIRE(facade.resourceCount("gold") == 10);
}
