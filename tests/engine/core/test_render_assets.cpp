#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <iostream>
#include "engine/core/render/render_layer.h"
#include "engine/core/render/asset_loader.h"
#include "engine/core/assets/texture_registry.h"
#include "engine/core/platform/opengl_renderer.h"
#include "engine/core/platform/renderer_backend.h"
#include "engine/core/scene/map_scene.h"

using namespace urpg;
using namespace urpg::scene;

namespace {

class FrameAdapterCaptureRenderer final : public RendererBackend {
public:
    RenderTier getTier() const override {
        return RenderTier::Basic;
    }

    bool initialize(IPlatformSurface* /*surface*/) override {
        return true;
    }

    void beginFrame() override {}
    void renderBatches(const std::vector<SpriteDrawData>& /*batches*/) override {}
    void endFrame() override {}

    void processCommands(const std::vector<std::shared_ptr<RenderCommand>>& commands) override {
        capturedCommands = commands;
    }

    void shutdown() override {}
    void onResize(int /*width*/, int /*height*/) override {}

    std::vector<std::shared_ptr<RenderCommand>> capturedCommands;
};

} // namespace

TEST_CASE("Render Layer Batching", "[render][core]") {
    auto& layer = RenderLayer::getInstance();
    layer.flush();
    
    SpriteCommand cmd1;
    cmd1.textureId = "test_tex";
    cmd1.x = 10;
    cmd1.y = 20;
    
    layer.submit(toFrameRenderCommand(cmd1));
    
    const auto& frameCommands = layer.getFrameCommands();
    REQUIRE(frameCommands.size() == 1);
    REQUIRE(frameCommands[0].type == RenderCmdType::Sprite);

    const auto* spriteData = frameCommands[0].tryGet<SpriteRenderData>();
    REQUIRE(spriteData != nullptr);
    REQUIRE(spriteData->textureId == "test_tex");

    const auto& legacyCommands = layer.getCommands();
    REQUIRE(legacyCommands.size() == 1);
    REQUIRE(legacyCommands[0]->type == RenderCmdType::Sprite);
}

TEST_CASE("Frame render commands preserve text and rect payloads through legacy conversion", "[render][core][td02]") {
    SECTION("Text commands retain authored fields") {
        TextCommand text;
        text.x = 12.0f;
        text.y = 34.0f;
        text.zOrder = 7;
        text.text = "Status: OK";
        text.fontFace = "MZ-System";
        text.fontSize = 18;
        text.maxWidth = 144;
        text.r = 10;
        text.g = 20;
        text.b = 30;
        text.a = 40;

        const auto frameCommand = toFrameRenderCommand(text);
        REQUIRE(frameCommand.type == RenderCmdType::Text);
        REQUIRE(frameCommand.x == 12.0f);
        REQUIRE(frameCommand.y == 34.0f);
        REQUIRE(frameCommand.zOrder == 7);

        const auto* frameText = frameCommand.tryGet<TextRenderData>();
        REQUIRE(frameText != nullptr);
        REQUIRE(frameText->text == "Status: OK");
        REQUIRE(frameText->fontFace == "MZ-System");
        REQUIRE(frameText->fontSize == 18);
        REQUIRE(frameText->maxWidth == 144);
        REQUIRE(frameText->r == 10);
        REQUIRE(frameText->g == 20);
        REQUIRE(frameText->b == 30);
        REQUIRE(frameText->a == 40);

        const auto legacyCommand = toLegacyRenderCommand(frameCommand);
        REQUIRE(legacyCommand != nullptr);
        REQUIRE(legacyCommand->type == RenderCmdType::Text);

        const auto* legacyText = dynamic_cast<const TextCommand*>(legacyCommand.get());
        REQUIRE(legacyText != nullptr);
        REQUIRE(legacyText->text == "Status: OK");
        REQUIRE(legacyText->fontFace == "MZ-System");
        REQUIRE(legacyText->fontSize == 18);
        REQUIRE(legacyText->maxWidth == 144);
        REQUIRE(legacyText->r == 10);
        REQUIRE(legacyText->g == 20);
        REQUIRE(legacyText->b == 30);
        REQUIRE(legacyText->a == 40);
    }

    SECTION("Rect commands retain geometry and color") {
        RectCommand rect;
        rect.x = 40.0f;
        rect.y = 80.0f;
        rect.zOrder = 3;
        rect.w = 160.0f;
        rect.h = 48.0f;
        rect.r = 0.1f;
        rect.g = 0.2f;
        rect.b = 0.3f;
        rect.a = 0.4f;

        const auto frameCommand = toFrameRenderCommand(rect);
        REQUIRE(frameCommand.type == RenderCmdType::Rect);
        REQUIRE(frameCommand.x == 40.0f);
        REQUIRE(frameCommand.y == 80.0f);
        REQUIRE(frameCommand.zOrder == 3);

        const auto* frameRect = frameCommand.tryGet<RectRenderData>();
        REQUIRE(frameRect != nullptr);
        REQUIRE(frameRect->w == 160.0f);
        REQUIRE(frameRect->h == 48.0f);
        REQUIRE(frameRect->r == 0.1f);
        REQUIRE(frameRect->g == 0.2f);
        REQUIRE(frameRect->b == 0.3f);
        REQUIRE(frameRect->a == 0.4f);

        const auto legacyCommand = toLegacyRenderCommand(frameCommand);
        REQUIRE(legacyCommand != nullptr);
        REQUIRE(legacyCommand->type == RenderCmdType::Rect);

        const auto* legacyRect = dynamic_cast<const RectCommand*>(legacyCommand.get());
        REQUIRE(legacyRect != nullptr);
        REQUIRE(legacyRect->w == 160.0f);
        REQUIRE(legacyRect->h == 48.0f);
        REQUIRE(legacyRect->r == 0.1f);
        REQUIRE(legacyRect->g == 0.2f);
        REQUIRE(legacyRect->b == 0.3f);
        REQUIRE(legacyRect->a == 0.4f);
    }
}

TEST_CASE("RendererBackend frame-command adapter preserves command payloads for legacy overrides", "[render][core][td02]") {
    FrameAdapterCaptureRenderer renderer;

    TextCommand text;
    text.x = 16.0f;
    text.y = 24.0f;
    text.zOrder = 2;
    text.text = "Frame-owned";
    text.fontFace = "GameFont";
    text.fontSize = 20;
    text.maxWidth = 180;
    text.r = 90;
    text.g = 91;
    text.b = 92;
    text.a = 93;

    RectCommand rect;
    rect.x = 32.0f;
    rect.y = 48.0f;
    rect.zOrder = 4;
    rect.w = 96.0f;
    rect.h = 24.0f;
    rect.r = 0.6f;
    rect.g = 0.5f;
    rect.b = 0.4f;
    rect.a = 0.3f;

    FrameRenderCommand clear;
    clear.type = RenderCmdType::Clear;
    clear.zOrder = 9;
    clear.x = 1.0f;
    clear.y = 2.0f;

    renderer.processFrameCommands({toFrameRenderCommand(text), toFrameRenderCommand(rect), clear});

    REQUIRE(renderer.capturedCommands.size() == 3);
    REQUIRE(renderer.capturedCommands[0] != nullptr);
    REQUIRE(renderer.capturedCommands[1] != nullptr);
    REQUIRE(renderer.capturedCommands[2] != nullptr);

    const auto* legacyText = dynamic_cast<const TextCommand*>(renderer.capturedCommands[0].get());
    REQUIRE(legacyText != nullptr);
    REQUIRE(legacyText->text == "Frame-owned");
    REQUIRE(legacyText->fontFace == "GameFont");
    REQUIRE(legacyText->fontSize == 20);
    REQUIRE(legacyText->maxWidth == 180);
    REQUIRE(legacyText->r == 90);
    REQUIRE(legacyText->g == 91);
    REQUIRE(legacyText->b == 92);
    REQUIRE(legacyText->a == 93);

    const auto* legacyRect = dynamic_cast<const RectCommand*>(renderer.capturedCommands[1].get());
    REQUIRE(legacyRect != nullptr);
    REQUIRE(legacyRect->w == 96.0f);
    REQUIRE(legacyRect->h == 24.0f);
    REQUIRE(legacyRect->r == 0.6f);
    REQUIRE(legacyRect->g == 0.5f);
    REQUIRE(legacyRect->b == 0.4f);
    REQUIRE(legacyRect->a == 0.3f);

    REQUIRE(renderer.capturedCommands[2]->type == RenderCmdType::Clear);
    REQUIRE(renderer.capturedCommands[2]->x == 1.0f);
    REQUIRE(renderer.capturedCommands[2]->y == 2.0f);
    REQUIRE(renderer.capturedCommands[2]->zOrder == 9);
}

TEST_CASE("OpenGLRenderer frame-owned text and rect commands stay silent before GL initialization", "[render][core][td02][opengl]") {
    OpenGLRenderer renderer;
    renderer.onResize(320, 240);

    TextCommand text;
    text.x = 32.0f;
    text.y = 48.0f;
    text.text = "AB AB";
    text.fontSize = 16;
    text.maxWidth = 48;
    text.r = 255;
    text.g = 128;
    text.b = 64;
    text.a = 255;

    RectCommand rect;
    rect.x = 20.0f;
    rect.y = 30.0f;
    rect.w = 64.0f;
    rect.h = 18.0f;
    rect.r = 0.25f;
    rect.g = 0.5f;
    rect.b = 0.75f;
    rect.a = 1.0f;

    std::ostringstream captured;
    auto* originalBuffer = std::cout.rdbuf(captured.rdbuf());

    REQUIRE_NOTHROW(renderer.processFrameCommands({
        toFrameRenderCommand(text),
        toFrameRenderCommand(rect),
        toFrameRenderCommand(text),
    }));

    std::cout.rdbuf(originalBuffer);

    REQUIRE(captured.str().empty());
}

TEST_CASE("OpenGLRenderer legacy command intake stays silent before GL initialization", "[render][core][td02][opengl]") {
    OpenGLRenderer renderer;
    renderer.onResize(160, 120);

    auto text = std::make_shared<TextCommand>();
    text->x = 8.0f;
    text->y = 12.0f;
    text->text = "Legacy";
    text->fontSize = 14;

    auto rect = std::make_shared<RectCommand>();
    rect->x = 4.0f;
    rect->y = 6.0f;
    rect->w = 20.0f;
    rect->h = 10.0f;

    std::ostringstream captured;
    auto* originalBuffer = std::cout.rdbuf(captured.rdbuf());

    REQUIRE_NOTHROW(renderer.processCommands({text, rect}));

    std::cout.rdbuf(originalBuffer);

    REQUIRE(captured.str().empty());
}

TEST_CASE("MapScene Render Sync", "[render][scene]") {
    auto& layer = RenderLayer::getInstance();
    layer.flush();
    
    MapScene map("RenderTest", 2, 2);
    map.setTile(0, 0, 101, true);
    
    // Trigger render submission
    map.onUpdate(0.016f);
    
    const auto& cmds = layer.getFrameCommands();
    // 2x2 map = 4 tile commands + 1 player sprite = 5
    REQUIRE(cmds.size() == 5);
    
    bool foundTile = false;
    bool foundPlayer = false;
    
    for (const auto& cmd : cmds) {
        if (cmd.type == RenderCmdType::Tile) {
            const auto* tileCmd = cmd.tryGet<TileRenderData>();
            if (tileCmd != nullptr && tileCmd->tileIndex == 101) {
                foundTile = true;
            }
        }
        if (cmd.type == RenderCmdType::Sprite) {
            const auto* spriteCmd = cmd.tryGet<SpriteRenderData>();
            if (spriteCmd != nullptr && spriteCmd->textureId == "hero_sprite") {
                foundPlayer = true;
            }
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
    AssetLoader::clearCaches();

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

TEST_CASE("AssetLoader bounded missing-texture cache re-warns after eviction", "[assets][core][loader]") {
    const std::string evictedPath = "img/__missing__/asset_loader_negative_cache_evicted.png";
    AssetLoader::clearCaches();

    std::ostringstream captured;
    auto* originalBuffer = std::cerr.rdbuf(captured.rdbuf());

    auto firstLoad = AssetLoader::loadTexture(evictedPath);
    REQUIRE(firstLoad == nullptr);

    for (int index = 0; index < 256; ++index) {
        const auto path = "img/__missing__/asset_loader_negative_cache_filler_" + std::to_string(index) + ".png";
        REQUIRE(AssetLoader::loadTexture(path) == nullptr);
    }

    auto secondLoad = AssetLoader::loadTexture(evictedPath);

    std::cerr.rdbuf(originalBuffer);

    REQUIRE(secondLoad == nullptr);

    const std::string output = captured.str();
    const std::string needle = "[URPG][AssetLoader] Failed to load texture: " + evictedPath;
    REQUIRE(output.find(needle) != std::string::npos);
    REQUIRE(output.find(needle, output.find(needle) + 1) != std::string::npos);
}

TEST_CASE("AssetLoader clearCaches drops negative cache state so missing-texture diagnostics reappear",
          "[assets][core][loader]") {
    const std::string path = "img/__missing__/asset_loader_negative_cache_reset.png";
    AssetLoader::clearCaches();

    std::ostringstream captured;
    auto* originalBuffer = std::cerr.rdbuf(captured.rdbuf());

    REQUIRE(AssetLoader::loadTexture(path) == nullptr);
    REQUIRE(AssetLoader::loadTexture(path) == nullptr);

    AssetLoader::clearCaches();
    REQUIRE(AssetLoader::loadTexture(path) == nullptr);

    std::cerr.rdbuf(originalBuffer);

    const std::string output = captured.str();
    const std::string needle = "[URPG][AssetLoader] Failed to load texture: " + path;
    REQUIRE(output.find(needle) != std::string::npos);
    REQUIRE(output.find(needle, output.find(needle) + 1) != std::string::npos);
}
