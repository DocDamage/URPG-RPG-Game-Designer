#include "engine/core/battle/battle_core.h"
#include "engine/core/save/save_runtime.h"
#include "editor/diagnostics/migration_wizard_model.h"

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

TEST_CASE("Integration: battle state serializes to save format", "[integration][battle][save]") {
    using namespace urpg::battle;

    BattleFlowController controller;
    controller.beginBattle(true);
    controller.enterInput();
    controller.enterAction();
    controller.endTurn();

    BattleActionQueue queue;
    queue.enqueue(BattleQueuedAction{"actor_001", "enemy_001", "attack", 10, 0});
    queue.enqueue(BattleQueuedAction{"actor_002", "enemy_001", "fire", 8, 1});

    nlohmann::json battle_state;
    battle_state["phase"] = static_cast<int>(controller.phase());
    battle_state["turn_count"] = controller.turnCount();
    battle_state["participants"] = nlohmann::json::array();

    for (const auto& action : queue.snapshotOrdered()) {
        nlohmann::json participant;
        participant["subject_id"] = action.subject_id;
        participant["target_id"] = action.target_id;
        participant["command"] = action.command;
        battle_state["participants"].push_back(participant);
    }

    REQUIRE(battle_state.contains("phase"));
    REQUIRE(battle_state["phase"] == static_cast<int>(BattleFlowPhase::TurnEnd));
    REQUIRE(battle_state.contains("turn_count"));
    REQUIRE(battle_state["turn_count"] == 2);
    REQUIRE(battle_state.contains("participants"));
    REQUIRE(battle_state["participants"].is_array());
    REQUIRE(battle_state["participants"].size() == 2);

    REQUIRE(battle_state["participants"][0]["subject_id"] == "actor_001");
    REQUIRE(battle_state["participants"][0]["command"] == "attack");
    REQUIRE(battle_state["participants"][1]["subject_id"] == "actor_002");
    REQUIRE(battle_state["participants"][1]["command"] == "fire");
}

TEST_CASE("Integration: save loader preserves battle metadata", "[integration][battle][save]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_integration_battle_save_meta";
    std::filesystem::create_directories(base);

    const auto primary_path = base / "save.json";
    const auto metadata_path = base / "meta.json";

    nlohmann::json save_document;
    save_document["_battle_state"] = {
        {"active", true},
        {"turn", 3},
        {"troop_id", "TRP_001"}
    };
    save_document["player"] = {{"hp", 42}, {"max_hp", 100}};
    WriteText(primary_path, save_document.dump());

    WriteText(metadata_path, R"({"_slot_id":7,"_map_display_name":"Battle Save Map"})");

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = primary_path;
    request.metadata_path = metadata_path;

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.loaded_from_recovery);

    const auto loaded = nlohmann::json::parse(result.payload);
    REQUIRE(loaded.contains("_battle_state"));
    REQUIRE(loaded["_battle_state"]["active"] == true);
    REQUIRE(loaded["_battle_state"]["turn"] == 3);
    REQUIRE(loaded["_battle_state"]["troop_id"] == "TRP_001");

    REQUIRE(result.active_meta.map_display_name == "Battle Save Map");
    REQUIRE(result.active_meta.slot_id == 7);

    std::filesystem::remove_all(base);
}

TEST_CASE("Integration: battle scene transitions to map scene after save/load", "[integration][battle][save]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_integration_battle_to_map";
    std::filesystem::create_directories(base);

    urpg::battle::BattleFlowController controller;
    controller.beginBattle(false);
    controller.markVictory();
    REQUIRE_FALSE(controller.isActive());

    nlohmann::json post_battle;
    post_battle["scene_context"] = "map";
    post_battle["map_id"] = "forest_01";
    post_battle["battle_active"] = false;

    const auto primary_path = base / "post_battle_save.json";
    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = primary_path;

    REQUIRE(urpg::RuntimeSaveLoader::Save(request, post_battle.dump()));

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);

    const auto loaded = nlohmann::json::parse(result.payload);
    REQUIRE(loaded.contains("scene_context"));
    REQUIRE(loaded["scene_context"] == "map");
    REQUIRE(loaded.contains("map_id"));
    REQUIRE(loaded["map_id"] == "forest_01");
    REQUIRE(loaded.contains("battle_active"));
    REQUIRE(loaded["battle_active"] == false);

    std::filesystem::remove_all(base);
}

TEST_CASE("Integration: migration wizard combined battle and save report", "[integration][battle][save]") {
    urpg::editor::MigrationWizardModel wizard;

    nlohmann::json project_data;
    project_data["troops"] = nlohmann::json::array();
    project_data["troops"].push_back({
        {"id", 1},
        {"name", "Goblin Patrol"},
        {"members", {
            {{"enemyId", 1}, {"x", 100}, {"y", 200}, {"hidden", false}}
        }}
    });

    project_data["save"] = {
        {"meta", {
            {"slotId", 1},
            {"mapName", "Test Map"}
        }}
    };

    wizard.runFullMigration(project_data);

    const auto report = nlohmann::json::parse(wizard.getReportJson());
    REQUIRE(report["is_complete"] == true);
    REQUIRE(report["total_files_processed"] == 2);
    REQUIRE(report["subsystem_results"].is_array());
    REQUIRE(report["subsystem_results"].size() == 2);

    bool found_battle = false;
    bool found_save = false;

    for (const auto& result : report["subsystem_results"]) {
        if (result["subsystem_id"] == "battle") {
            found_battle = true;
            REQUIRE(result["display_name"] == "Battle");
            REQUIRE(result["processed_count"] == 1);
            REQUIRE(result["warning_count"] == 0);
            REQUIRE(result["error_count"] == 0);
            REQUIRE(result["completed"] == true);
        } else if (result["subsystem_id"] == "save") {
            found_save = true;
            REQUIRE(result["display_name"] == "Save");
            REQUIRE(result["processed_count"] == 1);
            REQUIRE(result["warning_count"] == 0);
            REQUIRE(result["error_count"] == 0);
            REQUIRE(result["completed"] == true);
        }
    }

    REQUIRE(found_battle);
    REQUIRE(found_save);
}
