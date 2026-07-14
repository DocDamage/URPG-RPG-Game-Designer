#include "editor/project/main_menu_panel.h"
#include "editor/project/new_project_wizard_model.h"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("MainMenuModel exposes startup routes and project actions", "[project][main_menu]") {
    urpg::editor::MainMenuModel model;
    model.setOnboardingEnabled(true);
    model.setLastProject("C:/projects/last.urpg");
    model.addRecentProject("C:/projects/first.urpg");
    model.addRecentProject("C:/projects/second.urpg");
    model.pinProject("C:/projects/first.urpg");
    model.markProjectMissing("C:/projects/missing.urpg");

    auto snapshot = model.snapshot();
    REQUIRE(snapshot["surface"] == "main_menu");
    REQUIRE(snapshot["route"] == "main_menu");
    REQUIRE(snapshot["onboarding_enabled"] == true);
    REQUIRE(snapshot["commands"]["continue_last_project"]["enabled"] == true);
    REQUIRE(snapshot["commands"]["continue_last_project"]["projectPath"] == "C:/projects/last.urpg");
    REQUIRE(snapshot["recent_projects"].size() == 2);
    REQUIRE(snapshot["recent_projects"][0]["path"] == "C:/projects/second.urpg");
    REQUIRE(snapshot["pinned_projects"].size() == 1);
    REQUIRE(snapshot["missing_projects"].size() == 1);
    REQUIRE(snapshot["missing_projects"][0]["action"] == "prompt_locate_or_hide");

    REQUIRE(model.chooseNewProject());
    snapshot = model.snapshot();
    REQUIRE(snapshot["route"] == "onboarding");
    REQUIRE(snapshot["pending_action"]["action"] == "new_project");

    REQUIRE(model.chooseOpenProject("C:/projects/first.urpg"));
    snapshot = model.snapshot();
    REQUIRE(snapshot["route"] == "editor");
    REQUIRE(snapshot["pending_action"]["action"] == "open_project");
    REQUIRE(snapshot["pending_action"]["projectPath"] == "C:/projects/first.urpg");

    model.chooseOpenProjectRequest();
    snapshot = model.snapshot();
    REQUIRE(snapshot["route"] == "open_project");
    REQUIRE(snapshot["pending_action"]["action"] == "open_project_request");

    model.reportProjectOpenFailure("C:/projects/broken.urpg", "project.json is missing");
    snapshot = model.snapshot();
    REQUIRE(snapshot["route"] == "main_menu");
    REQUIRE(snapshot["pending_action"]["success"] == false);
    REQUIRE(snapshot["pending_action"]["message"] == "project.json is missing");

    model.enterEditor("C:/projects/from_template.urpg");
    snapshot = model.snapshot();
    REQUIRE(model.route() == "editor");
    REQUIRE(snapshot["pending_action"]["action"] == "enter_editor");
    REQUIRE(snapshot["recent_projects"][0]["path"] == "C:/projects/from_template.urpg");
}

TEST_CASE("MainMenuModel locates missing projects into recents", "[project][main_menu]") {
    urpg::editor::MainMenuModel model;
    model.addRecentProject("C:/projects/missing.urpg");
    model.markProjectMissing("C:/projects/missing.urpg");

    REQUIRE_FALSE(model.locateMissingProject("C:/projects/other.urpg", "C:/projects/recovered.urpg"));
    auto snapshot = model.snapshot();
    REQUIRE(snapshot["pending_action"]["success"] == false);
    REQUIRE(snapshot["missing_projects"].size() == 1);

    REQUIRE(model.locateMissingProject("C:/projects/missing.urpg", "D:/Recovered/missing.urpg"));
    snapshot = model.snapshot();
    REQUIRE(snapshot["route"] == "main_menu");
    REQUIRE(snapshot["pending_action"]["success"] == true);
    REQUIRE(snapshot["pending_action"]["replacementPath"] == "D:/Recovered/missing.urpg");
    REQUIRE(snapshot["missing_projects"].empty());
    REQUIRE(snapshot["recent_projects"][0]["path"] == "D:/Recovered/missing.urpg");
}

TEST_CASE("MainMenuModel requires an explicit replacement path when locating a missing project", "[project][main_menu]") {
    urpg::editor::MainMenuModel model;
    model.markProjectMissing("C:/projects/missing.urpg");
    REQUIRE(model.beginLocateMissingProject("C:/projects/missing.urpg"));
    auto snapshot = model.snapshot();
    REQUIRE(snapshot["route"] == "locate_project");
    REQUIRE(snapshot["pending_action"]["projectPath"] == "C:/projects/missing.urpg");
    REQUIRE_FALSE(model.beginLocateMissingProject("C:/projects/not-listed.urpg"));
}

TEST_CASE("MainMenuModel limits recents and hides missing projects", "[project][main_menu]") {
    urpg::editor::MainMenuModel model;
    for (int i = 0; i < 12; ++i) {
        model.addRecentProject("C:/projects/project_" + std::to_string(i) + ".urpg");
    }
    model.markProjectMissing("C:/projects/project_3.urpg");
    model.hideMissingProject("C:/projects/project_3.urpg");

    const auto snapshot = model.snapshot();
    REQUIRE(snapshot["recent_projects"].size() == 10);
    REQUIRE(snapshot["recent_projects"][0]["path"] == "C:/projects/project_11.urpg");
    REQUIRE(snapshot["recent_projects"][9]["path"] == "C:/projects/project_2.urpg");
    REQUIRE(snapshot["missing_projects"].empty());
    REQUIRE(snapshot["hidden_missing_projects"].size() == 1);
    REQUIRE(snapshot["hidden_missing_projects"][0] == "C:/projects/project_3.urpg");
}

TEST_CASE("MainMenuModel exposes editable settings route", "[project][main_menu][settings]") {
    urpg::editor::MainMenuModel model;
    model.setOnboardingEnabled(false);
    model.setHelpTipsEnabled(false);
    model.setAssetBrowserLayout("compact_list");
    model.setUiScale(1.5f);
    model.chooseSettings();

    auto snapshot = model.snapshot();
    REQUIRE(model.route() == "settings");
    REQUIRE(snapshot["pending_action"]["action"] == "settings");
    REQUIRE(snapshot["settings"]["onboarding_enabled"] == false);
    REQUIRE(snapshot["settings"]["help_tips_enabled"] == false);
    REQUIRE(snapshot["settings"]["asset_browser_layout"] == "compact_list");
    REQUIRE(snapshot["settings"]["ui_scale"] == 1.5f);
    REQUIRE(snapshot["commands"]["new_project"]["route"] == "template_picker");

    model.returnToMainMenu();
    snapshot = model.snapshot();
    REQUIRE(snapshot["route"] == "main_menu");
    REQUIRE(snapshot["pending_action"]["action"] == "main_menu");
}

TEST_CASE("MainMenuModel persists normalized project identity without losing display casing", "[project][main_menu][settings]") {
    urpg::settings::EditorSettings settings;
    settings.last_project = "C:/Creator/DEMO";
    settings.recent_projects = {"C:/Creator/DEMO", "c:\\creator\\demo", "D:/Creator/Other"};
    settings.pinned_projects = {"C:/Creator/DEMO", "c:/creator/demo"};
    settings.hidden_missing_projects = {"D:/Creator/Hidden", "d:\\creator\\hidden"};
    settings.onboarding_enabled = false;
    settings.help_tips_enabled = false;
    settings.asset_browser_layout = "compact_list";
    settings.accessibility.ui_scale = 1.5f;
    settings.external_asset_library_root = "G:/Creator Assets";

    urpg::editor::MainMenuModel model;
    model.applySettings(settings);
    auto snapshot = model.snapshot();
    REQUIRE(snapshot["recent_projects"].size() == 2);
    REQUIRE(snapshot["recent_projects"][0]["path"] == "C:/Creator/DEMO");
    REQUIRE(snapshot["pinned_projects"].size() == 1);
    REQUIRE(snapshot["hidden_missing_projects"].size() == 1);
    REQUIRE(snapshot["external_asset_library_root"] == "G:/Creator Assets");
    REQUIRE(snapshot["ui_scale"] == 1.5f);
    REQUIRE(snapshot["missing_projects"].size() == 2);

    model.chooseOpenProject("c:/CREATOR/demo");
    urpg::settings::EditorSettings saved;
    model.writeSettings(&saved);
    REQUIRE(saved.last_project == "c:/CREATOR/demo");
    REQUIRE(saved.recent_projects.size() == 2);
    REQUIRE(saved.recent_projects.front() == "c:/CREATOR/demo");
    REQUIRE(saved.onboarding_enabled == false);
    REQUIRE(saved.help_tips_enabled == false);
    REQUIRE(saved.asset_browser_layout == "compact_list");
    REQUIRE(saved.accessibility.ui_scale == 1.5f);
    REQUIRE(saved.external_asset_library_root == "G:/Creator Assets");
}

TEST_CASE("MainMenuPanel renders model-backed main menu snapshot", "[project][main_menu][editor][panel]") {
    urpg::editor::MainMenuModel model;
    model.setOnboardingEnabled(false);

    urpg::editor::MainMenuPanel panel;
    panel.bindModel(&model);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["panel"] == "main_menu");
    REQUIRE(snapshot["status"] == "ready");
    REQUIRE(snapshot["model"]["onboarding_enabled"] == false);
    REQUIRE(snapshot["model"]["commands"]["new_project"]["route"] == "template_picker");
}

TEST_CASE("MainMenuPanel nests a bound guided wizard in the creator flow", "[project][main_menu][editor][panel]") {
    urpg::editor::MainMenuModel model;
    urpg::editor::NewProjectWizardModel wizard;
    urpg::editor::MainMenuPanel panel;
    panel.bindModel(&model);
    panel.bindWizard(&wizard);
    REQUIRE(model.chooseNewProject());
    panel.render();
    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["model"]["route"] == "onboarding");
    REQUIRE(snapshot["wizard"]["step"] == "project");
    REQUIRE(snapshot["wizard"]["template_id"] == "jrpg");
}
