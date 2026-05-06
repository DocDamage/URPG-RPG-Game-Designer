#include "editor/project/new_project_wizard_model.h"

#include <algorithm>
#include <fstream>
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

void NewProjectWizardModel::setHelpTipsEnabled(bool enabled) {
    help_tips_enabled_ = enabled;
}

void NewProjectWizardModel::cancel() {
    cancelled_ = true;
}

bool NewProjectWizardModel::loadGameMakerTemplateManifests(const std::filesystem::path& manifest_directory,
                                                           std::string* error_message) {
    game_maker_templates_ = nlohmann::json::array();
    selected_game_maker_template_ = nlohmann::json::object();
    if (!std::filesystem::is_directory(manifest_directory)) {
        if (error_message != nullptr) {
            *error_message = "game_maker_template_manifest_directory_missing";
        }
        return false;
    }

    try {
        std::vector<nlohmann::json> manifests;
        for (const auto& entry : std::filesystem::directory_iterator(manifest_directory)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json") {
                continue;
            }
            std::ifstream stream(entry.path(), std::ios::binary);
            auto manifest = nlohmann::json::parse(stream);
            manifest["manifestPath"] = entry.path().generic_string();
            manifests.push_back(std::move(manifest));
        }
        std::sort(manifests.begin(), manifests.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.value("displayName", "") < rhs.value("displayName", "");
        });
        for (auto& manifest : manifests) {
            game_maker_templates_.push_back(std::move(manifest));
        }
        if (!game_maker_templates_.empty()) {
            selected_game_maker_template_ = game_maker_templates_.front();
            setTemplateId(selected_game_maker_template_.value("templateId", request_.template_id));
        }
        if (error_message != nullptr) {
            error_message->clear();
        }
        return !game_maker_templates_.empty();
    } catch (const std::exception&) {
        game_maker_templates_ = nlohmann::json::array();
        if (error_message != nullptr) {
            *error_message = "game_maker_template_manifest_parse_failed";
        }
        return false;
    }
}

bool NewProjectWizardModel::selectGameMakerTemplate(const std::string& template_id) {
    for (const auto& manifest : game_maker_templates_) {
        if (manifest.value("templateId", "") == template_id) {
            selected_game_maker_template_ = manifest;
            setTemplateId(template_id);
            return true;
        }
    }
    return false;
}

namespace {

nlohmann::json buildQuestionFlow(const nlohmann::json& manifest, bool help_tips_enabled) {
    if (!manifest.is_object() || manifest.empty()) {
        return {{"questions", nlohmann::json::array()}, {"question_count", 0}, {"help_tips_enabled", help_tips_enabled}};
    }
    const auto gameType = manifest.value("gameType", "");
    auto questions = nlohmann::json::array();
    questions.push_back({{"id", "project_identity"}, {"prompt", "Name the project."}, {"required", true}});
    questions.push_back({{"id", "world_size"},
                         {"prompt", "Choose the starting world size."},
                         {"default", manifest.value("defaultWorldSize", nlohmann::json::object())},
                         {"allows_override", true}});
    if (gameType == "party_rpg" || gameType == "action_rpg" || gameType == "tactical_rpg" ||
        gameType == "monster_collector" || gameType == "platform_adventure") {
        questions.push_back({{"id", "mechanic_depth"},
                             {"prompt", "Choose mechanic depth."},
                             {"choices", {"light", "standard", "deep"}},
                             {"recommended", "standard"},
                             {"reason", "Matches the selected template mechanics."}});
    }
    if (gameType == "visual_novel_hybrid") {
        questions.push_back({{"id", "story_branching"},
                             {"prompt", "Choose story branching depth."},
                             {"choices", {"linear", "choice_driven", "route_based"}},
                             {"recommended", "choice_driven"}});
    }
    if (gameType == "cozy_life") {
        questions.push_back({{"id", "daily_loop"},
                             {"prompt", "Choose the daily loop focus."},
                             {"choices", {"farming", "crafting", "relationships"}},
                             {"recommended", "relationships"}});
    }
    questions.push_back({{"id", "asset_scope"},
                         {"prompt", "Choose asset scope."},
                         {"choices", {"template_default", "full_library_opt_in"}},
                         {"recommended", "template_default"},
                         {"reason", manifest.value("fullLibraryPolicy", "opt_in_lazy_load")}});
    questions.push_back({{"id", "ui_theme"},
                         {"prompt", "Choose game UI theme."},
                         {"default", manifest.value("uiThemes", nlohmann::json::object())},
                         {"allows_override", true}});
    return {
        {"template_id", manifest.value("templateId", "")},
        {"game_type", gameType},
        {"question_profile", manifest.value("questionProfile", "")},
        {"recommended_mechanics", manifest.value("recommendedMechanics", nlohmann::json::array())},
        {"help_tips_enabled", help_tips_enabled},
        {"questions", questions},
        {"question_count", questions.size()},
    };
}

} // namespace

std::optional<std::filesystem::path> NewProjectWizardModel::selectedGameMakerTemplateManifestPath() const {
    if (!selected_game_maker_template_.is_object() || !selected_game_maker_template_.contains("manifestPath")) {
        return std::nullopt;
    }
    const auto path = selected_game_maker_template_.value("manifestPath", "");
    if (path.empty()) {
        return std::nullopt;
    }
    return std::filesystem::path(path);
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

bool NewProjectWizardModel::createProjectOnDisk(const std::filesystem::path& project_root,
                                                std::string* error_message) {
    const auto result = createProject();
    if (!result.success) {
        if (error_message != nullptr) {
            *error_message = result.errors.empty() ? "project_template_generation_failed" : result.errors.front();
        }
        return false;
    }

    try {
        std::filesystem::create_directories(project_root);
        std::filesystem::create_directories(project_root / "reports" / "onboarding");
        std::filesystem::create_directories(project_root / "content" / "assets" / "manifests");
        std::filesystem::create_directories(project_root / "content" / "maps");

        {
            std::ofstream out(project_root / "project.json", std::ios::binary);
            out << result.project.dump(2) << "\n";
        }
        {
            std::ofstream out(project_root / "reports" / "onboarding" / "project_template_audit.json",
                              std::ios::binary);
            out << result.audit_report.dump(2) << "\n";
        }
        if (selected_game_maker_template_.is_object() && !selected_game_maker_template_.empty()) {
            std::ofstream out(project_root / "content" / "game_template_manifest.json", std::ios::binary);
            out << selected_game_maker_template_.dump(2) << "\n";
        }
        if (error_message != nullptr) {
            error_message->clear();
        }
        return true;
    } catch (const std::exception&) {
        if (error_message != nullptr) {
            *error_message = "project_template_write_failed";
        }
        return false;
    }
}

nlohmann::json NewProjectWizardModel::snapshot() const {
    return {
        {"template_id", request_.template_id},
        {"project_id", request_.project_id},
        {"project_name", request_.project_name},
        {"cancelled", cancelled_},
        {"help_tips_enabled", help_tips_enabled_},
        {"last_audit", last_audit_.is_null() ? nlohmann::json::object() : last_audit_},
        {"game_maker_templates", game_maker_templates_},
        {"selected_game_maker_template", selected_game_maker_template_},
        {"question_flow", buildQuestionFlow(selected_game_maker_template_, help_tips_enabled_)},
    };
}

} // namespace urpg::editor
