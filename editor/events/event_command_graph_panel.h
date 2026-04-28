#pragma once

#include "engine/core/events/event_command_graph.h"

#include <string>

namespace urpg::editor {

struct EventCommandGraphPanelSnapshot {
    bool visible = true;
    bool rendered = false;
    bool disabled = true;
    std::string graph_id;
    std::string selected_node_id;
    size_t node_count = 0;
    size_t edge_count = 0;
    size_t runtime_command_count = 0;
    size_t diagnostic_count = 0;
    size_t dependency_edge_count = 0;
    bool runtime_executed = false;
    std::string saved_project_json;
    std::string status_message = "Load an event command graph before rendering this panel.";
};

class EventCommandGraphPanel {
public:
    void loadDocument(urpg::events::EventCommandGraphDocument document,
                      urpg::events::EventWorldState initial_state = {});
    void selectNode(std::string node_id);
    void render();

    const EventCommandGraphPanelSnapshot& snapshot() const { return snapshot_; }
    const urpg::events::EventCommandGraphRuntimeResult& runtimePreview() const { return runtime_preview_; }
    const urpg::events::EventDocument& runtimeDocument() const { return runtime_document_; }
    bool hasRenderedFrame() const { return snapshot_.rendered; }

private:
    void refreshPreview();

    urpg::events::EventCommandGraphDocument document_;
    urpg::events::EventWorldState initial_state_;
    urpg::events::EventCommandGraphRuntimeResult runtime_preview_;
    urpg::events::EventDocument runtime_document_;
    EventCommandGraphPanelSnapshot snapshot_{};
    bool loaded_ = false;
};

} // namespace urpg::editor
