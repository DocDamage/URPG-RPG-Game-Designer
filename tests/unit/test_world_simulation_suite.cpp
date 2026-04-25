#include "engine/core/world/world_map_graph.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("World route availability changes with flags", "[world][simulation][ffs13]") {
    urpg::world::WorldMapGraph graph;
    graph.addNode({"town", "Town"});
    graph.addNode({"ruins", "Ruins"});
    graph.addRoute({"town", "ruins", "bridge_repaired", "airship", true});

    REQUIRE_FALSE(graph.isRouteAvailable("town", "ruins", {}, {}));
    REQUIRE_FALSE(graph.isRouteAvailable("town", "ruins", {"bridge_repaired"}, {}));
    REQUIRE(graph.isRouteAvailable("town", "ruins", {"bridge_repaired"}, {"airship"}));
    REQUIRE(graph.validate().empty());
}

TEST_CASE("World route validator reports self-unlocking routes", "[world][simulation][ffs13]") {
    urpg::world::WorldMapGraph graph;
    graph.addNode({"gate", "Gate"});
    graph.addRoute({"gate", "gate", "gate", "", false});

    const auto diagnostics = graph.validate();
    REQUIRE_FALSE(diagnostics.empty());
    REQUIRE(diagnostics.front().code == "route_unlocks_itself");
}
