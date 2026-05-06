#include "editor/project/main_menu_panel.h"

#include <catch2/catch_test_macros.hpp>

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

    model.enterEditor("C:/projects/from_template.urpg");
    snapshot = model.snapshot();
    REQUIRE(model.route() == "editor");
    REQUIRE(snapshot["pending_action"]["action"] == "enter_editor");
    REQUIRE(snapshot["recent_projects"][0]["path"] == "C:/projects/from_template.urpg");
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
    model.chooseSettings();

    auto snapshot = model.snapshot();
    REQUIRE(model.route() == "settings");
    REQUIRE(snapshot["pending_action"]["action"] == "settings");
    REQUIRE(snapshot["settings"]["onboarding_enabled"] == false);
    REQUIRE(snapshot["settings"]["help_tips_enabled"] == false);
    REQUIRE(snapshot["settings"]["asset_browser_layout"] == "compact_list");
    REQUIRE(snapshot["commands"]["new_project"]["route"] == "template_picker");

    model.returnToMainMenu();
    snapshot = model.snapshot();
    REQUIRE(snapshot["route"] == "main_menu");
    REQUIRE(snapshot["pending_action"]["action"] == "main_menu");
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
