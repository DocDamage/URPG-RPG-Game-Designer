#include "editor/project/new_project_wizard_panel.h"
#include "editor/project/new_project_wizard_model.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path uniqueTempRoot(std::string_view stem) {
    return std::filesystem::temp_directory_path() /
           (std::string(stem) + "_" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
}

} // namespace

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

TEST_CASE("NewProjectWizardModel loads game-maker template manifests for onboarding",
          "[project][editor][panel][onboarding]") {
    const auto root = uniqueTempRoot("urpg_new_project_template_manifests");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    {
        std::ofstream out(root / "jrpg_starter.json");
        out << R"({
          "schemaVersion": 1,
          "templateId": "jrpg",
          "displayName": "Classic JRPG Starter",
          "gameType": "party_rpg",
          "questionProfile": "party_rpg_guided_setup",
          "defaultWorldSize": {"preset": "small", "maps": 3, "recommendedTileSize": 48},
          "recommendedMechanics": ["battle_loop", "save_loop"],
          "defaultCatalogs": ["content/part_catalogs/base_jrpg_parts.json"],
          "optionalCatalogs": ["content/part_catalogs/game_maker_all_parts.json"],
          "assetIndexPath": "content/asset_indexes/game_maker/jrpg_starter.json",
          "fullLibraryPolicy": "opt_in_lazy_load",
          "browserLayout": "left_collapsible_folder_tree",
          "indexBackend": "sqlite",
          "uiThemes": {
            "defaultGameUiTheme": "complete_ui_essential_flat",
            "availableGameUiThemes": ["complete_ui_essential_flat"]
          },
          "futureCommunityTemplateSlot": true
        })";
    }

    urpg::editor::NewProjectWizardModel model;
    std::string error;
    REQUIRE(model.loadGameMakerTemplateManifests(root, &error));
    REQUIRE(error.empty());
    REQUIRE(model.selectedGameMakerTemplateManifestPath() == root / "jrpg_starter.json");
    REQUIRE(model.selectGameMakerTemplate("jrpg"));

    const auto snapshot = model.snapshot();
    REQUIRE(snapshot["game_maker_templates"].size() == 1);
    REQUIRE(snapshot["game_maker_templates"][0]["templateId"] == "jrpg");
    REQUIRE(snapshot["game_maker_templates"][0]["manifestPath"].get<std::string>().find("jrpg_starter.json") !=
            std::string::npos);
    REQUIRE(snapshot["selected_game_maker_template"]["templateId"] == "jrpg");
    REQUIRE(snapshot["selected_game_maker_template"]["assetIndexPath"] ==
            "content/asset_indexes/game_maker/jrpg_starter.json");
    REQUIRE(snapshot["selected_game_maker_template"]["defaultWorldSize"]["preset"] == "small");
    REQUIRE(snapshot["selected_game_maker_template"]["recommendedMechanics"].size() == 2);

    std::filesystem::remove_all(root);
}

TEST_CASE("NewProjectWizardModel creates selected template project on disk",
          "[project][editor][panel][onboarding]") {
    const auto root = uniqueTempRoot("urpg_new_project_on_disk");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "templates");
    {
        std::ofstream out(root / "templates" / "jrpg_starter.json");
        out << R"({
          "schemaVersion": 1,
          "templateId": "jrpg",
          "displayName": "Classic JRPG Starter",
          "gameType": "party_rpg",
          "questionProfile": "party_rpg_guided_setup",
          "defaultWorldSize": {"preset": "small", "maps": 3, "recommendedTileSize": 48},
          "recommendedMechanics": ["battle_loop"],
          "defaultCatalogs": ["content/part_catalogs/base_jrpg_parts.json"],
          "optionalCatalogs": ["content/part_catalogs/game_maker_all_parts.json"],
          "assetIndexPath": "content/asset_indexes/game_maker/jrpg_starter.json",
          "fullLibraryPolicy": "opt_in_lazy_load",
          "browserLayout": "left_collapsible_folder_tree",
          "indexBackend": "sqlite",
          "uiThemes": {"defaultGameUiTheme": "complete_ui_essential_flat", "availableGameUiThemes": []},
          "futureCommunityTemplateSlot": true
        })";
    }

    urpg::editor::NewProjectWizardModel model;
    model.setProjectId("disk_project");
    model.setProjectName("Disk Project");
    REQUIRE(model.loadGameMakerTemplateManifests(root / "templates"));
    REQUIRE(model.selectGameMakerTemplate("jrpg"));

    const auto projectRoot = root / "created" / "disk_project";
    std::string error;
    REQUIRE(model.createProjectOnDisk(projectRoot, &error));
    REQUIRE(error.empty());
    REQUIRE(std::filesystem::exists(projectRoot / "project.json"));
    REQUIRE(std::filesystem::exists(projectRoot / "reports" / "onboarding" / "project_template_audit.json"));
    REQUIRE(std::filesystem::exists(projectRoot / "content" / "game_template_manifest.json"));
    REQUIRE(std::filesystem::is_directory(projectRoot / "content" / "assets" / "manifests"));

    {
        std::ifstream projectIn(projectRoot / "project.json", std::ios::binary);
        const auto project = nlohmann::json::parse(projectIn);
        REQUIRE(project["project_id"] == "disk_project");
        REQUIRE(project["project_name"] == "Disk Project");
        REQUIRE(project["template_id"] == "jrpg");
    }

    std::filesystem::remove_all(root);
}

TEST_CASE("NewProjectWizardModel exposes adaptive question flows for game-maker lanes",
          "[project][editor][panel][onboarding]") {
    const auto root = uniqueTempRoot("urpg_new_project_question_flows");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    const std::vector<std::pair<std::string, std::string>> lanes = {
        {"jrpg", "party_rpg"},
        {"action_rpg", "action_rpg"},
        {"tactical_rpg", "tactical_rpg"},
        {"visual_novel_hybrid", "visual_novel_hybrid"},
        {"cozy_life", "cozy_life"},
        {"monster_collector", "monster_collector"},
        {"platform_adventure", "platform_adventure"},
    };
    for (const auto& [id, gameType] : lanes) {
        std::ofstream out(root / (id + ".json"));
        out << nlohmann::json{
                   {"schemaVersion", 1},
                   {"templateId", id},
                   {"displayName", id},
                   {"gameType", gameType},
                   {"questionProfile", gameType + "_guided_setup"},
                   {"defaultWorldSize", {{"preset", "small"}, {"maps", 3}, {"recommendedTileSize", 48}}},
                   {"recommendedMechanics", {"core_loop", "save_loop"}},
                   {"defaultCatalogs", {"content/part_catalogs/base_jrpg_parts.json"}},
                   {"optionalCatalogs", {"content/part_catalogs/game_maker_all_parts.json"}},
                   {"assetIndexPath", "content/asset_indexes/game_maker/" + id + ".json"},
                   {"fullLibraryPolicy", "opt_in_lazy_load"},
                   {"browserLayout", "left_collapsible_folder_tree"},
                   {"indexBackend", "sqlite"},
                   {"uiThemes", {{"defaultGameUiTheme", "complete_ui_essential_flat"},
                                  {"availableGameUiThemes", {"complete_ui_essential_flat"}}}},
                   {"futureCommunityTemplateSlot", true},
               }.dump(2);
    }

    urpg::editor::NewProjectWizardModel model;
    model.setHelpTipsEnabled(false);
    REQUIRE(model.loadGameMakerTemplateManifests(root));
    for (const auto& [id, gameType] : lanes) {
        REQUIRE(model.selectGameMakerTemplate(id));
        const auto flow = model.snapshot()["question_flow"];
        REQUIRE(flow["template_id"] == id);
        REQUIRE(flow["game_type"] == gameType);
        REQUIRE(flow["help_tips_enabled"] == false);
        REQUIRE(flow["question_count"].get<size_t>() >= 5);
        REQUIRE(flow["questions"][0]["id"] == "project_identity");
        REQUIRE(flow["questions"][flow["questions"].size() - 2]["id"] == "asset_scope");
        REQUIRE(flow["questions"][flow["questions"].size() - 1]["id"] == "ui_theme");
    }

    std::filesystem::remove_all(root);
}

TEST_CASE("NewProjectWizardPanel starts selected game-maker template through asset browser callback",
          "[project][editor][panel][onboarding]") {
    const auto root = uniqueTempRoot("urpg_new_project_template_start");
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    const auto manifestPath = root / "jrpg_starter.json";
    {
        std::ofstream out(manifestPath);
        out << R"({
          "schemaVersion": 1,
          "templateId": "jrpg",
          "displayName": "Classic JRPG Starter",
          "gameType": "party_rpg",
          "questionProfile": "party_rpg_guided_setup",
          "defaultWorldSize": {"preset": "small", "maps": 3, "recommendedTileSize": 48},
          "recommendedMechanics": ["battle_loop"],
          "defaultCatalogs": ["content/part_catalogs/base_jrpg_parts.json"],
          "optionalCatalogs": ["content/part_catalogs/game_maker_all_parts.json"],
          "assetIndexPath": "content/asset_indexes/game_maker/jrpg_starter.json",
          "fullLibraryPolicy": "opt_in_lazy_load",
          "browserLayout": "left_collapsible_folder_tree",
          "indexBackend": "sqlite",
          "uiThemes": {"defaultGameUiTheme": "complete_ui_essential_flat", "availableGameUiThemes": []},
          "futureCommunityTemplateSlot": true
        })";
    }

    urpg::editor::NewProjectWizardModel model;
    REQUIRE(model.loadGameMakerTemplateManifests(root));
    REQUIRE(model.selectGameMakerTemplate("jrpg"));

    std::filesystem::path loadedPath;
    urpg::editor::NewProjectWizardPanel panel;
    panel.bindModel(&model);
    panel.setTemplateStartCallback([&](const std::filesystem::path& path, std::string* error) {
        loadedPath = path;
        if (error != nullptr) {
            error->clear();
        }
        return true;
    });

    std::string error;
    REQUIRE(panel.startSelectedTemplate(&error));
    REQUIRE(error.empty());
    REQUIRE(loadedPath == manifestPath);

    std::filesystem::remove_all(root);
}
