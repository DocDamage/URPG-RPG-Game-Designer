#pragma once

#include "engine/core/project/project_template_generator.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class NewProjectWizardModel {
public:
    void setTemplateId(std::string template_id);
    void setProjectId(std::string project_id);
    void setProjectName(std::string project_name);
    void cancel();
    urpg::project::ProjectTemplateResult createProject();
    nlohmann::json snapshot() const;

private:
    urpg::project::ProjectTemplateGenerator generator_;
    urpg::project::ProjectTemplateRequest request_{"jrpg", "new_project", "New Project"};
    bool cancelled_ = false;
    nlohmann::json last_audit_;
};

} // namespace urpg::editor
