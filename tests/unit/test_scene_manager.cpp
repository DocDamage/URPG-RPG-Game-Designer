#include "engine/core/audio/audio_core.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/global_state_hub.h"
#include "engine/core/message/chatbot_component.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/map_loader.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/menu_scene.h"
#include "engine/core/scene/movement_authority.h"
#include "engine/core/scene/options_scene.h"
#include "engine/core/scene/runtime_title_scene.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/scene/tileset_registry.h"
#include "engine/core/settings/app_settings_store.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <nlohmann/json.hpp>
#include <type_traits>
#include <utility>

using namespace urpg::scene;

namespace {

class TempRuntimeSettingsRoot {
  public:
    TempRuntimeSettingsRoot() {
        const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        root_ = std::filesystem::temp_directory_path() / ("urpg_options_scene_" + unique);
        std::filesystem::create_directories(root_);
    }

    ~TempRuntimeSettingsRoot() {
        std::error_code ec;
        std::filesystem::remove_all(root_, ec);
    }

    const std::filesystem::path& root() const { return root_; }

  private:
    std::filesystem::path root_;
};

template<typename LayerT> const auto& renderFrameCommands(const LayerT& layer) {
    if constexpr (requires { layer.getFrameCommands(); }) {
        return layer.getFrameCommands();
    } else {
        return layer.getCommands();
    }
}

template<typename StoredCommand> urpg::RenderCmdType renderCommandType(const StoredCommand& command) {
    if constexpr (requires { command.type; }) {
        return command.type;
    } else {
        return command->type;
    }
}

template<typename CommandT, typename StoredCommand> const CommandT* renderCommandAs(const StoredCommand& command) {
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

class CapturingChatService : public urpg::ai::IChatService {
  public:
    std::shared_ptr<urpg::ai::ChatRequestHandle>
    requestResponse(const std::vector<urpg::ai::ChatMessage>& history, ChatCallback callback) override {
        auto handle = std::make_shared<urpg::ai::ChatRequestHandle>();
        ++request_count;
        if (!history.empty()) {
            last_user_message = history.back().content;
        }
        if (!handle->cancelled()) {
            callback("reply to " + last_user_message, "");
        }
        return handle;
    }

    int request_count = 0;
    std::string last_user_message;
};

template<typename StoredCommand> TileSnapshot snapshotTileCommand(const StoredCommand& command) {
    const auto* tile = renderCommandAs<urpg::TileRenderData>(command);
    REQUIRE(tile != nullptr);
    return TileSnapshot{tile->tileIndex, command.x, command.y, command.zOrder};
}

} // namespace

TEST_CASE("MovementAuthority: Native Grid Transitions", "[scene][movement]") {
    urpg::MovementComponent m;
    m.gridPos = {5, 5};
    m.lastGridPos = {5, 5};
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
        m.gridPos = {4, 5};
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

TEST_CASE("MapScene routes path requests over the native collision grid", "[scene][map][pathfinding]") {
    MapScene map("map001", 4, 3);
    map.setTilePassable(1, 0, false);
    map.setTilePassable(1, 1, false);

    urpg::level::PathRequest request;
    request.request_id = "map:player:path_to_exit";
    request.actor_id = "player";
    request.source = urpg::level::PathRequestSource::MapRuntime;
    request.start = {0, 0};
    request.goal = {3, 2};

    const auto routed = map.routePathRequest(request);
    const auto json = routed.toJson();

    REQUIRE(routed.ok);
    REQUIRE(routed.surface_id == "map001");
    REQUIRE(routed.path.nodes.front() == urpg::level::PathGridPoint{0, 0});
    REQUIRE(routed.path.nodes.back() == urpg::level::PathGridPoint{3, 2});
    REQUIRE(json["source"] == "map_runtime");
    REQUIRE(json["route"]["found"] == true);
}

TEST_CASE("MapScene path requests expose blocked tile diagnostics", "[scene][map][pathfinding]") {
    MapScene map("map001", 2, 2);
    map.setTilePassable(1, 0, false);
    map.setTilePassable(0, 1, false);

    urpg::level::PathRequest request;
    request.request_id = "event:npc_001:blocked";
    request.actor_id = "npc_001";
    request.source = urpg::level::PathRequestSource::EventRuntime;
    request.start = {0, 0};
    request.goal = {1, 1};

    const auto routed = map.routePathRequest(request);

    REQUIRE_FALSE(routed.ok);
    REQUIRE(routed.status == "blocked");
    REQUIRE(routed.surface_id == "map001");
    REQUIRE(routed.path.diagnostics.size() == 2);
    REQUIRE(routed.path.diagnostics[0].reason == "map_collision");
}

TEST_CASE("MapLoader: Bridge DataManager to MapScene", "[scene][map][loader]") {
    // 1. Setup tileset collision in the registry
    urpg::TilesetData ts;
    ts.id = 1;
    ts.name = "Outdoor";
    ts.flags = {urpg::TileFlag::Passable, urpg::TileFlag::FullImpassable}; // 0=Floor, 1=Wall
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

        map.setTile(5, 5, 2, true); // Floor
        REQUIRE(map.checkCollision(5, 5) == false);
    }
}

TEST_CASE("MapScene: directional tile passage governs movement", "[scene][map][movement]") {
    MapScene map("directional", 2, 2);
    map.setTileWithTileset(0, 0, 1, true, "atlas", false, true, true, true);
    map.setTileWithTileset(0, 1, 2, true, "atlas", true, true, true, true);
    REQUIRE_FALSE(map.canMove(0, 0, urpg::Direction::Down));

    map.setTileWithTileset(0, 0, 1, true, "atlas", true, true, true, true);
    map.setTileWithTileset(0, 1, 2, true, "atlas", true, true, true, false);
    REQUIRE_FALSE(map.canMove(0, 0, urpg::Direction::Down));

    map.setTileWithTileset(0, 1, 2, true, "atlas", true, true, true, true);
    REQUIRE(map.canMove(0, 0, urpg::Direction::Down));
}

TEST_CASE("MapScene: directional tile passage governs path routing", "[scene][map][pathfinding]") {
    MapScene map("directional_path", 2, 1);
    map.setTileWithTileset(0, 0, 1, true, "atlas", true, true, false, true);
    map.setTileWithTileset(1, 0, 2, true, "atlas", true, true, true, true);

    urpg::level::PathRequest request;
    request.request_id = "map:player:directional_path";
    request.actor_id = "player";
    request.source = urpg::level::PathRequestSource::MapRuntime;
    request.start = {0, 0};
    request.goal = {1, 0};
    const auto routed = map.routePathRequest(request);

    REQUIRE_FALSE(routed.ok);
    REQUIRE(routed.status == "blocked");
    REQUIRE(routed.path.diagnostics == std::vector<urpg::level::PathfindingDiagnostic>{
        {"neighbor_traversal_blocked", {1, 0}, "map_passage_blocked"}});
}

TEST_CASE("MapScene: invalid dimensions create an empty collision-safe map", "[scene][map][robustness]") {
    std::unique_ptr<MapScene> map;
    REQUIRE_NOTHROW(map = std::make_unique<MapScene>("invalid", -3, 2));
    REQUIRE(map != nullptr);
    REQUIRE(map->getWidth() == 0);
    REQUIRE(map->getHeight() == 2);
    REQUIRE(map->checkCollision(0, 0));

    std::unique_ptr<MapScene> empty;
    REQUIRE_NOTHROW(empty = std::make_unique<MapScene>("empty", -3, -2));
    REQUIRE(empty != nullptr);
    REQUIRE(empty->getWidth() == 0);
    REQUIRE(empty->getHeight() == 0);
    REQUIRE(empty->checkCollision(0, 0));
}

TEST_CASE("TilemapRenderer ignores invalid layer indices", "[scene][map][render][robustness]") {
    urpg::TilemapRenderer renderer(1, 1);

    REQUIRE_NOTHROW(renderer.setLayer(-1, {1}));
    REQUIRE_NOTHROW(renderer.setLayer(0, {1}));
}

TEST_CASE("TilemapRenderer ignores invalid texture dimensions", "[scene][map][render][robustness]") {
    urpg::TilemapRenderer renderer(1, 1);
    renderer.setLayer(0, {1});
    renderer.setTileset(std::make_shared<urpg::Texture>());

    urpg::SpriteBatcher batcher;
    batcher.begin();
    REQUIRE_NOTHROW(renderer.draw(batcher));
    batcher.end();
    REQUIRE(batcher.getBatches().empty());
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

TEST_CASE("MapScene: AI animation commands move the player render path", "[scene][map][animation][ai]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    MapScene map("001", 2, 2);
    map.processAiAnimationCommands("[TARGET: player] [{\"time\": 0.0, \"x\": 0.0, \"y\": 0.0, \"z\": 0.0},"
                                   "{\"time\": 1.0, \"x\": 48.0, \"y\": 24.0, \"z\": 0.0}]");

    REQUIRE(map.hasActivePlayerAiAnimation());
    REQUIRE(map.playerAiAnimationOffset().x.ToFloat() == 0.0f);

    map.onUpdate(0.5f);

    REQUIRE(map.playerAiAnimationOffset().x.ToFloat() == 24.0f);
    REQUIRE(map.playerAiAnimationOffset().y.ToFloat() == 12.0f);

    bool sawAnimatedPlayer = false;
    for (const auto& command : renderFrameCommands(layer)) {
        if (renderCommandType(command) != urpg::RenderCmdType::Sprite) {
            continue;
        }
        const auto* sprite = renderCommandAs<urpg::SpriteRenderData>(command);
        sawAnimatedPlayer = sawAnimatedPlayer || (sprite != nullptr && sprite->textureId == "missing_player_sprite" &&
                                                  command.x == 24.0f && command.y == 12.0f);
    }
    REQUIRE(sawAnimatedPlayer);
}

TEST_CASE("MapScene: AI animation commands report unsupported targets", "[scene][map][animation][ai]") {
    urpg::diagnostics::RuntimeDiagnostics::clear();
    MapScene map("001", 2, 2);

    map.processAiAnimationCommands("[TARGET: npc_001] [{\"time\": 0.0, \"x\": 0.0, \"y\": 0.0, \"z\": 0.0},"
                                   "{\"time\": 1.0, \"x\": 48.0, \"y\": 0.0, \"z\": 0.0}]");

    REQUIRE_FALSE(map.hasActivePlayerAiAnimation());
    REQUIRE(map.aiAnimationDiagnostics().size() == 1);
    REQUIRE(map.aiAnimationDiagnostics()[0] == "unsupported_animation_target:npc_001");

    const auto diagnostics = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.subsystem == "scene.map" && diagnostic.code == "map.ai_animation_target_unsupported";
    }));
}

TEST_CASE("MapScene: authored ability assets can be granted and activated on the player runtime",
          "[scene][map][ability]") {
    MapScene map("001", 10, 10);

    urpg::ability::AuthoredAbilityAsset asset;
    asset.ability_id = "skill.map_authored";
    asset.mp_cost = 6.0f;
    asset.cooldown_seconds = 4.0f;
    asset.effect_id = "effect.map_guard";
    asset.effect_attribute = "Defense";
    asset.effect_operation = urpg::ModifierOp::Add;
    asset.effect_value = 15.0f;
    asset.effect_duration = 8.0f;
    asset.pattern = urpg::PatternField::MakeCross("Map Cross", 1);

    map.grantPlayerAbility(asset);
    REQUIRE(map.playerAbilitySystem().getAbilities().size() == 1);
    REQUIRE(map.playerAbilitySystem().getAbilities()[0]->getId() == "skill.map_authored");

    REQUIRE(map.tryActivatePlayerAbility("skill.map_authored"));
    REQUIRE(map.playerAbilitySystem().getAttribute("MP", 0.0f) == 24.0f);
    REQUIRE(map.playerAbilitySystem().getAttribute("Defense", 0.0f) == 115.0f);
}

TEST_CASE("MapScene: bound interaction abilities execute from confirm input through the map runtime",
          "[scene][map][ability][interaction]") {
    MapScene map("001", 10, 10);

    urpg::ability::AuthoredAbilityAsset asset;
    asset.ability_id = "skill.confirm_interact";
    asset.mp_cost = 6.0f;
    asset.effect_id = "effect.confirm_interact";
    asset.effect_attribute = "Attack";
    asset.effect_value = 9.0f;

    REQUIRE(map.bindInteractionAbility("confirm_interact", "content/abilities/confirm_interact.json", asset));
    REQUIRE(map.hasInteractionAbilityBinding("confirm_interact"));
    REQUIRE(map.interactionAbilityBindings().size() == 1);
    REQUIRE(map.interactionAbilityBindings()[0].asset_path == "content/abilities/confirm_interact.json");

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("Attack", 100.0f);

    urpg::input::InputCore input;
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);

    REQUIRE(map.playerAbilitySystem().getAttribute("MP", 0.0f) == 24.0f);
    REQUIRE(map.playerAbilitySystem().getAttribute("Attack", 0.0f) == 109.0f);
    REQUIRE_FALSE(map.playerAbilitySystem().getAbilityExecutionHistory().empty());
    REQUIRE(map.playerAbilitySystem().getAbilityExecutionHistory().back().ability_id == "skill.confirm_interact");
    REQUIRE(map.playerAbilitySystem().getAbilityExecutionHistory().back().outcome == "executed");
}

TEST_CASE("MapScene: tile and prop interaction bindings activate targeted abilities",
          "[scene][map][ability][interaction_targets]") {
    MapScene map("001", 10, 10);

    urpg::ability::AuthoredAbilityAsset tileAsset;
    tileAsset.ability_id = "skill.tile_trigger";
    tileAsset.effect_id = "effect.tile_trigger";
    tileAsset.effect_attribute = "Defense";
    tileAsset.effect_value = 13.0f;

    urpg::ability::AuthoredAbilityAsset propAsset;
    propAsset.ability_id = "skill.prop_trigger";
    propAsset.effect_id = "effect.prop_trigger";
    propAsset.effect_attribute = "MagicDefense";
    propAsset.effect_value = 8.0f;

    REQUIRE(map.bindTileInteractionAbility("confirm_interact", 2, 3, "content/abilities/tile_trigger.json", tileAsset));
    REQUIRE(
        map.bindPropInteractionAbility("inspect_prop", "rock_01", "content/abilities/prop_trigger.json", propAsset));
    REQUIRE(map.interactionAbilityBindings().size() == 2);

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("Defense", 100.0f);
    map.playerAbilitySystem().setAttribute("MagicDefense", 100.0f);

    REQUIRE(map.activateInteractionAbilityAtTile("confirm_interact", 2, 3));
    REQUIRE(map.playerAbilitySystem().getAttribute("Defense", 0.0f) == 113.0f);

    map.playerAbilitySystem().setCooldown("skill.tile_trigger", 0.0f);
    REQUIRE(map.activateInteractionAbilityForProp("inspect_prop", "rock_01"));
    REQUIRE(map.playerAbilitySystem().getAttribute("MagicDefense", 0.0f) == 108.0f);
    REQUIRE(map.playerAbilitySystem().getAbilityExecutionHistory().back().ability_id == "skill.prop_trigger");
}

TEST_CASE("MapScene: prop interaction bindings prefer stable instance ids with asset fallback",
          "[scene][map][ability][interaction_targets]") {
    MapScene map("001", 10, 10);

    urpg::ability::AuthoredAbilityAsset instanceAsset;
    instanceAsset.ability_id = "skill.prop_instance_trigger";
    instanceAsset.effect_id = "effect.prop_instance_trigger";
    instanceAsset.effect_attribute = "MagicDefense";
    instanceAsset.effect_value = 5.0f;

    urpg::ability::AuthoredAbilityAsset legacyAsset;
    legacyAsset.ability_id = "skill.prop_legacy_trigger";
    legacyAsset.effect_id = "effect.prop_legacy_trigger";
    legacyAsset.effect_attribute = "Attack";
    legacyAsset.effect_value = 3.0f;

    REQUIRE(map.bindPropInstanceInteractionAbility("inspect_prop", "001:rock_01:0", "rock_01",
                                                   "content/abilities/prop_instance_trigger.json", instanceAsset));
    REQUIRE(map.bindPropInteractionAbility("inspect_prop", "rock_01", "content/abilities/prop_legacy_trigger.json",
                                           legacyAsset));

    REQUIRE(map.interactionAbilityBindings().size() == 2);
    REQUIRE(map.interactionAbilityBindings()[0].prop_instance_id == "001:rock_01:0");
    REQUIRE(map.interactionAbilityBindings()[0].prop_asset_id == "rock_01");
    REQUIRE(map.interactionAbilityBindings()[1].prop_instance_id.empty());
    REQUIRE(map.interactionAbilityBindings()[1].prop_asset_id == "rock_01");

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("MagicDefense", 100.0f);
    map.playerAbilitySystem().setAttribute("Attack", 100.0f);

    REQUIRE(map.activateInteractionAbilityForPropInstance("inspect_prop", "001:rock_01:0", "rock_01"));
    REQUIRE(map.playerAbilitySystem().getAttribute("MagicDefense", 0.0f) == 105.0f);
    REQUIRE(map.playerAbilitySystem().getAttribute("Attack", 0.0f) == 100.0f);

    map.playerAbilitySystem().setCooldown("skill.prop_instance_trigger", 0.0f);
    REQUIRE(map.activateInteractionAbilityForPropInstance("inspect_prop", "001:rock_01:1", "rock_01"));
    REQUIRE(map.playerAbilitySystem().getAttribute("Attack", 0.0f) == 103.0f);
}

TEST_CASE("MapScene: region interaction bindings activate for tiles inside the painted area",
          "[scene][map][ability][interaction_region]") {
    MapScene map("001", 10, 10);

    urpg::ability::AuthoredAbilityAsset regionAsset;
    regionAsset.ability_id = "skill.region_trigger";
    regionAsset.effect_id = "effect.region_trigger";
    regionAsset.effect_attribute = "MagicDefense";
    regionAsset.effect_value = 12.0f;

    REQUIRE(map.bindRegionInteractionAbility("confirm_interact", 2, 3, 5, 6, "content/abilities/region_trigger.json",
                                             regionAsset));

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("MagicDefense", 100.0f);

    REQUIRE(map.activateInteractionAbilityAtTile("confirm_interact", 4, 5));
    REQUIRE(map.playerAbilitySystem().getAttribute("MagicDefense", 0.0f) == 112.0f);

    map.playerAbilitySystem().setCooldown("skill.region_trigger", 0.0f);
    REQUIRE_FALSE(map.activateInteractionAbilityAtTile("confirm_interact", 1, 1));
}

TEST_CASE("MapScene: multiple region interaction bindings normalize and activate independently",
          "[scene][map][ability][interaction_region]") {
    MapScene map("001", 10, 10);

    urpg::ability::AuthoredAbilityAsset regionA;
    regionA.ability_id = "skill.region_alpha";
    regionA.effect_id = "effect.region_alpha";
    regionA.effect_attribute = "Defense";
    regionA.effect_value = 10.0f;

    urpg::ability::AuthoredAbilityAsset regionB;
    regionB.ability_id = "skill.region_beta";
    regionB.effect_id = "effect.region_beta";
    regionB.effect_attribute = "MagicDefense";
    regionB.effect_value = 14.0f;

    REQUIRE(map.bindRegionInteractionAbility("confirm_interact", 7, 7, 4, 5, "content/abilities/region_alpha.json",
                                             regionA));
    REQUIRE(map.bindRegionInteractionAbility("confirm_interact", 1, 1, 2, 2, "content/abilities/region_beta.json",
                                             regionB));

    REQUIRE(map.interactionAbilityBindings().size() == 2);
    REQUIRE(map.interactionAbilityBindings()[0].region_min_x == 4);
    REQUIRE(map.interactionAbilityBindings()[0].region_min_y == 5);
    REQUIRE(map.interactionAbilityBindings()[0].region_max_x == 7);
    REQUIRE(map.interactionAbilityBindings()[0].region_max_y == 7);

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("Defense", 100.0f);
    map.playerAbilitySystem().setAttribute("MagicDefense", 100.0f);

    REQUIRE(map.activateInteractionAbilityAtTile("confirm_interact", 5, 6));
    REQUIRE(map.playerAbilitySystem().getAttribute("Defense", 0.0f) == 110.0f);
    REQUIRE(map.playerAbilitySystem().getAbilityExecutionHistory().back().ability_id == "skill.region_alpha");

    map.playerAbilitySystem().setCooldown("skill.region_alpha", 0.0f);
    REQUIRE(map.activateInteractionAbilityAtTile("confirm_interact", 2, 2));
    REQUIRE(map.playerAbilitySystem().getAttribute("MagicDefense", 0.0f) == 114.0f);
    REQUIRE(map.playerAbilitySystem().getAbilityExecutionHistory().back().ability_id == "skill.region_beta");

    map.playerAbilitySystem().setCooldown("skill.region_beta", 0.0f);
    REQUIRE_FALSE(map.activateInteractionAbilityAtTile("confirm_interact", 9, 9));
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

TEST_CASE("MapScene uses project map asset references for render commands", "[scene][map][render][assets]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();
    urpg::diagnostics::RuntimeDiagnostics::clear();

    MapScene map("AssetMap", 2, 2);
    map.setAssetReferences({
        {"project_player_sprite", {}},
        {"project_tileset", {}},
    });
    map.setTile(0, 0, 12, true);
    map.onUpdate(0.0f);

    bool sawPlayer = false;
    bool sawTileset = false;
    for (const auto& command : renderFrameCommands(layer)) {
        if (renderCommandType(command) == urpg::RenderCmdType::Sprite) {
            const auto* sprite = renderCommandAs<urpg::SpriteRenderData>(command);
            sawPlayer = sawPlayer || (sprite != nullptr && sprite->textureId == "project_player_sprite");
        }
        if (renderCommandType(command) == urpg::RenderCmdType::Tile) {
            const auto* tile = renderCommandAs<urpg::TileRenderData>(command);
            sawTileset = sawTileset || (tile != nullptr && tile->tilesetId == "project_tileset");
        }
    }

    REQUIRE(sawPlayer);
    REQUIRE(sawTileset);
    REQUIRE(map.assetDiagnostics().empty());
    REQUIRE(urpg::diagnostics::RuntimeDiagnostics::snapshot().empty());
}

TEST_CASE("MapScene projects validated authored event sprites into runtime render commands",
          "[scene][map][render][events]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    MapScene map("EventSpriteMap", 3, 2);
    REQUIRE(map.setEventSprites({
        {"event_vendor", {"asset.vendor", "content/assets/vendor.png"}, 2, 1},
        {"event_sign", {"asset.sign", "content/assets/sign.png"}, 0, 1},
    }));
    REQUIRE(map.eventSprites().size() == 2);
    REQUIRE(map.eventSprites()[0].event_id == "event_sign");
    REQUIRE_FALSE(map.setEventSprites({
        {"event_invalid", {"asset.invalid", "content/assets/invalid.png"}, 3, 1},
    }));
    REQUIRE(map.eventSprites().size() == 2);
    REQUIRE_FALSE(map.setEventSprites({
        {"event_a", {"asset.shared", "content/assets/a.png"}, 0, 0},
        {"event_b", {"asset.shared", "content/assets/b.png"}, 1, 0},
    }));
    REQUIRE(map.eventSprites().size() == 2);

    map.onUpdate(0.0f);
    bool sawVendor = false;
    bool sawSign = false;
    for (const auto& command : renderFrameCommands(layer)) {
        if (renderCommandType(command) != urpg::RenderCmdType::Sprite) {
            continue;
        }
        const auto* sprite = renderCommandAs<urpg::SpriteRenderData>(command);
        sawVendor = sawVendor || (sprite != nullptr && sprite->textureId == "asset.vendor" &&
                                  command.x == 96.0f && command.y == 48.0f && command.zOrder == 2);
        sawSign = sawSign || (sprite != nullptr && sprite->textureId == "asset.sign" &&
                               command.x == 0.0f && command.y == 48.0f && command.zOrder == 2);
    }
    REQUIRE(sawVendor);
    REQUIRE(sawSign);
}

TEST_CASE("MapScene advances authored event sprite-sheet frames deterministically",
          "[scene][map][render][events][animation]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    MapScene map("AnimatedEventSpriteMap", 2, 1);
    REQUIRE(map.setEventSprites({
        {"event_torch", {"asset.torch", "content/assets/torch.png"}, 1, 0, 16, 24, 3, 0.10f, true},
    }));

    map.onUpdate(0.11f);
    bool saw_second_frame = false;
    for (const auto& command : renderFrameCommands(layer)) {
        if (renderCommandType(command) != urpg::RenderCmdType::Sprite) {
            continue;
        }
        const auto* sprite = renderCommandAs<urpg::SpriteRenderData>(command);
        saw_second_frame = saw_second_frame ||
                           (sprite != nullptr && sprite->textureId == "asset.torch" && sprite->srcX == 16 &&
                            sprite->srcY == 0 && sprite->width == 16 && sprite->height == 24 &&
                            command.x == 48.0f && command.y == 0.0f && command.zOrder == 2);
    }
    REQUIRE(saw_second_frame);

    layer.flush();
    REQUIRE(map.setEventSprites(map.eventSprites()));
    map.onUpdate(0.10f);
    bool saw_third_frame = false;
    for (const auto& command : renderFrameCommands(layer)) {
        if (renderCommandType(command) != urpg::RenderCmdType::Sprite) {
            continue;
        }
        const auto* sprite = renderCommandAs<urpg::SpriteRenderData>(command);
        saw_third_frame = saw_third_frame ||
                          (sprite != nullptr && sprite->textureId == "asset.torch" && sprite->srcX == 32);
    }
    REQUIRE(saw_third_frame);
}

TEST_CASE("MapScene makes authored event sprites follow the final matching page visibility",
          "[scene][map][render][events][visibility]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();
    auto& state = urpg::GlobalStateHub::getInstance();
    state.setSwitch("torch_extinguished", false);

    MapEventSprite sprite{"event_torch", {"asset.torch", "content/assets/torch.png"}, 0, 0};
    MapEventSprite::PageCandidate extinguished;
    extinguished.page_id = "extinguished";
    extinguished.has_visible_override = true;
    extinguished.visible = false;
    extinguished.conditions.push_back({"switch", "torch_extinguished", "equals", "true"});
    sprite.page_candidates.push_back(std::move(extinguished));

    MapScene map("EventVisibilityMap", 1, 1);
    REQUIRE(map.setEventSprites({std::move(sprite)}));
    const auto saw_torch = [](const auto& commands) {
        return std::any_of(commands.begin(), commands.end(), [](const auto& command) {
            const auto* rendered = renderCommandAs<urpg::SpriteRenderData>(command);
            return rendered != nullptr && rendered->textureId == "asset.torch";
        });
    };
    map.onUpdate(0.0f);
    const auto visible_commands = renderFrameCommands(layer);
    REQUIRE(saw_torch(visible_commands));

    layer.flush();
    state.setSwitch("torch_extinguished", true);
    map.onUpdate(0.0f);
    const auto hidden_commands = renderFrameCommands(layer);
    REQUIRE_FALSE(saw_torch(hidden_commands));
    state.setSwitch("torch_extinguished", false);
}

TEST_CASE("MapScene emits diagnostics before rendering asset placeholders", "[scene][map][render][assets]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();
    urpg::diagnostics::RuntimeDiagnostics::clear();

    MapScene map("MissingAssetMap", 1, 1);
    map.onUpdate(0.0f);

    bool sawMissingPlayer = false;
    bool sawMissingTileset = false;
    for (const auto& command : renderFrameCommands(layer)) {
        if (renderCommandType(command) == urpg::RenderCmdType::Sprite) {
            const auto* sprite = renderCommandAs<urpg::SpriteRenderData>(command);
            sawMissingPlayer = sawMissingPlayer || (sprite != nullptr && sprite->textureId == "missing_player_sprite");
        }
        if (renderCommandType(command) == urpg::RenderCmdType::Tile) {
            const auto* tile = renderCommandAs<urpg::TileRenderData>(command);
            sawMissingTileset = sawMissingTileset || (tile != nullptr && tile->tilesetId == "missing_tileset");
        }
    }

    REQUIRE(sawMissingPlayer);
    REQUIRE(sawMissingTileset);
    REQUIRE(map.assetDiagnostics().size() == 2);

    const auto diagnostics = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    REQUIRE(std::count_if(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
                return diagnostic.subsystem == "scene.map" &&
                       (diagnostic.code == "map.player_sprite_missing" || diagnostic.code == "map.tileset_missing");
            }) == 2);
}

TEST_CASE("MapScene release mode escalates missing core render assets", "[scene][map][render][assets][release]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();
    urpg::diagnostics::RuntimeDiagnostics::clear();

    MapScene map("ReleaseMissingAssetMap", 1, 1);
    map.setRuntimeAssetMode(urpg::RuntimeAssetMode::Release);
    map.onUpdate(0.0f);

    REQUIRE(map.assetDiagnostics().size() == 2);
    REQUIRE(std::find(map.assetDiagnostics().begin(), map.assetDiagnostics().end(),
                      "player_sprite_id_missing_release_blocking") != map.assetDiagnostics().end());
    REQUIRE(std::find(map.assetDiagnostics().begin(), map.assetDiagnostics().end(),
                      "tileset_id_missing_release_blocking") != map.assetDiagnostics().end());

    const auto diagnostics = urpg::diagnostics::RuntimeDiagnostics::snapshot();
    REQUIRE(std::count_if(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
                return diagnostic.subsystem == "scene.map" &&
                       diagnostic.severity == urpg::diagnostics::DiagnosticSeverity::Error &&
                       (diagnostic.code == "map.player_sprite_missing" || diagnostic.code == "map.tileset_missing");
            }) == 2);

    urpg::diagnostics::RuntimeDiagnostics::clear();
}

TEST_CASE("Runtime map asset references load from project manifest", "[scene][map][assets]") {
    const TempRuntimeSettingsRoot temp;
    std::filesystem::create_directories(temp.root() / "content" / "actors");
    std::filesystem::create_directories(temp.root() / "content" / "tilesets");
    {
        std::ofstream actor(temp.root() / "content" / "actors" / "hero.png", std::ios::binary);
        actor << "png";
        std::ofstream tileset(temp.root() / "content" / "tilesets" / "world.png", std::ios::binary);
        tileset << "png";
        std::ofstream project(temp.root() / "project.json", std::ios::binary);
        project << R"({
  "startup": {
    "map": "AssetMap",
    "map_assets": {
      "player_sprite": {"id": "hero.runtime", "path": "content/actors/hero.png"},
      "tileset": {"id": "tileset.runtime", "path": "content/tilesets/world.png"}
    }
  }
})";
    }

    const auto references = loadRuntimeMapAssetReferences(temp.root(), "AssetMap");
    REQUIRE(references.player_sprite.id == "hero.runtime");
    REQUIRE(references.tileset.id == "tileset.runtime");
    REQUIRE(references.player_sprite.path == temp.root() / "content" / "actors" / "hero.png");
    REQUIRE(references.tileset.path == temp.root() / "content" / "tilesets" / "world.png");
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

TEST_CASE("MapScene executes a saved native Dialogue Graph through the message runtime",
          "[scene][map][dialogue][runtime]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    urpg::dialogue::DialogueGraph graph;
    REQUIRE(graph.addNode({"start", "guide", "Guide", "dialogue.start", "Choose a route.", false,
                           {{"continue", "Continue", "end", {}, {}, "dialogue.choice.continue"}}}));
    REQUIRE(graph.addNode({"end", "guide", "Guide", "dialogue.end", "Journey complete.", true, {}}));

    MapScene map("AuthoredDialogueMap", 2, 2);
    urpg::localization::LocaleCatalog locale;
    locale.loadFromJson(nlohmann::json{{"locale", "en-US"},
                                      {"keys", {{"dialogue.start", "Choose a localized route."},
                                                {"dialogue.choice.continue", "Continue locally"},
                                                {"dialogue.end", "Localized journey complete."}}}});
    map.setDialogueLocaleCatalog(locale);
    REQUIRE(map.startAuthoredDialogue(graph, "test.authored_dialogue"));
    REQUIRE(map.activeDialogueConversationId() == "test.authored_dialogue");
    REQUIRE(map.isDialogueActive());

    map.onUpdate(0.0f);
    bool saw_start = false;
    for (const auto& command : renderFrameCommands(layer)) {
        const auto* text = renderCommandAs<urpg::TextRenderData>(command);
        saw_start = saw_start || (text != nullptr && text->text == "Choose a localized route.");
    }
    REQUIRE(saw_start);

    urpg::input::InputCore input;
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);
    map.onUpdate(0.0f);
    bool saw_choice = false;
    for (const auto& command : renderFrameCommands(layer)) {
        const auto* text = renderCommandAs<urpg::TextRenderData>(command);
        saw_choice = saw_choice || (text != nullptr && text->text == "> Continue locally");
    }
    REQUIRE(saw_choice);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Released);
    map.handleInput(input);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);

    map.onUpdate(0.0f);
    bool saw_end = false;
    for (const auto& command : renderFrameCommands(layer)) {
        const auto* text = renderCommandAs<urpg::TextRenderData>(command);
        saw_end = saw_end || (text != nullptr && text->text == "Localized journey complete.");
    }
    REQUIRE(saw_end);

    auto& state = urpg::GlobalStateHub::getInstance();
    state.clearSessionState();
    state.setVariable("flag", int32_t{1});
    state.setVariable("score", std::numeric_limits<int32_t>::max() - 1);
    urpg::dialogue::DialogueGraph stateful;
    REQUIRE(stateful.addNode({"start", "guide", "Guide", "dialogue.start", "Stateful.", false,
                              {{"continue", "Continue", "end", {{"flag", "==", 1}}, {{"score", 5}}}}}));
    REQUIRE(stateful.addNode({"end", "guide", "Guide", "dialogue.end", "End.", true, {}}));
    REQUIRE(map.startAuthoredDialogue(stateful, "test.stateful_dialogue"));
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Released);
    map.handleInput(input);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Released);
    map.handleInput(input);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);
    REQUIRE(std::get<int32_t>(state.getVariable("score")) == std::numeric_limits<int32_t>::max());
    const auto choice_effect_state = map.authoredDialogueStateSnapshot();
    REQUIRE(choice_effect_state.revision > 0);
    REQUIRE(std::find_if(choice_effect_state.variables.begin(), choice_effect_state.variables.end(),
                         [](const auto& entry) {
                             return entry.key == "score" &&
                                    entry.value == std::to_string(std::numeric_limits<int32_t>::max());
                         }) != choice_effect_state.variables.end());

    state.setVariable("flag", int32_t{0});
    REQUIRE(map.startAuthoredDialogue(stateful, "test.stateful_dialogue"));
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Released);
    map.handleInput(input);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);
    map.onUpdate(0.0f);
    bool saw_disabled_choice = false;
    for (const auto& command : renderFrameCommands(layer)) {
        const auto* text = renderCommandAs<urpg::TextRenderData>(command);
        saw_disabled_choice = saw_disabled_choice || (text != nullptr && text->text == "  [Unavailable] Continue");
    }
    REQUIRE(saw_disabled_choice);
    state.clearSessionState();
}

TEST_CASE("MapScene restores a saved project Dialogue Graph checkpoint at its active node",
          "[scene][map][dialogue][runtime][save]") {
    TempRuntimeSettingsRoot project;
    std::filesystem::create_directories(project.root() / "content" / "dialogues");

    urpg::dialogue::DialogueGraph graph;
    REQUIRE(graph.addNode({"start", "guide", "Guide", "", "Start.", false,
                           {{"continue", "Continue", "end", {}, {}}}}));
    REQUIRE(graph.addNode({"end", "guide", "Guide", "", "Resumed ending.", true, {}}));
    graph.setStartNode("start");
    {
        std::ofstream output(project.root() / "content" / "dialogues" / "checkpoint_dialogue.json", std::ios::binary);
        REQUIRE(output.good());
        output << graph.serialize().dump(2) << '\n';
    }

    MapScene map("CheckpointMap", 2, 2);
    map.setProjectRoot(project.root());
    REQUIRE(map.startAuthoredDialogueFromProject("checkpoint_dialogue"));
    urpg::input::InputCore input;
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Released);
    map.handleInput(input);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);
    REQUIRE(map.activeAuthoredDialogueNodeId() == "end");
    REQUIRE(map.saveGame(37));

    map.startDialogue({{"replacement", "Replacement runtime dialogue.", {}, true, {}, 0}});
    REQUIRE(map.loadGame(37));
    REQUIRE(map.activeDialogueConversationId() == "project.dialogue.checkpoint_dialogue");
    REQUIRE(map.activeAuthoredDialogueNodeId() == "end");

    map.onUpdate(0.0f);
    const auto& resumed_commands = renderFrameCommands(urpg::RenderLayer::getInstance());
    const bool saw_resumed_text = std::any_of(resumed_commands.begin(), resumed_commands.end(),
                                              [](const auto& command) {
                                                  const auto* text = renderCommandAs<urpg::TextRenderData>(command);
                                                  return text != nullptr && text->text == "Resumed ending.";
                                              });
    REQUIRE(saw_resumed_text);

    std::filesystem::create_directories(project.root() / "saves");
    {
        std::ofstream malformed(project.root() / "saves" / "slot_38.json", std::ios::binary);
        REQUIRE(malformed.good());
        malformed << nlohmann::json{{"switches", nlohmann::json::object()},
                                    {"variables", nlohmann::json::object()},
                                    {"differential", false},
                                    {"map_scene_dialogue_checkpoint", {{"version", "not-an-integer"}}}}
                         .dump();
    }
    const auto malformed_load = map.loadGameDetailed(38);
    REQUIRE(malformed_load.ok);
    REQUIRE(std::find(malformed_load.diagnostics.begin(), malformed_load.diagnostics.end(),
                      "map_dialogue_checkpoint_restore_failed") != malformed_load.diagnostics.end());
    REQUIRE_FALSE(map.isDialogueActive());
}

TEST_CASE("MapScene presents authored dialogue captions and dispatches voice through its native audio core",
          "[scene][map][dialogue][runtime][audio]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    urpg::dialogue::DialogueGraph graph;
    urpg::dialogue::DialogueNode node;
    node.id = "voice_caption";
    node.speaker_id = "guide";
    node.speaker_name = "Guide";
    node.text_preview = "A spoken line.";
    node.ending = true;
    node.voice_asset_id = "guide_voice_line";
    node.caption_localization_key = "dialogue.guide.caption";
    REQUIRE(graph.addNode(node));
    graph.setStartNode("voice_caption");

    MapScene map("VoiceCaptionMap", 2, 2);
    auto audio = std::make_shared<urpg::audio::AudioCore>();
    map.setAudioCore(audio);
    urpg::localization::LocaleCatalog locale;
    locale.loadFromJson(nlohmann::json{{"locale", "en-US"},
                                      {"keys", {{"dialogue.guide.caption", "[Guide voice caption]"}}}});
    map.setDialogueLocaleCatalog(locale);
    REQUIRE(map.startAuthoredDialogue(graph, "test.voice_caption"));
    REQUIRE(map.activeAuthoredDialogueCaption() == "[Guide voice caption]");
    REQUIRE(map.activeAuthoredDialogueVoiceAssetId() == "guide_voice_line");
    REQUIRE(audio->activeSourceCount() == 1);

    map.onUpdate(0.0f);
    const auto& commands = renderFrameCommands(layer);
    REQUIRE(std::any_of(commands.begin(), commands.end(), [](const auto& command) {
        const auto* text = renderCommandAs<urpg::TextRenderData>(command);
        return text != nullptr && text->text == "[Guide voice caption]";
    }));

    MapScene without_audio("VoiceCaptionNoAudio", 2, 2);
    REQUIRE(without_audio.startAuthoredDialogue(graph, "test.voice_caption_no_audio"));
    REQUIRE(std::find(without_audio.dialogueRuntimeDiagnostics().begin(),
                      without_audio.dialogueRuntimeDiagnostics().end(),
                      "authored_dialogue_voice_audio_core_missing:voice_caption") !=
            without_audio.dialogueRuntimeDiagnostics().end());
}

TEST_CASE("MapScene consumes governed caption takes and inclusive voice policy",
          "[scene][map][dialogue][runtime][captions][accessibility][pcq651][pcq653]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    urpg::dialogue::DialogueGraph graph;
    urpg::dialogue::DialogueNode node;
    node.id = "arrival";
    node.speaker_id = "guide";
    node.text_preview = "We have arrived.";
    node.ending = true;
    node.voice_asset_id = "voice.default";
    node.caption_localization_key = "dialogue.arrival";
    REQUIRE(graph.addNode(node));
    graph.setStartNode("arrival");

    urpg::accessibility::CaptionCue cue{"arrival.fr-FR", "guide", "fr-FR", "take.fr.02",
        "Nous sommes arrives.", {"door opens"}, 0, 1200, 1200, true};
    MapScene map("GovernedCaptionMap", 2, 2);
    auto audio = std::make_shared<urpg::audio::AudioCore>();
    map.setAudioCore(audio);
    urpg::localization::LocaleCatalog locale;
    locale.loadFromJson(nlohmann::json{{"locale", "fr-FR"},
                                       {"keys", {{"dialogue.arrival", "Fallback caption"}}}});
    map.setDialogueLocaleCatalog(locale);
    REQUIRE(map.setDialogueCaptionTrack({cue}, {{"take.fr.02", "voice.fr.02"}}));

    auto settings = urpg::accessibility::InclusiveSettings::safeDefaults();
    settings.captions = false;
    settings.voice_volume = 0.0F;
    settings.caption_scale = 2.0F;
    REQUIRE(map.setDialogueInclusiveSettings(settings));
    REQUIRE(map.startAuthoredDialogue(graph, "test.governed_caption"));
    REQUIRE(map.activeAuthoredDialogueVoiceAssetId() == "voice.fr.02");
    REQUIRE(map.activeAuthoredDialogueCaption() == "Nous sommes arrives. [door opens]");
    REQUIRE(map.activeAuthoredDialogueCaptionCue().has_value());
    REQUIRE(map.activeAuthoredDialogueCaptionCue()->take_id == "take.fr.02");
    REQUIRE(map.authoredDialogueCaptionScale() == 2.0F);
    REQUIRE(audio->activeSourceCount() == 0);
    REQUIRE(std::find(map.dialogueRuntimeDiagnostics().begin(), map.dialogueRuntimeDiagnostics().end(),
                      "authored_dialogue_caption_forced_for_muted_voice:arrival") !=
            map.dialogueRuntimeDiagnostics().end());

    map.onUpdate(0.0F);
    const auto& commands = renderFrameCommands(layer);
    REQUIRE(std::any_of(commands.begin(), commands.end(), [](const auto& command) {
        const auto* text = renderCommandAs<urpg::TextRenderData>(command);
        return text != nullptr && text->text == "Nous sommes arrives. [door opens]" && text->fontSize == 36;
    }));

    auto invalid = cue;
    invalid.take_id.clear();
    REQUIRE_FALSE(map.setDialogueCaptionTrack({invalid}, {}));
    REQUIRE(map.activeAuthoredDialogueCaptionCue()->take_id == "take.fr.02");
}

TEST_CASE("InputCore stores text input, editing text, and backspace for one input frame",
          "[scene][map][input][chatbot]") {
    urpg::input::InputCore input;

    input.appendTextInput("he");
    input.appendTextInput("llo");
    input.setTextEditing("hell");
    input.recordBackspace();

    REQUIRE(input.textInput() == "hello");
    REQUIRE(input.textEditing() == "hell");
    REQUIRE(input.backspaceCount() == 1);

    input.endFrame();

    REQUIRE(input.textInput().empty());
    REQUIRE(input.textEditing().empty());
    REQUIRE(input.backspaceCount() == 0);
}

TEST_CASE("MapScene: runtime chat submits typed text to ChatbotComponent", "[scene][map][input][chatbot]") {
    MapScene map("001", 10, 10);
    auto service = std::make_shared<CapturingChatService>();
    map.startChatbot("Guide", service);
    map.openChatInput();

    urpg::input::InputCore input;
    input.appendTextInput("hello guide");
    map.handleInput(input);
    REQUIRE(map.currentChatInputBuffer() == "hello guide");

    input.endFrame();
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.handleInput(input);

    REQUIRE(service->request_count == 1);
    REQUIRE(service->last_user_message == "hello guide");
    REQUIRE(map.currentChatInputBuffer().empty());
    REQUIRE(map.isChatInputOpen());
}

TEST_CASE("MapScene: runtime chat handles backspace cancel and empty submit", "[scene][map][input][chatbot]") {
    MapScene map("001", 10, 10);
    auto service = std::make_shared<CapturingChatService>();
    map.startChatbot("Guide", service);
    map.openChatInput();

    urpg::input::InputCore input;
    input.appendTextInput("ab\xC3\xA9");
    map.handleInput(input);
    input.endFrame();

    input.recordBackspace();
    map.handleInput(input);
    REQUIRE(map.currentChatInputBuffer() == "ab");

    input.endFrame();
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    map.openChatInput();
    map.handleInput(input);
    REQUIRE(service->request_count == 0);
    REQUIRE(map.isChatInputOpen());

    input.endFrame();
    input.updateActionState(urpg::input::InputAction::Cancel, urpg::input::ActionState::Pressed);
    input.appendTextInput("discarded");
    map.handleInput(input);

    REQUIRE_FALSE(map.isChatInputOpen());
    REQUIRE(map.currentChatInputBuffer().empty());
    REQUIRE(service->request_count == 0);
}

class MockScene : public GameScene {
  public:
    MockScene(SceneType type, const std::string& name) : m_type(type), m_name(name) {}

    SceneType getType() const override { return m_type; }
    std::string getName() const override { return m_name; }

    void onStart() override { m_startCount++; }
    void onStop() override { m_stopCount++; }
    void onUpdate(float /*dt*/) override { m_updateCount++; }
    void handleInput(const urpg::input::InputCore& input) override {
        if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) {
            m_inputCount++;
        }
    }

    int m_startCount = 0;
    int m_stopCount = 0;
    int m_updateCount = 0;
    int m_inputCount = 0;

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

TEST_CASE("SceneManager pause and resume preserve stack and clear stale input edges", "[scene][core][input][pause]") {
    SceneManager manager;
    auto title = std::make_shared<MockScene>(SceneType::TITLE, "TitleScreen");
    auto map = std::make_shared<MockScene>(SceneType::MAP, "Overworld");
    manager.gotoScene(title);
    manager.pushScene(map);

    urpg::input::InputCore input;
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    REQUIRE(input.isActionJustPressed(urpg::input::InputAction::Confirm));

    manager.pauseActiveScene(&input);
    REQUIRE(manager.stackSize() == 2);
    REQUIRE(manager.getActiveScene() == map);
    REQUIRE(map->isPaused());
    REQUIRE_FALSE(input.isActionActive(urpg::input::InputAction::Confirm));

    manager.handleInput(input);
    manager.update(0.016f);
    REQUIRE(map->m_inputCount == 0);
    REQUIRE(map->m_updateCount == 0);

    manager.resumeActiveScene();
    REQUIRE_FALSE(map->isPaused());

    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    manager.handleInput(input);
    manager.update(0.016f);
    REQUIRE(map->m_inputCount == 1);
    REQUIRE(map->m_updateCount == 1);
    REQUIRE(manager.stackSize() == 2);
}

TEST_CASE("MenuScene routes runtime confirm and cancel input through the native menu graph", "[scene][menu][input]") {
    MenuScene runtimeMenu("RuntimeMenu");
    auto graphScene = std::make_shared<urpg::ui::MenuScene>("root");

    urpg::MenuCommandMeta command;
    command.id = "open_items";
    command.label = "Items";
    command.route = urpg::MenuRouteTarget::Item;

    urpg::ui::MenuPane pane;
    pane.id = "commands";
    pane.isActive = true;
    pane.commands = {command};
    graphScene->addPane(pane);

    auto& graph = runtimeMenu.getSceneGraphMutable();
    graph.registerScene(graphScene);
    graph.pushScene("root");

    urpg::input::InputCore input;
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    runtimeMenu.handleInput(input);

    REQUIRE(graph.getLastBlockedCommandId() == "open_items");
    REQUIRE(graph.getLastBlockedReason() == "Route resolver unavailable.");

    input.endFrame();
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Released);
    input.endFrame();
    input.updateActionState(urpg::input::InputAction::Cancel, urpg::input::ActionState::Pressed);
    graph.setAllowRootCancelPop(true);
    runtimeMenu.handleInput(input);

    REQUIRE(graph.stackSize() == 0);
}

TEST_CASE("RuntimeTitleScene exposes startup commands with explicit disabled states", "[scene][runtime][title]") {
    RuntimeTitleScene title;

    REQUIRE(title.getType() == SceneType::TITLE);
    REQUIRE(title.getName() == "RuntimeTitle");
    REQUIRE(title.commands().size() == 4);

    const auto* newGame = title.findCommand(RuntimeTitleCommandId::NewGame);
    REQUIRE(newGame != nullptr);
    REQUIRE(newGame->label == "New Game");
    REQUIRE(newGame->enabled);

    const auto* continueGame = title.findCommand(RuntimeTitleCommandId::Continue);
    REQUIRE(continueGame != nullptr);
    REQUIRE(continueGame->label == "Continue");
    REQUIRE_FALSE(continueGame->enabled);
    REQUIRE(continueGame->disabled_reason == "No save data found");

    const auto* options = title.findCommand(RuntimeTitleCommandId::Options);
    REQUIRE(options != nullptr);
    REQUIRE(options->label == "Options");
    REQUIRE_FALSE(options->enabled);
    REQUIRE(options->disabled_reason == "Options flow is not wired yet");

    const auto* exit = title.findCommand(RuntimeTitleCommandId::Exit);
    REQUIRE(exit != nullptr);
    REQUIRE(exit->label == "Exit");
    REQUIRE(exit->enabled);
}

TEST_CASE("RuntimeTitleScene routes New Game and Exit through callbacks", "[scene][runtime][title]") {
    bool newGameStarted = false;
    bool exitRequested = false;
    RuntimeTitleScene title({
        [&newGameStarted] { newGameStarted = true; },
        [&exitRequested] { exitRequested = true; },
        {},
        {},
    });

    const auto newGameResult = title.activateCommand(RuntimeTitleCommandId::NewGame);
    REQUIRE(newGameResult.handled);
    REQUIRE(newGameResult.success);
    REQUIRE(newGameResult.code == "new_game_started");
    REQUIRE(newGameStarted);

    const auto exitResult = title.activateCommand(RuntimeTitleCommandId::Exit);
    REQUIRE(exitResult.handled);
    REQUIRE(exitResult.success);
    REQUIRE(exitResult.code == "exit_requested");
    REQUIRE(exitRequested);
}

TEST_CASE("RuntimeTitleScene enables Options when runtime supplies an options flow", "[scene][runtime][title]") {
    bool optionsOpened = false;
    RuntimeTitleScene title({
        {},
        {},
        {},
        [&optionsOpened] { optionsOpened = true; },
    });

    const auto* options = title.findCommand(RuntimeTitleCommandId::Options);
    REQUIRE(options != nullptr);
    REQUIRE(options->enabled);
    REQUIRE(options->disabled_reason.empty());

    const auto result = title.activateCommand(RuntimeTitleCommandId::Options);
    REQUIRE(result.handled);
    REQUIRE(result.success);
    REQUIRE(result.code == "options_opened");
    REQUIRE(optionsOpened);
}

TEST_CASE("RuntimeTitleScene rejects disabled startup commands with actionable reasons", "[scene][runtime][title]") {
    RuntimeTitleScene title;

    const auto continueResult = title.activateCommand(RuntimeTitleCommandId::Continue);
    REQUIRE(continueResult.handled);
    REQUIRE_FALSE(continueResult.success);
    REQUIRE(continueResult.code == "command_disabled");
    REQUIRE(continueResult.message == "No save data found");

    const auto optionsResult = title.activateCommand(RuntimeTitleCommandId::Options);
    REQUIRE(optionsResult.handled);
    REQUIRE_FALSE(optionsResult.success);
    REQUIRE(optionsResult.code == "command_disabled");
    REQUIRE(optionsResult.message == "Options flow is not wired yet");
}

TEST_CASE("RuntimeTitleScene runtime input moves focus across visible commands", "[scene][runtime][title][input]") {
    RuntimeTitleScene title;
    urpg::input::InputCore input;

    REQUIRE(title.selectedCommandIndex() == 0);
    REQUIRE(title.selectedCommand() != nullptr);
    REQUIRE(title.selectedCommand()->id == RuntimeTitleCommandId::NewGame);

    input.updateActionState(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Pressed);
    title.handleInput(input);
    REQUIRE(title.selectedCommandIndex() == 1);
    REQUIRE(title.selectedCommand() != nullptr);
    REQUIRE(title.selectedCommand()->id == RuntimeTitleCommandId::Continue);

    input.updateActionState(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Released);
    input.updateActionState(urpg::input::InputAction::MoveUp, urpg::input::ActionState::Pressed);
    title.handleInput(input);
    REQUIRE(title.selectedCommandIndex() == 0);
    REQUIRE(title.selectedCommand() != nullptr);
    REQUIRE(title.selectedCommand()->id == RuntimeTitleCommandId::NewGame);

    input.updateActionState(urpg::input::InputAction::MoveUp, urpg::input::ActionState::Released);
    title.handleInput(input);
    input.updateActionState(urpg::input::InputAction::MoveUp, urpg::input::ActionState::Pressed);
    title.handleInput(input);
    REQUIRE(title.selectedCommandIndex() == 3);
    REQUIRE(title.selectedCommand() != nullptr);
    REQUIRE(title.selectedCommand()->id == RuntimeTitleCommandId::Exit);
}

TEST_CASE("RuntimeTitleScene runtime input activates the selected command", "[scene][runtime][title][input]") {
    bool exitRequested = false;
    RuntimeTitleScene title({
        {},
        [&exitRequested] { exitRequested = true; },
        {},
        {},
    });
    urpg::input::InputCore input;

    input.updateActionState(urpg::input::InputAction::MoveUp, urpg::input::ActionState::Pressed);
    title.handleInput(input);
    REQUIRE(title.selectedCommand() != nullptr);
    REQUIRE(title.selectedCommand()->id == RuntimeTitleCommandId::Exit);

    input.updateActionState(urpg::input::InputAction::MoveUp, urpg::input::ActionState::Released);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    title.handleInput(input);

    REQUIRE(exitRequested);
    REQUIRE(title.lastCommandResult().handled);
    REQUIRE(title.lastCommandResult().success);
    REQUIRE(title.lastCommandResult().code == "exit_requested");
}

TEST_CASE("RuntimeTitleScene runtime input reports disabled selected command", "[scene][runtime][title][input]") {
    RuntimeTitleScene title;
    urpg::input::InputCore input;

    input.updateActionState(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Pressed);
    title.handleInput(input);
    REQUIRE(title.selectedCommand() != nullptr);
    REQUIRE(title.selectedCommand()->id == RuntimeTitleCommandId::Continue);

    input.updateActionState(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Released);
    input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    title.handleInput(input);

    REQUIRE(title.lastCommandResult().handled);
    REQUIRE_FALSE(title.lastCommandResult().success);
    REQUIRE(title.lastCommandResult().code == "command_disabled");
    REQUIRE(title.lastCommandResult().message == "No save data found");
}

TEST_CASE("RuntimeTitleScene exposes startup warnings and errors for title UI",
          "[scene][runtime][title][diagnostics]") {
    RuntimeTitleScene title;
    urpg::RuntimeStartupReport report;
    report.subsystems.push_back({"LocaleCatalog", urpg::RuntimeStartupSubsystemStatus::Warning,
                                 "localization.catalog_missing", "No locale catalog was found under project content."});
    report.subsystems.push_back({"PerfProfiler", urpg::RuntimeStartupSubsystemStatus::Initialized, "profiler.ready",
                                 "PerfProfiler initialized."});
    report.subsystems.push_back({"RuntimeBundleLoader", urpg::RuntimeStartupSubsystemStatus::Error,
                                 "runtime_bundle.validation_failed", "Runtime asset bundle validation failed."});

    title.setStartupReport(report);

    REQUIRE(title.startupNotices().size() == 2);
    REQUIRE(title.startupNotices()[0].status == urpg::RuntimeStartupSubsystemStatus::Warning);
    REQUIRE(title.startupNotices()[0].subsystem == "LocaleCatalog");
    REQUIRE(title.startupNotices()[0].code == "localization.catalog_missing");
    REQUIRE(title.startupNotices()[1].status == urpg::RuntimeStartupSubsystemStatus::Error);
    REQUIRE(title.startupNotices()[1].subsystem == "RuntimeBundleLoader");
}

TEST_CASE("RuntimeTitleScene New Game can transition to the existing runtime boot map", "[scene][runtime][title]") {
    SceneManager manager;
    auto title = makeDefaultRuntimeTitleScene({
        [&manager] { manager.gotoScene(std::make_shared<MapScene>("RuntimeBoot", 16, 12)); },
        {},
        {},
        {},
    });

    manager.gotoScene(title);
    REQUIRE(manager.getActiveScene() != nullptr);
    REQUIRE(manager.getActiveScene()->getType() == SceneType::TITLE);
    REQUIRE(manager.getActiveScene()->getName() == "RuntimeTitle");

    const auto result = title->activateCommand(RuntimeTitleCommandId::NewGame);
    REQUIRE(result.success);
    REQUIRE(manager.getActiveScene() != nullptr);
    REQUIRE(manager.getActiveScene()->getType() == SceneType::MAP);
    REQUIRE(manager.getActiveScene()->getName() == "Map_RuntimeBoot");
}

TEST_CASE("RuntimeOptionsScene edits display audio input and accessibility settings", "[scene][runtime][options]") {
    const TempRuntimeSettingsRoot temp;
    const auto paths = urpg::settings::appSettingsPaths(temp.root());
    auto settings = urpg::settings::defaultRuntimeSettings();
    settings.window.width = 1280;
    settings.audio.master_volume = 1.0f;
    settings.accessibility.high_contrast = false;

    bool savedCallback = false;
    RuntimeOptionsScene scene(settings, paths.runtime_settings,
                              {
                                  {},
                                  [&savedCallback](const urpg::settings::RuntimeSettings& savedSettings) {
                                      savedCallback = true;
                                      REQUIRE(savedSettings.window.width == 1360);
                                      REQUIRE(savedSettings.audio.master_volume == 0.9f);
                                      REQUIRE(savedSettings.accessibility.high_contrast);
                                  },
                              });

    REQUIRE(scene.getType() == SceneType::OPTIONS);
    REQUIRE(scene.getName() == "RuntimeOptions");
    REQUIRE(scene.rows().size() == 23);
    REQUIRE(scene.selectedRow() != nullptr);
    REQUIRE(scene.selectedRow()->id == RuntimeOptionsRowId::WindowWidth);

    scene.adjustSelected(1);
    REQUIRE(scene.settings().window.width == 1360);

    urpg::input::InputCore input;
    auto press = [&](urpg::input::InputAction action) {
        input.updateActionState(action, urpg::input::ActionState::Pressed);
        scene.handleInput(input);
        input.endFrame();
        input.updateActionState(action, urpg::input::ActionState::Released);
        scene.handleInput(input);
        input.endFrame();
    };
    auto moveTo = [&](RuntimeOptionsRowId id) {
        for (size_t attempt = 0; attempt < scene.rows().size() && scene.selectedRow()->id != id; ++attempt) {
            press(urpg::input::InputAction::MoveDown);
        }
        REQUIRE(scene.selectedRow()->id == id);
    };

    moveTo(RuntimeOptionsRowId::MasterVolume);
    REQUIRE(scene.selectedRow() != nullptr);
    REQUIRE(scene.selectedRow()->id == RuntimeOptionsRowId::MasterVolume);
    press(urpg::input::InputAction::MoveLeft);
    REQUIRE(scene.settings().audio.master_volume == 0.9f);

    moveTo(RuntimeOptionsRowId::InputMapping);
    REQUIRE(scene.selectedRow() != nullptr);
    REQUIRE(scene.selectedRow()->id == RuntimeOptionsRowId::InputMapping);
    const auto inputResult = scene.activateSelected();
    REQUIRE(inputResult.success);
    REQUIRE(scene.settings().input_mapping_path == urpg::settings::defaultRuntimeSettings().input_mapping_path);

    moveTo(RuntimeOptionsRowId::HighContrast);
    REQUIRE(scene.selectedRow() != nullptr);
    REQUIRE(scene.selectedRow()->id == RuntimeOptionsRowId::HighContrast);
    press(urpg::input::InputAction::Confirm);
    REQUIRE(scene.settings().accessibility.high_contrast);

    moveTo(RuntimeOptionsRowId::Save);
    REQUIRE(scene.selectedRow() != nullptr);
    REQUIRE(scene.selectedRow()->id == RuntimeOptionsRowId::Save);
    press(urpg::input::InputAction::Confirm);

    REQUIRE(savedCallback);
    REQUIRE(scene.lastCommandResult().success);
    REQUIRE(scene.lastCommandResult().code == "settings_saved");

    const auto loaded = urpg::settings::loadRuntimeSettings(paths.runtime_settings);
    REQUIRE(loaded.report.loaded);
    REQUIRE(loaded.settings.window.width == 1360);
    REQUIRE(loaded.settings.audio.master_volume == 0.9f);
    REQUIRE(loaded.settings.accessibility.high_contrast);
}

TEST_CASE("RuntimeOptionsScene supports cancel/back navigation", "[scene][runtime][options][input]") {
    const TempRuntimeSettingsRoot temp;
    const auto paths = urpg::settings::appSettingsPaths(temp.root());

    bool backRequested = false;
    RuntimeOptionsScene scene(urpg::settings::defaultRuntimeSettings(), paths.runtime_settings,
                              {
                                  [&backRequested] { backRequested = true; },
                                  {},
                              });

    urpg::input::InputCore input;
    input.updateActionState(urpg::input::InputAction::Cancel, urpg::input::ActionState::Pressed);
    scene.handleInput(input);

    REQUIRE(backRequested);
    REQUIRE(scene.lastCommandResult().handled);
    REQUIRE(scene.lastCommandResult().success);
    REQUIRE(scene.lastCommandResult().code == "options_back");
}

TEST_CASE("RuntimeOptionsScene previews resets and rolls back complete inclusive settings",
          "[scene][runtime][options][accessibility][pcq651]") {
    const TempRuntimeSettingsRoot temp;
    const auto paths = urpg::settings::appSettingsPaths(temp.root());
    auto settings = urpg::settings::defaultRuntimeSettings();
    settings.accessibility.high_contrast = false;
    settings.accessibility.captions = true;
    settings.audio.se_volume = 0.8F;

    int previewCount = 0;
    int cancelCount = 0;
    urpg::settings::RuntimeSettings latestPreview;
    urpg::settings::RuntimeSettings cancelledTo;
    RuntimeOptionsScene scene(
        settings, paths.runtime_settings,
        {{}, {}, {},
         [&](const urpg::settings::RuntimeSettings& preview) {
             ++previewCount;
             latestPreview = preview;
         },
         [&](const urpg::settings::RuntimeSettings& baseline) {
             ++cancelCount;
             cancelledTo = baseline;
         }});

    urpg::input::InputCore input;
    auto press = [&](urpg::input::InputAction action) {
        input.updateActionState(action, urpg::input::ActionState::Pressed);
        scene.handleInput(input);
        input.endFrame();
        input.updateActionState(action, urpg::input::ActionState::Released);
        scene.handleInput(input);
        input.endFrame();
    };
    auto moveTo = [&](RuntimeOptionsRowId id) {
        for (size_t attempt = 0; attempt < scene.rows().size() && scene.selectedRow()->id != id; ++attempt) {
            press(urpg::input::InputAction::MoveDown);
        }
        REQUIRE(scene.selectedRow()->id == id);
    };

    moveTo(RuntimeOptionsRowId::EffectsVolume);
    press(urpg::input::InputAction::MoveLeft);
    REQUIRE(latestPreview.audio.se_volume == Catch::Approx(0.7F));

    moveTo(RuntimeOptionsRowId::ColorFilter);
    press(urpg::input::InputAction::MoveRight);
    REQUIRE(latestPreview.accessibility.color_filter == "protanopia");

    moveTo(RuntimeOptionsRowId::ReduceMotion);
    press(urpg::input::InputAction::Confirm);
    REQUIRE(latestPreview.accessibility.reduce_motion);
    REQUIRE(latestPreview.accessibility.screen_shake == 0.0F);

    moveTo(RuntimeOptionsRowId::Captions);
    press(urpg::input::InputAction::Confirm);
    REQUIRE_FALSE(latestPreview.accessibility.captions);

    moveTo(RuntimeOptionsRowId::CaptionScale);
    press(urpg::input::InputAction::MoveRight);
    REQUIRE(latestPreview.accessibility.caption_scale == Catch::Approx(1.1F));

    moveTo(RuntimeOptionsRowId::MonoAudio);
    press(urpg::input::InputAction::Confirm);
    REQUIRE(latestPreview.accessibility.mono_audio);

    moveTo(RuntimeOptionsRowId::ResetAccessibility);
    press(urpg::input::InputAction::Confirm);
    REQUIRE(scene.lastCommandResult().code == "accessibility_reset");
    REQUIRE_FALSE(latestPreview.accessibility.high_contrast);
    REQUIRE_FALSE(latestPreview.accessibility.reduce_motion);
    REQUIRE(latestPreview.accessibility.captions);
    REQUIRE(latestPreview.accessibility.non_color_cues);
    REQUIRE_FALSE(latestPreview.accessibility.mono_audio);
    REQUIRE(latestPreview.audio.master_volume == 1.0F);
    REQUIRE(latestPreview.audio.se_volume == 1.0F);
    REQUIRE(previewCount >= 7);

    press(urpg::input::InputAction::Cancel);
    REQUIRE(cancelCount == 1);
    REQUIRE(cancelledTo.audio.se_volume == Catch::Approx(0.8F));
    REQUIRE(scene.settings().audio.se_volume == Catch::Approx(0.8F));
    REQUIRE(scene.settings().accessibility.captions);
}

TEST_CASE("RuntimeOptionsScene exposes calibration replay from settings", "[scene][runtime][options][calibration][pcq606]") {
    const TempRuntimeSettingsRoot temp;
    const auto paths = urpg::settings::appSettingsPaths(temp.root());
    bool calibrationRequested = false;
    RuntimeOptionsScene scene(urpg::settings::defaultRuntimeSettings(), paths.runtime_settings,
                              {{}, {}, [&calibrationRequested] { calibrationRequested = true; }});

    REQUIRE(scene.rows().back().id == RuntimeOptionsRowId::FirstRunCalibration);
    REQUIRE(scene.rows().back().value == "Replay");

    urpg::input::InputCore input;
    for (size_t index = 0; index + 1 < scene.rows().size(); ++index) {
        input.updateActionState(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Pressed);
        scene.handleInput(input);
        input.endFrame();
        input.updateActionState(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Released);
        scene.handleInput(input);
        input.endFrame();
    }
    REQUIRE(scene.selectedRow()->id == RuntimeOptionsRowId::FirstRunCalibration);
    const auto result = scene.activateSelected();
    REQUIRE(result.success);
    REQUIRE(result.code == "calibration_replay_requested");
    REQUIRE(calibrationRequested);
}

TEST_CASE("RuntimeTitleScene emits visible title and disabled command labels", "[scene][runtime][title][render]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    RuntimeTitleScene title;
    title.onUpdate(0.016f);

    bool sawTitle = false;
    bool sawNewGame = false;
    bool sawDisabledContinue = false;
    bool sawDisabledOptions = false;
    bool sawExit = false;
    bool sawFocusHighlight = false;

    for (const auto& command : renderFrameCommands(layer)) {
        if (renderCommandType(command) == urpg::RenderCmdType::Rect) {
            const auto* rect = renderCommandAs<urpg::RectRenderData>(command);
            REQUIRE(rect != nullptr);
            sawFocusHighlight = sawFocusHighlight || (command.x == 56.0f && command.y == 112.0f && rect->w == 528.0f &&
                                                      rect->h == 30.0f && command.zOrder == 1);
            continue;
        }
        if (renderCommandType(command) != urpg::RenderCmdType::Text) {
            continue;
        }
        const auto* text = renderCommandAs<urpg::TextRenderData>(command);
        REQUIRE(text != nullptr);
        sawTitle = sawTitle || text->text == "URPG";
        sawNewGame = sawNewGame || text->text == "New Game";
        sawDisabledContinue = sawDisabledContinue || text->text == "Continue - No save data found";
        sawDisabledOptions = sawDisabledOptions || text->text == "Options - Options flow is not wired yet";
        sawExit = sawExit || text->text == "Exit";
    }

    REQUIRE(sawTitle);
    REQUIRE(sawNewGame);
    REQUIRE(sawDisabledContinue);
    REQUIRE(sawDisabledOptions);
    REQUIRE(sawExit);
    REQUIRE(sawFocusHighlight);

    layer.flush();
}

TEST_CASE("RuntimeTitleScene renders startup diagnostics on the title screen", "[scene][runtime][title][render]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    RuntimeTitleScene title;
    urpg::RuntimeStartupReport report;
    report.subsystems.push_back({"LocaleCatalog", urpg::RuntimeStartupSubsystemStatus::Warning,
                                 "localization.catalog_missing", "No locale catalog was found under project content."});
    title.setStartupReport(report);
    title.onUpdate(0.016f);

    bool sawDiagnosticsHeading = false;
    bool sawLocalizationWarning = false;
    for (const auto& command : renderFrameCommands(layer)) {
        if (renderCommandType(command) != urpg::RenderCmdType::Text) {
            continue;
        }
        const auto* text = renderCommandAs<urpg::TextRenderData>(command);
        REQUIRE(text != nullptr);
        sawDiagnosticsHeading = sawDiagnosticsHeading || text->text == "Startup Diagnostics";
        sawLocalizationWarning =
            sawLocalizationWarning ||
            text->text.find("Startup warning [LocaleCatalog:localization.catalog_missing]") != std::string::npos;
    }

    REQUIRE(sawDiagnosticsHeading);
    REQUIRE(sawLocalizationWarning);

    layer.flush();
}
