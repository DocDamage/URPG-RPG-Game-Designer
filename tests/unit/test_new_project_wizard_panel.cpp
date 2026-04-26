#include "editor/project/new_project_wizard_panel.h"
#include "editor/project/new_project_wizard_model.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("NewProjectWizardPanel renders explicit disabled state without model", "[project][editor][panel]") {
    urpg::editor::NewProjectWizardPanel panel;

    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["panel"] == "new_project_wizard");
    REQUIRE(snapshot["status"] == "disabled");
    REQUIRE(snapshot["disabled_reason"] == "No NewProjectWizardModel is bound.");
    REQUIRE(snapshot["owner"] == "editor/project");
    REQUIRE(snapshot["unlock_condition"] == "Bind NewProjectWizardModel before rendering the new project wizard.");
}

TEST_CASE("NewProjectWizardPanel renders model-backed ready state", "[project][editor][panel]") {
    urpg::editor::NewProjectWizardModel model;
    model.setTemplateId("jrpg");
    model.setProjectId("demo_project");
    model.setProjectName("Demo Project");

    urpg::editor::NewProjectWizardPanel panel;
    panel.bindModel(&model);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["panel"] == "new_project_wizard");
    REQUIRE(snapshot["status"] == "ready");
    REQUIRE(snapshot["model"]["template_id"] == "jrpg");
    REQUIRE(snapshot["model"]["project_id"] == "demo_project");
    REQUIRE(snapshot["model"]["project_name"] == "Demo Project");
}
