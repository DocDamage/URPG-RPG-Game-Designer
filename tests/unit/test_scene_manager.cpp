#include <catch2/catch_test_macros.hpp>
#include "engine/core/scene/scene_manager.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/map_loader.h"
#include "engine/core/scene/tileset_registry.h"
#include "engine/core/scene/movement_authority.h"
#include "engine/core/audio/audio_core.h"

using namespace urpg::scene;

TEST_CASE("MovementAuthority: Native Grid Transitions", "[scene][movement]") {
    urpg::MovementComponent m;
    m.gridPos = { 5, 5 };
    m.lastGridPos = { 5, 5 };
    m.moveSpeed = 4.0f; // 4 tiles per second
    m.isMoving = false;

    SECTION("TryMove respects collision") {
        auto checkCollision = [](int x, int y) {
            return x == 5 && y == 6; // Wall on the bottom
        };

        // Try move down (into wall)
        bool result = urpg::MovementSystem::TryMove(m, urpg::Direction::Down, checkCollision);
        REQUIRE_FALSE(result);
        REQUIRE(m.gridPos.y == 5);
        REQUIRE_FALSE(m.isMoving);

        // Try move left (passable)
        result = urpg::MovementSystem::TryMove(m, urpg::Direction::Left, checkCollision);
        REQUIRE(result);
        REQUIRE(m.gridPos.x == 4);
        REQUIRE(m.isMoving);
    }

    SECTION("Update handles progress and completion") {
        m.gridPos = { 4, 5 };
        m.isMoving = true;
        m.moveProgress = 0.0f;

        // Update 0.1s later
        urpg::MovementSystem::Update(m, 0.1f);
        REQUIRE(m.isMoving);
        REQUIRE(m.moveProgress > 0.0f);

        // Update to completion (0.2s more, total 0.3s)
        // With speed 4, 1.0f progress requires 0.25s
        bool done = urpg::MovementSystem::Update(m, 0.2f);
        REQUIRE(done);
        REQUIRE_FALSE(m.isMoving);
    }
}

TEST_CASE("MovementAuthority: Input Integration", "[scene][movement][input]") {
    MapScene map("001", 10, 10);
    urpg::input::InputCore input;
    input.mapKey(1, urpg::input::InputAction::MoveRight); // Mock key 1 = Right

    SECTION("Input triggers movement in MapScene") {
        // Press key 1 (MoveRight)
        input.processKeyEvent(1, urpg::input::ActionState::Pressed);
        
        // Before update, player is at (0,0)
        REQUIRE(map.getPlayerMovement().gridPos.x == 0);
        
        // Process input and transition
        map.handleInput(input);
        REQUIRE(map.getPlayerMovement().isMoving);
        REQUIRE(map.getPlayerMovement().gridPos.x == 1);
        REQUIRE(map.getPlayerMovement().direction == urpg::Direction::Right);
    }
}

TEST_CASE("MapLoader: Bridge DataManager to MapScene", "[scene][map][loader]") {
    // 1. Setup tileset collision in the registry
    urpg::TilesetData ts;
    ts.id = 1;
    ts.name = "Outdoor";
    ts.flags = { urpg::TileFlag::Passable, urpg::TileFlag::FullImpassable }; // 0=Floor, 1=Wall
    urpg::TilesetRegistry::instance().registerTileset(ts);

    // 2. Load map through the loader
    auto nativeMap = urpg::MapLoader::LoadToNative(1);

    REQUIRE(nativeMap != nullptr);
    REQUIRE(nativeMap->getName() == "Map_1");
    REQUIRE(nativeMap->getWidth() == 20);
    REQUIRE(nativeMap->getHeight() == 15);

    SECTION("Coordinate integrity after loading (using TilesetRegistry)") {
        // According to our mock loader and registry, (1, 1) is floor (tile 0) = passable.
        // Boundary tiles (0,0 etc) are walls (tile 1) = impassable.
        REQUIRE_FALSE(nativeMap->checkCollision(1, 1));
        REQUIRE(nativeMap->checkCollision(0, 0));
        REQUIRE(nativeMap->checkCollision(19, 14));
    }
}

/* Deprecated BattleScene tests moved to test_battle_scene.cpp */
#if 0
TEST_CASE("BattleScene: Phase and Turn Authority", "[scene][battle]") {
    BattleScene battle({"slime_01", "slime_02"});
    
    SECTION("Initial battle state") {
        REQUIRE(battle.getType() == SceneType::BATTLE);
        REQUIRE(battle.getEnemies().size() == 2);
    }

    SECTION("Phase transitions") {
        battle.onStart();
        REQUIRE(battle.getCurrentPhase() == BattlePhase::START);
        REQUIRE(battle.getTurnCount() == 1);

        battle.setPhase(BattlePhase::INPUT);
        REQUIRE(battle.getCurrentPhase() == BattlePhase::INPUT);

        battle.nextTurn();
        REQUIRE(battle.getTurnCount() == 2);
    }
}
#endif

TEST_CASE("MapScene: Coordinate and Collision Authority", "[scene][map]") {
    MapScene map("001", 10, 10);
    
    SECTION("Initial boundary checks") {
        REQUIRE(map.checkCollision(-1, 0) == true);
        REQUIRE(map.checkCollision(10, 0) == true);
        REQUIRE(map.checkCollision(0, -1) == true);
        REQUIRE(map.checkCollision(0, 10) == true);
    }

    SECTION("Tile data affects collision") {
        map.setTile(5, 5, 1, false); // Wall
        REQUIRE(map.checkCollision(5, 5) == true);

        map.setTile(5, 5, 2, true);  // Floor
        REQUIRE(map.checkCollision(5, 5) == false);
    }
}

TEST_CASE("MapScene: AI audio commands use injected AudioCore service", "[scene][map][audio]") {
    MapScene map("001", 10, 10);
    auto audio = std::make_shared<urpg::audio::AudioCore>();
    map.setAudioCore(audio);

    map.processAiAudioCommands("[ACTION: CROSSFADE, ASSET: Boss_Dark, VOL: 0.8, FADE: 3.0]");
    REQUIRE(audio->currentBGM() == "Boss_Dark");

    map.processAiAudioCommands("[ACTION: PLAY_SE, ASSET: Confirm_01, VOL: 1.0, FADE: 0.0]");
    REQUIRE(audio->activeSourceCount() == 1);

    map.processAiAudioCommands("[ACTION: STOP, ASSET: anything, VOL: 0.0, FADE: 0.0]");
    REQUIRE(audio->currentBGM().empty());
    REQUIRE(audio->activeSourceCount() == 0);
}

class MockScene : public GameScene {
public:
    MockScene(SceneType type, const std::string& name) : m_type(type), m_name(name) {}
    
    SceneType getType() const override { return m_type; }
    std::string getName() const override { return m_name; }

    void onStart() override { m_startCount++; }
    void onStop() override { m_stopCount++; }
    void onUpdate(float dt) override { m_updateCount++; }

    int m_startCount = 0;
    int m_stopCount = 0;
    int m_updateCount = 0;

private:
    SceneType m_type;
    std::string m_name;
};

TEST_CASE("SceneManager: Basic Lifecycle & Stack", "[scene][core]") {
    SceneManager manager;

    auto title = std::make_shared<MockScene>(SceneType::TITLE, "TitleScreen");
    auto map = std::make_shared<MockScene>(SceneType::MAP, "Overworld");
    auto battle = std::make_shared<MockScene>(SceneType::BATTLE, "Encounter");

    SECTION("Initial state is empty") {
        REQUIRE(manager.getActiveScene() == nullptr);
        REQUIRE(manager.stackSize() == 0);
    }

    SECTION("Pushing scenes") {
        manager.pushScene(title);
        REQUIRE(manager.getActiveScene()->getName() == "TitleScreen");
        REQUIRE(title->m_startCount == 1);
        REQUIRE(manager.stackSize() == 1);

        manager.pushScene(map);
        REQUIRE(manager.getActiveScene()->getName() == "Overworld");
        REQUIRE(title->m_stopCount == 1);
        REQUIRE(map->m_startCount == 1);
        REQUIRE(manager.stackSize() == 2);
    }

    SECTION("Popping scenes") {
        manager.pushScene(title);
        manager.pushScene(map);
        
        manager.popScene();
        REQUIRE(manager.getActiveScene()->getName() == "TitleScreen");
        REQUIRE(title->m_startCount == 2); // Recalled from stack
        REQUIRE(map->m_stopCount == 1);
    }

    SECTION("GotoScene clears stack") {
        manager.pushScene(title);
        manager.pushScene(map);
        manager.gotoScene(battle);
        
        REQUIRE(manager.getActiveScene()->getName() == "Encounter");
        REQUIRE(manager.stackSize() == 1);
    }

    SECTION("Update only affects top scene") {
        manager.pushScene(title);
        manager.pushScene(map);
        
        manager.update(0.16f);
        REQUIRE(map->m_updateCount == 1);
        REQUIRE(title->m_updateCount == 0);
    }
}
