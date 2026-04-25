#include "engine/core/crafting/crafting_registry.h"
#include "engine/core/time/calendar_runtime.h"
#include "engine/core/world/world_map_graph.h"

#include <catch2/catch_test_macros.hpp>

#include <set>

TEST_CASE("simulation systems share flags across world calendar and crafting", "[integration][simulation][ffs13]") {
    urpg::time::CalendarRuntime calendar;
    calendar.addEvent({"market_day", 1, 8, 12, "market_open"});

    urpg::world::WorldMapGraph world;
    world.addNode({"town", "Town"});
    world.addNode({"market", "Market"});
    world.addRoute({"town", "market", "market_open", "", true});

    urpg::crafting::CraftingRegistry crafting;
    crafting.addRecipe({"recipe.soup", {{"vegetable", 2}}, {{"soup", 1}}, "market_open"});

    const auto triggered = calendar.advanceTo(1, 9).triggeredFlags;
    const std::set<std::string> flags(triggered.begin(), triggered.end());

    REQUIRE(world.isRouteAvailable("town", "market", flags, {}));
    REQUIRE(crafting.preview("recipe.soup", {{"vegetable", 2}}, triggered).canCraft);
}
