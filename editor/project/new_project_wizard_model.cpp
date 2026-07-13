#include "editor/project/new_project_wizard_model.h"

#include <array>
#include <system_error>
#include <utility>

namespace urpg::editor {

namespace {
constexpr std::array<const char*, 7> kStepIds = {
    "project", "template", "visual_style", "asset_library", "starter_map", "review", "create"};

nlohmann::json externalLibrarySnapshot(const std::filesystem::path& root) {
    if (root.empty()) {
        return {{"configured", false}, {"status", "not_configured"},
                {"message", "No external library is configured. You can configure it later in Settings."}};
    }
    std::error_code error;
    if (!std::filesystem::is_directory(root, error) || error) {
        return {{"configured", true}, {"status", "root_missing"}, {"root", root.generic_string()},
                {"message", "The configured external library root is unavailable. Choose another root or configure it later."}};
    }
    const auto index = root / ".urpg" / "asset-index" / "catalog_meta.json";
    if (std::filesystem::is_regular_file(index, error) && !error) {
        return {{"configured", true}, {"status", "indexed"}, {"root", root.generic_string()},
                {"catalog_meta_path", index.generic_string()},
                {"message", "An existing local asset catalog was detected and will remain user-local."}};
    }
    return {{"configured", true}, {"status", "not_indexed"}, {"root", root.generic_string()},
            {"catalog_meta_path", index.generic_string()},
            {"message", "This library is configured but has not been indexed yet. Create the project, then refresh the local asset index."}};
}
}

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

void NewProjectWizardModel::setDestination(std::filesystem::path destination) {
    request_.destination = std::move(destination);
    cancelled_ = false;
}

void NewProjectWizardModel::setDisplayPreset(std::string display_preset) {
    request_.display_preset = std::move(display_preset);
    cancelled_ = false;
}

void NewProjectWizardModel::setInputPreset(std::string input_preset) {
    request_.input_preset = std::move(input_preset);
    cancelled_ = false;
}

void NewProjectWizardModel::setVisualStyle(std::string visual_style) {
    visual_style_ = std::move(visual_style);
}

void NewProjectWizardModel::setExternalAssetLibraryRoot(std::filesystem::path root) {
    request_.external_asset_library_root = std::move(root);
}

void NewProjectWizardModel::setStarterMap(std::string starter_map) {
    request_.starter_map = std::move(starter_map);
}

bool NewProjectWizardModel::nextStep() {
    if (step_ == NewProjectWizardStep::Create) return false;
    step_ = static_cast<NewProjectWizardStep>(static_cast<int>(step_) + 1);
    return true;
}

bool NewProjectWizardModel::previousStep() {
    if (step_ == NewProjectWizardStep::Project) return false;
    step_ = static_cast<NewProjectWizardStep>(static_cast<int>(step_) - 1);
    return true;
}

void NewProjectWizardModel::cancel() {
    cancelled_ = true;
}

urpg::project::ProjectCreationResult NewProjectWizardModel::createProject() {
    if (cancelled_) {
        urpg::project::ProjectCreationResult result;
        result.code = "wizard_cancelled";
        result.message = "Project creation was cancelled.";
        result.errors.push_back(result.code);
        return result;
    }
    auto result = creation_service_.createProject(request_);
    last_audit_ = result.audit_report;
    return result;
}

nlohmann::json NewProjectWizardModel::snapshot() const {
    return {
        {"template_id", request_.template_id},
        {"project_id", request_.project_id},
        {"project_name", request_.project_name},
        {"destination", request_.destination.generic_string()},
        {"display_preset", request_.display_preset},
        {"input_preset", request_.input_preset},
        {"visual_style", visual_style_},
        {"external_asset_library_root", request_.external_asset_library_root.generic_string()},
        {"external_asset_library", externalLibrarySnapshot(request_.external_asset_library_root)},
        {"starter_map", request_.starter_map},
        {"step", kStepIds[static_cast<size_t>(step_)]},
        {"steps", kStepIds},
        {"cancelled", cancelled_},
        {"last_audit", last_audit_.is_null() ? nlohmann::json::object() : last_audit_},
    };
}

} // namespace urpg::editor
