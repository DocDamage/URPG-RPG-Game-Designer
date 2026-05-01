#include "editor/assets/asset_library_panel.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

namespace {

AssetLibraryPanel::ImportWizardRenderSnapshot buildImportWizardRenderSnapshot(const nlohmann::json& wizard) {
    AssetLibraryPanel::ImportWizardRenderSnapshot snapshot;
    if (!wizard.is_object()) {
        return snapshot;
    }

    snapshot.status = wizard.value("status", snapshot.status);
    snapshot.current_step = wizard.value("current_step", snapshot.current_step);
    snapshot.pending_request = wizard.contains("pending_request") ? wizard["pending_request"] : nlohmann::json(nullptr);

    if (wizard.contains("steps") && wizard["steps"].is_array()) {
        for (const auto& step : wizard["steps"]) {
            AssetLibraryPanel::ImportWizardStepSnapshot row;
            row.id = step.value("id", "");
            row.label = step.value("label", "");
            row.state = step.value("state", "");
            row.count = step.value("count", 0u);
            row.active = row.state == "active";
            row.complete = row.state == "complete";
            row.available = row.state == "available";
            snapshot.steps.push_back(std::move(row));
        }
    }

    if (wizard.contains("actions") && wizard["actions"].is_object()) {
        for (const auto& [id, action] : wizard["actions"].items()) {
            AssetLibraryPanel::ImportWizardActionSnapshot row;
            row.id = id;
            row.action = action.value("action", "");
            row.enabled = action.value("enabled", false);
            row.pending_request = action.value("pending_request", false);
            row.eligible_count = action.value("eligible_count", 0u);
            if (action.contains("disabled_reason") && action["disabled_reason"].is_string()) {
                row.disabled_reason = action["disabled_reason"].get<std::string>();
            }
            snapshot.actions.push_back(std::move(row));
        }
    }

    const auto package = std::find_if(snapshot.actions.begin(), snapshot.actions.end(), [](const auto& action) {
        return action.id == "package_validate";
    });
    snapshot.package_validation_ready = package != snapshot.actions.end() && package->enabled;
    return snapshot;
}

} // namespace

void AssetLibraryPanel::render() {
    if (!visible_) {
        return;
    }
    last_render_snapshot_ = model_.snapshot();
    last_import_wizard_snapshot_ = buildImportWizardRenderSnapshot(last_render_snapshot_.import_wizard);
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
