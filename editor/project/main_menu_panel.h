#pragma once

#include "engine/core/settings/app_settings_store.h"

#include <filesystem>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::editor {

class NewProjectWizardModel;

class MainMenuModel {
public:
    void setOnboardingEnabled(bool enabled);
    void setHelpTipsEnabled(bool enabled);
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
    bool chooseNewProject();
    void chooseOpenProjectRequest();
    bool chooseOpenProject(std::string path);
    void reportProjectOpenFailure(std::string path, std::string message);
    bool beginLocateMissingProject(std::string missing_path);
    bool locateMissingProject(const std::string& missing_path, std::string replacement_path);
    void cancelMissingProjectLocate();
    void chooseSettings();
    void returnToMainMenu();
    void enterEditor(std::string project_path);
    const std::string& route() const { return route_; }
    bool onboardingEnabled() const { return onboarding_enabled_; }
    bool helpTipsEnabled() const { return help_tips_enabled_; }
    const std::string& assetBrowserLayout() const { return asset_browser_layout_; }
    nlohmann::json snapshot() const;

private:
    std::string route_ = "main_menu";
    bool onboarding_enabled_ = true;
    bool help_tips_enabled_ = true;
    std::string asset_browser_layout_ = "left_collapsible_folder_tree";
    std::filesystem::path external_asset_library_root_;
    std::string last_project_;
    std::vector<std::string> recent_projects_;
    std::vector<std::string> pinned_projects_;
    std::vector<std::string> missing_projects_;
    std::vector<std::string> hidden_missing_projects_;
    std::string pending_missing_project_;
    nlohmann::json pending_action_ = nlohmann::json::object();
};

class MainMenuPanel {
public:
    void bindModel(MainMenuModel* model);
    void bindWizard(NewProjectWizardModel* wizard);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    MainMenuModel* model_ = nullptr;
    NewProjectWizardModel* wizard_ = nullptr;
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
    bool settings_inputs_initialized_ = false;
    std::string settings_external_library_root_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
