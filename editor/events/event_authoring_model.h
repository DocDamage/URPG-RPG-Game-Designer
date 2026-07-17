#pragma once

#include "engine/core/events/event_debugger.h"
#include "engine/core/events/event_dependency_graph.h"
#include "engine/core/events/event_template_project_service.h"

#include <optional>

namespace urpg::editor {

struct EventAuthoringModelSnapshot {
    size_t event_count = 0;
    size_t page_count = 0;
    size_t command_count = 0;
    size_t draggable_event_count = 0;
    size_t diagnostic_count = 0;
    size_t dependency_edge_count = 0;
    size_t template_count = 0;
    size_t template_instance_count = 0;
    size_t template_review_affected_instance_count = 0;
    size_t template_review_caller_count = 0;
    bool has_active_page = false;
    bool debugger_running = false;
    bool template_project_open = false;
    bool template_review_pending = false;
    bool template_review_can_apply = false;
    std::string selected_template_id;
    std::string template_review_code;
    std::vector<std::string> template_picker_ids;
};

class EventAuthoringModel {
public:
    void load(events::EventDocument document, events::EventWorldState state = {});
    void clear();
    void startDebugging(const std::string& event_id);
    bool stepDebugger();
    void bindTemplateProject(events::EventTemplateProjectService* service);
    bool selectTemplate(std::string template_id);
    events::EventTemplateImpact previewTemplateUpdate(events::EventTemplateDefinition replacement);
    events::EventTemplateImpact previewTemplateDelete(std::string template_id,
                                                      std::string replacement_template_id);
    events::EventTemplateProjectResult applyReviewedTemplateChange(std::string operation_id);
    project::ProjectOperationResult undoTemplateChange();
    project::ProjectOperationResult redoTemplateChange();

    const events::EventDocument& document() const { return document_; }
    const events::EventDependencyGraph& dependencyGraph() const { return graph_; }
    const std::vector<events::EventDiagnostic>& diagnostics() const { return diagnostics_; }
    const EventAuthoringModelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();
    void refreshTemplateState();

    enum class PendingTemplateChange { None, Update, Delete };

    events::EventDocument document_;
    events::EventWorldState state_;
    events::EventDependencyGraph graph_;
    events::EventDebugger debugger_;
    std::vector<events::EventDiagnostic> diagnostics_;
    EventAuthoringModelSnapshot snapshot_{};
    events::EventTemplateProjectService* template_project_ = nullptr;
    std::string selected_template_id_;
    PendingTemplateChange pending_template_change_ = PendingTemplateChange::None;
    std::optional<events::EventTemplateDefinition> pending_template_update_;
    std::string pending_template_id_;
    std::string pending_replacement_template_id_;
    uint64_t pending_template_revision_ = 0;
    events::EventTemplateImpact pending_template_impact_;
};

} // namespace urpg::editor
