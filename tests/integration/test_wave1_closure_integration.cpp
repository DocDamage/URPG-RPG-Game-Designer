#include "engine/core/battle/battle_core.h"
#include "engine/core/migrate/migration_runner.h"
#include "engine/core/message/message_core.h"
#include "engine/core/save/save_runtime.h"
#include "engine/core/ui/menu_scene_graph.h"
#include "engine/core/ui/menu_serializer.h"

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

void WriteText(const std::filesystem::path& path, const std::string& value) {
    std::ofstream out(path, std::ios::binary);
    out << value;
}

} // namespace

TEST_CASE("Integration: battle save round-trip preserves phase and turn count", "[integration][wave1][closure]") {
    using namespace urpg::battle;

    BattleFlowController controller;
    controller.beginBattle(true);
    controller.enterInput();
    controller.enterAction();
    controller.endTurn();

    REQUIRE(controller.phase() == BattleFlowPhase::TurnEnd);
    REQUIRE(controller.turnCount() == 2);

    nlohmann::json battle_meta;
    battle_meta["phase"] = static_cast<int>(controller.phase());
    battle_meta["turn_count"] = controller.turnCount();
    battle_meta["can_escape"] = controller.canEscape();

    const auto base = std::filesystem::temp_directory_path() / "urpg_wave1_battle_roundtrip";
    std::filesystem::create_directories(base);
    const auto save_path = base / "battle_save.json";

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = save_path;

    REQUIRE(urpg::RuntimeSaveLoader::Save(request, battle_meta.dump()));

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.loaded_from_recovery);

    const auto loaded = nlohmann::json::parse(result.payload);
    REQUIRE(loaded["phase"] == static_cast<int>(BattleFlowPhase::TurnEnd));
    REQUIRE(loaded["turn_count"] == 2);
    REQUIRE(loaded["can_escape"] == true);

    std::filesystem::remove_all(base);
}

TEST_CASE("Integration: save migration preserves battle metadata", "[integration][wave1][closure]") {
    nlohmann::json save_document;
    save_document["_urpg_format_version"] = "1.0";
    save_document["player"] = {
        {"hp", 42},
        {"max_hp", 100},
        {"atk", 12}
    };
    save_document["_battle_state"] = {
        {"active", true},
        {"phase", 3},
        {"turn_count", 5},
        {"troop_id", "TRP_001"}
    };

    const nlohmann::json migration_spec = nlohmann::json::parse(
        R"({
            "from": "1.0",
            "to": "1.1",
            "ops": [
                {"op": "rename", "fromPath": "/player/atk", "toPath": "/player/attack"}
            ]
        })"
    );

    const auto migration_error = urpg::MigrationRunner::Apply(migration_spec, save_document);
    REQUIRE_FALSE(migration_error.has_value());

    REQUIRE(save_document.contains("_battle_state"));
    REQUIRE(save_document["_battle_state"]["active"] == true);
    REQUIRE(save_document["_battle_state"]["phase"] == 3);
    REQUIRE(save_document["_battle_state"]["turn_count"] == 5);
    REQUIRE(save_document["_battle_state"]["troop_id"] == "TRP_001");

    REQUIRE(save_document.contains("player"));
    REQUIRE(save_document["player"]["attack"] == 12);
    REQUIRE_FALSE(save_document["player"].contains("atk"));
}

TEST_CASE("Integration: menu state survives save and load", "[integration][wave1][closure]") {
    using namespace urpg::ui;

    MenuSceneGraph graph;
    auto scene = std::make_shared<MenuScene>("MainMenu");

    MenuPane pane;
    pane.id = "main";
    pane.displayName = "Main";
    pane.selectedCommandIndex = 2;

    urpg::MenuCommandMeta cmd1;
    cmd1.id = "cmd_item";
    cmd1.label = "Item";
    cmd1.route = urpg::MenuRouteTarget::Item;
    pane.commands.push_back(cmd1);

    urpg::MenuCommandMeta cmd2;
    cmd2.id = "cmd_skill";
    cmd2.label = "Skill";
    cmd2.route = urpg::MenuRouteTarget::Skill;
    pane.commands.push_back(cmd2);

    urpg::MenuCommandMeta cmd3;
    cmd3.id = "cmd_equip";
    cmd3.label = "Equip";
    cmd3.route = urpg::MenuRouteTarget::Equip;
    pane.commands.push_back(cmd3);

    scene->addPane(pane);
    graph.registerScene(scene);
    graph.pushScene("MainMenu");

    nlohmann::json menu_state;
    menu_state["graph"] = MenuSceneSerializer::SerializeGraph(graph);
    menu_state["runtime"] = {
        {"active_scene", "MainMenu"},
        {"pane_runtime", {
            {"main", {{"selected_index", pane.selectedCommandIndex}}}
        }}
    };

    const auto base = std::filesystem::temp_directory_path() / "urpg_wave1_menu_state";
    std::filesystem::create_directories(base);
    const auto save_path = base / "menu_save.json";

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = save_path;

    REQUIRE(urpg::RuntimeSaveLoader::Save(request, menu_state.dump()));

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.loaded_from_recovery);

    const auto loaded = nlohmann::json::parse(result.payload);

    MenuSceneGraph restored_graph;
    REQUIRE(MenuSceneSerializer::DeserializeGraph(loaded["graph"], restored_graph));

    const std::string active_scene = loaded["runtime"]["active_scene"];
    restored_graph.pushScene(active_scene);

    const auto restored_scene = restored_graph.getActiveScene();
    REQUIRE(restored_scene != nullptr);
    REQUIRE(restored_scene->getId() == "MainMenu");

    const auto& restored_panes = restored_scene->getPanes();
    REQUIRE(restored_panes.size() == 1);
    REQUIRE(restored_panes[0].commands.size() == 3);

    const int restored_selected_index = loaded["runtime"]["pane_runtime"]["main"]["selected_index"];
    REQUIRE(restored_selected_index == 2);

    std::filesystem::remove_all(base);
}

TEST_CASE("Integration: message dialogue state survives save and load", "[integration][wave1][closure]") {
    using namespace urpg::message;

    std::vector<DialoguePage> pages;
    DialoguePage page1;
    page1.id = "pg_01";
    page1.body = "Hello, world.";
    page1.variant.speaker = "Alice";
    pages.push_back(page1);

    DialoguePage page2;
    page2.id = "pg_02";
    page2.body = "Greetings.";
    page2.variant.speaker = "Bob";
    pages.push_back(page2);

    MessageFlowRunner runner;
    runner.begin(pages);
    runner.markPagePresented();
    runner.advance();

    REQUIRE(runner.isActive());
    REQUIRE(runner.currentPageIndex() == 1);
    REQUIRE(runner.currentPage() != nullptr);
    REQUIRE(runner.currentPage()->variant.speaker == "Bob");

    const auto snapshot = runner.snapshot();
    nlohmann::json dialogue_state;
    dialogue_state["page_index"] = snapshot.page_index;
    dialogue_state["state"] = static_cast<int>(snapshot.state);
    dialogue_state["selected_choice_index"] = snapshot.selected_choice_index;
    dialogue_state["speaker"] = runner.currentPage()->variant.speaker;

    const auto base = std::filesystem::temp_directory_path() / "urpg_wave1_message_state";
    std::filesystem::create_directories(base);
    const auto save_path = base / "message_save.json";

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = save_path;

    REQUIRE(urpg::RuntimeSaveLoader::Save(request, dialogue_state.dump()));

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.loaded_from_recovery);

    const auto loaded = nlohmann::json::parse(result.payload);
    REQUIRE(loaded["page_index"] == 1);
    REQUIRE(loaded["speaker"] == "Bob");

    MessageFlowRunner restored_runner;
    restored_runner.begin(pages);

    MessageFlowSnapshot restored_snapshot;
    restored_snapshot.page_index = loaded["page_index"];
    restored_snapshot.state = static_cast<MessageFlowState>(loaded["state"].get<int>());
    restored_snapshot.selected_choice_index = loaded["selected_choice_index"];

    REQUIRE(restored_runner.restore(restored_snapshot));
    REQUIRE(restored_runner.currentPageIndex() == 1);
    REQUIRE(restored_runner.currentPage() != nullptr);
    REQUIRE(restored_runner.currentPage()->variant.speaker == "Bob");

    std::filesystem::remove_all(base);
}
