#include "engine/core/events/event_runtime.h"
#include "engine/core/events/map_event_interpreter.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg;

TEST_CASE("MapEventInterpreter: Command Flow", "[event][interpreter]") {
    std::vector<EventCommand> list = {{EventOpcode::ControlSwitches, 0, {1, true}}, // Switch 1 = ON
                                      {EventOpcode::PlaySE, 0, {"Cursor1", 100, 100, 0}},
                                      {EventOpcode::End, 0, {}}};

    MapEventInterpreter interp(list);

    SECTION("Interprets commands sequentially") {
        REQUIRE(interp.isRunning());
        REQUIRE(interp.getIndex() == 0);

        bool cont = interp.update(); // Switch
        REQUIRE(cont == true);       // Immediate continue
        REQUIRE(interp.getIndex() == 1);

        cont = interp.update(); // PlaySE
        REQUIRE(cont == true);
        REQUIRE(interp.getIndex() == 2);

        cont = interp.update(); // End
        REQUIRE_FALSE(interp.isRunning());
    }

    SECTION("ShowMessage triggers waiting status") {
        std::vector<EventCommand> msgList = {{EventOpcode::ShowMessage, 0, {"Hello World"}}};
        MapEventInterpreter msgInterp(msgList);

        bool cont = msgInterp.update();
        REQUIRE(cont == false); // Should wait (e.g. for user input)
    }
}

TEST_CASE("Event runtime orders by priority then registration order", "[events][runtime]") {
    std::vector<urpg::EventInvocation> invocations{
        urpg::EventInvocation{"evt_normal_2", urpg::EventPriority::Normal, 20},
        urpg::EventInvocation{"evt_high", urpg::EventPriority::High, 7},
        urpg::EventInvocation{"evt_normal_1", urpg::EventPriority::Normal, 5},
        urpg::EventInvocation{"evt_last", urpg::EventPriority::Last, 1},
        urpg::EventInvocation{"evt_critical", urpg::EventPriority::Critical, 2},
    };

    const auto ordered = urpg::EventRuntimeKernel::BuildExecutionOrder(std::move(invocations));
    REQUIRE(ordered.size() == 5);
    REQUIRE(ordered[0].event_id == "evt_critical");
    REQUIRE(ordered[1].event_id == "evt_high");
    REQUIRE(ordered[2].event_id == "evt_normal_1");
    REQUIRE(ordered[3].event_id == "evt_normal_2");
    REQUIRE(ordered[4].event_id == "evt_last");
}

TEST_CASE("Event runtime cancellation toggles canceled state", "[events][runtime]") {
    urpg::EventInvocation invocation{"evt_cancel", urpg::EventPriority::Normal, 1};
    REQUIRE_FALSE(urpg::EventRuntimeKernel::IsCanceled(invocation));

    urpg::EventRuntimeKernel::Cancel(invocation);
    REQUIRE(urpg::EventRuntimeKernel::IsCanceled(invocation));
}

TEST_CASE("Event runtime preventDefault controls default behavior", "[events][runtime]") {
    urpg::EventInvocation invocation{"evt_default", urpg::EventPriority::Normal, 1};
    REQUIRE(urpg::EventRuntimeKernel::ShouldRunDefaultBehavior(invocation));

    urpg::EventRuntimeKernel::PreventDefault(invocation);
    REQUIRE_FALSE(urpg::EventRuntimeKernel::ShouldRunDefaultBehavior(invocation));
}

TEST_CASE("Event execution timeline tracks reentrancy depth and order", "[events][runtime]") {
    urpg::EventExecutionTimeline timeline;

    timeline.Enter("evt_root");
    timeline.Enter("evt_nested");
    timeline.Exit("evt_nested");
    timeline.Exit("evt_root");

    REQUIRE(timeline.CurrentDepth() == 0);
    REQUIRE(timeline.MaxDepth() == 2);

    const auto& entries = timeline.Entries();
    REQUIRE(entries.size() == 4);

    REQUIRE(entries[0].event_id == "evt_root");
    REQUIRE(entries[0].phase == urpg::EventExecutionPhase::Enter);
    REQUIRE(entries[0].depth == 1);

    REQUIRE(entries[1].event_id == "evt_nested");
    REQUIRE(entries[1].phase == urpg::EventExecutionPhase::Enter);
    REQUIRE(entries[1].depth == 2);

    REQUIRE(entries[2].event_id == "evt_nested");
    REQUIRE(entries[2].phase == urpg::EventExecutionPhase::Exit);
    REQUIRE(entries[2].depth == 2);

    REQUIRE(entries[3].event_id == "evt_root");
    REQUIRE(entries[3].phase == urpg::EventExecutionPhase::Exit);
    REQUIRE(entries[3].depth == 1);
}

TEST_CASE("Event execution timeline tolerates unmatched exit without underflow", "[events][runtime]") {
    urpg::EventExecutionTimeline timeline;

    timeline.Exit("evt_orphan");

    REQUIRE(timeline.CurrentDepth() == 0);
    REQUIRE(timeline.MaxDepth() == 0);
    REQUIRE(timeline.Entries().size() == 1);
    REQUIRE(timeline.Entries()[0].phase == urpg::EventExecutionPhase::Exit);
    REQUIRE(timeline.Entries()[0].depth == 0);
}

TEST_CASE("Event dispatch session enforces non-reentrant event rule", "[events][runtime]") {
    urpg::EventDispatchSession session;
    urpg::EventInvocation invocation{"evt_gate", urpg::EventPriority::Normal, 1};

    REQUIRE(session.CanEnter(invocation));
    session.BeginInvocation(invocation);
    REQUIRE_FALSE(session.CanEnter(invocation));

    auto plan = session.BuildDispatchPlan(std::vector<urpg::EventInvocation>{invocation});
    REQUIRE(plan.empty());

    session.EndInvocation(invocation);
    REQUIRE(session.CanEnter(invocation));
}

TEST_CASE("Event dispatch session allows bounded reentrancy and records timeline", "[events][runtime]") {
    urpg::EventDispatchSession session;
    urpg::EventInvocation invocation{"evt_reentrant", urpg::EventPriority::Normal, 1};
    invocation.reentrancy_enabled = true;
    invocation.reentrancy_depth_limit = 2;

    REQUIRE(session.CanEnter(invocation));
    session.BeginInvocation(invocation);
    REQUIRE(session.CanEnter(invocation));
    session.BeginInvocation(invocation);
    REQUIRE_FALSE(session.CanEnter(invocation));

    session.EndInvocation(invocation);
    session.EndInvocation(invocation);

    const auto& entries = session.Timeline().Entries();
    REQUIRE(entries.size() == 4);
    REQUIRE(entries[0].phase == urpg::EventExecutionPhase::Enter);
    REQUIRE(entries[1].phase == urpg::EventExecutionPhase::Enter);
    REQUIRE(entries[2].phase == urpg::EventExecutionPhase::Exit);
    REQUIRE(entries[3].phase == urpg::EventExecutionPhase::Exit);
}
