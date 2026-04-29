#pragma once

#include "engine/core/events/event_debugger.h"
#include "engine/core/events/event_dependency_graph.h"

namespace urpg::editor {

struct EventAuthoringModelSnapshot {
    size_t event_count = 0;
    size_t page_count = 0;
    size_t command_count = 0;
    size_t draggable_event_count = 0;
    size_t diagnostic_count = 0;
    size_t dependency_edge_count = 0;
    bool has_active_page = false;
    bool debugger_running = false;
};

class EventAuthoringModel {
public:
    void load(events::EventDocument document, events::EventWorldState state = {});
    void clear();
    void startDebugging(const std::string& event_id);
    bool stepDebugger();

    const events::EventDocument& document() const { return document_; }
    const events::EventDependencyGraph& dependencyGraph() const { return graph_; }
    const std::vector<events::EventDiagnostic>& diagnostics() const { return diagnostics_; }
    const EventAuthoringModelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    events::EventDocument document_;
    events::EventWorldState state_;
    events::EventDependencyGraph graph_;
    events::EventDebugger debugger_;
    std::vector<events::EventDiagnostic> diagnostics_;
    EventAuthoringModelSnapshot snapshot_{};
};

} // namespace urpg::editor
