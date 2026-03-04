#include "engine/core/events/event_runtime.h"

#include <algorithm>

namespace urpg {

void EventExecutionTimeline::Enter(std::string_view event_id) {
    ++current_depth_;
    max_depth_ = std::max(max_depth_, current_depth_);
    entries_.push_back(EventExecutionTrace{std::string(event_id), EventExecutionPhase::Enter, current_depth_});
}

void EventExecutionTimeline::Exit(std::string_view event_id) {
    const size_t depth = current_depth_;
    entries_.push_back(EventExecutionTrace{std::string(event_id), EventExecutionPhase::Exit, depth});
    if (current_depth_ > 0) {
        --current_depth_;
    }
}

size_t EventExecutionTimeline::CurrentDepth() const {
    return current_depth_;
}

size_t EventExecutionTimeline::MaxDepth() const {
    return max_depth_;
}

const std::vector<EventExecutionTrace>& EventExecutionTimeline::Entries() const {
    return entries_;
}

bool EventDispatchSession::CanEnter(const EventInvocation& invocation) const {
    const auto it = active_depth_by_event_.find(invocation.event_id);
    const size_t active_depth = (it == active_depth_by_event_.end()) ? 0 : it->second;

    if (!invocation.reentrancy_enabled) {
        return active_depth == 0;
    }

    return active_depth < invocation.reentrancy_depth_limit;
}

void EventDispatchSession::BeginInvocation(const EventInvocation& invocation) {
    if (!CanEnter(invocation)) {
        return;
    }

    ++active_depth_by_event_[invocation.event_id];
    timeline_.Enter(invocation.event_id);
}

void EventDispatchSession::EndInvocation(const EventInvocation& invocation) {
    timeline_.Exit(invocation.event_id);

    auto it = active_depth_by_event_.find(invocation.event_id);
    if (it == active_depth_by_event_.end()) {
        return;
    }

    if (it->second <= 1) {
        active_depth_by_event_.erase(it);
        return;
    }

    --it->second;
}

std::vector<EventInvocation> EventDispatchSession::BuildDispatchPlan(std::vector<EventInvocation> invocations) const {
    auto ordered = EventRuntimeKernel::BuildExecutionOrder(std::move(invocations));
    ordered.erase(
        std::remove_if(
            ordered.begin(),
            ordered.end(),
            [&](const EventInvocation& invocation) {
                return !CanEnter(invocation);
            }
        ),
        ordered.end()
    );
    return ordered;
}

const EventExecutionTimeline& EventDispatchSession::Timeline() const {
    return timeline_;
}

EventExecutionTimeline& EventDispatchSession::Timeline() {
    return timeline_;
}

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
