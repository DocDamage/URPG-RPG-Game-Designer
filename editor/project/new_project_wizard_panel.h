#pragma once

#include "editor/project/new_project_wizard_model.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class NewProjectWizardPanel {
public:
    void bindModel(NewProjectWizardModel* model);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    NewProjectWizardModel* model_ = nullptr;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
