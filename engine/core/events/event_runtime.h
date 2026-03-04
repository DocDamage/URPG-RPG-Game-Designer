#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg {

enum class EventPriority : uint8_t {
    Critical = 0,
    High = 1,
    Normal = 2,
    Low = 3,
    Last = 4
};

struct EventInvocation {
    std::string event_id;
    EventPriority priority = EventPriority::Normal;
    uint64_t registration_order = 0;
    bool canceled = false;
    bool default_prevented = false;
};

class EventRuntimeKernel {
public:
    static std::vector<EventInvocation> BuildExecutionOrder(std::vector<EventInvocation> invocations);

    static void Cancel(EventInvocation& invocation);
    static void PreventDefault(EventInvocation& invocation);

    static bool IsCanceled(const EventInvocation& invocation);
    static bool ShouldRunDefaultBehavior(const EventInvocation& invocation);
};

} // namespace urpg
