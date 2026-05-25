#include "editor/diagnostics/pathfinding_debug_panel.h"
#include "engine/core/level/path_request_router.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("PathfindingDebugPanel exposes route nodes and costs", "[editor][diagnostics][pathfinding]") {
    urpg::level::PathfindingGraph graph(4, 3);
    graph.setBlocked(1, 0, true, "rock");

    urpg::level::PathRequest request;
    request.request_id = "event:npc_001:path";
    request.actor_id = "npc_001";
    request.surface_id = "map001";
    request.source = urpg::level::PathRequestSource::EventRuntime;
    request.start = {0, 0};
    request.goal = {3, 2};

    urpg::editor::PathfindingDebugPanel panel;
    panel.bindRoute(urpg::level::RoutePathRequest(graph, request));
    const auto snapshot = panel.snapshot();
    const auto json = snapshot.toJson();

    REQUIRE(snapshot.has_route);
    REQUIRE(snapshot.status == "routed");
    REQUIRE(snapshot.node_rows.size() == snapshot.node_count);
    REQUIRE(snapshot.node_rows.front().x == 0);
    REQUIRE(snapshot.node_rows.back().x == 3);
    REQUIRE(json["request_id"] == "event:npc_001:path");
    REQUIRE(json["node_count"] == snapshot.node_count);
    REQUIRE(json["nodes"].size() == snapshot.node_count);
}

TEST_CASE("PathfindingDebugPanel exposes blocked movement reasons", "[editor][diagnostics][pathfinding]") {
    urpg::level::PathfindingGraph graph(2, 2);
    graph.setBlocked(1, 0, true, "closed_gate");
    graph.setBlocked(0, 1, true, "water");

    urpg::level::PathRequest request;
    request.request_id = "map:player:blocked";
    request.actor_id = "player";
    request.surface_id = "map001";
    request.start = {0, 0};
    request.goal = {1, 1};

    urpg::editor::PathfindingDebugPanel panel;
    panel.bindRoute(urpg::level::RoutePathRequest(graph, request));

    const auto snapshot = panel.snapshot();
    REQUIRE_FALSE(snapshot.has_route);
    REQUIRE(snapshot.status == "blocked");
    REQUIRE(snapshot.diagnostic_rows.size() == 2);
    REQUIRE(snapshot.diagnostic_rows[0].reason == "closed_gate");
    REQUIRE(snapshot.toJson()["diagnostics"][1]["reason"] == "water");
}
