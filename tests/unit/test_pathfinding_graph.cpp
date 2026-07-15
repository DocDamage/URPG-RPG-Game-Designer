#include "engine/core/level/path_request_router.h"
#include "engine/core/level/pathfinding_graph.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::level;

TEST_CASE("PathfindingGraph returns deterministic lowest-cost cardinal routes", "[pathfinding][graph]") {
    PathfindingGraph graph(5, 4);
    graph.setCellCost(1, 0, 5);
    graph.setCellCost(1, 1, 5);
    graph.setBlocked(2, 1, true, "wall");
    graph.setBlocked(2, 2, true, "wall");

    const auto first = graph.findPath({0, 0}, {4, 3});
    const auto second = graph.findPath({0, 0}, {4, 3});

    REQUIRE(first.found);
    REQUIRE(second.found);
    REQUIRE(first.total_cost == 7);
    REQUIRE(first.nodes == second.nodes);
    REQUIRE(first.nodes.front() == PathGridPoint{0, 0});
    REQUIRE(first.nodes.back() == PathGridPoint{4, 3});
    REQUIRE(first.nodes[1] == PathGridPoint{0, 1});
    REQUIRE(first.diagnostics.empty());
}

TEST_CASE("PathfindingGraph reports deterministic block reasons without mutating the graph", "[pathfinding][graph]") {
    PathfindingGraph graph(3, 3);
    graph.setBlocked(1, 0, true, "stone_wall");
    graph.setBlocked(0, 1, true, "water");

    const auto blocked = graph.findPath({0, 0}, {2, 2});
    const auto stillBlocked = graph.findPath({0, 0}, {2, 2});

    REQUIRE_FALSE(blocked.found);
    REQUIRE(blocked.reason == "no_route");
    REQUIRE(blocked.diagnostics.size() == 2);
    REQUIRE(blocked.diagnostics[0].code == "neighbor_blocked");
    REQUIRE(blocked.diagnostics[0].reason == "stone_wall");
    REQUIRE(blocked.diagnostics[1].reason == "water");
    REQUIRE(stillBlocked.diagnostics == blocked.diagnostics);
}

TEST_CASE("PathfindingGraph honors directed traversal blocks", "[pathfinding][graph]") {
    PathfindingGraph graph(2, 2);
    REQUIRE(graph.setTraversalBlocked({0, 0}, {1, 0}, true, "one_way_gate"));
    REQUIRE_FALSE(graph.setTraversalBlocked({0, 0}, {1, 1}, true, "diagonal"));

    const auto routed = graph.findPath({0, 0}, {1, 0});
    REQUIRE(routed.found);
    REQUIRE(routed.nodes == std::vector<PathGridPoint>{{0, 0}, {0, 1}, {1, 1}, {1, 0}});
    REQUIRE(graph.traversalBlocked({0, 0}, {1, 0}));
    REQUIRE(graph.traversalBlockReason({0, 0}, {1, 0}) == "one_way_gate");
    REQUIRE_FALSE(graph.traversalBlocked({1, 0}, {0, 0}));
}

TEST_CASE("PathfindingGraph rejects invalid endpoints with explicit diagnostics", "[pathfinding][graph]") {
    PathfindingGraph graph(2, 2);
    graph.setBlocked(1, 1, true, "goal_blocked");

    const auto outside = graph.findPath({-1, 0}, {1, 1});
    const auto blockedGoal = graph.findPath({0, 0}, {1, 1});

    REQUIRE_FALSE(outside.found);
    REQUIRE(outside.reason == "start_out_of_bounds");
    REQUIRE(outside.diagnostics[0].code == "start_out_of_bounds");
    REQUIRE_FALSE(blockedGoal.found);
    REQUIRE(blockedGoal.reason == "goal_blocked");
    REQUIRE(blockedGoal.diagnostics[0].reason == "goal_blocked");
}

TEST_CASE("PathRequestRouter projects route diagnostics for map and event movement", "[pathfinding][router]") {
    PathfindingGraph graph(4, 3);
    graph.setBlocked(1, 0, true, "rock");
    graph.setBlocked(1, 1, true, "tree");

    PathRequest request;
    request.request_id = "event:npc_001:route_to_player";
    request.actor_id = "npc_001";
    request.surface_id = "map001";
    request.source = PathRequestSource::EventRuntime;
    request.start = {0, 0};
    request.goal = {3, 2};

    const auto routed = RoutePathRequest(graph, request);
    const auto json = routed.toJson();

    REQUIRE(routed.ok);
    REQUIRE(routed.path.found);
    REQUIRE(routed.path.nodes.front() == PathGridPoint{0, 0});
    REQUIRE(routed.path.nodes.back() == PathGridPoint{3, 2});
    REQUIRE(json["request_id"] == "event:npc_001:route_to_player");
    REQUIRE(json["actor_id"] == "npc_001");
    REQUIRE(json["source"] == "event_runtime");
    REQUIRE(json["status"] == "routed");
    REQUIRE(json["route"]["node_count"] == routed.path.nodes.size());
    REQUIRE(json["diagnostics"].empty());
}

TEST_CASE("PathRequestRouter reports blocked movement without mutating request inputs", "[pathfinding][router]") {
    PathfindingGraph graph(2, 2);
    graph.setBlocked(1, 0, true, "closed_gate");
    graph.setBlocked(0, 1, true, "water");

    PathRequest request;
    request.request_id = "map:player:blocked";
    request.actor_id = "player";
    request.surface_id = "map001";
    request.source = PathRequestSource::MapRuntime;
    request.start = {0, 0};
    request.goal = {1, 1};

    const auto routed = RoutePathRequest(graph, request);
    const auto repeated = RoutePathRequest(graph, request);

    REQUIRE_FALSE(routed.ok);
    REQUIRE(routed.status == "blocked");
    REQUIRE(routed.path.reason == "no_route");
    REQUIRE(routed.path.diagnostics.size() == 2);
    REQUIRE(routed.path.diagnostics[0].reason == "closed_gate");
    REQUIRE(repeated.toJson() == routed.toJson());
}
