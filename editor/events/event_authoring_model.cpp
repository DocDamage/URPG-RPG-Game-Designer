#include "editor/events/event_authoring_model.h"

namespace urpg::editor {

void EventAuthoringModel::load(events::EventDocument document, events::EventWorldState state) {
    document_ = std::move(document);
    state_ = std::move(state);
    refresh();
}

void EventAuthoringModel::clear() {
    document_ = {};
    state_ = {};
    graph_ = {};
    diagnostics_.clear();
    selected_template_id_.clear();
    snapshot_ = {};
}

void EventAuthoringModel::startDebugging(const std::string& event_id) {
    debugger_.start(document_, event_id, state_);
    refresh();
}

bool EventAuthoringModel::stepDebugger() {
    const bool running = debugger_.step();
    refresh();
    return running;
}

void EventAuthoringModel::bindTemplateProject(events::EventTemplateProjectService* service) {
    template_project_ = service;
    selected_template_id_.clear();
    pending_template_change_ = PendingTemplateChange::None;
    pending_template_update_.reset();
    pending_template_id_.clear();
    pending_replacement_template_id_.clear();
    pending_template_impact_ = {};
    if (template_project_ != nullptr && template_project_->isOpen()) {
        document_ = template_project_->document();
    }
    refresh();
}

bool EventAuthoringModel::selectTemplate(std::string template_id) {
    if (template_project_ == nullptr ||
        !template_project_->library().definitions().contains(template_id)) {
        return false;
    }
    selected_template_id_ = std::move(template_id);
    refreshTemplateState();
    return true;
}

events::EventTemplateImpact EventAuthoringModel::previewTemplateUpdate(
    events::EventTemplateDefinition replacement) {
    if (template_project_ == nullptr || !template_project_->isOpen()) {
        pending_template_impact_ = {};
        pending_template_impact_.code = "event_template_project_not_open";
        refreshTemplateState();
        return pending_template_impact_;
    }
    pending_template_impact_ = template_project_->previewUpdate(replacement);
    pending_template_change_ = PendingTemplateChange::Update;
    pending_template_update_ = std::move(replacement);
    pending_template_id_ = pending_template_update_->id;
    pending_replacement_template_id_.clear();
    pending_template_revision_ = template_project_->revision();
    refreshTemplateState();
    return pending_template_impact_;
}

events::EventTemplateImpact EventAuthoringModel::previewTemplateDelete(
    std::string template_id, std::string replacement_template_id) {
    if (template_project_ == nullptr || !template_project_->isOpen()) {
        pending_template_impact_ = {};
        pending_template_impact_.code = "event_template_project_not_open";
        refreshTemplateState();
        return pending_template_impact_;
    }
    pending_template_impact_ = template_project_->previewDelete(template_id, replacement_template_id);
    pending_template_change_ = PendingTemplateChange::Delete;
    pending_template_update_.reset();
    pending_template_id_ = std::move(template_id);
    pending_replacement_template_id_ = std::move(replacement_template_id);
    pending_template_revision_ = template_project_->revision();
    refreshTemplateState();
    return pending_template_impact_;
}

events::EventTemplateProjectResult EventAuthoringModel::applyReviewedTemplateChange(
    std::string operation_id) {
    if (template_project_ == nullptr || pending_template_change_ == PendingTemplateChange::None) {
        return {false, "event_template_review_missing", "Preview a template change before applying it.", {}};
    }
    events::EventTemplateProjectResult result;
    if (pending_template_change_ == PendingTemplateChange::Update && pending_template_update_) {
        result = template_project_->applyReviewedUpdate(std::move(operation_id),
                                                        *pending_template_update_,
                                                        pending_template_revision_);
    } else {
        result = template_project_->applyReviewedDelete(std::move(operation_id), pending_template_id_,
                                                        pending_replacement_template_id_,
                                                        pending_template_revision_);
    }
    if (result.success) {
        document_ = template_project_->document();
        pending_template_change_ = PendingTemplateChange::None;
        pending_template_update_.reset();
        pending_template_id_.clear();
        pending_replacement_template_id_.clear();
        pending_template_impact_ = {};
    }
    refresh();
    return result;
}

project::ProjectOperationResult EventAuthoringModel::undoTemplateChange() {
    if (template_project_ == nullptr) {
        return {false, false, "event_template_project_not_open", "No template project is bound.", {}};
    }
    auto result = template_project_->undoLast();
    if (result.success) document_ = template_project_->document();
    refresh();
    return result;
}

project::ProjectOperationResult EventAuthoringModel::redoTemplateChange() {
    if (template_project_ == nullptr) {
        return {false, false, "event_template_project_not_open", "No template project is bound.", {}};
    }
    auto result = template_project_->redoLast();
    if (result.success) document_ = template_project_->document();
    refresh();
    return result;
}

void EventAuthoringModel::refresh() {
    graph_ = events::EventDependencyGraph::build(document_);
    diagnostics_ = document_.validate(state_);
    snapshot_ = {};
    snapshot_.event_count = document_.events().size();
    for (const auto& event : document_.events()) {
        if (event.drag.enabled) {
            ++snapshot_.draggable_event_count;
        }
        snapshot_.page_count += event.pages.size();
        snapshot_.has_active_page = snapshot_.has_active_page || document_.resolveActivePage(event.id, state_).has_value();
        for (const auto& page : event.pages) {
            snapshot_.command_count += page.commands.size();
        }
    }
    snapshot_.diagnostic_count = diagnostics_.size();
    snapshot_.dependency_edge_count = graph_.edges().size();
    snapshot_.debugger_running = debugger_.snapshot().running;
    refreshTemplateState();
}

void EventAuthoringModel::refreshTemplateState() {
    snapshot_.template_project_open = template_project_ != nullptr && template_project_->isOpen();
    snapshot_.template_count = 0;
    snapshot_.template_instance_count = 0;
    snapshot_.template_picker_ids.clear();
    if (snapshot_.template_project_open) {
        snapshot_.template_count = template_project_->library().definitions().size();
        snapshot_.template_instance_count = template_project_->library().instances().size();
        for (const auto& [id, definition] : template_project_->library().definitions()) {
            (void)definition;
            snapshot_.template_picker_ids.push_back(id);
        }
        if (!selected_template_id_.empty() &&
            !template_project_->library().definitions().contains(selected_template_id_)) {
            selected_template_id_.clear();
        }
    } else {
        selected_template_id_.clear();
    }
    snapshot_.selected_template_id = selected_template_id_;
    snapshot_.template_review_pending = pending_template_change_ != PendingTemplateChange::None;
    snapshot_.template_review_can_apply = snapshot_.template_review_pending && pending_template_impact_.valid;
    snapshot_.template_review_code = pending_template_impact_.code;
    snapshot_.template_review_affected_instance_count = pending_template_impact_.affected_instance_count;
    snapshot_.template_review_caller_count = pending_template_impact_.caller_count;
}

} // namespace urpg::editor
