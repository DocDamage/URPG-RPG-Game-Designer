#pragma once
#include "engine/core/events/event_document.h"
namespace urpg::events {
struct EventStaticFinding { std::string severity,code,message,suggestion,event_id,page_id,command_id; };
std::vector<EventStaticFinding> analyzeEventDocument(const EventDocument& document,const EventWorldState& state={});
} // namespace urpg::events
