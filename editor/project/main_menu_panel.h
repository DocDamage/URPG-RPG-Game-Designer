#pragma once

#include "editor/project/project_import_job.h"
#include "engine/core/settings/app_settings_store.h"

#include <cstddef>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <memory>

#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

class NewProjectWizardModel;
class EditorProjectSession;
class EditorRecoveryService;

struct StartupRecoveryProject {
    std::string project_path;
    std::size_t snapshot_count = 0;
    bool unclean_session = false;
};

class MainMenuModel {
public:
    void setOnboardingEnabled(bool enabled);
    void setHelpTipsEnabled(bool enabled);
    void setUiScale(float scale);
    void setHighContrast(bool enabled);
    void setReducedMotion(bool enabled);
    void setAssetBrowserLayout(std::string layout);
    void setExternalAssetLibraryRoot(std::filesystem::path root);
    void applySettings(const urpg::settings::EditorSettings& settings);
    void writeSettings(urpg::settings::EditorSettings* settings) const;
    void setLastProject(std::string path);
    void addRecentProject(std::string path);
    void pinProject(std::string path);
    void unpinProject(const std::string& path);
    void markProjectMissing(std::string path);
    void hideMissingProject(const std::string& path);
    void refreshProjectAvailability();
    void setRecoveryProjects(std::vector<StartupRecoveryProject> projects);
    void setExampleProjectAvailable(bool available);
    bool chooseNewProject();
    void chooseOpenProjectRequest();
    bool chooseImportProject();
    bool chooseExamples();
    bool chooseProjectHealth(std::string path);
    bool chooseRecoveryProject(std::string path);
    bool chooseOpenProject(std::string path);
    void reportProjectOpenFailure(std::string path, std::string message);
    bool beginLocateMissingProject(std::string missing_path);
    bool locateMissingProject(const std::string& missing_path, std::string replacement_path);
    void cancelMissingProjectLocate();
    void chooseSettings();
    void returnToMainMenu();
    void enterEditor(std::string project_path, bool playtest_starter = false);
    const std::string& route() const { return route_; }
    bool onboardingEnabled() const { return onboarding_enabled_; }
    bool helpTipsEnabled() const { return help_tips_enabled_; }
    float uiScale() const { return ui_scale_; }
    bool highContrast() const { return high_contrast_; }
    bool reducedMotion() const { return reduced_motion_; }
    const std::string& assetBrowserLayout() const { return asset_browser_layout_; }
    nlohmann::json snapshot() const;

private:
    std::string route_ = "main_menu";
    bool onboarding_enabled_ = true;
    bool help_tips_enabled_ = true;
    float ui_scale_ = 1.0F;
    bool high_contrast_ = false;
    bool reduced_motion_ = false;
    std::string asset_browser_layout_ = "left_collapsible_folder_tree";
    std::filesystem::path external_asset_library_root_;
    std::string last_project_;
    std::vector<std::string> recent_projects_;
    std::vector<std::string> pinned_projects_;
    std::vector<std::string> missing_projects_;
    std::vector<std::string> hidden_missing_projects_;
    std::vector<StartupRecoveryProject> recovery_projects_;
    bool example_project_available_ = false;
    std::string pending_missing_project_;
    nlohmann::json pending_action_ = nlohmann::json::object();
};

class MainMenuPanel {
public:
    void bindModel(MainMenuModel* model);
    void bindWizard(NewProjectWizardModel* wizard);
    void bindProjectServices(EditorProjectSession* session, EditorRecoveryService* recovery_service);
    void bindExampleProjectRoot(std::filesystem::path root);
    void refreshStartupRecovery();
    void setAccessibleOpenProjectPath(std::string path);
    [[nodiscard]] const std::string& accessibleOpenProjectPath() const { return open_project_path_; }
    bool openAccessibleProject();
    bool setAccessibleWizardValue(std::string_view field, std::string value);
    bool createAccessibleWizardProject(bool playtest_starter);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    MainMenuModel* model_ = nullptr;
    NewProjectWizardModel* wizard_ = nullptr;
    EditorProjectSession* project_session_ = nullptr;
    EditorRecoveryService* recovery_service_ = nullptr;
    bool wizard_inputs_initialized_ = false;
    std::string wizard_project_id_;
    std::string wizard_project_name_;
    std::string wizard_destination_;
    int wizard_display_preset_ = 0;
    int wizard_input_preset_ = 0;
    std::string wizard_external_root_;
    std::string wizard_status_;
    std::string open_project_path_;
    std::string open_project_status_;
    std::string locate_missing_path_;
    std::string locate_replacement_path_;
    std::string locate_status_;
    std::string recovery_route_status_;
    std::filesystem::path restored_recovery_path_;
    std::filesystem::path example_project_root_;
    std::unique_ptr<ProjectImportJob> import_job_;
    std::string import_source_;
    std::string import_destination_;
    std::string import_project_id_ = "imported_project";
    std::string import_project_name_ = "Imported Project";
    std::string import_status_;
    bool settings_inputs_initialized_ = false;
    std::string settings_external_library_root_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
