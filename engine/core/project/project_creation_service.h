#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::project {

struct ProjectCreationRequest {
    std::string template_id = "jrpg";
    std::string project_id;
    std::string project_name;
    std::filesystem::path destination;
    std::string display_preset = "1280x720";
    std::string input_preset = "keyboard_gamepad";
    std::string starter_map = "map_intro";
    bool include_creator_vertical_slice_seed = false;
    std::filesystem::path external_asset_library_root;
};

struct ProjectCreationResult {
    bool success = false;
    std::string code;
    std::string message;
    std::filesystem::path project_root;
    nlohmann::json audit_report = nlohmann::json::object();
    std::vector<std::string> errors;
};

class ProjectCreationService {
  public:
    ProjectCreationResult createProject(const ProjectCreationRequest& request) const;
};

} // namespace urpg::project
