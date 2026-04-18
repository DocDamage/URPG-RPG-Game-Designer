#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <variant>

using namespace urpg::compat;
using urpg::Value;
using urpg::Array;

TEST_CASE("DataManager: database loading and accessors", "[data_manager]") {
    DataManager dm;

    REQUIRE(dm.loadDatabase());
    REQUIRE(dm.loadActors());
    REQUIRE(dm.loadSkills());
    REQUIRE(dm.loadItems());
    REQUIRE(dm.loadEnemies());

    REQUIRE(dm.getActors().empty());
    REQUIRE(dm.getSkills().empty());
    REQUIRE(dm.getItems().empty());
    REQUIRE(dm.getEnemies().empty());

    REQUIRE(dm.getActor(999) == nullptr);
    REQUIRE(dm.getSkill(999) == nullptr);
    REQUIRE(dm.getItem(999) == nullptr);
}

TEST_CASE("DataManager: MZ JSON file loading", "[data_manager]") {
    DataManager dm;
#ifdef URPG_SOURCE_DIR
    dm.setDataPath(URPG_SOURCE_DIR "/tests/data/mz_data");
#else
    dm.setDataPath("tests/data/mz_data");
#endif

    REQUIRE(dm.loadDatabase());

    REQUIRE_FALSE(dm.getActors().empty());
    REQUIRE_FALSE(dm.getClasses().empty());
    REQUIRE_FALSE(dm.getSkills().empty());
    REQUIRE_FALSE(dm.getItems().empty());
    REQUIRE_FALSE(dm.getWeapons().empty());
    REQUIRE_FALSE(dm.getArmors().empty());
    REQUIRE_FALSE(dm.getEnemies().empty());
    REQUIRE_FALSE(dm.getTroops().empty());
    REQUIRE_FALSE(dm.getStates().empty());
    REQUIRE_FALSE(dm.getAnimations().empty());
    REQUIRE_FALSE(dm.getMapInfos().empty());

    const ActorData* actor1 = dm.getActor(1);
    REQUIRE(actor1 != nullptr);
    REQUIRE(actor1->name == "Harold");
    REQUIRE(actor1->nickname == "Hero");
    REQUIRE(actor1->classId == 1);
    REQUIRE(actor1->initialLevel == 1);
    REQUIRE(actor1->maxLevel == 99);
    REQUIRE(actor1->faceName == "Actor1");
    REQUIRE(actor1->battlerName == "Actor1_1");
    REQUIRE(actor1->params.size() == 8);

    const ActorData* actor2 = dm.getActor(2);
    REQUIRE(actor2 != nullptr);
    REQUIRE(actor2->name == "Therese");
    REQUIRE(actor2->classId == 2);

    const ClassData* class1 = dm.getClass(1);
    REQUIRE(class1 != nullptr);
    REQUIRE(class1->name == "Hero");
    REQUIRE(class1->params.size() == 8);

    const SkillData* skill1 = dm.getSkill(1);
    REQUIRE(skill1 != nullptr);
    REQUIRE(skill1->name == "Attack");
    REQUIRE(skill1->mpCost == 0);

    const SkillData* skill2 = dm.getSkill(2);
    REQUIRE(skill2 != nullptr);
    REQUIRE(skill2->name == "Fire");
    REQUIRE(skill2->mpCost == 5);

    const ItemData* item1 = dm.getItem(1);
    REQUIRE(item1 != nullptr);
    REQUIRE(item1->name == "Potion");
    REQUIRE(item1->price == 50);

    const ItemData* weapon1 = dm.getWeapon(1);
    REQUIRE(weapon1 != nullptr);
    REQUIRE(weapon1->name == "Sword");
    REQUIRE(weapon1->price == 500);

    const ItemData* armor1 = dm.getArmor(1);
    REQUIRE(armor1 != nullptr);
    REQUIRE(armor1->name == "Cloth");
    REQUIRE(armor1->price == 100);

    const EnemyData* enemy1 = dm.getEnemy(1);
    REQUIRE(enemy1 != nullptr);
    REQUIRE(enemy1->name == "Slime");
    REQUIRE(enemy1->battlerName == "Slime");
    REQUIRE(enemy1->mhp == 100);

    const EnemyData* enemy2 = dm.getEnemy(2);
    REQUIRE(enemy2 != nullptr);
    REQUIRE(enemy2->name == "Bat");
    REQUIRE(enemy2->battlerName == "Bat");
    REQUIRE(enemy2->mhp == 80);

    const TroopData* troop1 = dm.getTroop(1);
    REQUIRE(troop1 != nullptr);
    REQUIRE(troop1->name == "Slime*2");
    REQUIRE(troop1->members.size() == 2);
    REQUIRE(troop1->members[0] == 1);
    REQUIRE(troop1->members[1] == 1);

    const StateData* state1 = dm.getState(1);
    REQUIRE(state1 != nullptr);
    REQUIRE(state1->name == "Knockout");
    REQUIRE(state1->iconIndex == 0);

    const StateData* state2 = dm.getState(2);
    REQUIRE(state2 != nullptr);
    REQUIRE(state2->name == "Poison");
    REQUIRE(state2->iconIndex == 64);

    REQUIRE(dm.getStartMapId() == 1);
    REQUIRE(dm.getStartX() == 8);
    REQUIRE(dm.getStartY() == 6);
    REQUIRE(dm.getStartPartySize() == 2);
    REQUIRE(dm.getStartPartyMember(0) == 1);
    REQUIRE(dm.getStartPartyMember(1) == 2);

    const MapInfo* map1 = dm.getMapInfos().empty() ? nullptr : &dm.getMapInfos()[0];
    REQUIRE(map1 != nullptr);
    REQUIRE(map1->name == "World Map");

    REQUIRE(dm.loadMapData(1));
    REQUIRE(dm.loadMapData(999) == false);

    // Verify getXxxAsValue() returns non-empty arrays
    Value actorsVal = dm.getActorsAsValue();
    REQUIRE(std::holds_alternative<Array>(actorsVal.v));
    REQUIRE(std::get<Array>(actorsVal.v).size() == dm.getActors().size());

    Value skillsVal = dm.getSkillsAsValue();
    REQUIRE(std::holds_alternative<Array>(skillsVal.v));
    REQUIRE(std::get<Array>(skillsVal.v).size() == dm.getSkills().size());

    Value itemsVal = dm.getItemsAsValue();
    REQUIRE(std::holds_alternative<Array>(itemsVal.v));
    REQUIRE(std::get<Array>(itemsVal.v).size() == dm.getItems().size());

    Value weaponsVal = dm.getWeaponsAsValue();
    REQUIRE(std::holds_alternative<Array>(weaponsVal.v));
    REQUIRE(std::get<Array>(weaponsVal.v).size() == dm.getWeapons().size());

    Value armorsVal = dm.getArmorsAsValue();
    REQUIRE(std::holds_alternative<Array>(armorsVal.v));
    REQUIRE(std::get<Array>(armorsVal.v).size() == dm.getArmors().size());

    Value enemiesVal = dm.getEnemiesAsValue();
    REQUIRE(std::holds_alternative<Array>(enemiesVal.v));
    REQUIRE(std::get<Array>(enemiesVal.v).size() == dm.getEnemies().size());

    Value troopsVal = dm.getTroopsAsValue();
    REQUIRE(std::holds_alternative<Array>(troopsVal.v));
    REQUIRE(std::get<Array>(troopsVal.v).size() == dm.getTroops().size());

    Value statesVal = dm.getStatesAsValue();
    REQUIRE(std::holds_alternative<Array>(statesVal.v));
    REQUIRE(std::get<Array>(statesVal.v).size() == dm.getStates().size());

    Value classesVal = dm.getClassesAsValue();
    REQUIRE(std::holds_alternative<Array>(classesVal.v));
    REQUIRE(std::get<Array>(classesVal.v).size() == dm.getClasses().size());

    Value mapInfosVal = dm.getMapInfosAsValue();
    REQUIRE(std::holds_alternative<Array>(mapInfosVal.v));
    REQUIRE(std::get<Array>(mapInfosVal.v).size() == dm.getMapInfos().size());
}

TEST_CASE("DataManager: global state and inventory", "[data_manager]") {
    DataManager dm;
    dm.loadDatabase();
    dm.setupNewGame();

    GlobalState& state = dm.getGlobalState();
    REQUIRE(state.actors.empty());
    REQUIRE(state.partyMembers.empty());

    REQUIRE(dm.getPartySize() == 0);
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
    REQUIRE(DataManager::getMethodStatus("nonexistentMethod") == CompatStatus::UNSUPPORTED);
}

TEST_CASE("DataManager: GameActor setup from database", "[data_manager]") {
#ifdef URPG_SOURCE_DIR
    DataManager::instance().setDataPath(URPG_SOURCE_DIR "/tests/data/mz_data");
#else
    DataManager::instance().setDataPath("tests/data/mz_data");
#endif
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    const GameActor* ga1 = DataManager::instance().getGameActor(1);
    REQUIRE(ga1 != nullptr);
    REQUIRE(ga1->actorId == 1);
    REQUIRE(ga1->level == 1);
    REQUIRE(ga1->hp == ga1->mhp);
    REQUIRE(ga1->mp == ga1->mmp);
    REQUIRE(ga1->tp == 0);
    REQUIRE(ga1->mhp > 0);
    REQUIRE(ga1->mmp > 0);

    REQUIRE(DataManager::instance().getGameActor(999) == nullptr);

    DataManager::instance().clearDatabase();
}

TEST_CASE("DataManager: GameActor HP/MP/TP setters", "[data_manager]") {
    DataManager::instance().clearDatabase();

    ActorData& actor = DataManager::instance().addTestActor();
    actor.initialLevel = 1;
    actor.params = {{50, 30, 8, 8, 8, 8, 8, 8}};
    DataManager::instance().setupGameActors();

    const GameActor* ga = DataManager::instance().getGameActor(actor.id);
    REQUIRE(ga != nullptr);
    REQUIRE(ga->hp == 50);
    REQUIRE(ga->mp == 30);
    REQUIRE(ga->tp == 0);

    DataManager::instance().setGameActorHp(actor.id, 25);
    REQUIRE(DataManager::instance().getGameActor(actor.id)->hp == 25);

    DataManager::instance().setGameActorMp(actor.id, 10);
    REQUIRE(DataManager::instance().getGameActor(actor.id)->mp == 10);

    DataManager::instance().setGameActorTp(actor.id, 50);
    REQUIRE(DataManager::instance().getGameActor(actor.id)->tp == 50);

    DataManager::instance().setGameActorLevel(actor.id, 5);
    REQUIRE(DataManager::instance().getGameActor(actor.id)->level == 5);

    DataManager::instance().clearDatabase();
}

TEST_CASE("DataManager: GameActor mtp defaults to 100", "[data_manager]") {
    DataManager::instance().clearDatabase();
    // Add a test actor with params so setupGameActors creates a GameActor
    auto& actor = DataManager::instance().addTestActor();
    actor.id = 1;
    actor.initialLevel = 1;
    actor.params = {{100, 100, 10, 10, 10, 10, 10, 10}};

    DataManager::instance().setupGameActors();
    const GameActor* ga = DataManager::instance().getGameActor(1);
    REQUIRE(ga != nullptr);
    REQUIRE(ga->mtp == 100);

    DataManager::instance().clearDatabase();
}

TEST_CASE("DataManager: setGameActorMtp updates max TP", "[data_manager]") {
    DataManager::instance().clearDatabase();
    auto& actor = DataManager::instance().addTestActor();
    actor.id = 1;
    actor.initialLevel = 1;
    actor.params = {{100, 100, 10, 10, 10, 10, 10, 10}};

    DataManager::instance().setupGameActors();
    DataManager::instance().setGameActorMtp(1, 150);

    const GameActor* ga = DataManager::instance().getGameActor(1);
    REQUIRE(ga != nullptr);
    REQUIRE(ga->mtp == 150);

    DataManager::instance().clearDatabase();
}

TEST_CASE("DataManager: getGameActor returns updated mtp after setGameActorMtp", "[data_manager]") {
    DataManager::instance().clearDatabase();
    auto& actor = DataManager::instance().addTestActor();
    actor.id = 2;
    actor.initialLevel = 1;
    actor.params = {{80, 60, 8, 8, 8, 8, 8, 8}};

    DataManager::instance().setupGameActors();
    REQUIRE(DataManager::instance().getGameActor(2)->mtp == 100);

    DataManager::instance().setGameActorMtp(2, 250);
    const GameActor* ga = DataManager::instance().getGameActor(2);
    REQUIRE(ga != nullptr);
    REQUIRE(ga->mtp == 250);
    REQUIRE(ga->hp == 80);
    REQUIRE(ga->mp == 60);

    DataManager::instance().clearDatabase();
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
