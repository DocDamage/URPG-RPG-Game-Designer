#pragma once

#include <nlohmann/json.hpp>

#include <set>
#include <string>
#include <vector>

namespace urpg::project {

struct ProjectTemplateRequest {
    std::string template_id;
    std::string project_id;
    std::string project_name;
};

struct ProjectTemplateResult {
    bool success = false;
    nlohmann::json project;
    nlohmann::json audit_report;
    std::vector<std::string> errors;
};

class ProjectTemplateGenerator {
public:
    ProjectTemplateResult generate(const ProjectTemplateRequest& request);
    std::vector<std::string> validateProjectDocument(const nlohmann::json& project) const;
    void resetIssuedProjectIds();

private:
    std::set<std::string> issued_project_ids_;
};

} // namespace urpg::project
