#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <iostream>
#include "engine/core/render/render_layer.h"
#include "engine/core/render/asset_loader.h"
#include "engine/core/assets/texture_registry.h"
#include "engine/core/scene/map_scene.h"

using namespace urpg;
using namespace urpg::scene;

TEST_CASE("Render Layer Batching", "[render][core]") {
    auto& layer = RenderLayer::getInstance();
    layer.flush();
    
    auto cmd1 = std::make_shared<SpriteCommand>();
    cmd1->textureId = "test_tex";
    cmd1->x = 10;
    cmd1->y = 20;
    
    layer.submit(cmd1);
    
    REQUIRE(layer.getCommands().size() == 1);
    REQUIRE(layer.getCommands()[0]->type == RenderCmdType::Sprite);
}

TEST_CASE("MapScene Render Sync", "[render][scene]") {
    auto& layer = RenderLayer::getInstance();
    layer.flush();
    
    MapScene map("RenderTest", 2, 2);
    map.setTile(0, 0, 101, true);
    
    // Trigger render submission
    map.onUpdate(0.016f);
    
    auto& cmds = layer.getCommands();
    // 2x2 map = 4 tile commands + 1 player sprite = 5
    REQUIRE(cmds.size() == 5);
    
    bool foundTile = false;
    bool foundPlayer = false;
    
    for (const auto& cmd : cmds) {
        if (cmd->type == RenderCmdType::Tile) {
            auto tileCmd = std::static_pointer_cast<TileCommand>(cmd);
            if (tileCmd->tileIndex == 101) foundTile = true;
        }
        if (cmd->type == RenderCmdType::Sprite) {
            auto spriteCmd = std::static_pointer_cast<SpriteCommand>(cmd);
            if (spriteCmd->textureId == "hero_sprite") foundPlayer = true;
        }
    }
    
    REQUIRE(foundTile);
    REQUIRE(foundPlayer);
}

TEST_CASE("Asset Cache LRU Eviction Test", "[assets][core][lru]") {
    AssetCache<TextureMeta> cache(2); // Capacity of 2
    
    auto a1 = std::make_shared<TextureMeta>(); a1->setId("1");
    auto a2 = std::make_shared<TextureMeta>(); a2->setId("2");
    auto a3 = std::make_shared<TextureMeta>(); a3->setId("3");
    
    cache.store(a1);
    cache.store(a2);
    REQUIRE(cache.size() == 2);
    
    // Use A1 (moves it to the front)
    cache.get("1");
    
    // Store A3, should evict A2 (since A1 was used)
    cache.store(a3);
    
    REQUIRE(cache.size() == 2);
    REQUIRE(cache.has("1") == true);
    REQUIRE(cache.has("3") == true);
    REQUIRE(cache.has("2") == false); // EVICTED
}

TEST_CASE("Texture Registry Persistence", "[assets][core]") {
    auto& registry = TextureRegistry::getInstance();
    registry.clear();
    
    TextureMeta meta;
    meta.filePath = "img/characters/Hero.png";
    meta.width = 144;
    meta.height = 192;
    
    registry.registerTexture("hero_sprite", meta);
    
    auto cached = registry.getTexture("hero_sprite");
    REQUIRE(cached != nullptr);
    REQUIRE(cached->width == 144);
    REQUIRE(cached->filePath == "img/characters/Hero.png");
}

TEST_CASE("AssetLoader caches missing texture failures to avoid repeated warning spam", "[assets][core][loader]") {
    const std::string missingPath = "img/__missing__/asset_loader_negative_cache_once.png";

    std::ostringstream captured;
    auto* originalBuffer = std::cerr.rdbuf(captured.rdbuf());

    auto firstLoad = AssetLoader::loadTexture(missingPath);
    auto secondLoad = AssetLoader::loadTexture(missingPath);

    std::cerr.rdbuf(originalBuffer);

    REQUIRE(firstLoad == nullptr);
    REQUIRE(secondLoad == nullptr);

    const std::string output = captured.str();
    const std::string needle = "[URPG][AssetLoader] Failed to load texture: " + missingPath;
    REQUIRE(output.find(needle) != std::string::npos);
    REQUIRE(output.find(needle, output.find(needle) + 1) == std::string::npos);
}
