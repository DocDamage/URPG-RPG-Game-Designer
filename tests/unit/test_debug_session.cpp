#include "engine/core/debug/debug_session.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Breakpoint store adds, deduplicates, and removes breakpoints", "[debug][breakpoint]") {
    urpg::BreakpointStore store;

    REQUIRE(store.Add(urpg::Breakpoint{"evt_1", "block_a", true}));
    REQUIRE_FALSE(store.Add(urpg::Breakpoint{"evt_1", "block_a", true}));
    REQUIRE(store.Has("evt_1", "block_a"));
    REQUIRE(store.Count() == 1);

    REQUIRE(store.Remove("evt_1", "block_a"));
    REQUIRE_FALSE(store.Has("evt_1", "block_a"));
    REQUIRE(store.Count() == 0);
}

TEST_CASE("Watch table tracks values and emits deterministic snapshot", "[debug][watch]") {
    urpg::WatchTable table;

    table.Set("zeta", "100");
    table.Set("alpha", "42");
    table.Set("alpha", "43");

    const auto alpha = table.Get("alpha");
    REQUIRE(alpha.has_value());
    REQUIRE(alpha.value() == "43");

    const auto snapshot = table.SnapshotSorted();
    REQUIRE(snapshot.size() == 2);
    REQUIRE(snapshot[0].key == "alpha");
    REQUIRE(snapshot[1].key == "zeta");

    REQUIRE(table.Remove("zeta"));
    REQUIRE_FALSE(table.Get("zeta").has_value());
}
