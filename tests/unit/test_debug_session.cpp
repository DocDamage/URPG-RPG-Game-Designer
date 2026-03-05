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

TEST_CASE("Call stack push and pop are deterministic", "[debug][stack]") {
    urpg::CallStack stack;

    stack.Push(urpg::DebugFrame{"Root", "evt_root", "block_root"});
    stack.Push(urpg::DebugFrame{"Nested", "evt_nested", "block_nested"});

    REQUIRE(stack.Depth() == 2);
    REQUIRE(stack.Frames()[0].function_name == "Root");
    REQUIRE(stack.Frames()[1].function_name == "Nested");

    REQUIRE(stack.Pop());
    REQUIRE(stack.Depth() == 1);
    REQUIRE(stack.Pop());
    REQUIRE(stack.Depth() == 0);
    REQUIRE_FALSE(stack.Pop());
}

TEST_CASE("Step controller step into pauses on next tick", "[debug][step]") {
    urpg::StepController controller;

    controller.Request(urpg::StepMode::StepInto, 3);
    REQUIRE(controller.Mode() == urpg::StepMode::StepInto);
    REQUIRE_FALSE(controller.ShouldPause(3));
    REQUIRE(controller.ShouldPause(4));
    REQUIRE(controller.Mode() == urpg::StepMode::Continue);
}

TEST_CASE("Step controller step over pauses when depth unwinds to start", "[debug][step]") {
    urpg::StepController controller;

    controller.Request(urpg::StepMode::StepOver, 2);
    REQUIRE_FALSE(controller.ShouldPause(2));
    REQUIRE_FALSE(controller.ShouldPause(3));
    REQUIRE(controller.ShouldPause(2));
}

TEST_CASE("Step controller step out pauses when depth drops below start", "[debug][step]") {
    urpg::StepController controller;

    controller.Request(urpg::StepMode::StepOut, 2);
    REQUIRE_FALSE(controller.ShouldPause(2));
    REQUIRE_FALSE(controller.ShouldPause(2));
    REQUIRE(controller.ShouldPause(1));
}

TEST_CASE("Debug runtime session wires breakpoints and watches", "[debug][session]") {
    urpg::DebugRuntimeSession session;

    REQUIRE(session.AddBreakpoint(urpg::Breakpoint{"evt_42", "block_7", true}));
    REQUIRE(session.ShouldBreak("evt_42", "block_7"));

    session.SetWatch("hp", "75");
    const auto hp = session.GetWatch("hp");
    REQUIRE(hp.has_value());
    REQUIRE(hp.value() == "75");

    REQUIRE(session.RemoveBreakpoint("evt_42", "block_7"));
    REQUIRE_FALSE(session.ShouldBreak("evt_42", "block_7"));
}

TEST_CASE("Debug runtime session step flow tracks stack depth", "[debug][session]") {
    urpg::DebugRuntimeSession session;

    session.EnterFrame(urpg::DebugFrame{"Root", "evt_root", "block_root"});
    session.RequestStep(urpg::StepMode::StepInto);

    REQUIRE_FALSE(session.ShouldPauseForStep());

    session.EnterFrame(urpg::DebugFrame{"Nested", "evt_nested", "block_nested"});
    REQUIRE(session.ShouldPauseForStep());

    session.RequestStep(urpg::StepMode::StepOut);
    REQUIRE_FALSE(session.ShouldPauseForStep());
    REQUIRE(session.ExitFrame());
    REQUIRE(session.ShouldPauseForStep());
    REQUIRE(session.Stack().Depth() == 1);
}
