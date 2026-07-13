#pragma once

#include "engine/core/project/project_creation_service.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

enum class NewProjectWizardStep { Project, Template, VisualStyle, AssetLibrary, StarterMap, Review, Create };

class NewProjectWizardModel {
public:
    void setTemplateId(std::string template_id);
    void setProjectId(std::string project_id);
    void setProjectName(std::string project_name);
    void setDestination(std::filesystem::path destination);
    void setDisplayPreset(std::string display_preset);
    void setInputPreset(std::string input_preset);
    void setVisualStyle(std::string visual_style);
    void setExternalAssetLibraryRoot(std::filesystem::path root);
    void setStarterMap(std::string starter_map);
    bool nextStep();
    bool previousStep();
    void cancel();
    urpg::project::ProjectCreationResult createProject();
    nlohmann::json snapshot() const;

private:
    urpg::project::ProjectCreationService creation_service_;
    urpg::project::ProjectCreationRequest request_ = [] {
        urpg::project::ProjectCreationRequest request;
        request.project_id = "new_project";
        request.project_name = "New Project";
        return request;
    }();
    NewProjectWizardStep step_ = NewProjectWizardStep::Project;
    std::string visual_style_ = "classic";
    bool cancelled_ = false;
    nlohmann::json last_audit_;
};

} // namespace urpg::editor
