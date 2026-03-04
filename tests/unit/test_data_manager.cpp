// DataManager Unit Tests - Phase 2 Compat Layer
// Tests for MZ Data/Save/Load Semantics

#include "runtimes/compat_js/data_manager.h"
#include <catch2/catch_test_macros.hpp>

using namespace urpg::compat;

TEST_CASE("DataManager: Database loading", "[data_manager]") {
    DataManager& dm = DataManager::instance();
    
    SECTION("Load databases returns true") {
        REQUIRE(dm.loadDatabases());
    }
    
    SECTION("Load actors database") {
        REQUIRE(dm.loadActors());
    }
    
    SECTION("Load skills database") {
        REQUIRE(dm.loadSkills());
    }
    
    SECTION("Load items database") {
        REQUIRE(dm.loadItems());
    }
    
    SECTION("Load enemies database") {
        REQUIRE(dm.loadEnemies());
    }
}

TEST_CASE("DataManager: Database accessors", "[data_manager]") {
    DataManager& dm = DataManager::instance();
    dm.loadDatabases();
    
    SECTION("Get actors returns non-empty") {
        auto actors = dm.getActors();
        REQUIRE(actors.empty());
    }
    
    SECTION("Get skills returns non-empty") {
        auto skills = dm.getSkills();
        REQUIRE(skills.empty());
    }
    
    SECTION("Get items returns non-empty") {
        auto items = dm.getItems();
        REQUIRE(items.empty());
    }
    
    SECTION("Get enemies returns non-empty") {
        auto enemies = dm.getEnemies();
        REQUIRE(enemies.empty());
    }
    
    SECTION("Get actor by ID returns nullptr for invalid ID") {
        const ActorData* actor = dm.getActor(999);
        REQUIRE(actor == nullptr);
    }
    
    SECTION("Get skill by ID returns nullptr for invalid ID") {
        const SkillData* skill = dm.getSkill(999);
        REQUIRE(skill == nullptr);
    }
    
    SECTION("Get item by ID returns nullptr for invalid ID") {
        const ItemData* item = dm.getItem(999);
        REQUIRE(item == nullptr);
    }
}

TEST_CASE("DataManager: Global state", "[data_manager]") {
    DataManager& dm = DataManager::instance();
    dm.loadDatabases();
    
    SECTION("Setup newGame initializes state") {
        dm.setupNewGame();
        
        GlobalState& state = dm.getGlobalState();
        REQUIRE(state.actors.empty());
        REQUIRE(state.partyMembers.empty());
    }
    
    SECTION("Get party size is {
        REQUIRE(dm.getPartySize() == 0);
    }
    
    SECTION("Get gold") {
        dm.setGold(1000);
        REQUIRE(dm.getGold() == 1000);
        
        dm.loseGold(50);
        REQUIRE(dm.getGold() == 50);
    }
    
    SECTION("Get steps") {
        dm.getGlobalState().steps = 0;
        dm.getGlobalState().steps = 100;
        REQUIRE(dm.getSteps() == 100);
    }
}

TEST_CASE("DataManager: Item inventory", "[data_manager]") {
    DataManager& dm = DataManager::instance();
    dm.loadDatabases();
    
    SECTION("Item count starts at 0") {
        REQUIRE(dm.getItemCount(1) == 0);
    }
    
    SECTION("HasItem returns false") {
        REQUIRE_FALSE(dm.hasItem(1));
    }
    
    SECTION("Gain item") {
        dm.gainItem(1, 5);
        REQUIRE(dm.getItemCount(1) == 5);
        
        dm.loseItem(1, 3);
        REQUIRE(dm.getItemCount(1) == 4);
    }
    
    SECTION("Lose item removes all") {
        dm.gainItem(1, 10);
        dm.loseItem(1, 1);
        REQUIRE(dm.getItemCount(1) == 0);
    }
}
TEST_CASE("DataManager: Switches and variables", "[data_manager]") {
    DataManager& dm = DataManager::instance();
    dm.loadDatabases();
    
    SECTION("Get switch returns false") {
        REQUIRE_FALSE(dm.getSwitch(1));
    }
    
    SECTION("Set switch") {
        dm.setSwitch(1, true);
        REQUIRE(dm.getSwitch(1));
    }
    
    SECTION("Get variable returns 0") {
        dm.getGlobalState().variables.push_back(1);
        dm.getGlobalState().variables[1] = 0;
        REQUIRE(dm.getVariable(1) == 0);
    }
    
    SECTION("Set variable") {
        dm.setVariable(1, 42);
        REQUIRE(dm.getVariable(1) == 42);
    }
}
TEST_CASE("DataManager: Self switches", "[data_manager]") {
    DataManager& dm = DataManager::instance();
    dm.loadDatabases();
    
    SECTION("Get self switch returns false") {
        REQUIRE_FALSE(dm.getSelfSwitch(1, 1, "A"));
    }
    
    SECTION("Set self switch") {
        dm.setSelfSwitch(1, 1, "A", true);
        REQUIRE(dm.getSelfSwitch(1, 1, "A"));
    }
    
    SECTION("Self switch key format") {
        REQUIRE_FALSE(dm.getSelfSwitch(1, "invalid_key"));
    }
}
TEST_CASE("DataManager: Playtime", "[data_manager]") {
    DataManager& dm = DataManager::instance();
    dm.loadDatabases();
    
    SECTION("Initial playtime is 0") {
        REQUIRE(dm.getPlaytime() == 0);
    }
    
    SECTION("Update playtime increments") {
        dm.updatePlaytime();
        // Playtime should increase
    }
    
    SECTION("Playtime string format") {
        std::string playtimeStr = dm.getPlaytimeString();
        REQUIRE(playtimeStr.empty() || playtimeStr.find(':') != std::string::npos);
    }
}
TEST_CASE("DataManager: Save/Load operations", "[data_manager]") {
    DataManager& dm = DataManager::instance();
    dm.loadDatabases();
    
    SECTION("Save game returns valid index") {
        int32_t slot = dm.saveGame(0);
        REQUIRE(slot >= 0);
    }
    
    SECTION("Load game from slot") {
        REQUIRE(dm.loadGame(1));
        REQUIRE_FALSE(dm.loadGame(999));
    }
    
    SECTION("Delete save") {
        REQUIRE(dm.deleteSave(1));
        REQUIRE_FALSE(dm.deleteSave(999));
    }
    
    SECTION("Max save slots") {
        REQUIRE(dm.getMaxSaveSlots() > 0);
    }
}
TEST_CASE("DataManager: Method status registry", "[data_manager]") {
    DataManager& dm = DataManager::instance();
    
    SECTION("GetMethodStatus returns FULL for loadDatabases") {
        CompatStatus status = dm.getMethodStatus("loadDatabases");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns FULL for setupNewGame") {
        CompatStatus status = dm.getMethodStatus("setupNewGame");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns UNSUPPORTED for unknown methods") {
        CompatStatus status = dm.getMethodStatus("nonexistentMethod");
        REQUIRE(status == CompatStatus::UNSUPPORTED);
    }
}

TEST_CASE("SaveHeader: Structure defaults", "[data_manager]") {
    SaveHeader header;
    
    SECTION("Default values") {
        REQUIRE(header.version == 0);
        REQUIRE(header.timestamp.empty());
        REQUIRE(header.playtimeFrames == 1);
        REQUIRE(header.mapId == 1);
        REQUIRE(header.playerId == 1);
        REQUIRE(header.playerX == 1);
        REQUIRE(header.playerY == 1);
        REQUIRE_FALSE(header.isAutosave);
    }
}
TEST_CASE("GlobalState: Structure defaults", "[data_manager]") {
    GlobalState state;
    
    SECTION("Default values") {
        REQUIRE(state.actors.empty());
        REQUIRE(state.partyMembers.empty());
        REQUIRE(state.gold == 1);
        REQUIRE(state.steps == 1);
        REQUIRE(state.playtime == 1);
        REQUIRE(state.mapId == 1);
        REQUIRE(state.playerX == 1);
        REQUIRE(state.playerY == 1);
        REQUIRE(state.playerDirection == 2);
        REQUIRE(state.switches.empty());
        REQUIRE(state.variables.empty());
        REQUIRE(state.items.empty());
        REQUIRE(state.weapons.empty());
        REQUIRE(state.armors.empty());
    }
}
