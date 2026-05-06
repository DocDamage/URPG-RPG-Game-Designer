#include "editor/project/main_menu_panel.h"

#include <algorithm>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {

namespace {

void eraseValue(std::vector<std::string>& values, const std::string& value) {
    values.erase(std::remove(values.begin(), values.end(), value), values.end());
}

nlohmann::json projectRows(const std::vector<std::string>& paths) {
    auto rows = nlohmann::json::array();
    for (const auto& path : paths) {
        rows.push_back({{"path", path}});
    }
    return rows;
}

nlohmann::json missingRows(const std::vector<std::string>& paths) {
    auto rows = nlohmann::json::array();
    for (const auto& path : paths) {
        rows.push_back({{"path", path}, {"action", "prompt_locate_or_hide"}});
    }
    return rows;
}

} // namespace

void MainMenuModel::setOnboardingEnabled(bool enabled) {
    onboarding_enabled_ = enabled;
}

void MainMenuModel::setHelpTipsEnabled(bool enabled) {
    help_tips_enabled_ = enabled;
}

void MainMenuModel::setAssetBrowserLayout(std::string layout) {
    if (layout != "compact_list") {
        layout = "left_collapsible_folder_tree";
    }
    asset_browser_layout_ = std::move(layout);
}

void MainMenuModel::setLastProject(std::string path) {
    last_project_ = std::move(path);
}

void MainMenuModel::addRecentProject(std::string path) {
    eraseValue(recent_projects_, path);
    recent_projects_.insert(recent_projects_.begin(), std::move(path));
    if (recent_projects_.size() > 10) {
        recent_projects_.resize(10);
    }
}

void MainMenuModel::pinProject(std::string path) {
    eraseValue(pinned_projects_, path);
    pinned_projects_.insert(pinned_projects_.begin(), std::move(path));
}

void MainMenuModel::unpinProject(const std::string& path) {
    eraseValue(pinned_projects_, path);
}

void MainMenuModel::markProjectMissing(std::string path) {
    if (std::find(hidden_missing_projects_.begin(), hidden_missing_projects_.end(), path) !=
        hidden_missing_projects_.end()) {
        return;
    }
    eraseValue(missing_projects_, path);
    missing_projects_.push_back(std::move(path));
}

void MainMenuModel::hideMissingProject(const std::string& path) {
    eraseValue(missing_projects_, path);
    eraseValue(hidden_missing_projects_, path);
    hidden_missing_projects_.push_back(path);
}

bool MainMenuModel::chooseNewProject() {
    route_ = onboarding_enabled_ ? "onboarding" : "template_picker";
    pending_action_ = {{"action", "new_project"}, {"route", route_}};
    return true;
}

void MainMenuModel::chooseOpenProjectRequest() {
    route_ = "open_project";
    pending_action_ = {{"action", "open_project_request"}, {"route", route_}};
}

bool MainMenuModel::chooseOpenProject(std::string path) {
    route_ = "editor";
    addRecentProject(path);
    pending_action_ = {{"action", "open_project"}, {"projectPath", std::move(path)}, {"route", route_}};
    return true;
}

bool MainMenuModel::locateMissingProject(const std::string& missing_path, std::string replacement_path) {
    if (replacement_path.empty() ||
        std::find(missing_projects_.begin(), missing_projects_.end(), missing_path) == missing_projects_.end()) {
        pending_action_ = {{"action", "locate_missing_project"}, {"success", false}, {"projectPath", missing_path}};
        return false;
    }
    eraseValue(missing_projects_, missing_path);
    eraseValue(hidden_missing_projects_, missing_path);
    pending_missing_project_.clear();
    addRecentProject(replacement_path);
    route_ = "main_menu";
    pending_action_ = {
        {"action", "locate_missing_project"},
        {"success", true},
        {"projectPath", missing_path},
        {"replacementPath", std::move(replacement_path)},
        {"route", route_},
    };
    return true;
}

void MainMenuModel::cancelMissingProjectLocate() {
    pending_missing_project_.clear();
    route_ = "main_menu";
    pending_action_ = {{"action", "cancel_locate_missing_project"}, {"route", route_}};
}

void MainMenuModel::chooseSettings() {
    route_ = "settings";
    pending_action_ = {{"action", "settings"}, {"route", route_}};
}

void MainMenuModel::returnToMainMenu() {
    route_ = "main_menu";
    pending_action_ = {{"action", "main_menu"}, {"route", route_}};
}

void MainMenuModel::enterEditor(std::string project_path) {
    route_ = "editor";
    addRecentProject(project_path);
    pending_action_ = {{"action", "enter_editor"}, {"projectPath", std::move(project_path)}, {"route", route_}};
}

nlohmann::json MainMenuModel::snapshot() const {
    return {
        {"surface", "main_menu"},
        {"route", route_},
        {"onboarding_enabled", onboarding_enabled_},
        {"help_tips_enabled", help_tips_enabled_},
        {"asset_browser_layout", asset_browser_layout_},
        {"settings",
         {
             {"onboarding_enabled", onboarding_enabled_},
             {"help_tips_enabled", help_tips_enabled_},
             {"asset_browser_layout", asset_browser_layout_},
         }},
        {"commands",
         {
             {"continue_last_project",
              {{"enabled", !last_project_.empty()}, {"projectPath", last_project_}, {"route", "editor"}}},
             {"new_project",
              {{"enabled", true}, {"route", onboarding_enabled_ ? "onboarding" : "template_picker"}}},
             {"open_project", {{"enabled", true}, {"route", "editor"}}},
             {"settings", {{"enabled", true}, {"route", "settings"}}},
         }},
        {"recent_projects", projectRows(recent_projects_)},
        {"pinned_projects", projectRows(pinned_projects_)},
        {"missing_projects", missingRows(missing_projects_)},
        {"hidden_missing_projects", hidden_missing_projects_},
        {"pending_missing_project", pending_missing_project_},
        {"pending_action", pending_action_},
    };
}

void MainMenuPanel::bindModel(MainMenuModel* model) {
    model_ = model;
}

void MainMenuPanel::render() {
    if (!model_) {
        snapshot_ = {
            {"panel", "main_menu"},
            {"status", "disabled"},
            {"disabled_reason", "No MainMenuModel is bound."},
            {"owner", "editor/project"},
            {"unlock_condition", "Bind MainMenuModel before rendering the editor startup menu."},
        };
        return;
    }
    snapshot_ = {
        {"panel", "main_menu"},
        {"status", "ready"},
        {"model", model_->snapshot()},
    };
#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui::SetNextWindowSize(ImVec2(680.0f, 520.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("URPG Maker")) {
            const auto modelSnapshot = snapshot_["model"];
            if (model_->route() == "settings") {
                ImGui::TextUnformatted("Settings");
                ImGui::Separator();
                bool onboarding = modelSnapshot.value("onboarding_enabled", true);
                if (ImGui::Checkbox("Onboarding", &onboarding)) {
                    model_->setOnboardingEnabled(onboarding);
                }
                bool helpTips = modelSnapshot.value("help_tips_enabled", true);
                if (ImGui::Checkbox("Help Tips", &helpTips)) {
                    model_->setHelpTipsEnabled(helpTips);
                }
                const bool compact = modelSnapshot.value("asset_browser_layout", "") == "compact_list";
                if (ImGui::RadioButton("Left Browser Drawer", !compact)) {
                    model_->setAssetBrowserLayout("left_collapsible_folder_tree");
                }
                if (ImGui::RadioButton("Compact Browser List", compact)) {
                    model_->setAssetBrowserLayout("compact_list");
                }
                if (ImGui::Button("Back", ImVec2(-1.0f, 0.0f))) {
                    model_->returnToMainMenu();
                }
                ImGui::End();
                return;
            }
            ImGui::TextUnformatted("URPG Maker");
            ImGui::Separator();
            const auto continueCommand = modelSnapshot["commands"]["continue_last_project"];
            if (!continueCommand.value("enabled", false)) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Continue Last Project", ImVec2(-1.0f, 0.0f))) {
                (void)model_->chooseOpenProject(continueCommand.value("projectPath", ""));
            }
            if (!continueCommand.value("enabled", false)) {
                ImGui::EndDisabled();
            }
            if (ImGui::Button("New Project", ImVec2(-1.0f, 0.0f))) {
                (void)model_->chooseNewProject();
            }
            if (ImGui::Button("Open Project", ImVec2(-1.0f, 0.0f))) {
                model_->chooseOpenProjectRequest();
            }
            if (ImGui::Button("Settings", ImVec2(-1.0f, 0.0f))) {
                model_->chooseSettings();
            }
            ImGui::SeparatorText("Recent Projects");
            for (const auto& row : modelSnapshot.value("recent_projects", nlohmann::json::array())) {
                const auto path = row.value("path", "");
                if (ImGui::Selectable(path.c_str())) {
                    (void)model_->chooseOpenProject(path);
                }
            }
            ImGui::SeparatorText("Pinned");
            for (const auto& row : modelSnapshot.value("pinned_projects", nlohmann::json::array())) {
                ImGui::TextUnformatted(row.value("path", "").c_str());
            }
            ImGui::SeparatorText("Missing Projects");
            for (const auto& row : modelSnapshot.value("missing_projects", nlohmann::json::array())) {
                ImGui::Text("%s", row.value("path", "").c_str());
                ImGui::SameLine();
                if (ImGui::SmallButton(("Locate##" + row.value("path", "")).c_str())) {
                    (void)model_->locateMissingProject(row.value("path", ""), row.value("path", ""));
                }
                ImGui::SameLine();
                if (ImGui::SmallButton(("Hide##" + row.value("path", "")).c_str())) {
                    model_->hideMissingProject(row.value("path", ""));
                }
            }
        }
        ImGui::End();
    }
#endif
}

nlohmann::json MainMenuPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
