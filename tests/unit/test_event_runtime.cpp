#include "engine/core/events/event_runtime.h"

#include <catch2/catch_test_macros.hpp>

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
