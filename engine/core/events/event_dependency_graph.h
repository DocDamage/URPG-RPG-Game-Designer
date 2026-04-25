#pragma once

#include "engine/core/events/event_document.h"

#include <string>
#include <vector>

namespace urpg::events {

enum class EventDependencyAccess : uint8_t {
    Read,
    Write,
    Call
};

struct EventDependencyEdge {
    std::string source_id;
    std::string target_type;
    std::string target_id;
    EventDependencyAccess access = EventDependencyAccess::Read;
};

class EventDependencyGraph {
public:
    static EventDependencyGraph build(const EventDocument& document);

    const std::vector<EventDependencyEdge>& edges() const { return edges_; }
    std::vector<EventDependencyEdge> edgesForSource(const std::string& source_id) const;

private:
    std::vector<EventDependencyEdge> edges_;
};

} // namespace urpg::events
