#include "engine/core/perf/primary_route_work.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Primary route audit requires bounded jobs indexes and latency traces", "[perf][primary_routes][pcq701]") {
    using urpg::perf::PrimaryRoute;
    std::vector<urpg::perf::PrimaryRouteTrace> traces;
    for (const auto route : {PrimaryRoute::ProjectOpen, PrimaryRoute::AssetBrowse, PrimaryRoute::Search,
                             PrimaryRoute::MapEdit, PrimaryRoute::Save, PrimaryRoute::PlaytestLaunch,
                             PrimaryRoute::Package, PrimaryRoute::ProjectGraph}) {
        const bool indexed = route == PrimaryRoute::AssetBrowse || route == PrimaryRoute::Search ||
                             route == PrimaryRoute::ProjectGraph;
        traces.push_back({route, false, false, true, indexed, 1000, 8000, 10000, 64});
    }
    const auto result = urpg::perf::PrimaryRouteWorkAudit{}.evaluate(traces);
    REQUIRE(result.complete);
    REQUIRE(result.within_budgets);
    REQUIRE(result.diagnostics.empty());

    traces[0].blocking_io = true;
    traces[1].incremental_index = false;
    traces[2].interaction_p95_us = 10001;
    const auto failed = urpg::perf::PrimaryRouteWorkAudit{}.evaluate(traces);
    REQUIRE_FALSE(failed.within_budgets);
    REQUIRE(failed.diagnostics.size() == 3);
}

TEST_CASE("Primary route jobs and indexes remain incrementally bounded", "[perf][primary_routes][pcq701]") {
    urpg::perf::BoundedWorkQueue queue(2);
    REQUIRE(queue.enqueue({"scan-a", 5}));
    REQUIRE(queue.enqueue({"scan-b", 3}));
    REQUIRE_FALSE(queue.enqueue({"scan-c", 1}));
    REQUIRE(queue.advance(4).empty());
    REQUIRE(queue.size() == 2);
    REQUIRE(queue.advance(2) == std::vector<std::string>{"scan-a"});
    REQUIRE(queue.size() == 1);
    REQUIRE(queue.cancel("scan-b"));
    REQUIRE(queue.size() == 0);
    REQUIRE(queue.consumedUnits() == 6);

    urpg::perf::IncrementalRouteIndex index;
    REQUIRE(index.upsert("map:2", "Forest Shrine"));
    REQUIRE(index.upsert("map:1", "Forest Entrance"));
    REQUIRE(index.search("FOREST", 1) == std::vector<std::string>{"map:1"});
    REQUIRE(index.revision() == 2);
    REQUIRE(index.remove("map:1"));
    REQUIRE(index.search("forest", 4) == std::vector<std::string>{"map:2"});
}
