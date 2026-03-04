#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
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
    bool reentrancy_enabled = false;
    size_t reentrancy_depth_limit = 1;
    bool canceled = false;
    bool default_prevented = false;
};

enum class EventExecutionPhase : uint8_t {
    Enter = 0,
    Exit = 1
};

struct EventExecutionTrace {
    std::string event_id;
    EventExecutionPhase phase = EventExecutionPhase::Enter;
    size_t depth = 0;
};

class EventExecutionTimeline {
public:
    void Enter(std::string_view event_id);
    void Exit(std::string_view event_id);

    size_t CurrentDepth() const;
    size_t MaxDepth() const;
    const std::vector<EventExecutionTrace>& Entries() const;

private:
    size_t current_depth_ = 0;
    size_t max_depth_ = 0;
    std::vector<EventExecutionTrace> entries_;
};

class EventDispatchSession {
public:
    bool CanEnter(const EventInvocation& invocation) const;
    void BeginInvocation(const EventInvocation& invocation);
    void EndInvocation(const EventInvocation& invocation);

    std::vector<EventInvocation> BuildDispatchPlan(std::vector<EventInvocation> invocations) const;

    const EventExecutionTimeline& Timeline() const;
    EventExecutionTimeline& Timeline();

private:
    EventExecutionTimeline timeline_;
    std::unordered_map<std::string, size_t> active_depth_by_event_;
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
