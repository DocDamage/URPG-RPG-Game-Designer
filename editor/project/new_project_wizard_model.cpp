#include "editor/project/new_project_wizard_model.h"

#include <utility>

namespace urpg::editor {

void NewProjectWizardModel::setTemplateId(std::string template_id) {
    request_.template_id = std::move(template_id);
    cancelled_ = false;
}

void NewProjectWizardModel::setProjectId(std::string project_id) {
    request_.project_id = std::move(project_id);
    cancelled_ = false;
}

void NewProjectWizardModel::setProjectName(std::string project_name) {
    request_.project_name = std::move(project_name);
    cancelled_ = false;
}

void NewProjectWizardModel::cancel() {
    cancelled_ = true;
}

urpg::project::ProjectTemplateResult NewProjectWizardModel::createProject() {
    if (cancelled_) {
        urpg::project::ProjectTemplateResult result;
        result.errors.push_back("wizard_cancelled");
        return result;
    }
    auto result = generator_.generate(request_);
    last_audit_ = result.audit_report;
    return result;
}

nlohmann::json NewProjectWizardModel::snapshot() const {
    return {
        {"template_id", request_.template_id},
        {"project_id", request_.project_id},
        {"project_name", request_.project_name},
        {"cancelled", cancelled_},
        {"last_audit", last_audit_.is_null() ? nlohmann::json::object() : last_audit_},
    };
}

} // namespace urpg::editor
