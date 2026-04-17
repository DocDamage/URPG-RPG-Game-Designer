#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <variant>

using namespace urpg::compat;
using urpg::Array;

TEST_CASE("DataManager: database loading and accessors", "[data_manager]") {
    DataManager dm;
    DataManager::setDataDirectory(URPG_SOURCE_DIR "\\third_party\\rpgmaker-mz\\visumz-sample-project\\VisuMZ_Sample_Game_Project\\data");

    REQUIRE(dm.loadDatabase());
    // loadDatabase now orchestrates sub-loaders and reads real MZ JSON files
    REQUIRE_FALSE(dm.getActors().empty());
    REQUIRE_FALSE(dm.getSkills().empty());
    REQUIRE_FALSE(dm.getItems().empty());
    REQUIRE_FALSE(dm.getClasses().empty());
    REQUIRE_FALSE(dm.getWeapons().empty());
    REQUIRE_FALSE(dm.getArmors().empty());
    REQUIRE_FALSE(dm.getEnemies().empty());
    REQUIRE_FALSE(dm.getTroops().empty());
    REQUIRE_FALSE(dm.getStates().empty());
    REQUIRE_FALSE(dm.getMapInfos().empty());

    // Direct sub-loader calls are idempotent
    REQUIRE(dm.loadActors());
    REQUIRE(dm.loadSkills());
    REQUIRE(dm.loadItems());
    REQUIRE(dm.loadEnemies());

    // Verify real records from VisuMZ sample project are loaded
    REQUIRE(dm.getActor(1) != nullptr);
    REQUIRE(dm.getActor(1)->name == "Reid");
    REQUIRE(dm.getActor(2) != nullptr);
    REQUIRE(dm.getActor(2)->name == "Priscilla");
    REQUIRE(dm.getActor(1)->params.size() == 8);
    REQUIRE(dm.getActor(1)->params[0].size() > 1);
    REQUIRE(dm.getActor(1)->params[6].size() > 1);
    REQUIRE(dm.getActor(1)->params[0][1] > 0);
    REQUIRE(dm.getActor(1)->params[6][1] > 0);

    REQUIRE(dm.getSkill(1) != nullptr);
    REQUIRE(dm.getSkill(1)->name == "Attack");
    REQUIRE(dm.getSkill(52) != nullptr);
    REQUIRE(dm.getSkill(52)->name == "Heal I");

    REQUIRE(dm.getItem(7) != nullptr);
    REQUIRE(dm.getItem(7)->name == "Potion");

    REQUIRE(dm.getClass(1) != nullptr);
    REQUIRE(dm.getClass(1)->name == "Swordsman");

    REQUIRE(dm.getWeapon(1) != nullptr);
    REQUIRE(dm.getWeapon(1)->name == "Short Sword");

    REQUIRE(dm.getArmor(2) != nullptr);
    REQUIRE(dm.getArmor(2)->name == "Linen Clothing");

    REQUIRE(dm.getEnemy(1) != nullptr);
    REQUIRE(dm.getEnemy(1)->name == "Goblin");

    REQUIRE(dm.getTroop(1) != nullptr);
    REQUIRE(dm.getTroop(1)->name == "Goblin x2");

    REQUIRE(dm.getState(4) != nullptr);
    REQUIRE(dm.getState(4)->name == "Poison");

    REQUIRE(!dm.getMapInfos().empty());
    REQUIRE(dm.getMapInfos()[0].name == "Debug Room");

    REQUIRE(dm.getStartMapId() == 2);
    REQUIRE(dm.getStartX() == 16);
    REQUIRE(dm.getStartY() == 23);
    REQUIRE(dm.getStartPartySize() == 8);

    REQUIRE(dm.getActor(999) == nullptr);
    REQUIRE(dm.getSkill(999) == nullptr);
    REQUIRE(dm.getItem(999) == nullptr);
}

TEST_CASE("DataManager: global state and inventory", "[data_manager]") {
    DataManager::setDataDirectory("");
    DataManager dm;
    dm.loadDatabase();
    dm.setupNewGame();

    GlobalState& state = dm.getGlobalState();
    REQUIRE(state.actors.empty());
    // setupNewGame now copies the start party (populated by loadDatabase)
    REQUIRE_FALSE(state.partyMembers.empty());

    REQUIRE(dm.getPartySize() == 1);
    REQUIRE(dm.getPartyMember(0) == 1);
    REQUIRE(dm.getGold() == 0);

    dm.setGold(1000);
    REQUIRE(dm.getGold() == 1000);

    dm.loseGold(50);
    REQUIRE(dm.getGold() == 950);

    dm.gainGold(25);
    REQUIRE(dm.getGold() == 975);

    REQUIRE(dm.getItemCount(1) == 0);
    REQUIRE_FALSE(dm.hasItem(1));

    dm.gainItem(1, 5);
    REQUIRE(dm.getItemCount(1) == 5);
    REQUIRE(dm.hasItem(1));

    dm.loseItem(1, 3);
    REQUIRE(dm.getItemCount(1) == 2);

    dm.loseItem(1, 50);
    REQUIRE(dm.getItemCount(1) == 0);
    REQUIRE_FALSE(dm.hasItem(1));
}

TEST_CASE("DataManager: actor param access stays safe when only actor records are loaded", "[data_manager]") {
    DataManager::setDataDirectory("");
    DataManager dm;

    REQUIRE(dm.loadActors());
    REQUIRE(dm.getActor(1) != nullptr);
    REQUIRE(dm.getActor(1)->params.empty());

    REQUIRE(dm.getActorParam(1, 0, 1) == 100);
    REQUIRE(dm.getActorParam(1, 1, 1) == 30);
    REQUIRE(dm.getActorParam(1, 2, 1) == 10);
    REQUIRE(dm.getActorParam(1, 6, 1) == 10);
    REQUIRE(dm.getActorParam(999, 2, 1) == 10);
}

TEST_CASE("DataManager: switches, variables, and self switches", "[data_manager]") {
    DataManager dm;

    REQUIRE_FALSE(dm.getSwitch(1));
    dm.setSwitch(1, true);
    REQUIRE(dm.getSwitch(1));

    REQUIRE(dm.getVariable(1) == 0);
    dm.setVariable(1, 42);
    REQUIRE(dm.getVariable(1) == 42);

    REQUIRE_FALSE(dm.getSelfSwitch(1, 1, "A"));
    dm.setSelfSwitch(1, 1, "A", true);
    REQUIRE(dm.getSelfSwitch(1, 1, "A"));
}

TEST_CASE("DataManager: playtime and player position", "[data_manager]") {
    DataManager dm;
    dm.setupNewGame();

    REQUIRE(dm.getPlaytime() == 0);
    dm.updatePlaytime();
    REQUIRE(dm.getPlaytime() >= 0);
    REQUIRE(dm.getPlaytimeString().find(':') != std::string::npos);

    dm.setPlayerPosition(2, 7, 9);
    REQUIRE(dm.getPlayerMapId() == 2);
    REQUIRE(dm.getPlayerX() == 7);
    REQUIRE(dm.getPlayerY() == 9);

    dm.setPlayerDirection(8);
    REQUIRE(dm.getPlayerDirection() == 8);
}

TEST_CASE("DataManager: transfer reservation and processing", "[data_manager]") {
    DataManager dm;
    dm.setupNewGame();
    dm.setPlayerPosition(1, 2, 3);

    REQUIRE_FALSE(dm.isTransferring());

    dm.reserveTransfer(5, 9, 11, 4);
    REQUIRE(dm.isTransferring());

    // Position is applied only after processTransfer.
    REQUIRE(dm.getPlayerMapId() == 1);
    REQUIRE(dm.getPlayerX() == 2);
    REQUIRE(dm.getPlayerY() == 3);

    dm.processTransfer();
    REQUIRE_FALSE(dm.isTransferring());
    REQUIRE(dm.getPlayerMapId() == 5);
    REQUIRE(dm.getPlayerX() == 9);
    REQUIRE(dm.getPlayerY() == 11);
    REQUIRE(dm.getPlayerDirection() == 4);
}

TEST_CASE("DataManager: save/load round-trip semantics", "[data_manager]") {
    DataManager dm;
    dm.setupNewGame();

    dm.setGold(450);
    dm.gainItem(2, 7);
    dm.setSwitch(2, true);
    dm.setVariable(4, 88);
    dm.setPlayerPosition(9, 10, 11);
    dm.setPlayerDirection(6);

    REQUIRE_FALSE(dm.doesSaveFileExist(1));
    REQUIRE(dm.saveGame(1));
    REQUIRE(dm.doesSaveFileExist(1));

    auto generatedHeader = dm.getSaveHeader(1);
    REQUIRE(generatedHeader.has_value());
    REQUIRE(generatedHeader->mapId == 9);
    REQUIRE(generatedHeader->playerX == 10);
    REQUIRE(generatedHeader->playerY == 11);
    REQUIRE_FALSE(generatedHeader->isAutosave);

    dm.setGold(0);
    dm.loseItem(2, 7);
    dm.setSwitch(2, false);
    dm.setVariable(4, 0);
    dm.setPlayerPosition(1, 0, 0);
    dm.setPlayerDirection(2);
    REQUIRE(dm.loadGame(1));

    REQUIRE(dm.getGold() == 450);
    REQUIRE(dm.getItemCount(2) == 7);
    REQUIRE(dm.getSwitch(2));
    REQUIRE(dm.getVariable(4) == 88);
    REQUIRE(dm.getPlayerMapId() == 9);
    REQUIRE(dm.getPlayerX() == 10);
    REQUIRE(dm.getPlayerY() == 11);
    REQUIRE(dm.getPlayerDirection() == 6);

    SaveHeader customHeader;
    customHeader.version = 7;
    customHeader.mapId = 77;
    customHeader.playerX = 12;
    customHeader.playerY = 13;
    customHeader.partyNames = "Haru,Noa";
    customHeader.mapDisplayName = "Custom Map";

    REQUIRE(dm.saveGameWithHeader(2, customHeader));
    auto loadedCustomHeader = dm.getSaveHeader(2);
    REQUIRE(loadedCustomHeader.has_value());
    REQUIRE(loadedCustomHeader->version == 7);
    REQUIRE(loadedCustomHeader->mapId == 77);
    REQUIRE(loadedCustomHeader->playerX == 12);
    REQUIRE(loadedCustomHeader->playerY == 13);
    REQUIRE(loadedCustomHeader->partyNames == "Haru,Noa");
    REQUIRE(loadedCustomHeader->mapDisplayName == "Custom Map");

    REQUIRE_FALSE(dm.saveGame(dm.getMaxSaveFiles() + 1));
    REQUIRE_FALSE(dm.loadGame(dm.getMaxSaveFiles() + 1));
    REQUIRE_FALSE(dm.deleteSaveFile(dm.getMaxSaveFiles() + 1));

    REQUIRE(dm.deleteSaveFile(2));
    REQUIRE_FALSE(dm.doesSaveFileExist(2));
    REQUIRE_FALSE(dm.loadGame(2));
    REQUIRE_FALSE(dm.deleteSaveFile(2));
}

TEST_CASE("DataManager: autosave and save header extensions", "[data_manager]") {
    DataManager dm;
    dm.setupNewGame();

    REQUIRE(dm.isAutosaveEnabled());
    dm.setAutosaveEnabled(false);
    REQUIRE_FALSE(dm.isAutosaveEnabled());
    REQUIRE_FALSE(dm.saveAutosave());

    REQUIRE(dm.setSaveHeaderExtension(1, "plugin.reward", urpg::Value::Int(42)));
    auto extension = dm.getSaveHeaderExtension(1, "plugin.reward");
    REQUIRE(extension.has_value());
    REQUIRE(std::holds_alternative<int64_t>(extension->v));
    REQUIRE(std::get<int64_t>(extension->v) == 42);

    REQUIRE_FALSE(dm.setSaveHeaderExtension(dm.getMaxSaveFiles() + 1, "bad.slot", urpg::Value::Int(1)));
    REQUIRE_FALSE(dm.setSaveHeaderExtension(1, "", urpg::Value::Int(1)));
    REQUIRE_FALSE(dm.getSaveHeaderExtension(dm.getMaxSaveFiles() + 1, "bad.slot").has_value());

    dm.setAutosaveEnabled(true);
    REQUIRE(dm.isAutosaveEnabled());
    REQUIRE(dm.saveAutosave());
    REQUIRE(dm.loadAutosave());
    auto autosaveHeader = dm.getSaveHeader(0);
    REQUIRE(autosaveHeader.has_value());
    REQUIRE(autosaveHeader->isAutosave);

    REQUIRE(dm.deleteSaveFile(1));
    REQUIRE_FALSE(dm.getSaveHeaderExtension(1, "plugin.reward").has_value());
}

TEST_CASE("DataManager: method status registry", "[data_manager]") {
    DataManager dm;
    (void)dm;

    REQUIRE(DataManager::getMethodStatus("loadDatabase") == CompatStatus::PARTIAL);
    REQUIRE(DataManager::getMethodStatus("setupNewGame") == CompatStatus::PARTIAL);
    REQUIRE(DataManager::getMethodStatus("setSaveHeaderExtension") == CompatStatus::PARTIAL);
    REQUIRE(DataManager::getMethodDeviation("loadDatabase").empty());
    REQUIRE(DataManager::getMethodStatus("nonexistentMethod") == CompatStatus::UNSUPPORTED);
}

TEST_CASE("DataManager: database accessors as Value", "[data_manager]") {
    DataManager dm;
    dm.loadDatabase();

    auto actors = dm.getActorsAsValue();
    REQUIRE(std::holds_alternative<Array>(actors.v));
    auto& actorArr = std::get<Array>(actors.v);
    REQUIRE(!actorArr.empty());

    auto items = dm.getItemsAsValue();
    REQUIRE(std::holds_alternative<Array>(items.v));
    auto& itemArr = std::get<Array>(items.v);
    REQUIRE(!itemArr.empty());

    auto skills = dm.getSkillsAsValue();
    REQUIRE(std::holds_alternative<Array>(skills.v));
    auto& skillArr = std::get<Array>(skills.v);
    REQUIRE(!skillArr.empty());
}

TEST_CASE("DataManager: JS API bindings via registerAPI", "[data_manager]") {
    DataManager dm;
    dm.loadDatabase();
    dm.setupNewGame();

    QuickJSContext ctx;
    QuickJSConfig config;
    REQUIRE(ctx.initialize(config));

    DataManager::registerAPI(ctx);

    // getGold / setGold
    auto getGoldResult = ctx.callMethod("DataManager", "getGold", {});
    REQUIRE(getGoldResult.success);
    REQUIRE(std::holds_alternative<int64_t>(getGoldResult.value.v));
    REQUIRE(std::get<int64_t>(getGoldResult.value.v) == 0);

    ctx.callMethod("DataManager", "setGold", {urpg::Value::Int(250)});
    getGoldResult = ctx.callMethod("DataManager", "getGold", {});
    REQUIRE(getGoldResult.success);
    REQUIRE(std::get<int64_t>(getGoldResult.value.v) == 250);

    // getSwitch / setSwitch
    auto getSwitchResult = ctx.callMethod("DataManager", "getSwitch", {urpg::Value::Int(1)});
    REQUIRE(getSwitchResult.success);
    REQUIRE(std::holds_alternative<int64_t>(getSwitchResult.value.v));
    REQUIRE(std::get<int64_t>(getSwitchResult.value.v) == 0);

    ctx.callMethod("DataManager", "setSwitch", {urpg::Value::Int(1), urpg::Value::Int(1)});
    getSwitchResult = ctx.callMethod("DataManager", "getSwitch", {urpg::Value::Int(1)});
    REQUIRE(getSwitchResult.success);
    REQUIRE(std::get<int64_t>(getSwitchResult.value.v) == 1);

    // getVariable / setVariable
    auto getVarResult = ctx.callMethod("DataManager", "getVariable", {urpg::Value::Int(5)});
    REQUIRE(getVarResult.success);
    REQUIRE(std::holds_alternative<int64_t>(getVarResult.value.v));
    REQUIRE(std::get<int64_t>(getVarResult.value.v) == 0);

    ctx.callMethod("DataManager", "setVariable", {urpg::Value::Int(5), urpg::Value::Int(99)});
    getVarResult = ctx.callMethod("DataManager", "getVariable", {urpg::Value::Int(5)});
    REQUIRE(getVarResult.success);
    REQUIRE(std::get<int64_t>(getVarResult.value.v) == 99);

    // getItemCount / gainItem
    auto getItemResult = ctx.callMethod("DataManager", "getItemCount", {urpg::Value::Int(3)});
    REQUIRE(getItemResult.success);
    REQUIRE(std::holds_alternative<int64_t>(getItemResult.value.v));
    REQUIRE(std::get<int64_t>(getItemResult.value.v) == 0);

    ctx.callMethod("DataManager", "gainItem", {urpg::Value::Int(3), urpg::Value::Int(12)});
    getItemResult = ctx.callMethod("DataManager", "getItemCount", {urpg::Value::Int(3)});
    REQUIRE(getItemResult.success);
    REQUIRE(std::get<int64_t>(getItemResult.value.v) == 12);
}

TEST_CASE("DataManager structs: defaults", "[data_manager]") {
    SaveHeader header;
    REQUIRE(header.version == 0);
    REQUIRE(header.timestamp.empty());
    REQUIRE(header.playtimeFrames == 0);
    REQUIRE(header.mapId == 0);
    REQUIRE(header.playerId == 0);
    REQUIRE(header.playerX == 0);
    REQUIRE(header.playerY == 0);
    REQUIRE_FALSE(header.isAutosave);

    GlobalState state;
    REQUIRE(state.actors.empty());
    REQUIRE(state.partyMembers.empty());
    REQUIRE(state.gold == 0);
    REQUIRE(state.steps == 0);
    REQUIRE(state.playtime == 0);
    REQUIRE(state.mapId == 0);
    REQUIRE(state.playerX == 0);
    REQUIRE(state.playerY == 0);
    REQUIRE(state.playerDirection == 2);
    REQUIRE(state.switches.empty());
    REQUIRE(state.variables.empty());
    REQUIRE(state.items.empty());
    REQUIRE(state.weapons.empty());
    REQUIRE(state.armors.empty());
}
