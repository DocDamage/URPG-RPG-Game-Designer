#include "editor/project/new_project_wizard_panel.h"

namespace urpg::editor {

void NewProjectWizardPanel::bindModel(NewProjectWizardModel* model) {
    model_ = model;
}

void NewProjectWizardPanel::render() {
    if (!model_) {
        snapshot_ = nlohmann::json::object();
        return;
    }
    snapshot_ = {
        {"panel", "new_project_wizard"},
        {"model", model_->snapshot()},
    };
}

nlohmann::json NewProjectWizardPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
