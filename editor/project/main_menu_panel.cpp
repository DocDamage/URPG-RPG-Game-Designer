#include "editor/project/main_menu_panel.h"

#include "editor/assets/asset_library_panel.h"
#include "editor/diagnostics/editor_error_card.h"
#include "editor/project/new_project_wizard_model.h"

#include <algorithm>
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
    const auto startupDestinations = nlohmann::json::array({
        {{"id", "recent_projects"}, {"label", "Recent projects"}, {"available", true},
         {"route", "main_menu"}, {"item_count", recent_projects_.size()}},
        {{"id", "create"}, {"label", "Create from a template"}, {"available", true},
         {"route", onboarding_enabled_ ? "onboarding" : "template_picker"}},
        {{"id", "open"}, {"label", "Open a project"}, {"available", true}, {"route", "open_project"}},
        {{"id", "import"}, {"label", "Import another project"}, {"available", false},
         {"route", nullptr},
         {"reason", "Project import is not installed in this build; asset import remains available after opening a project."}},
        {{"id", "recovery"}, {"label", "Recover a missing project"}, {"available", !missing_projects_.empty()},
         {"route", missing_projects_.empty() ? nlohmann::json(nullptr) : nlohmann::json("main_menu")},
         {"item_count", missing_projects_.size()},
         {"reason", missing_projects_.empty() ? "No missing recent or pinned projects need recovery." : "Choose Locate beside a missing project."}},
        {{"id", "health"}, {"label", "Project health"}, {"available", false}, {"route", nullptr},
         {"reason", "Open a project to run its health and diagnostics workspace."}},
        {{"id", "templates"}, {"label", "Certified templates"}, {"available", true},
         {"route", onboarding_enabled_ ? "onboarding" : "template_picker"}},
        {{"id", "examples"}, {"label", "Example projects"}, {"available", false}, {"route", nullptr},
         {"reason", "No qualified example-project bundle is installed in this build."}},
    });
    return {
        {"surface", "main_menu"},
        {"route", route_},
        {"onboarding_enabled", onboarding_enabled_},
        {"help_tips_enabled", help_tips_enabled_},
        {"asset_browser_layout", asset_browser_layout_},
        {"external_asset_library_root", external_asset_library_root_.generic_string()},
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
             {"new_project", {{"enabled", true}, {"route", onboarding_enabled_ ? "onboarding" : "template_picker"}}},
             {"open_project", {{"enabled", true}, {"route", "editor"}}},
             {"settings", {{"enabled", true}, {"route", "settings"}}},
         }},
        {"recent_projects", projectRows(recent_projects_)},
        {"pinned_projects", projectRows(pinned_projects_)},
        {"missing_projects", missingRows(missing_projects_)},
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
        ImGui::SetNextWindowSize(ImVec2(680.0f, 520.0f), ImGuiCond_FirstUseEver);
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
            if (pending.value("success", true) == false && pending.contains("message")) {
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
