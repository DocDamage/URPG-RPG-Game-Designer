#pragma once
#include "engine/core/events/event_document.h"
#include <map>
#include <set>

namespace urpg::events {
struct NativeEventCommandCapability {
    EventCommandKind kind=EventCommandKind::Unsupported; std::string id, category, authoring_ui, serialization_key, runtime_effect;
    std::vector<std::string> diagnostic_codes; bool undo_supported=true; bool controlled_call=false;
};
struct NativeEventRuntimeState {
    std::map<std::string,bool> switches, self_switches; std::map<std::string,int64_t> variables;
    size_t branch_depth=0, loop_depth=0, parallel_lane_count=0; int64_t wait_frames=0;
    std::vector<std::string> actions, changes; std::set<std::string> allowed_calls;
};
struct NativeEventCommandResult { bool success=false; bool waits=false; bool branch_taken=false; std::string code, effect; std::vector<std::string> changes; };
const std::vector<NativeEventCommandCapability>& nativeEventCommandMatrix();
std::vector<EventDiagnostic> validateNativeEventCommandMatrix();
NativeEventCommandResult executeNativeEventCommand(const EventCommand& command, NativeEventRuntimeState& state);
} // namespace urpg::events
