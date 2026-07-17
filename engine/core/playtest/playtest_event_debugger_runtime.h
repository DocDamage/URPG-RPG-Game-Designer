#pragma once

#include "engine/core/playtest/playtest_event_trace_bridge.h"

namespace urpg::playtest {

// Runtime-side owner for the bounded playtest event-debugger lane. It advances
// at most one event command per runtime frame and publishes after each admitted
// control batch or automatic step.
class PlaytestEventDebuggerRuntime {
public:
    explicit PlaytestEventDebuggerRuntime(std::filesystem::path session_directory)
        : bridge_(std::move(session_directory)) {}

    bool start(std::string session_id, const events::EventDocument& document,
               const std::string& event_id, events::EventWorldState state = {},
               std::string* diagnostic = nullptr);
    bool tick(std::string* diagnostic = nullptr);

    bool active() const { return active_; }
    uint64_t revision() const { return revision_; }
    const events::EventDebugSnapshot snapshot() const { return debugger_.snapshot(); }

private:
    bool publish(std::string* diagnostic);

    PlaytestEventTraceBridge bridge_;
    events::EventDocument document_;
    events::EventDebugger debugger_;
    std::string session_id_;
    uint64_t revision_ = 0;
    bool active_ = false;
};

} // namespace urpg::playtest
