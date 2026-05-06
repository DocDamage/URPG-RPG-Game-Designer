#pragma once

#include "editor/project/new_project_wizard_model.h"

#include <nlohmann/json.hpp>

#include <functional>
#include <string>

namespace urpg::editor {

class NewProjectWizardPanel {
public:
    using TemplateStartCallback = std::function<bool(const std::filesystem::path&, std::string*)>;

    void bindModel(NewProjectWizardModel* model);
    void setTemplateStartCallback(TemplateStartCallback callback);
    bool startSelectedTemplate(std::string* error_message = nullptr);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    NewProjectWizardModel* model_ = nullptr;
    TemplateStartCallback template_start_callback_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
