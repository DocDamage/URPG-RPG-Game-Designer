#include "editor/action/controller_binding_panel.h"

namespace {

std::string issueSeverityToString(urpg::action::ControllerBindingIssueSeverity severity) {
    return severity == urpg::action::ControllerBindingIssueSeverity::Error ? "error" : "warning";
}

std::string issueCategoryToString(urpg::action::ControllerBindingIssueCategory category) {
    switch (category) {
    case urpg::action::ControllerBindingIssueCategory::MissingRequiredAction:
        return "missing_required_action";
    }

    return "unknown";
}

} // namespace

namespace urpg::editor {

void ControllerBindingPanel::bindRuntime(urpg::action::ControllerBindingRuntime* runtime) {
    runtime_ = runtime;
}

void ControllerBindingPanel::render() {
    nlohmann::json snapshot;
    snapshot["binding_count"] = 0;
    snapshot["unsaved_changes"] = false;
    snapshot["bindings"] = nlohmann::json::array();
    snapshot["missing_required_actions"] = nlohmann::json::array();
    snapshot["issue_count"] = 0;
    snapshot["issues"] = nlohmann::json::array();

    if (runtime_ == nullptr) {
        last_render_snapshot_ = std::move(snapshot);
        return;
    }

    const auto bindings = runtime_->getAllBindings();
    const auto missing_actions = runtime_->getMissingRequiredActions();
    const auto issues = runtime_->getIssues();

    for (const auto& [button, action] : bindings) {
        snapshot["bindings"].push_back({
            {"button", urpg::action::ControllerBindingRuntime::buttonToString(button)},
            {"action", urpg::action::ControllerBindingRuntime::actionToString(action)},
        });
    }

    for (const auto action : missing_actions) {
        snapshot["missing_required_actions"].push_back(
            urpg::action::ControllerBindingRuntime::actionToString(action));
    }

    for (const auto& issue : issues) {
        snapshot["issues"].push_back({
            {"category", issueCategoryToString(issue.category)},
            {"severity", issueSeverityToString(issue.severity)},
            {"action", urpg::action::ControllerBindingRuntime::actionToString(issue.action)},
            {"message", issue.message},
        });
    }

    snapshot["binding_count"] = bindings.size();
    snapshot["unsaved_changes"] = runtime_->hasUnsavedChanges();
    snapshot["issue_count"] = issues.size();
    last_render_snapshot_ = std::move(snapshot);
}

nlohmann::json ControllerBindingPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
