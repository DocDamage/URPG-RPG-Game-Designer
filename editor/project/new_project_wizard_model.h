#pragma once

#include "engine/core/project/project_template_generator.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <optional>
#include <string>

namespace urpg::editor {

class NewProjectWizardModel {
public:
    void setTemplateId(std::string template_id);
    void setProjectId(std::string project_id);
    void setProjectName(std::string project_name);
    void setHelpTipsEnabled(bool enabled);
    void cancel();
    bool loadGameMakerTemplateManifests(const std::filesystem::path& manifest_directory,
                                        std::string* error_message = nullptr);
    bool selectGameMakerTemplate(const std::string& template_id);
    std::optional<std::filesystem::path> selectedGameMakerTemplateManifestPath() const;
    urpg::project::ProjectTemplateResult createProject();
    bool createProjectOnDisk(const std::filesystem::path& project_root, std::string* error_message = nullptr);
    nlohmann::json snapshot() const;

private:
    urpg::project::ProjectTemplateGenerator generator_;
    urpg::project::ProjectTemplateRequest request_{"jrpg", "new_project", "New Project"};
    bool cancelled_ = false;
    bool help_tips_enabled_ = true;
    nlohmann::json last_audit_;
    nlohmann::json game_maker_templates_ = nlohmann::json::array();
    nlohmann::json selected_game_maker_template_ = nlohmann::json::object();
};

} // namespace urpg::editor
