#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::editor {

class MainMenuModel {
public:
    void setOnboardingEnabled(bool enabled);
    void setHelpTipsEnabled(bool enabled);
    void setAssetBrowserLayout(std::string layout);
    void setLastProject(std::string path);
    void addRecentProject(std::string path);
    void pinProject(std::string path);
    void unpinProject(const std::string& path);
    void markProjectMissing(std::string path);
    void hideMissingProject(const std::string& path);
    bool chooseNewProject();
    void chooseOpenProjectRequest();
    bool chooseOpenProject(std::string path);
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
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    MainMenuModel* model_ = nullptr;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
