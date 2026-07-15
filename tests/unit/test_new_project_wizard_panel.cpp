#include "editor/project/new_project_wizard_panel.h"
#include "editor/project/new_project_wizard_model.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>

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

TEST_CASE("NewProjectWizardModel creates a persisted starter project", "[project][new project][wizard]") {
    const auto root = std::filesystem::temp_directory_path() /
                      ("urpg_wizard_project_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    urpg::editor::NewProjectWizardModel model;
    model.setProjectId("wizard_demo");
    model.setProjectName("Wizard Demo");
    model.setDestination(root / "WizardDemo");

    const auto result = model.createProject();
    REQUIRE(result.success);
    REQUIRE(std::filesystem::is_regular_file(result.project_root / "project.json"));
    REQUIRE(model.snapshot()["last_audit"]["runtime_preflight"] == "passed");
    std::filesystem::remove_all(root);
}

TEST_CASE("NewProjectWizardModel exposes the complete guided creator flow", "[project][new project][wizard]") {
    urpg::editor::NewProjectWizardModel model;
    REQUIRE(model.snapshot()["step"] == "project");
    REQUIRE_FALSE(model.previousStep());
    for (int i = 0; i < 6; ++i) REQUIRE(model.nextStep());
    REQUIRE(model.snapshot()["step"] == "create");
    REQUIRE_FALSE(model.nextStep());
    model.setVisualStyle("hand_drawn");
    model.setDisplayPreset("1920x1080");
    model.setInputPreset("keyboard");
    const auto missingLibraryRoot = std::filesystem::temp_directory_path() /
                                    ("urpg_wizard_missing_library_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    model.setExternalAssetLibraryRoot(missingLibraryRoot);
    model.setStarterMap("forest_intro");
    const auto snapshot = model.snapshot();
    REQUIRE(snapshot["steps"].size() == 7);
    REQUIRE(snapshot["visual_style"] == "hand_drawn");
    REQUIRE(snapshot["display_preset"] == "1920x1080");
    REQUIRE(snapshot["input_preset"] == "keyboard");
    REQUIRE(snapshot["external_asset_library_root"] == missingLibraryRoot.generic_string());
    REQUIRE(snapshot["starter_map"] == "forest_intro");
    REQUIRE(snapshot["external_asset_library"]["status"] == "root_missing");
}

TEST_CASE("NewProjectWizardModel detects an existing user-local asset catalog", "[project][new project][wizard][assets]") {
    const auto root = std::filesystem::temp_directory_path() /
                      ("urpg_wizard_asset_catalog_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root / ".urpg" / "asset-index");
    std::ofstream(root / ".urpg" / "asset-index" / "catalog_meta.json") << "{}";

    urpg::editor::NewProjectWizardModel model;
    model.setExternalAssetLibraryRoot(root);
    const auto library = model.snapshot()["external_asset_library"];
    REQUIRE(library["status"] == "indexed");
    REQUIRE(library["catalog_meta_path"] == (root / ".urpg" / "asset-index" / "catalog_meta.json").generic_string());
    std::filesystem::remove_all(root);
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
