#include <catch2/catch_test_macros.hpp>
#include "engine/core/scene/scene_manager.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/map_loader.h"
#include "engine/core/scene/tileset_registry.h"
#include "engine/core/scene/movement_authority.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/render/render_layer.h"
#include <type_traits>
#include <utility>

using namespace urpg::scene;

namespace {

template <typename LayerT>
const auto& renderFrameCommands(const LayerT& layer) {
    if constexpr (requires { layer.getFrameCommands(); }) {
        return layer.getFrameCommands();
    } else {
        return layer.getCommands();
    }
}

template <typename StoredCommand>
urpg::RenderCmdType renderCommandType(const StoredCommand& command) {
    if constexpr (requires { command.type; }) {
        return command.type;
    } else {
        return command->type;
    }
}

template <typename CommandT, typename StoredCommand>
const CommandT* renderCommandAs(const StoredCommand& command) {
    if constexpr (requires { command.template tryGet<CommandT>(); }) {
        return command.template tryGet<CommandT>();
    } else if constexpr (requires { command.get(); }) {
        return dynamic_cast<const CommandT*>(command.get());
    } else if constexpr (std::is_pointer_v<std::remove_cvref_t<StoredCommand>>) {
        return dynamic_cast<const CommandT*>(command);
    } else if constexpr (std::is_base_of_v<CommandT, std::remove_cvref_t<StoredCommand>>) {
        return &command;
    } else {
        return nullptr;
    }
}

struct TileSnapshot {
    int32_t tileIndex = 0;
    float x = 0.0f;
    float y = 0.0f;
    int32_t zOrder = 0;
};

template <typename StoredCommand>
TileSnapshot snapshotTileCommand(const StoredCommand& command) {
    const auto* tile = renderCommandAs<urpg::TileRenderData>(command);
    REQUIRE(tile != nullptr);
    return TileSnapshot{tile->tileIndex, command.x, command.y, command.zOrder};
}

} // namespace

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

TEST_CASE("MapScene: injected AudioCore can be replaced without stale state leakage", "[scene][map][audio]") {
    MapScene map("001", 10, 10);
    auto firstAudio = std::make_shared<urpg::audio::AudioCore>();
    auto secondAudio = std::make_shared<urpg::audio::AudioCore>();

    map.setAudioCore(firstAudio);
    map.processAiAudioCommands("[ACTION: CROSSFADE, ASSET: Boss_Dark, VOL: 0.8, FADE: 3.0]");
    REQUIRE(firstAudio->currentBGM() == "Boss_Dark");

    map.setAudioCore(secondAudio);
    map.processAiAudioCommands("[ACTION: PLAY_SE, ASSET: Confirm_01, VOL: 1.0, FADE: 0.0]");

    REQUIRE(firstAudio->activeSourceCount() == 0);
    REQUIRE(secondAudio->activeSourceCount() == 1);
}

TEST_CASE("MapScene: audio service binding is explicit and observable", "[scene][map][audio]") {
    MapScene map("001", 10, 10);
    REQUIRE(map.audioCore() == nullptr);

    auto audio = std::make_shared<urpg::audio::AudioCore>();
    map.setAudioCore(audio);

    REQUIRE(map.audioCore() == audio);
}

TEST_CASE("MapScene: retained tile render commands only rebuild when map data changes", "[scene][map][render]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    MapScene map("001", 2, 2);

    map.onUpdate(0.0f);
    const auto& firstFrame = renderFrameCommands(layer);
    REQUIRE(firstFrame.size() == 5);

    const auto firstTile0 = snapshotTileCommand(firstFrame[0]);
    const auto firstTile1 = snapshotTileCommand(firstFrame[1]);
    const auto firstTile2 = snapshotTileCommand(firstFrame[2]);
    const auto firstTile3 = snapshotTileCommand(firstFrame[3]);

    map.onUpdate(0.0f);
    const auto& secondFrame = renderFrameCommands(layer);
    REQUIRE(secondFrame.size() == 5);
    REQUIRE(snapshotTileCommand(secondFrame[0]).tileIndex == firstTile0.tileIndex);
    REQUIRE(snapshotTileCommand(secondFrame[0]).x == firstTile0.x);
    REQUIRE(snapshotTileCommand(secondFrame[1]).tileIndex == firstTile1.tileIndex);
    REQUIRE(snapshotTileCommand(secondFrame[1]).x == firstTile1.x);
    REQUIRE(snapshotTileCommand(secondFrame[2]).tileIndex == firstTile2.tileIndex);
    REQUIRE(snapshotTileCommand(secondFrame[2]).y == firstTile2.y);
    REQUIRE(snapshotTileCommand(secondFrame[3]).tileIndex == firstTile3.tileIndex);
    REQUIRE(snapshotTileCommand(secondFrame[3]).y == firstTile3.y);

    map.setTile(1, 0, 7, true);
    map.onUpdate(0.0f);
    const auto& thirdFrame = renderFrameCommands(layer);
    REQUIRE(thirdFrame.size() == 5);
    REQUIRE(renderCommandType(thirdFrame[1]) == urpg::RenderCmdType::Tile);

    const auto* changedTile = renderCommandAs<urpg::TileRenderData>(thirdFrame[1]);
    REQUIRE(changedTile != nullptr);
    REQUIRE(changedTile->tileIndex == 7);
}

TEST_CASE("MapScene: retained tile frame commands stay value-stable across unchanged frames", "[scene][map][render]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    MapScene map("001", 2, 2);

    map.onUpdate(0.0f);
    const auto firstFrame = renderFrameCommands(layer);
    REQUIRE(firstFrame.size() == 5);

    map.onUpdate(0.0f);
    const auto secondFrame = renderFrameCommands(layer);
    REQUIRE(secondFrame.size() == firstFrame.size());

    for (size_t i = 0; i < 4; ++i) {
        const auto before = snapshotTileCommand(firstFrame[i]);
        const auto after = snapshotTileCommand(secondFrame[i]);
        REQUIRE(after.tileIndex == before.tileIndex);
        REQUIRE(after.x == before.x);
        REQUIRE(after.y == before.y);
        REQUIRE(after.zOrder == before.zOrder);
    }
}

TEST_CASE("MapScene: message runner submits render commands during dialogue", "[scene][map][message]") {
    using namespace urpg::message;

    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    MapScene map("001", 2, 2);

    map.startDialogue({
        {"page_1", "Hello from native message", variantFromCompatRoute("speaker", "Elder", 1), true, {}, 0},
    });

    REQUIRE(map.isDialogueActive());

    map.onUpdate(0.0f);
    const auto& commands = renderFrameCommands(layer);

    bool hasTextCmd = false;
    bool hasRectCmd = false;
    for (const auto& cmd : commands) {
        if (renderCommandType(cmd) == urpg::RenderCmdType::Text) {
            const auto* textCmd = renderCommandAs<urpg::TextRenderData>(cmd);
            if (textCmd != nullptr && textCmd->text == "Hello from native message") {
                hasTextCmd = true;
            }
        }
        if (renderCommandType(cmd) == urpg::RenderCmdType::Rect) {
            const auto* rectCmd = renderCommandAs<urpg::RectRenderData>(cmd);
            if (rectCmd != nullptr && rectCmd->w > 0.0f && rectCmd->h > 0.0f) {
                hasRectCmd = true;
            }
        }
    }

    REQUIRE(hasTextCmd);
    REQUIRE(hasRectCmd);
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
