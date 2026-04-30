#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <variant>

using namespace urpg::compat;
using urpg::Array;
using urpg::Object;

namespace {

std::string sampleProjectDataDirectory() {
    return (std::filesystem::path(URPG_SOURCE_DIR) / "third_party" / "rpgmaker-mz" / "visumz-sample-project" /
            "VisuMZ_Sample_Game_Project" / "data")
        .string();
}

} // namespace

TEST_CASE("DataManager: database loading and accessors", "[data_manager]") {
    DataManager dm;
    const auto dataDirectory = std::filesystem::path(sampleProjectDataDirectory());
    const bool sampleProjectAvailable = std::filesystem::exists(dataDirectory / "Actors.json");
    DataManager::setDataDirectory(dataDirectory.string());

    REQUIRE(dm.loadDatabase());
    REQUIRE_FALSE(dm.getActors().empty());
    REQUIRE_FALSE(dm.getSkills().empty());
    REQUIRE_FALSE(dm.getItems().empty());
    REQUIRE_FALSE(dm.getClasses().empty());
    REQUIRE_FALSE(dm.getEnemies().empty());
    REQUIRE_FALSE(dm.getTroops().empty());

    // Direct sub-loader calls are idempotent
    REQUIRE(dm.loadActors());
    REQUIRE(dm.loadSkills());
    REQUIRE(dm.loadItems());
    REQUIRE(dm.loadEnemies());

    if (!sampleProjectAvailable) {
        REQUIRE(dm.getActor(1) != nullptr);
        REQUIRE(dm.getActor(1)->name == "Hero");
        REQUIRE(dm.getSkill(1) != nullptr);
        REQUIRE(dm.getSkill(1)->name == "Heal");
        REQUIRE(dm.getItem(1) != nullptr);
        REQUIRE(dm.getItem(1)->name == "Potion");
        REQUIRE(dm.getClass(1) != nullptr);
        REQUIRE(dm.getClass(1)->name == "Warrior");
        REQUIRE(dm.getEnemy(1) != nullptr);
        REQUIRE(dm.getEnemy(1)->name == "Slime");
        REQUIRE(dm.getTroop(1) != nullptr);
        REQUIRE(dm.getTroop(1)->name == "Slime x2");
        return;
    }

    REQUIRE_FALSE(dm.getWeapons().empty());
    REQUIRE_FALSE(dm.getArmors().empty());
    REQUIRE_FALSE(dm.getStates().empty());
    REQUIRE_FALSE(dm.getMapInfos().empty());

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
    REQUIRE(dm.getEnemy(1)->battlerName == "Goblin");

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

TEST_CASE("DataManager loadDatabase populates seeded database containers", "[data_manager]") {
    DataManager::setDataDirectory("");
    DataManager dm;

    REQUIRE(dm.loadDatabase());
    REQUIRE_FALSE(dm.getActors().empty());
    REQUIRE_FALSE(dm.getSkills().empty());
    REQUIRE_FALSE(dm.getItems().empty());
    REQUIRE_FALSE(dm.getClasses().empty());
    REQUIRE_FALSE(dm.getEnemies().empty());
    REQUIRE_FALSE(dm.getTroops().empty());
    REQUIRE_FALSE(dm.getAnimations().empty());
    REQUIRE(dm.getActor(1) != nullptr);
    REQUIRE(dm.getActor(1)->name == "Hero");
    REQUIRE(dm.getSkill(1) != nullptr);
    REQUIRE(dm.getSkill(1)->name == "Heal");
    REQUIRE(dm.getItem(1) != nullptr);
    REQUIRE(dm.getItem(1)->name == "Potion");
    REQUIRE(dm.getClass(1) != nullptr);
    REQUIRE(dm.getClass(1)->name == "Warrior");
    REQUIRE(dm.getEnemy(1) != nullptr);
    REQUIRE(dm.getEnemy(1)->name == "Slime");
    REQUIRE(dm.getEnemy(1)->battlerName == "Monster");
    REQUIRE(dm.getTroop(1) != nullptr);
    REQUIRE(dm.getTroop(1)->name == "Slime x2");
}

TEST_CASE("DataManager: enemy battler names preserve string shape across real and seeded loads", "[data_manager]") {
    SECTION("real MZ enemy data keeps battlerName strings") {
        DataManager dm;
        const auto dataDirectory = std::filesystem::path(sampleProjectDataDirectory());
        const bool sampleProjectAvailable = std::filesystem::exists(dataDirectory / "Enemies.json");
        DataManager::setDataDirectory(dataDirectory.string());

        REQUIRE(dm.loadEnemies());
        const EnemyData* enemy = dm.getEnemy(1);
        REQUIRE(enemy != nullptr);
        if (sampleProjectAvailable) {
            REQUIRE(enemy->name == "Goblin");
            REQUIRE(enemy->battlerName == "Goblin");
        } else {
            REQUIRE(enemy->name == "Slime");
            REQUIRE(enemy->battlerName == "Monster");
        }

        auto enemiesValue = dm.getEnemiesAsValue();
        REQUIRE(std::holds_alternative<Array>(enemiesValue.v));
        const auto& enemies = std::get<Array>(enemiesValue.v);
        REQUIRE(enemies.size() >= 1);
        REQUIRE(std::holds_alternative<Object>(enemies.front().v));
        const auto& enemyObject = std::get<Object>(enemies.front().v);
        REQUIRE(std::holds_alternative<std::string>(enemyObject.at("battlerName").v));
        REQUIRE(std::get<std::string>(enemyObject.at("battlerName").v) ==
                (sampleProjectAvailable ? "Goblin" : "Monster"));
    }

    SECTION("seeded enemy data also exposes battlerName strings") {
        DataManager::setDataDirectory("");
        DataManager dm;

        REQUIRE(dm.loadEnemies());
        const EnemyData* enemy = dm.getEnemy(2);
        REQUIRE(enemy != nullptr);
        REQUIRE(enemy->name == "Goblin");
        REQUIRE(enemy->battlerName == "Monster");

        auto enemiesValue = dm.getEnemiesAsValue();
        REQUIRE(std::holds_alternative<Array>(enemiesValue.v));
        const auto& enemies = std::get<Array>(enemiesValue.v);
        REQUIRE(enemies.size() >= 2);
        REQUIRE(std::holds_alternative<Object>(enemies[1].v));
        const auto& enemyObject = std::get<Object>(enemies[1].v);
        REQUIRE(std::holds_alternative<std::string>(enemyObject.at("battlerName").v));
        REQUIRE(std::get<std::string>(enemyObject.at("battlerName").v) == "Monster");
    }
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

TEST_CASE("DataManager: global state Value snapshot exposes party switches variables and inventory", "[data_manager]") {
    DataManager::setDataDirectory("");
    DataManager dm;
    dm.loadDatabase();
    dm.setupNewGame();
    dm.setGold(77);
    dm.setSwitch(2, true);
    dm.setVariable(3, 42);
    dm.setSelfSwitch(1, 9, "A", true);
    dm.gainItem(4, 5);
    dm.gainWeapon(6, 2);
    dm.gainArmor(8, 1);
    dm.setPlayerPosition(12, 3, 4);
    dm.setPlayerDirection(6);

    auto value = dm.getGlobalStateAsValue();
    REQUIRE(std::holds_alternative<Object>(value.v));
    const auto& root = std::get<Object>(value.v);

    REQUIRE(std::get<int64_t>(root.at("gold").v) == 77);
    REQUIRE(std::get<int64_t>(root.at("mapId").v) == 12);
    REQUIRE(std::get<int64_t>(root.at("playerX").v) == 3);
    REQUIRE(std::get<int64_t>(root.at("playerY").v) == 4);
    REQUIRE(std::get<int64_t>(root.at("playerDirection").v) == 6);

    const auto& party = std::get<Array>(root.at("partyMembers").v);
    REQUIRE_FALSE(party.empty());
    REQUIRE(std::get<int64_t>(party.front().v) == 1);

    const auto& switches = std::get<Array>(root.at("switches").v);
    REQUIRE(switches.size() >= 2);
    REQUIRE(std::get<bool>(switches[1].v));

    const auto& variables = std::get<Array>(root.at("variables").v);
    REQUIRE(variables.size() >= 3);
    REQUIRE(std::get<int64_t>(variables[2].v) == 42);

    const auto& selfSwitches = std::get<Object>(root.at("selfSwitches").v);
    REQUIRE(std::get<bool>(selfSwitches.at("1_9_A").v));

    REQUIRE(std::get<int64_t>(std::get<Object>(root.at("items").v).at("4").v) == 5);
    REQUIRE(std::get<int64_t>(std::get<Object>(root.at("weapons").v).at("6").v) == 2);
    REQUIRE(std::get<int64_t>(std::get<Object>(root.at("armors").v).at("8").v) == 1);
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

    REQUIRE(DataManager::getMethodStatus("loadDatabase") == CompatStatus::FULL);
    REQUIRE(DataManager::getMethodStatus("setupNewGame") == CompatStatus::FULL);
    REQUIRE(DataManager::getMethodStatus("setSaveHeaderExtension") == CompatStatus::FULL);
    REQUIRE(DataManager::getMethodDeviation("loadDatabase").empty());
    REQUIRE(DataManager::getMethodStatus("nonexistentMethod") == CompatStatus::UNSUPPORTED);
}

TEST_CASE("DataManager: database accessors as Value", "[data_manager]") {
    DataManager dm;
    DataManager::setDataDirectory(sampleProjectDataDirectory());
    REQUIRE(dm.loadDatabase());

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

    auto enemies = dm.getEnemiesAsValue();
    REQUIRE(std::holds_alternative<Array>(enemies.v));
    auto& enemyArr = std::get<Array>(enemies.v);
    REQUIRE(!enemyArr.empty());
    REQUIRE(std::holds_alternative<Object>(enemyArr.front().v));
    REQUIRE(std::holds_alternative<std::string>(std::get<Object>(enemyArr.front().v).at("battlerName").v));
    REQUIRE(std::get<std::string>(std::get<Object>(enemyArr.front().v).at("battlerName").v) == "Goblin");

    auto troops = dm.getTroopsAsValue();
    REQUIRE(std::holds_alternative<Array>(troops.v));
    auto& troopArr = std::get<Array>(troops.v);
    REQUIRE(!troopArr.empty());
    REQUIRE(std::holds_alternative<Object>(troopArr.front().v));
    const auto& troop = std::get<Object>(troopArr.front().v);
    REQUIRE(std::get<int64_t>(troop.at("id").v) > 0);
    REQUIRE(std::holds_alternative<std::string>(troop.at("name").v));
    REQUIRE(std::holds_alternative<Array>(troop.at("members").v));
    REQUIRE(!std::get<Array>(troop.at("members").v).empty());
    REQUIRE(std::holds_alternative<Object>(std::get<Array>(troop.at("members").v).front().v));
    REQUIRE(std::get<Object>(std::get<Array>(troop.at("members").v).front().v).contains("enemyId"));
    REQUIRE(troop.contains("pages"));

    auto states = dm.getStatesAsValue();
    REQUIRE(std::holds_alternative<Array>(states.v));
    auto& stateArr = std::get<Array>(states.v);
    REQUIRE(!stateArr.empty());
    REQUIRE(std::holds_alternative<Object>(stateArr.front().v));
    const auto& state = std::get<Object>(stateArr.front().v);
    REQUIRE(std::get<int64_t>(state.at("id").v) > 0);
    REQUIRE(std::holds_alternative<std::string>(state.at("name").v));
    REQUIRE(state.contains("iconIndex"));
    REQUIRE(state.contains("autoRemovalTiming"));

    REQUIRE(dm.loadMapInfos());
    auto mapInfos = dm.getMapInfosAsValue();
    REQUIRE(std::holds_alternative<Array>(mapInfos.v));
    auto& mapInfoArr = std::get<Array>(mapInfos.v);
    REQUIRE_FALSE(dm.getMapInfos().empty());
    REQUIRE(!mapInfoArr.empty());
    REQUIRE(std::holds_alternative<Object>(mapInfoArr.front().v));
    REQUIRE(std::get<std::string>(std::get<Object>(mapInfoArr.front().v).at("name").v) == "Debug Room");

    auto tilesets = dm.getTilesetsAsValue();
    REQUIRE(std::holds_alternative<Array>(tilesets.v));
    auto& tilesetArr = std::get<Array>(tilesets.v);
    REQUIRE(!tilesetArr.empty());
    REQUIRE(std::holds_alternative<Object>(tilesetArr.front().v));
    REQUIRE(std::get<int64_t>(std::get<Object>(tilesetArr.front().v).at("id").v) > 0);
}

TEST_CASE("DataManager: map data loads real MZ JSON when available", "[data_manager]") {
    DataManager dm;
    const auto dataDirectory = std::filesystem::path(sampleProjectDataDirectory());
    const bool sampleProjectAvailable = std::filesystem::exists(dataDirectory / "Map002.json");
    DataManager::setDataDirectory(dataDirectory.string());

    REQUIRE(dm.loadDatabase());
    REQUIRE(dm.loadMapData(2));

    const MapData* map = dm.getCurrentMap();
    REQUIRE(map != nullptr);
    REQUIRE(map->id == 2);
    if (!sampleProjectAvailable) {
        REQUIRE(map->width == 20);
        REQUIRE(map->height == 15);
        REQUIRE(map->tilesetId == 1);
        REQUIRE(map->data.size() == 6);
        REQUIRE(map->data[0].size() == static_cast<size_t>(20 * 15));
        return;
    }

    REQUIRE(map->width == 34);
    REQUIRE(map->height == 39);
    REQUIRE(map->tilesetId == 12);
    REQUIRE(map->data.size() == 6);
    REQUIRE(map->data[0].size() == static_cast<size_t>(34 * 39));

    auto mapValue = dm.getMapDataAsValue();
    REQUIRE(std::holds_alternative<Object>(mapValue.v));
    const auto& mapObject = std::get<Object>(mapValue.v);
    REQUIRE(std::get<int64_t>(mapObject.at("width").v) == 34);
    REQUIRE(std::get<int64_t>(mapObject.at("height").v) == 39);
    REQUIRE(std::get<int64_t>(mapObject.at("tilesetId").v) == 12);
    REQUIRE(std::holds_alternative<Array>(mapObject.at("data").v));
    const auto& layers = std::get<Array>(mapObject.at("data").v);
    REQUIRE(layers.size() == 6);
    REQUIRE(std::holds_alternative<Array>(layers.front().v));
    REQUIRE(std::get<Array>(layers.front().v).size() == static_cast<size_t>(34 * 39));
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(mapObject.at("events").v));
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

TEST_CASE("DataManager: gainExp triggers level-up and skill learning", "[data_manager]") {
    DataManager::setDataDirectory("");
    DataManager dm;
    dm.loadDatabase();
    dm.setupNewGame();

    ActorData* actor = dm.getActor(1);
    ClassData* cls = dm.getClass(1);
    REQUIRE(actor != nullptr);
    REQUIRE(cls != nullptr);

    actor->level = 1;
    actor->exp = 0;
    actor->skills.clear();
    cls->expTable = {15, 30, 60};
    cls->skillsToLearn = {{2, 1}, {3, 2}};
    cls->maxLevel = 99;

    dm.gainExp(1, 10);
    REQUIRE(actor->level == 1);
    REQUIRE(actor->exp == 10);

    dm.gainExp(1, 10);
    REQUIRE(actor->level == 2);
    REQUIRE(actor->exp == 5);
    REQUIRE(std::find(actor->skills.begin(), actor->skills.end(), 1) != actor->skills.end());

    dm.gainExp(1, 100);
    REQUIRE(actor->level == 4);
    REQUIRE(actor->exp == 15);
    REQUIRE(std::find(actor->skills.begin(), actor->skills.end(), 2) != actor->skills.end());

    cls->maxLevel = 4;
    dm.gainExp(1, 9999);
    REQUIRE(actor->level == 4);
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
