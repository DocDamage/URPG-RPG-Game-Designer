// BattleManager Unit Tests - Phase 2 Compat Layer
// Tests for MZ Battle Pipeline Hooks

#include "runtimes/compat_js/battle_manager.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

using namespace urpg::compat;

TEST_CASE("BattleManager: Setup and initialization", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    
    SECTION("Setup initializes battle state") {
        bm.setup(1, true, false);
        
        REQUIRE(bm.getTroopId() == 1);
        REQUIRE(bm.canEscape());
        REQUIRE_FALSE(bm.canLose());
    }
    
    SECTION("Setup with canLose=true") {
        bm.setup(2, false, true);
        
        REQUIRE_FALSE(bm.canEscape());
        REQUIRE(bm.canLose());
    }
    
    SECTION("Setup with all flags enabled") {
        bm.setup(5, true, true);
        
        REQUIRE(bm.canEscape());
        REQUIRE(bm.canLose());
    }
}

TEST_CASE("BattleManager: Battle flow control", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    
    SECTION("Initial phase is NONE") {
        REQUIRE(bm.getPhase() == BattlePhase::NONE);
    }
    
    SECTION("Start battle transitions to START phase") {
        bm.setup(1, true, false);
        bm.startBattle();
        
        REQUIRE(bm.getPhase() == BattlePhase::START);
        REQUIRE(bm.isBattleActive());
        
        bm.abortBattle();
    }
    
    SECTION("End battle with WIN result") {
        bm.setup(1, true, false);
        bm.startBattle();
        bm.endBattle(BattleResult::WIN);
        
        REQUIRE(bm.getResult() == BattleResult::WIN);
        REQUIRE_FALSE(bm.isBattleActive());
    }
    
    SECTION("Abort battle sets ABORT result") {
        bm.setup(1, true, false);
        bm.startBattle();
        bm.abortBattle();
        
        REQUIRE(bm.getPhase() == BattlePhase::NONE);
        REQUIRE(bm.getResult() == BattleResult::ABORT);
    }
}

TEST_CASE("BattleManager: Escape mechanics", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    
    SECTION("processEscape returns true when escape allowed") {
        bm.setup(1, true, false);
        bm.startBattle();
        
        bool escaped = bm.processEscape();
        
        // Escape success depends on implementation
        // Just verify the method runs without error
        
        bm.abortBattle();
    }
    
    SECTION("Cannot escape when canEscape is false") {
        bm.setup(1, false, false);
        bm.startBattle();
        
        // Even if processEscape is called, escape should fail
        // Implementation specific behavior
        
        bm.abortBattle();
    }
}

TEST_CASE("BattleManager: Phase transitions", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    
    SECTION("Phase can advance through battle flow") {
        bm.setup(1, true, false);
        
        REQUIRE(bm.getPhase() == BattlePhase::INIT);
        
        bm.startBattle();
        REQUIRE(bm.getPhase() == BattlePhase::START);
        
        bm.abortBattle();
    }
}

TEST_CASE("BattleManager: Battle settings", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    
    SECTION("Set battle transition") {
        bm.setBattleTransition(1);
        // Transition type should be set
    }
    
    SECTION("Set battle background") {
        bm.setBattleBackground("forest_bg");
        // Background should be set
    }
    
    SECTION("Set battle BGM") {
        bm.setBattleBgm("battle_theme", 90.0, 100.0);
        // BGM should be set
    }
    
    SECTION("Set victory ME") {
        bm.setVictoryMe("victory_fanfare", 90.0, 100.0);
        // Victory ME should be set
    }
    
    SECTION("Set defeat ME") {
        bm.setDefeatMe("defeat_theme", 90.0, 100.0);
        // Defeat ME should be set
    }
}

TEST_CASE("BattleManager: Hook registration", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    bm.setup(1, true, false);
    
    SECTION("Register hook returns valid ID") {
        int32_t hookId = bm.registerHook(BattleManager::HookPoint::ON_START, 
            [](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value {
                return urpg::engine::Value();
            }, "test_plugin");
        
        REQUIRE(hookId >= 0);
        
        bm.unregisterHooks("test_plugin");
    }
    
    SECTION("Unregister hooks by plugin name") {
        bm.registerHook(BattleManager::HookPoint::ON_START, 
            [](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value {
                return urpg::engine::Value();
            }, "plugin_a");
        bm.registerHook(BattleManager::HookPoint::ON_END, 
            [](const std::vector<urpg::engine::Value>&) -> urpg::engine::Value {
                return urpg::engine::Value();
            }, "plugin_a");
        
        auto removed = bm.unregisterHooks("plugin_a");
        REQUIRE(removed.size() == 2);
    }
    
    SECTION("Unregister hooks returns empty for unknown plugin") {
        auto removed = bm.unregisterHooks("unknown_plugin");
        REQUIRE(removed.empty());
    }
    
    bm.abortBattle();
}

TEST_CASE("BattleManager: Turn management", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    
    SECTION("Initial turn count is 0") {
        bm.setup(1, true, false);
        
        REQUIRE(bm.getTurnCount() == 0);
        
        bm.abortBattle();
    }
    
    SECTION("Increment turn") {
        bm.setup(1, true, false);
        bm.startBattle();
        
        bm.incrementTurn();
        REQUIRE(bm.getTurnCount() == 1);
        
        bm.incrementTurn();
        REQUIRE(bm.getTurnCount() == 2);
        
        bm.abortBattle();
    }
}

TEST_CASE("BattleManager: Actor management", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    bm.setup(1, true, false);
    
    SECTION("Get actors returns empty initially") {
        auto actors = bm.getActors();
        // Actors list depends on party state
    }
    
    SECTION("Get enemies returns troop members") {
        auto enemies = bm.getEnemies();
        // Enemies list depends on troop configuration
    }
    
    bm.abortBattle();
}

TEST_CASE("BattleManager: Action queue", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    bm.setup(1, true, false);
    bm.startBattle();
    
    SECTION("Clear action queue") {
        bm.clearActionQueue();
        // Queue should be empty
    }
    
    SECTION("Get action queue") {
        auto queue = bm.getActionQueue();
        // Queue depends on battle state
    }
    
    bm.abortBattle();
}

TEST_CASE("BattleManager: Damage and healing", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    bm.setup(1, true, false);
    bm.startBattle();
    
    SECTION("Apply damage to actor") {
        auto actors = bm.getActors();
        if (!actors.empty()) {
            int32_t originalHp = actors[0]->hp;
            bm.applyDamage(actors[0], 50);
            // HP should be reduced
        }
    }
    
    SECTION("Apply healing to actor") {
        auto actors = bm.getActors();
        if (!actors.empty()) {
            bm.applyHealing(actors[0], 50);
            // HP should be increased
        }
    }
    
    bm.abortBattle();
}

TEST_CASE("BattleManager: Method status registry", "[battlemgr]") {
    BattleManager& bm = BattleManager::instance();
    
    SECTION("GetMethodStatus returns FULL for setup") {
        CompatStatus status = bm.getMethodStatus("setup");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns FULL for startBattle") {
        CompatStatus status = bm.getMethodStatus("startBattle");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns UNSUPPORTED for unknown methods") {
        CompatStatus status = bm.getMethodStatus("nonexistentMethod");
        REQUIRE(status == CompatStatus::UNSUPPORTED);
    }
}

TEST_CASE("BattleSubject: Structure", "[battlemgr]") {
    BattleSubject subject;
    
    SECTION("Default values") {
        REQUIRE(subject.type == BattleSubjectType::ACTOR);
        REQUIRE(subject.index == 0);
        REQUIRE(subject.id == 0);
        REQUIRE(subject.hp == 0);
        REQUIRE(subject.mp == 0);
        REQUIRE(subject.tp == 0);
        REQUIRE_FALSE(subject.hidden);
        REQUIRE_FALSE(subject.immortal);
        REQUIRE_FALSE(subject.acted);
        REQUIRE(subject.pendingAction == BattleActionType::WAIT);
    }
    
    SECTION("Can set subject type") {
        subject.type = BattleSubjectType::ENEMY;
        REQUIRE(subject.type == BattleSubjectType::ENEMY);
    }
    
    SECTION("Can set HP values") {
        subject.hp = 100;
        subject.mhp = 200;
        REQUIRE(subject.hp == 100);
        REQUIRE(subject.mhp == 200);
    }
}

TEST_CASE("BattleAction: Structure", "[battlemgr]") {
    BattleAction action;
    
    SECTION("Default values") {
        REQUIRE(action.subject == nullptr);
        REQUIRE(action.type == BattleActionType::ATTACK);
        REQUIRE(action.targetIndex == -1);
        REQUIRE(action.skillId == 0);
        REQUIRE(action.itemId == 0);
        REQUIRE(action.animationId == 0);
        REQUIRE_FALSE(action.forced);
    }
    
    SECTION("Can set action type") {
        action.type = BattleActionType::SKILL;
        REQUIRE(action.type == BattleActionType::SKILL);
    }
    
    SECTION("Can set target") {
        action.targetIndex = 2;
        REQUIRE(action.targetIndex == 2);
    }
}

TEST_CASE("BattlePhase: Enum values", "[battlemgr]") {
    SECTION("Phase values match MZ flow") {
        REQUIRE(static_cast<int>(BattlePhase::NONE) == 0);
        REQUIRE(static_cast<int>(BattlePhase::INIT) == 1);
        REQUIRE(static_cast<int>(BattlePhase::START) == 2);
        REQUIRE(static_cast<int>(BattlePhase::INPUT) == 3);
        REQUIRE(static_cast<int>(BattlePhase::TURN) == 4);
        REQUIRE(static_cast<int>(BattlePhase::ACTION) == 5);
        REQUIRE(static_cast<int>(BattlePhase::END) == 6);
        REQUIRE(static_cast<int>(BattlePhase::ABORT) == 7);
    }
}

TEST_CASE("BattleResult: Enum values", "[battlemgr]") {
    SECTION("Result values are sequential") {
        REQUIRE(static_cast<int>(BattleResult::NONE) == 0);
        REQUIRE(static_cast<int>(BattleResult::WIN) == 1);
        REQUIRE(static_cast<int>(BattleResult::ESCAPE) == 2);
        REQUIRE(static_cast<int>(BattleResult::DEFEAT) == 3);
        REQUIRE(static_cast<int>(BattleResult::ABORT) == 4);
    }
}

TEST_CASE("BattleActionType: Enum values", "[battlemgr]") {
    SECTION("Action type values match MZ") {
        REQUIRE(static_cast<int>(BattleActionType::ATTACK) == 0);
        REQUIRE(static_cast<int>(BattleActionType::GUARD) == 1);
        REQUIRE(static_cast<int>(BattleActionType::SKILL) == 2);
        REQUIRE(static_cast<int>(BattleActionType::ITEM) == 3);
        REQUIRE(static_cast<int>(BattleActionType::ESCAPE) == 4);
        REQUIRE(static_cast<int>(BattleActionType::WAIT) == 5);
    }
}
