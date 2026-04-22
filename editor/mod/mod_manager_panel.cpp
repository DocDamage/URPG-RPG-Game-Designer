#include "editor/mod/mod_manager_panel.h"

#include "engine/core/mod/mod_registry_validator.h"

namespace {

std::string issueCategoryToString(urpg::mod::ModIssueCategory category) {
    using urpg::mod::ModIssueCategory;

    switch (category) {
    case ModIssueCategory::EmptyId:
        return "empty_id";
    case ModIssueCategory::EmptyName:
        return "empty_name";
    case ModIssueCategory::EmptyVersion:
        return "empty_version";
    case ModIssueCategory::DuplicateDependency:
        return "duplicate_dependency";
    case ModIssueCategory::SelfDependency:
        return "self_dependency";
    case ModIssueCategory::MissingEntryPoint:
        return "missing_entry_point";
    }

    return "unknown";
}

std::string issueSeverityToString(urpg::mod::ModIssueSeverity severity) {
    return severity == urpg::mod::ModIssueSeverity::Error ? "error" : "warning";
}

} // namespace

namespace urpg::editor {

void ModManagerPanel::bindRegistry(urpg::mod::ModRegistry* registry) {
    registry_ = registry;
}

void ModManagerPanel::render() {
    nlohmann::json snapshot;
    snapshot["registered_count"] = 0;
    snapshot["active_count"] = 0;
    snapshot["resolved_load_order"] = nlohmann::json::array();
    snapshot["cycle_warning"] = nullptr;

    if (!registry_) {
        last_render_snapshot_ = snapshot;
        return;
    }

    const auto manifests = registry_->listMods();
    const auto activeMods = registry_->listActiveMods();
    urpg::mod::ModRegistryValidator validator;
    const auto issues = validator.validate(manifests);

    snapshot["registered_mod_ids"] = nlohmann::json::array();
    for (const auto& manifest : manifests) {
        snapshot["registered_mod_ids"].push_back(manifest.id);
    }
    snapshot["active_count"] = activeMods.size();
    snapshot["validation_issue_count"] = issues.size();
    snapshot["validation_issues"] = nlohmann::json::array();
    for (const auto& issue : issues) {
        snapshot["validation_issues"].push_back({
            {"mod_id", issue.modId},
            {"category", issueCategoryToString(issue.category)},
            {"severity", issueSeverityToString(issue.severity)},
            {"message", issue.message},
        });
    }

    try {
        const auto loadOrder = registry_->resolveLoadOrder();
        snapshot["resolved_load_order"] = loadOrder;
        snapshot["registered_count"] = manifests.size();
    } catch (const std::runtime_error& e) {
        snapshot["cycle_warning"] = e.what();
        snapshot["registered_count"] = manifests.size();
    }

    last_render_snapshot_ = snapshot;
}

nlohmann::json ModManagerPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
