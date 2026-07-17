#include "editor/project/main_menu_panel.h"

#include "editor/assets/asset_library_panel.h"
#include "editor/diagnostics/editor_error_card.h"
#include "editor/project/new_project_wizard_model.h"
#include "editor/project/editor_project_session.h"
#include "editor/project/editor_recovery_service.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#endif

namespace urpg::editor {

namespace {

std::string projectPathKey(const std::string& value) {
    if (value.empty()) {
        return {};
    }
    const auto normalized = std::filesystem::path(value).lexically_normal().generic_string();
    std::string key = normalized.empty() ? value : normalized;
    std::transform(key.begin(), key.end(), key.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return key;
}

bool containsProject(const std::vector<std::string>& values, const std::string& value) {
    const auto key = projectPathKey(value);
    return std::any_of(values.begin(), values.end(), [&](const auto& existing) { return projectPathKey(existing) == key; });
}

void eraseValue(std::vector<std::string>& values, const std::string& value) {
    const auto key = projectPathKey(value);
    values.erase(std::remove_if(values.begin(), values.end(), [&](const auto& existing) {
                     return projectPathKey(existing) == key;
                 }),
                 values.end());
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

void MainMenuModel::setUiScale(const float scale) {
    if (std::isfinite(scale)) ui_scale_ = std::clamp(scale, 0.5F, 3.0F);
}

void MainMenuModel::setHighContrast(const bool enabled) {
    high_contrast_ = enabled;
}

void MainMenuModel::setReducedMotion(const bool enabled) {
    reduced_motion_ = enabled;
}

void MainMenuModel::setAssetBrowserLayout(std::string layout) {
    if (layout != "compact_list") {
        layout = "left_collapsible_folder_tree";
    }
    asset_browser_layout_ = std::move(layout);
}

void MainMenuModel::setExternalAssetLibraryRoot(std::filesystem::path root) {
    external_asset_library_root_ = std::move(root);
}

void MainMenuModel::applySettings(const urpg::settings::EditorSettings& settings) {
    onboarding_enabled_ = settings.onboarding_enabled;
    help_tips_enabled_ = settings.help_tips_enabled;
    setUiScale(settings.accessibility.ui_scale);
    high_contrast_ = settings.accessibility.high_contrast;
    reduced_motion_ = settings.accessibility.reduce_motion;
    setAssetBrowserLayout(settings.asset_browser_layout);
    external_asset_library_root_ = settings.external_asset_library_root;
    last_project_ = settings.last_project;
    recent_projects_.clear();
    for (const auto& project : settings.recent_projects) {
        if (!project.empty() && !containsProject(recent_projects_, project) && recent_projects_.size() < 10) {
            recent_projects_.push_back(project);
        }
    }
    pinned_projects_.clear();
    for (const auto& project : settings.pinned_projects) {
        if (!project.empty() && !containsProject(pinned_projects_, project)) {
            pinned_projects_.push_back(project);
        }
    }
    hidden_missing_projects_.clear();
    for (const auto& project : settings.hidden_missing_projects) {
        if (!project.empty() && !containsProject(hidden_missing_projects_, project)) {
            hidden_missing_projects_.push_back(project);
        }
    }
    refreshProjectAvailability();
}

void MainMenuModel::writeSettings(urpg::settings::EditorSettings* settings) const {
    if (!settings) {
        return;
    }
    settings->last_project = last_project_;
    settings->recent_projects = recent_projects_;
    settings->pinned_projects = pinned_projects_;
    settings->hidden_missing_projects = hidden_missing_projects_;
    settings->onboarding_enabled = onboarding_enabled_;
    settings->help_tips_enabled = help_tips_enabled_;
    settings->accessibility.ui_scale = ui_scale_;
    settings->accessibility.high_contrast = high_contrast_;
    settings->accessibility.reduce_motion = reduced_motion_;
    settings->asset_browser_layout = asset_browser_layout_;
    settings->external_asset_library_root = external_asset_library_root_;
}

void MainMenuModel::setLastProject(std::string path) {
    last_project_ = std::move(path);
}

void MainMenuModel::addRecentProject(std::string path) {
    std::vector<std::string> updated;
    updated.reserve(10);
    updated.push_back(std::move(path));

    for (const auto& recent_project : recent_projects_) {
        if (containsProject(updated, recent_project)) {
            continue;
        }
        if (updated.size() == 10) {
            break;
        }
        updated.push_back(recent_project);
    }

    recent_projects_ = std::move(updated);
}

void MainMenuModel::pinProject(std::string path) {
    eraseValue(pinned_projects_, path);
    pinned_projects_.insert(pinned_projects_.begin(), std::move(path));
}

void MainMenuModel::unpinProject(const std::string& path) {
    eraseValue(pinned_projects_, path);
}

void MainMenuModel::markProjectMissing(std::string path) {
    if (containsProject(hidden_missing_projects_, path)) {
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

void MainMenuModel::setRecoveryProjects(std::vector<StartupRecoveryProject> projects) {
    std::sort(projects.begin(), projects.end(), [](const auto& left, const auto& right) {
        return projectPathKey(left.project_path) < projectPathKey(right.project_path);
    });
    projects.erase(std::unique(projects.begin(), projects.end(), [](const auto& left, const auto& right) {
                       return projectPathKey(left.project_path) == projectPathKey(right.project_path);
                   }), projects.end());
    recovery_projects_ = std::move(projects);
}

void MainMenuModel::setExampleProjectAvailable(const bool available) {
    example_project_available_ = available;
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

bool MainMenuModel::chooseImportProject() {
    route_ = "import_project";
    pending_action_ = {{"action", "import_project"}, {"route", route_}};
    return true;
}

bool MainMenuModel::chooseExamples() {
    if (!example_project_available_) return false;
    route_ = "examples";
    pending_action_ = {{"action", "clone_example_project"}, {"route", route_}};
    return true;
}

bool MainMenuModel::chooseProjectHealth(std::string path) {
    if (path.empty()) return false;
    route_ = "project_health";
    pending_action_ = {{"action", "inspect_project_health"}, {"projectPath", std::move(path)}, {"route", route_}};
    return true;
}

bool MainMenuModel::chooseRecoveryProject(std::string path) {
    const auto found = std::find_if(recovery_projects_.begin(), recovery_projects_.end(), [&](const auto& project) {
        return projectPathKey(project.project_path) == projectPathKey(path) && project.snapshot_count > 0;
    });
    if (found == recovery_projects_.end()) return false;
    route_ = "recovery";
    pending_action_ = {{"action", "recover_project"}, {"projectPath", std::move(path)}, {"route", route_}};
    return true;
}

bool MainMenuModel::chooseOpenProject(std::string path) {
    route_ = "editor";
    setLastProject(path);
    addRecentProject(path);
    pending_action_ = {{"action", "open_project"}, {"projectPath", std::move(path)}, {"route", route_}};
    return true;
}

void MainMenuModel::reportProjectOpenFailure(std::string path, std::string message) {
    const auto errorCard = editorErrorCardJson({
        "project_open_failed", "The project could not be opened.", path,
        "The editor remains on the startup screen and no project data was changed.",
        "Locate the project folder, repair project.json, or choose another project.",
        {"Locate Project", "startup.locate", true}, {"Retry Open", "startup.open", true},
        {{"project_path", path}, {"reason", message}},
    });
    markProjectMissing(path);
    route_ = "main_menu";
    pending_action_ = {{"action", "open_project"},
                       {"success", false},
                       {"projectPath", std::move(path)},
                       {"message", std::move(message)},
                       {"error_card", errorCard},
                       {"route", route_}};
}

bool MainMenuModel::beginLocateMissingProject(std::string missing_path) {
    if (!containsProject(missing_projects_, missing_path)) {
        pending_action_ = {{"action", "locate_missing_project"}, {"success", false},
                           {"message", "The requested missing project is not available to locate."}, {"route", route_}};
        return false;
    }
    route_ = "locate_project";
    pending_missing_project_ = std::move(missing_path);
    pending_action_ = {{"action", "locate_missing_project"}, {"projectPath", pending_missing_project_}, {"route", route_}};
    return true;
}

bool MainMenuModel::locateMissingProject(const std::string& missing_path, std::string replacement_path) {
    if (replacement_path.empty() ||
        !containsProject(missing_projects_, missing_path)) {
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

void MainMenuModel::enterEditor(std::string project_path, const bool playtest_starter) {
    route_ = "editor";
    setLastProject(project_path);
    addRecentProject(project_path);
    pending_action_ = {{"action", "enter_editor"}, {"projectPath", std::move(project_path)}, {"route", route_}};
    if (playtest_starter) {
        pending_action_["playtest_starter"] = true;
    }
}

void MainMenuModel::refreshProjectAvailability() {
    missing_projects_.clear();
    const auto inspect = [&](const std::string& path) {
        if (path.empty() || containsProject(hidden_missing_projects_, path)) {
            return;
        }
        std::error_code error;
        if (!std::filesystem::is_directory(std::filesystem::path(path), error)) {
            if (!containsProject(missing_projects_, path)) {
                missing_projects_.push_back(path);
            }
        }
    };
    inspect(last_project_);
    for (const auto& project : recent_projects_) {
        inspect(project);
    }
    for (const auto& project : pinned_projects_) {
        inspect(project);
    }
}

nlohmann::json MainMenuModel::snapshot() const {
    auto recoveryRows = nlohmann::json::array();
    std::size_t recoverySnapshotCount = 0;
    for (const auto& project : recovery_projects_) {
        recoverySnapshotCount += project.snapshot_count;
        recoveryRows.push_back({{"projectPath", project.project_path}, {"snapshot_count", project.snapshot_count},
                                {"unclean_session", project.unclean_session}});
    }
    const bool canInspectHealth = !recent_projects_.empty() || !pinned_projects_.empty() || !last_project_.empty();
    const auto startupDestinations = nlohmann::json::array({
        {{"id", "recent_projects"}, {"label", "Recent projects"}, {"available", true},
         {"route", "main_menu"}, {"item_count", recent_projects_.size()}},
        {{"id", "create"}, {"label", "Create from a template"}, {"available", true},
         {"route", onboarding_enabled_ ? "onboarding" : "template_picker"}},
        {{"id", "open"}, {"label", "Open a project"}, {"available", true}, {"route", "open_project"}},
        {{"id", "import"}, {"label", "Import another project"}, {"available", true},
         {"route", "import_project"},
         {"reason", "Clone a validated project through bounded private staging before opening it."}},
        {{"id", "recovery"}, {"label", "Recovery snapshots"}, {"available", recoverySnapshotCount > 0},
         {"route", recoverySnapshotCount > 0 ? nlohmann::json("recovery") : nlohmann::json(nullptr)},
         {"item_count", recoverySnapshotCount},
         {"reason", recoverySnapshotCount == 0 ? "No private recovery snapshots were found for recent projects."
                                                : "Restore a private snapshot to a separate project folder."}},
        {{"id", "health"}, {"label", "Pre-open project health"}, {"available", canInspectHealth},
         {"route", canInspectHealth ? nlohmann::json("project_health") : nlohmann::json(nullptr)},
         {"reason", canInspectHealth ? "Inspect project identity and manifest readiness without opening it."
                                      : "Add or open a recent project before running pre-open health."}},
        {{"id", "templates"}, {"label", "Certified templates"}, {"available", true},
         {"route", onboarding_enabled_ ? "onboarding" : "template_picker"}},
        {{"id", "examples"}, {"label", "Example projects"}, {"available", example_project_available_},
         {"route", example_project_available_ ? nlohmann::json("examples") : nlohmann::json(nullptr)},
         {"reason", example_project_available_ ? "Clone the qualified creator vertical slice into a writable project folder."
                                                : "No qualified example-project bundle was found in this build."}},
    });
    return {
        {"surface", "main_menu"},
        {"route", route_},
        {"onboarding_enabled", onboarding_enabled_},
        {"help_tips_enabled", help_tips_enabled_},
        {"ui_scale", ui_scale_},
        {"high_contrast", high_contrast_},
        {"reduced_motion", reduced_motion_},
        {"asset_browser_layout", asset_browser_layout_},
        {"external_asset_library_root", external_asset_library_root_.generic_string()},
        {"settings",
         {
             {"onboarding_enabled", onboarding_enabled_},
             {"help_tips_enabled", help_tips_enabled_},
             {"asset_browser_layout", asset_browser_layout_},
             {"ui_scale", ui_scale_},
             {"high_contrast", high_contrast_},
             {"reduced_motion", reduced_motion_},
         }},
        {"commands",
         {
             {"continue_last_project",
              {{"enabled", !last_project_.empty()}, {"projectPath", last_project_}, {"route", "editor"}}},
             {"new_project", {{"enabled", true}, {"route", onboarding_enabled_ ? "onboarding" : "template_picker"}}},
             {"open_project", {{"enabled", true}, {"route", "editor"}}},
             {"settings", {{"enabled", true}, {"route", "settings"}}},
         }},
        {"recent_projects", projectRows(recent_projects_)},
        {"pinned_projects", projectRows(pinned_projects_)},
        {"missing_projects", missingRows(missing_projects_)},
        {"recovery_projects", std::move(recoveryRows)},
        {"hidden_missing_projects", hidden_missing_projects_},
        {"pending_missing_project", pending_missing_project_},
        {"startup_destinations", startupDestinations},
        {"pending_action", pending_action_},
    };
}

void MainMenuPanel::bindModel(MainMenuModel* model) {
    model_ = model;
}

void MainMenuPanel::bindWizard(NewProjectWizardModel* wizard) {
    wizard_ = wizard;
    wizard_inputs_initialized_ = false;
}

void MainMenuPanel::bindProjectServices(EditorProjectSession* session,
                                        EditorRecoveryService* recovery_service) {
    project_session_ = session;
    recovery_service_ = recovery_service;
    refreshStartupRecovery();
}

void MainMenuPanel::bindExampleProjectRoot(std::filesystem::path root) {
    example_project_root_ = std::move(root);
    if (model_) model_->setExampleProjectAvailable(!example_project_root_.empty());
}

void MainMenuPanel::refreshStartupRecovery() {
    if (!model_ || !recovery_service_) return;
    const auto snapshot = model_->snapshot();
    std::vector<std::string> paths;
    const auto appendRows = [&](const char* field) {
        for (const auto& row : snapshot.value(field, nlohmann::json::array())) {
            const auto path = row.value("path", "");
            if (!path.empty() && !containsProject(paths, path)) paths.push_back(path);
        }
    };
    appendRows("recent_projects");
    appendRows("pinned_projects");
    const auto last = snapshot["commands"]["continue_last_project"].value("projectPath", "");
    if (!last.empty() && !containsProject(paths, last)) paths.push_back(last);
    std::vector<StartupRecoveryProject> projects;
    for (const auto& path : paths) {
        const auto snapshots = recovery_service_->listSnapshots(path);
        if (!snapshots.empty()) {
            projects.push_back({path, snapshots.size(), recovery_service_->hasUncleanSessionMarker(path)});
        }
    }
    model_->setRecoveryProjects(std::move(projects));
}

void MainMenuPanel::setAccessibleOpenProjectPath(std::string path) {
    open_project_path_ = std::move(path);
}

bool MainMenuPanel::openAccessibleProject() {
    if (!model_ || open_project_path_.empty()) {
        open_project_status_ = "Enter a project folder before opening it.";
        return false;
    }
    const bool accepted = model_->chooseOpenProject(open_project_path_);
    if (!accepted) open_project_status_ = "The entered project could not be opened.";
    return accepted;
}

bool MainMenuPanel::setAccessibleWizardValue(const std::string_view field, std::string value) {
    if (!wizard_) return false;
    if (field == "project_id") {
        wizard_project_id_ = value;
        wizard_->setProjectId(std::move(value));
    } else if (field == "project_name") {
        wizard_project_name_ = value;
        wizard_->setProjectName(std::move(value));
    } else if (field == "destination") {
        wizard_destination_ = value;
        wizard_->setDestination(std::move(value));
    } else if (field == "external_asset_library_root") {
        wizard_external_root_ = value;
        wizard_->setExternalAssetLibraryRoot(value);
        if (model_) model_->setExternalAssetLibraryRoot(std::move(value));
    } else {
        return false;
    }
    wizard_inputs_initialized_ = true;
    return true;
}

bool MainMenuPanel::createAccessibleWizardProject(const bool playtest_starter) {
    if (!wizard_ || !model_) return false;
    const auto result = wizard_->createProject();
    wizard_status_ = result.message;
    if (!result.success) return false;
    model_->enterEditor(result.project_root.generic_string(), playtest_starter);
    return true;
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
    const auto pickerAvailability = AssetLibraryPanel::nativeImportSourcePickerAvailability();
    snapshot_ = {
        {"panel", "main_menu"},
        {"status", "ready"},
        {"model", model_->snapshot()},
        {"wizard", wizard_ ? wizard_->snapshot() : nlohmann::json(nullptr)},
        {"folder_picker", {{"available", pickerAvailability.available},
                            {"path_entry_available", pickerAvailability.path_entry_available},
                            {"code", pickerAvailability.code}, {"message", pickerAvailability.message}}},
    };
#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() != nullptr) {
        const auto display = ImGui::GetIO().DisplaySize;
        const auto scale = model_->uiScale();
        ImGui::SetNextWindowSize(
            ImVec2(std::min(680.0F * scale, std::max(160.0F, display.x - 24.0F * scale)),
                   std::min(520.0F * scale, std::max(120.0F, display.y - 24.0F * scale))),
            ImGuiCond_FirstUseEver);
        if (ImGui::Begin("URPG Maker")) {
            const auto modelSnapshot = snapshot_["model"];
            const auto folderPickerAvailability = pickerAvailability;
            const auto pickFolder = [&](const std::string& sessionId) {
                AssetLibraryPanel::ImportSourcePickerRequest request;
                request.mode = AssetLibraryPanel::ImportSourcePickerMode::Folder;
                request.session_id = sessionId;
                return AssetLibraryPanel::pickNativeImportSource(request);
            };
            if (model_->route() == "settings") {
                ImGui::TextUnformatted("Settings");
                ImGui::Separator();
                if (!settings_inputs_initialized_) {
                    settings_external_library_root_ = modelSnapshot.value("external_asset_library_root", "");
                    settings_inputs_initialized_ = true;
                }
                bool onboarding = modelSnapshot.value("onboarding_enabled", true);
                if (ImGui::Checkbox("Onboarding", &onboarding)) {
                    model_->setOnboardingEnabled(onboarding);
                }
                bool helpTips = modelSnapshot.value("help_tips_enabled", true);
                if (ImGui::Checkbox("Help Tips", &helpTips)) {
                    model_->setHelpTipsEnabled(helpTips);
                }
                float uiScale = modelSnapshot.value("ui_scale", 1.0F);
                if (ImGui::SliderFloat("UI Scale", &uiScale, 0.5F, 3.0F, "%.2fx",
                                       ImGuiSliderFlags_AlwaysClamp)) {
                    model_->setUiScale(uiScale);
                }
                ImGui::TextDisabled("Scale range: 50%% to 300%%. Changes apply to fonts and layout immediately.");
                bool highContrast = modelSnapshot.value("high_contrast", false);
                if (ImGui::Checkbox("High Contrast", &highContrast)) model_->setHighContrast(highContrast);
                bool reducedMotion = modelSnapshot.value("reduced_motion", false);
                if (ImGui::Checkbox("Reduced Motion", &reducedMotion)) model_->setReducedMotion(reducedMotion);
                const bool compact = modelSnapshot.value("asset_browser_layout", "") == "compact_list";
                if (ImGui::RadioButton("Left Browser Drawer", !compact)) {
                    model_->setAssetBrowserLayout("left_collapsible_folder_tree");
                }
                if (ImGui::RadioButton("Compact Browser List", compact)) {
                    model_->setAssetBrowserLayout("compact_list");
                }
                ImGui::InputText("External Asset Library Root", &settings_external_library_root_);
                if (ImGui::Button("Save External Library Root")) {
                    model_->setExternalAssetLibraryRoot(settings_external_library_root_);
                }
                ImGui::TextDisabled("This local user setting is never written into a project or release manifest.");
                if (ImGui::Button("Back", ImVec2(-1.0f, 0.0f))) {
                    settings_inputs_initialized_ = false;
                    model_->returnToMainMenu();
                }
                ImGui::End();
                return;
            }
            if (model_->route() == "project_health") {
                const auto pending = modelSnapshot.value("pending_action", nlohmann::json::object());
                const auto projectPath = pending.value("projectPath", "");
                ImGui::TextUnformatted("Pre-open Project Health");
                ImGui::Separator();
                if (!project_session_) {
                    ImGui::TextWrapped("Project health is unavailable because no project-session validator is bound.");
                } else {
                    const auto inspection = project_session_->inspectProject(projectPath);
                    ImGui::TextWrapped("Project: %s", projectPath.c_str());
                    ImGui::Text("Status: %s", inspection.result.success ? "ready" : "blocked");
                    ImGui::TextWrapped("%s", inspection.result.message.c_str());
                    if (inspection.result.success) {
                        ImGui::BulletText("ID: %s", inspection.identity.project_id.c_str());
                        ImGui::BulletText("Name: %s", inspection.identity.display_name.c_str());
                        ImGui::BulletText("Schema: %s", inspection.identity.schema_version.c_str());
                        if (ImGui::Button("Open Validated Project", ImVec2(-1.0F, 0.0F))) {
                            (void)model_->chooseOpenProject(projectPath);
                        }
                    }
                }
                if (ImGui::Button("Back", ImVec2(-1.0F, 0.0F))) model_->returnToMainMenu();
                ImGui::End();
                return;
            }
            if (model_->route() == "recovery") {
                const auto pending = modelSnapshot.value("pending_action", nlohmann::json::object());
                const auto projectPath = std::filesystem::path(pending.value("projectPath", ""));
                ImGui::TextUnformatted("Recover Project");
                ImGui::Separator();
                ImGui::TextWrapped("Saved project: %s", projectPath.generic_string().c_str());
                if (!recovery_service_) {
                    ImGui::TextWrapped("Recovery is unavailable because no recovery service is bound.");
                } else {
                    const auto snapshots = recovery_service_->listSnapshots(projectPath);
                    ImGui::Text("Validated private snapshots: %zu", snapshots.size());
                    if (!snapshots.empty()) {
                        ImGui::BulletText("Latest ID: %s", snapshots.front().snapshot_id.c_str());
                        ImGui::BulletText("Dirty documents: %zu", snapshots.front().dirty_document_ids.size());
                        if (ImGui::Button("Restore Latest to a New Folder", ImVec2(-1.0F, 0.0F))) {
                            const auto stamp = std::chrono::duration_cast<std::chrono::seconds>(
                                                   std::chrono::system_clock::now().time_since_epoch()).count();
                            const auto destination = projectPath.parent_path() /
                                (projectPath.filename().string() + "_recovered_" + std::to_string(stamp));
                            if (recovery_service_->restoreRecoverySnapshot(snapshots.front().path, destination)) {
                                restored_recovery_path_ = destination;
                                recovery_route_status_ = "Restored the snapshot to a separate project folder.";
                            } else {
                                recovery_route_status_ =
                                    "Recovery failed; the saved project and snapshot were unchanged.";
                            }
                        }
                    }
                }
                if (!recovery_route_status_.empty()) ImGui::TextWrapped("%s", recovery_route_status_.c_str());
                if (!restored_recovery_path_.empty() &&
                    ImGui::Button("Open Recovered Project", ImVec2(-1.0F, 0.0F))) {
                    (void)model_->chooseOpenProject(restored_recovery_path_.generic_string());
                }
                if (ImGui::Button("Back", ImVec2(-1.0F, 0.0F))) model_->returnToMainMenu();
                ImGui::End();
                return;
            }
            if (model_->route() == "import_project" || model_->route() == "examples") {
                const bool exampleRoute = model_->route() == "examples";
                if (exampleRoute && !import_job_ && import_source_ != example_project_root_.generic_string()) {
                    import_source_ = example_project_root_.generic_string();
                    import_project_id_ = "lantern_of_the_willow_copy";
                    import_project_name_ = "Lantern of the Willow (Copy)";
                }
                if (import_job_ && !import_job_->snapshot().complete) (void)import_job_->advance(64);
                ImGui::TextUnformatted(exampleRoute ? "Example Projects" : "Import Project");
                ImGui::Separator();
                if (exampleRoute) {
                    ImGui::TextWrapped("Qualified example: Lantern of the Willow creator vertical slice");
                    ImGui::TextDisabled("The bundled example is never edited in place; cloning publishes a new project only after validation.");
                } else {
                    ImGui::InputText("Source project folder", &import_source_);
                    if (!folderPickerAvailability.available) ImGui::BeginDisabled();
                    if (ImGui::Button("Browse for Source", ImVec2(-1.0F, 0.0F))) {
                        if (const auto selected = pickFolder("import_project_source")) {
                            import_source_ = selected->generic_string();
                        }
                    }
                    if (!folderPickerAvailability.available) ImGui::EndDisabled();
                }
                ImGui::InputText("New project ID", &import_project_id_);
                ImGui::InputText("New project name", &import_project_name_);
                ImGui::InputText("Destination project folder", &import_destination_);
                if (!folderPickerAvailability.available) ImGui::BeginDisabled();
                if (ImGui::Button("Choose Destination Parent", ImVec2(-1.0F, 0.0F))) {
                    if (const auto selected = pickFolder("import_project_destination")) {
                        import_destination_ = (*selected / import_project_id_).generic_string();
                    }
                }
                if (!folderPickerAvailability.available) ImGui::EndDisabled();

                if (!import_job_) {
                    const bool ready = !import_source_.empty() && !import_destination_.empty() &&
                                       !import_project_id_.empty() && !import_project_name_.empty();
                    if (!ready) ImGui::BeginDisabled();
                    if (ImGui::Button(exampleRoute ? "Clone Example" : "Start Import", ImVec2(-1.0F, 0.0F))) {
                        import_job_ = std::make_unique<ProjectImportJob>(ProjectImportRequest{
                            import_source_, import_destination_, import_project_id_, import_project_name_});
                        import_status_ = "Import queued; discovery and copying run in bounded frame slices.";
                    }
                    if (!ready) ImGui::EndDisabled();
                } else {
                    const auto importSnapshot = import_job_->snapshot();
                    ImGui::Text("Stage: %s", projectImportJobStateName(importSnapshot.state));
                    ImGui::Text("Discovered: %zu  Copied: %zu", importSnapshot.discovered_items,
                                importSnapshot.copied_items);
                    ImGui::TextWrapped("%s", importSnapshot.message.c_str());
                    if (!importSnapshot.complete && ImGui::Button("Cancel Import", ImVec2(-1.0F, 0.0F))) {
                        import_job_->cancel();
                    }
                    if (importSnapshot.success &&
                        ImGui::Button("Open Imported Project", ImVec2(-1.0F, 0.0F))) {
                        (void)model_->chooseOpenProject(importSnapshot.destination_root.generic_string());
                    }
                    if (importSnapshot.complete && !importSnapshot.success &&
                        ImGui::Button("Reset Import", ImVec2(-1.0F, 0.0F))) {
                        import_job_.reset();
                    }
                }
                if (!import_status_.empty()) ImGui::TextWrapped("%s", import_status_.c_str());
                if (ImGui::Button("Back", ImVec2(-1.0F, 0.0F))) {
                    if (import_job_ && !import_job_->snapshot().complete) import_job_->cancel();
                    import_job_.reset();
                    model_->returnToMainMenu();
                }
                ImGui::End();
                return;
            }
            if (model_->route() == "open_project") {
                ImGui::TextUnformatted("Open Project");
                ImGui::TextDisabled("Enter a project folder containing project.json.");
                ImGui::InputText("Project folder", &open_project_path_);
                if (!folderPickerAvailability.available) ImGui::BeginDisabled();
                if (ImGui::Button("Browse for Project", ImVec2(-1.0f, 0.0f))) {
                    if (const auto selected = pickFolder("open_project")) open_project_path_ = selected->generic_string();
                }
                if (!folderPickerAvailability.available) ImGui::EndDisabled();
                if (!folderPickerAvailability.available) ImGui::TextWrapped("%s", folderPickerAvailability.message.c_str());
                if (ImGui::Button("Open Entered Project", ImVec2(-1.0f, 0.0f))) {
                    if (open_project_path_.empty()) {
                        open_project_status_ = "Enter a project folder before opening it.";
                    } else {
                        (void)model_->chooseOpenProject(open_project_path_);
                    }
                }
                if (!open_project_status_.empty()) ImGui::TextWrapped("%s", open_project_status_.c_str());
                if (ImGui::Button("Back", ImVec2(-1.0f, 0.0f))) model_->returnToMainMenu();
                ImGui::End();
                return;
            }
            if (model_->route() == "locate_project") {
                const auto pending = modelSnapshot.value("pending_action", nlohmann::json::object());
                const auto missingPath = pending.value("projectPath", "");
                ImGui::TextUnformatted("Locate Missing Project");
                ImGui::TextWrapped("Previously opened project: %s", missingPath.c_str());
                ImGui::InputText("Replacement project folder", &locate_replacement_path_);
                if (!folderPickerAvailability.available) ImGui::BeginDisabled();
                if (ImGui::Button("Browse for Replacement", ImVec2(-1.0f, 0.0f))) {
                    if (const auto selected = pickFolder("locate_project"))
                        locate_replacement_path_ = selected->generic_string();
                }
                if (!folderPickerAvailability.available) ImGui::EndDisabled();
                if (ImGui::Button("Use Replacement", ImVec2(-1.0f, 0.0f))) {
                    if (locate_replacement_path_.empty()) {
                        locate_status_ = "Enter a replacement project folder before locating.";
                    } else if (model_->locateMissingProject(missingPath, locate_replacement_path_)) {
                        locate_status_ = "Project location updated.";
                    } else {
                        locate_status_ = "Replacement could not be applied. Keep or hide the missing project.";
                    }
                }
                if (!locate_status_.empty()) ImGui::TextWrapped("%s", locate_status_.c_str());
                if (ImGui::Button("Back", ImVec2(-1.0f, 0.0f))) model_->returnToMainMenu();
                ImGui::End();
                return;
            }
            if (model_->route() == "onboarding" || model_->route() == "template_picker") {
                ImGui::TextUnformatted("New Project");
                ImGui::TextDisabled("Guided creator setup");
                ImGui::Separator();
                if (!wizard_) {
                    ImGui::TextWrapped("The project wizard is unavailable. Restart the editor or open Settings for remediation.");
                    if (ImGui::Button("Back", ImVec2(-1.0f, 0.0f))) model_->returnToMainMenu();
                    ImGui::End();
                    return;
                }
                const auto wizardSnapshot = wizard_->snapshot();
                if (!wizard_inputs_initialized_) {
                    wizard_project_id_ = wizardSnapshot.value("project_id", "new_project");
                    wizard_project_name_ = wizardSnapshot.value("project_name", "New Project");
                    wizard_destination_ = wizardSnapshot.value("destination", "");
                    wizard_external_root_ = wizardSnapshot.value("external_asset_library_root", "");
                    wizard_display_preset_ = wizardSnapshot.value("display_preset", "1280x720") == "1920x1080" ? 1 : 0;
                    wizard_input_preset_ = wizardSnapshot.value("input_preset", "keyboard_gamepad") == "keyboard" ? 1 : 0;
                    wizard_inputs_initialized_ = true;
                }
                const auto step = wizardSnapshot.value("step", "project");
                ImGui::Text("Step: %s", step.c_str());
                if (step == "project") {
                    if (ImGui::InputText("Project ID", &wizard_project_id_)) wizard_->setProjectId(wizard_project_id_);
                    if (ImGui::InputText("Project Name", &wizard_project_name_)) wizard_->setProjectName(wizard_project_name_);
                    if (ImGui::InputText("Destination", &wizard_destination_)) wizard_->setDestination(wizard_destination_);
                    if (!folderPickerAvailability.available) ImGui::BeginDisabled();
                    if (ImGui::Button("Choose Destination Folder", ImVec2(-1.0f, 0.0f))) {
                        if (const auto selected = pickFolder("new_project_destination")) {
                            wizard_destination_ = (*selected / wizard_project_id_).generic_string();
                            wizard_->setDestination(wizard_destination_);
                        }
                    }
                    if (!folderPickerAvailability.available) ImGui::EndDisabled();
                    const char* displayPresets[] = {"1280 x 720 (recommended)", "1920 x 1080"};
                    if (ImGui::Combo("Display", &wizard_display_preset_, displayPresets, IM_ARRAYSIZE(displayPresets))) {
                        wizard_->setDisplayPreset(wizard_display_preset_ == 1 ? "1920x1080" : "1280x720");
                    }
                    const char* inputPresets[] = {"Keyboard + gamepad", "Keyboard"};
                    if (ImGui::Combo("Input", &wizard_input_preset_, inputPresets, IM_ARRAYSIZE(inputPresets))) {
                        wizard_->setInputPreset(wizard_input_preset_ == 1 ? "keyboard" : "keyboard_gamepad");
                    }
                } else if (step == "template") {
                    ImGui::TextUnformatted("Certified presets");
                    if (ImGui::Button("Classic JRPG")) wizard_->setTemplateId("jrpg");
                    ImGui::SameLine();
                    if (ImGui::Button("Action RPG")) wizard_->setTemplateId("arpg");
                    ImGui::SameLine();
                    if (ImGui::Button("Tactics")) wizard_->setTemplateId("tactics_rpg");
                    if (ImGui::SameLine(), ImGui::Button("Visual Novel")) wizard_->setTemplateId("visual_novel");
                    ImGui::TextDisabled("Blank/native is deferred until a certified profile is available.");
                    const auto selectedTemplate = wizard_->snapshot().value("template_id", "jrpg");
                    if (selectedTemplate == "arpg") {
                        ImGui::TextWrapped("Action RPG creates the certified starter map and runtime controls; advanced action-combat authoring remains a later creator milestone.");
                    } else if (selectedTemplate == "tactics_rpg") {
                        ImGui::TextWrapped("Tactics creates the certified starter map and grid-first foundation; scenario tooling remains deliberately scoped.");
                    } else if (selectedTemplate == "visual_novel") {
                        ImGui::TextWrapped("Visual Novel creates the certified starter map and dialogue-ready project data; presentation customization is available after creation.");
                    } else {
                        ImGui::TextWrapped("Classic JRPG creates a controllable starter map, player spawn, input profile, save profile, and project manifest.");
                    }
                    if (selectedTemplate == "jrpg") {
                        bool verticalSliceSeed = wizard_->snapshot().value("creator_vertical_slice_seed", false);
                        if (ImGui::Checkbox("Seed Lantern of the Willow draft", &verticalSliceSeed)) {
                            wizard_->setCreatorVerticalSliceSeed(verticalSliceSeed);
                        }
                        ImGui::TextDisabled("Creates a draft two-map scenario through the wizard; it is not M9 completion evidence.");
                    }
                } else if (step == "visual_style") {
                    if (ImGui::Button("Classic")) wizard_->setVisualStyle("classic");
                    ImGui::SameLine();
                    if (ImGui::Button("Pixel")) wizard_->setVisualStyle("pixel");
                } else if (step == "asset_library") {
                    if (ImGui::InputText("External Library (optional)", &wizard_external_root_)) {
                        wizard_->setExternalAssetLibraryRoot(wizard_external_root_);
                        model_->setExternalAssetLibraryRoot(wizard_external_root_);
                    }
                    if (ImGui::Button("Skip / Configure Later")) {
                        wizard_external_root_.clear();
                        wizard_->setExternalAssetLibraryRoot({});
                        model_->setExternalAssetLibraryRoot({});
                    }
                    const auto library = wizard_->snapshot().value("external_asset_library", nlohmann::json::object());
                    if (!library.empty()) {
                        ImGui::TextWrapped("%s", library.value("message", "").c_str());
                    }
                } else if (step == "starter_map") {
                    ImGui::TextUnformatted("The selected template creates a starter map with a player spawn.");
                    const bool verticalSliceSeed = wizard_->snapshot().value("creator_vertical_slice_seed", false);
                    if (verticalSliceSeed) {
                        ImGui::TextUnformatted("Starter Map: Willow Village (required by the Lantern of the Willow draft)");
                    } else if (ImGui::Button("Starter Map: Intro")) {
                        wizard_->setStarterMap("map_intro");
                    }
                    ImGui::TextDisabled("Additional starter-map variants are deferred until they have a certified playable route.");
                } else if (step == "review") {
                    const auto review = wizard_->snapshot();
                    ImGui::TextWrapped("Review the project details, then continue to create a runtime-valid starter project.");
                    ImGui::BulletText("Template: %s", review.value("template_id", "jrpg").c_str());
                    ImGui::BulletText("Display: %s", review.value("display_preset", "1280x720").c_str());
                    ImGui::BulletText("Input: %s", review.value("input_preset", "keyboard_gamepad").c_str());
                    ImGui::BulletText("Starter map: %s", review.value("starter_map", "map_intro").c_str());
                    if (review.value("creator_vertical_slice_seed", false)) {
                        ImGui::BulletText("Scenario seed: Lantern of the Willow (draft)");
                    }
                } else if (step == "create") {
                    const bool enabled = !wizard_destination_.empty() && !wizard_project_id_.empty();
                    if (!enabled) ImGui::BeginDisabled();
                    const auto createProject = [&](const bool playtestStarter) {
                        const auto result = wizard_->createProject();
                        wizard_status_ = result.message;
                        if (result.success) model_->enterEditor(result.project_root.generic_string(), playtestStarter);
                    };
                    if (ImGui::Button("Create Project", ImVec2(-1.0f, 0.0f))) {
                        createProject(false);
                    }
                    if (ImGui::Button("Create Project and Playtest", ImVec2(-1.0f, 0.0f))) createProject(true);
                    if (!enabled) ImGui::EndDisabled();
                    if (!enabled) ImGui::TextDisabled("Enter a project ID and a destination before creating.");
                }
                if (!wizard_status_.empty()) ImGui::TextWrapped("%s", wizard_status_.c_str());
                if (ImGui::Button("Back")) (void)wizard_->previousStep();
                ImGui::SameLine();
                if (ImGui::Button("Next")) (void)wizard_->nextStep();
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    wizard_->cancel();
                    model_->returnToMainMenu();
                }
                ImGui::End();
                return;
            }
            ImGui::TextUnformatted("URPG Maker");
            ImGui::Separator();
            const auto pending = modelSnapshot.value("pending_action", nlohmann::json::object());
            if (pending.value("success", true) == false && pending.contains("error_card")) {
                if (const auto card = editorErrorCardFromJson(pending["error_card"]); card.has_value()) {
                    const auto action = renderEditorErrorCard(*card);
                    if (action.go_to_requested) (void)model_->beginLocateMissingProject(card->affected_object);
                    if (action.retry_requested) (void)model_->chooseOpenProject(card->affected_object);
                }
            } else if (pending.value("success", true) == false && pending.contains("message")) {
                ImGui::TextWrapped("%s", pending.value("message", "Unable to open the requested project.").c_str());
            }
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
            if (ImGui::Button("Import Project", ImVec2(-1.0f, 0.0f))) {
                (void)model_->chooseImportProject();
            }
            const bool examplesAvailable = modelSnapshot["startup_destinations"][7].value("available", false);
            if (!examplesAvailable) ImGui::BeginDisabled();
            if (ImGui::Button("Clone Example Project", ImVec2(-1.0f, 0.0f)) && examplesAvailable) {
                (void)model_->chooseExamples();
            }
            if (!examplesAvailable) ImGui::EndDisabled();
            if (ImGui::Button("Settings", ImVec2(-1.0f, 0.0f))) {
                model_->chooseSettings();
            }
            ImGui::SeparatorText("Startup capabilities");
            for (const auto& destination : modelSnapshot.value("startup_destinations", nlohmann::json::array())) {
                const bool available = destination.value("available", false);
                ImGui::BulletText("%s: %s", destination.value("label", "Unknown").c_str(),
                                  available ? "available" : "unavailable");
                if (!available || destination.contains("reason")) {
                    ImGui::Indent();
                    ImGui::TextDisabled("%s", destination.value("reason", "Ready from this startup surface.").c_str());
                    ImGui::Unindent();
                }
            }
            ImGui::SeparatorText("Recent Projects");
            if (ImGui::SmallButton("Refresh Recovery Snapshots")) refreshStartupRecovery();
            const auto recoveryRows = modelSnapshot.value("recovery_projects", nlohmann::json::array());
            if (ImGui::BeginTable("RecentProjectRoutes", 3, ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("Project", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Health", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Recovery", ImGuiTableColumnFlags_WidthFixed);
                for (const auto& row : modelSnapshot.value("recent_projects", nlohmann::json::array())) {
                    const auto path = row.value("path", "");
                    const auto hasRecovery = std::any_of(
                        recoveryRows.begin(), recoveryRows.end(),
                        [&](const auto& recovery) {
                            return projectPathKey(recovery.value("projectPath", "")) == projectPathKey(path) &&
                                   recovery.value("snapshot_count", std::size_t{0}) > 0;
                        });
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable((path + "##open").c_str())) (void)model_->chooseOpenProject(path);
                    ImGui::TableNextColumn();
                    if (ImGui::SmallButton(("Inspect##" + path).c_str())) (void)model_->chooseProjectHealth(path);
                    ImGui::TableNextColumn();
                    if (!hasRecovery) ImGui::BeginDisabled();
                    if (ImGui::SmallButton(("Restore##" + path).c_str()) && hasRecovery) {
                        (void)model_->chooseRecoveryProject(path);
                    }
                    if (!hasRecovery) ImGui::EndDisabled();
                }
                ImGui::EndTable();
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
                    (void)model_->beginLocateMissingProject(row.value("path", ""));
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
