#include "editor/project/new_project_wizard_panel.h"

namespace urpg::editor {

void NewProjectWizardPanel::bindModel(NewProjectWizardModel* model) {
    model_ = model;
}

void NewProjectWizardPanel::render() {
    if (!model_) {
        snapshot_ = {
            {"panel", "new_project_wizard"},
            {"status", "disabled"},
            {"disabled_reason", "No NewProjectWizardModel is bound."},
            {"owner", "editor/project"},
            {"unlock_condition", "Bind NewProjectWizardModel before rendering the new project wizard."},
        };
        return;
    }
    snapshot_ = {
        {"panel", "new_project_wizard"},
        {"status", "ready"},
        {"model", model_->snapshot()},
    };
}

nlohmann::json NewProjectWizardPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
