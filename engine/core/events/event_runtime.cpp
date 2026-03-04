#include "engine/core/events/event_runtime.h"

#include <algorithm>

namespace urpg {

std::vector<EventInvocation> EventRuntimeKernel::BuildExecutionOrder(std::vector<EventInvocation> invocations) {
    std::stable_sort(
        invocations.begin(),
        invocations.end(),
        [](const EventInvocation& left, const EventInvocation& right) {
            if (left.priority != right.priority) {
                return static_cast<uint8_t>(left.priority) < static_cast<uint8_t>(right.priority);
            }
            return left.registration_order < right.registration_order;
        }
    );
    return invocations;
}

void EventRuntimeKernel::Cancel(EventInvocation& invocation) {
    invocation.canceled = true;
}

void EventRuntimeKernel::PreventDefault(EventInvocation& invocation) {
    invocation.default_prevented = true;
}

bool EventRuntimeKernel::IsCanceled(const EventInvocation& invocation) {
    return invocation.canceled;
}

bool EventRuntimeKernel::ShouldRunDefaultBehavior(const EventInvocation& invocation) {
    return !invocation.default_prevented;
}

} // namespace urpg
