#include "engine/core/project/project_template_generator.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace urpg::project {

namespace {

struct TemplateSpec {
    std::string id;
    std::string display_name;
    std::vector<std::string> tags;
    std::vector<std::string> maps;
    std::string battle_mode;
};

const std::array<TemplateSpec, 3> kTemplates = {{
    {"jrpg", "Classic JRPG", {"party", "battle", "exploration"}, {"map_intro", "map_town", "map_field"}, "turn_based"},
    {"visual_novel", "Visual Novel", {"dialogue", "choices", "save"}, {"scene_intro", "scene_choice"}, "none"},
    {"turn_based_rpg", "Turn-Based RPG", {"tactics", "battle", "progression"}, {"map_intro", "map_arena"}, "turn_based"},
}};

const std::array<std::string, 8> kRequiredSubsystems = {
    "maps",
    "menu",
    "message",
    "battle",
    "save",
    "localization",
    "input",
    "export_profile",
};

const TemplateSpec* findTemplate(const std::string& id) {
    const auto it = std::find_if(kTemplates.begin(), kTemplates.end(), [&](const TemplateSpec& spec) {
        return spec.id == id;
    });
    return it == kTemplates.end() ? nullptr : &*it;
}

bool isValidProjectId(const std::string& id) {
    if (id.empty()) {
        return false;
    }
    return std::all_of(id.begin(), id.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_' || c == '-';
    });
}

nlohmann::json makeMaps(const TemplateSpec& spec) {
    nlohmann::json maps = nlohmann::json::array();
    for (std::size_t i = 0; i < spec.maps.size(); ++i) {
        maps.push_back({
            {"id", spec.maps[i]},
            {"display_name", i == 0 ? "Opening" : "Onboarding " + std::to_string(i)},
            {"tileset", "starter_world"},
            {"spawn", {{"x", 4}, {"y", 6}}},
        });
    }
    return maps;
}

nlohmann::json makeSubsystems(const TemplateSpec& spec) {
    return {
        {"maps", makeMaps(spec)},
        {"menu", {{"style", spec.id == "visual_novel" ? "minimal" : "classic"}, {"commands", {"items", "skills", "save"}}}},
        {"message", {{"default_speaker", "Guide"}, {"sample_line", "Welcome to your first playable slice."}}},
        {"battle", {{"mode", spec.battle_mode}, {"encounter_id", spec.battle_mode == "none" ? "" : "battle_training_01"}}},
        {"save", {{"slot_count", 3}, {"autosave", true}}},
        {"localization", {{"default_locale", "en-US"}, {"locales", {"en-US"}}}},
        {"input", {{"profile", "keyboard_gamepad"}, {"confirm", "Enter"}, {"cancel", "Escape"}}},
        {"export_profile", {{"target", "Windows_x64"}, {"integrity", "strict"}, {"output_name", spec.id + "_starter"}}},
    };
}

std::vector<std::string> missingSubsystems(const nlohmann::json& project) {
    std::vector<std::string> missing;
    if (!project.contains("subsystems") || !project["subsystems"].is_object()) {
        return std::vector<std::string>(kRequiredSubsystems.begin(), kRequiredSubsystems.end());
    }
    for (const auto& subsystem : kRequiredSubsystems) {
        if (!project["subsystems"].contains(subsystem)) {
            missing.push_back(subsystem);
        }
    }
    return missing;
}

} // namespace

ProjectTemplateResult ProjectTemplateGenerator::generate(const ProjectTemplateRequest& request) {
    ProjectTemplateResult result;
    const TemplateSpec* spec = findTemplate(request.template_id);
    if (!spec) {
        result.errors.push_back("unknown_template:" + request.template_id);
        return result;
    }
    if (!isValidProjectId(request.project_id)) {
        result.errors.push_back("invalid_project_id");
        return result;
    }
    if (request.project_name.empty()) {
        result.errors.push_back("missing_project_name");
        return result;
    }
    if (issued_project_ids_.contains(request.project_id)) {
        result.errors.push_back("duplicate_project_id:" + request.project_id);
        return result;
    }

    result.project = {
        {"schema_version", "urpg.project.v1"},
        {"project_id", request.project_id},
        {"project_name", request.project_name},
        {"template_id", spec->id},
        {"template_display_name", spec->display_name},
        {"tags", spec->tags},
        {"subsystems", makeSubsystems(*spec)},
    };
    result.audit_report = {
        {"project_id", request.project_id},
        {"template_id", spec->id},
        {"status", "passed"},
        {"checked_subsystems", kRequiredSubsystems},
        {"issues", nlohmann::json::array()},
    };
    issued_project_ids_.insert(request.project_id);
    result.success = true;
    return result;
}

std::vector<std::string> ProjectTemplateGenerator::validateProjectDocument(const nlohmann::json& project) const {
    std::vector<std::string> errors;
    if (!project.is_object()) {
        return {"project_document_not_object"};
    }
    if (project.value("schema_version", "") != "urpg.project.v1") {
        errors.push_back("invalid_project_schema_version");
    }
    if (!isValidProjectId(project.value("project_id", ""))) {
        errors.push_back("invalid_project_id");
    }
    if (!findTemplate(project.value("template_id", ""))) {
        errors.push_back("unknown_template:" + project.value("template_id", ""));
    }
    for (const auto& subsystem : missingSubsystems(project)) {
        errors.push_back("missing_subsystem:" + subsystem);
    }
    return errors;
}

void ProjectTemplateGenerator::resetIssuedProjectIds() {
    issued_project_ids_.clear();
}

} // namespace urpg::project
