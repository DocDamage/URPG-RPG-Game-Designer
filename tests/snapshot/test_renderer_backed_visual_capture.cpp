#include "engine/core/engine_shell.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/menu_scene.h"
#include "engine/core/message/message_core.h"
#include "engine/core/platform/gl_texture.h"
#include "engine/core/platform/opengl_renderer.h"
#include "engine/core/sprite_batcher.h"
#include "engine/core/testing/visual_regression_harness.h"
#include "engine/core/ui/chat_window.h"
#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/window_compat.h"

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string_view>

using namespace urpg;
using namespace urpg::testing;

#ifndef URPG_HEADLESS
namespace {

std::filesystem::path getGoldenRoot() {
    return std::filesystem::path(URPG_SOURCE_DIR) / "tests" / "snapshot" / "goldens";
}

bool shouldRegenerateRendererBackedGoldens() {
    const char* value = std::getenv("URPG_REGEN_RENDERER_BACKED_GOLDENS");
    if (value == nullptr) {
        return false;
    }

    return std::string_view(value) == "1";
}

SceneSnapshot cropSnapshot(const SceneSnapshot& source, int x, int y, int width, int height) {
    SceneSnapshot cropped;
    if (source.width <= 0 || source.height <= 0 || source.pixels.empty()) {
        return cropped;
    }

    const int clampedX = std::clamp(x, 0, source.width);
    const int clampedY = std::clamp(y, 0, source.height);
    const int clampedWidth = std::clamp(width, 0, source.width - clampedX);
    const int clampedHeight = std::clamp(height, 0, source.height - clampedY);

    cropped.width = clampedWidth;
    cropped.height = clampedHeight;
    cropped.pixels.reserve(static_cast<size_t>(clampedWidth) * clampedHeight);

    for (int row = 0; row < clampedHeight; ++row) {
        const int sourceY = clampedY + row;
        for (int col = 0; col < clampedWidth; ++col) {
            const int sourceX = clampedX + col;
            const size_t sourceIndex = static_cast<size_t>(sourceY) * source.width + sourceX;
            cropped.pixels.push_back(source.pixels[sourceIndex]);
        }
    }

    return cropped;
}

SceneSnapshot captureMapSceneDialogueOverlayCrop(bool startDialogue) {
    auto& layer = RenderLayer::getInstance();
    layer.flush();

    urpg::scene::MapScene map("SnapshotMap", 2, 2);
    map.setAssetReferences({
        {"hero_sprite", {}},
        {"default_tileset", {}},
    });
    if (startDialogue) {
        map.startDialogue({
            {"page_1",
             "Hello from native message",
             urpg::message::variantFromCompatRoute("speaker", "Elder", 1),
             true,
             {},
             0},
        });
    }

    map.onUpdate(0.0f);

    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame(layer.getFrameCommands(), 640, 400, &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    const auto cropped = cropSnapshot(*snapshot, 20, 280, 200, 80);
    layer.flush();
    return cropped;
}

SceneSnapshot captureMapSceneWorldSnapshot() {
    auto& layer = RenderLayer::getInstance();
    layer.flush();

    urpg::scene::MapScene map("SnapshotWorld", 2, 2);
    map.setAssetReferences({
        {"hero_sprite", {}},
        {"default_tileset", {}},
    });
    map.setTile(0, 0, 101, true);
    map.setTile(1, 0, 5, true);
    map.setTile(0, 1, 17, true);
    map.setTile(1, 1, 33, true);
    map.onUpdate(0.0f);

    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame(layer.getFrameCommands(), 96, 96, &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    layer.flush();
    return *snapshot;
}

std::vector<uint8_t> makeMapSceneTilesetPixels() {
    constexpr int textureWidth = 96;
    constexpr int textureHeight = 96;
    constexpr int tileSize = 48;
    std::vector<uint8_t> pixels(static_cast<size_t>(textureWidth) * textureHeight * 4, 0);

    const auto writeTile = [&](int tileColumn, int tileRow, uint8_t r, uint8_t g, uint8_t b) {
        const int startX = tileColumn * tileSize;
        const int startY = tileRow * tileSize;
        for (int y = 0; y < tileSize; ++y) {
            for (int x = 0; x < tileSize; ++x) {
                const size_t index = (static_cast<size_t>(startY + y) * textureWidth + (startX + x)) * 4;
                pixels[index + 0] = r;
                pixels[index + 1] = g;
                pixels[index + 2] = b;
                pixels[index + 3] = 255;
            }
        }
    };

    // Tile IDs map as: 1 -> top-right, 2 -> bottom-left, 3 -> bottom-right.
    writeTile(1, 0, 220, 64, 64);
    writeTile(0, 1, 72, 186, 94);
    writeTile(1, 1, 66, 135, 245);

    return pixels;
}

std::vector<uint8_t> makeWindowskinPixels() {
    constexpr int textureWidth = 192;
    constexpr int textureHeight = 96;
    std::vector<uint8_t> pixels(static_cast<size_t>(textureWidth) * textureHeight * 4, 0);

    const auto fillRect = [&](int x0, int y0, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        for (int y = y0; y < y0 + h; ++y) {
            for (int x = x0; x < x0 + w; ++x) {
                const size_t index = (static_cast<size_t>(y) * textureWidth + x) * 4;
                pixels[index + 0] = r;
                pixels[index + 1] = g;
                pixels[index + 2] = b;
                pixels[index + 3] = a;
            }
        }
    };

    fillRect(0, 0, 96, 96, 36, 52, 96, 255);
    fillRect(96, 0, 24, 24, 214, 224, 255, 255);
    fillRect(168, 0, 24, 24, 214, 224, 255, 255);
    fillRect(96, 72, 24, 24, 214, 224, 255, 255);
    fillRect(168, 72, 24, 24, 214, 224, 255, 255);
    fillRect(192 - 32, 64, 32, 32, 120, 170, 255, 255);

    return pixels;
}

std::vector<uint8_t> makeBattleActorPixels() {
    constexpr int textureWidth = 144;
    constexpr int textureHeight = 192;
    constexpr int frameWidth = 48;
    constexpr int frameHeight = 48;
    std::vector<uint8_t> pixels(static_cast<size_t>(textureWidth) * textureHeight * 4, 0);

    const auto fillFrame =
        [&](int frameColumn, int frameRow, uint8_t r, uint8_t g, uint8_t b, uint8_t accentR, uint8_t accentG, uint8_t accentB) {
            const int startX = frameColumn * frameWidth;
            const int startY = frameRow * frameHeight;
            for (int y = 0; y < frameHeight; ++y) {
                for (int x = 0; x < frameWidth; ++x) {
                    const bool accent = x > 10 && x < 38 && y > 8 && y < 40;
                    const size_t index = (static_cast<size_t>(startY + y) * textureWidth + (startX + x)) * 4;
                    pixels[index + 0] = accent ? accentR : r;
                    pixels[index + 1] = accent ? accentG : g;
                    pixels[index + 2] = accent ? accentB : b;
                    pixels[index + 3] = 255;
                }
            }
        };

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 3; ++col) {
            const uint8_t baseR = static_cast<uint8_t>(70 + row * 30 + col * 12);
            const uint8_t baseG = static_cast<uint8_t>(90 + row * 18);
            const uint8_t baseB = static_cast<uint8_t>(140 + col * 20);
            fillFrame(col, row, baseR, baseG, baseB, 240, 228, 170);
        }
    }

    return pixels;
}

std::vector<uint8_t> makeBattleEnemyPixels() {
    constexpr int textureWidth = 128;
    constexpr int textureHeight = 128;
    std::vector<uint8_t> pixels(static_cast<size_t>(textureWidth) * textureHeight * 4, 0);

    for (int y = 0; y < textureHeight; ++y) {
        for (int x = 0; x < textureWidth; ++x) {
            const bool core = x > 20 && x < 108 && y > 20 && y < 108;
            const bool stripe = ((x / 8) + (y / 8)) % 2 == 0;
            const size_t index = (static_cast<size_t>(y) * textureWidth + x) * 4;
            pixels[index + 0] = core ? static_cast<uint8_t>(stripe ? 182 : 126) : 48;
            pixels[index + 1] = core ? static_cast<uint8_t>(stripe ? 74 : 132) : 34;
            pixels[index + 2] = core ? static_cast<uint8_t>(stripe ? 86 : 164) : 62;
            pixels[index + 3] = 255;
        }
    }

    return pixels;
}

SceneSnapshot captureBattleSceneSnapshot(bool includeEnemyAndCues) {
    auto& layer = RenderLayer::getInstance();
    layer.flush();

    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLScene(
        [&](urpg::OpenGLRenderer& renderer) {
            auto actorTexture = std::make_shared<urpg::Texture>();
            const auto actorPixels = makeBattleActorPixels();
            if (!actorTexture->loadFromMemory(actorPixels, 144, 192)) {
                throw std::runtime_error("Failed to load in-memory battle actor texture.");
            }

            auto enemyTexture = std::make_shared<urpg::Texture>();
            const auto enemyPixels = makeBattleEnemyPixels();
            if (!enemyTexture->loadFromMemory(enemyPixels, 128, 128)) {
                throw std::runtime_error("Failed to load in-memory battle enemy texture.");
            }

            urpg::scene::BattleScene battle({});
            battle.addActor("1", "Hero", 96, 28, {120.0f, 280.0f}, actorTexture);
            if (includeEnemyAndCues) {
                battle.addEnemy("1", "Slime", 44, 0, {540.0f, 170.0f}, enemyTexture);
            }

            auto& participants = const_cast<std::vector<urpg::scene::BattleParticipant>&>(battle.getParticipants());
            if (!participants.empty()) {
                auto& actor = participants.front();
                actor.hp = 68;
                actor.mp = 14;
                actor.isGuarding = includeEnemyAndCues;
                if (includeEnemyAndCues) {
                    actor.states = {5, 6};
                    actor.DamagePopupTimer = 0.75f;
                    actor.DamagePopupValue = 42.0f;
                    actor.DamagePopupColor = 0x00FF00FFu;
                } else {
                    actor.states.clear();
                    actor.DamagePopupTimer = 0.0f;
                    actor.DamagePopupValue = 0.0f;
                }
            }

            battle.onUpdate(0.0f);

            urpg::SpriteBatcher batcher;
            batcher.begin();
            battle.draw(batcher);
            batcher.end();

            renderer.renderBatches(batcher.getBatches());
        },
        800,
        600,
        &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    layer.flush();
    return *snapshot;
}

SceneSnapshot captureSpriteFrameCommandSnapshot(bool registerTextureHandle) {
    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLScene(
        [&](urpg::OpenGLRenderer& renderer) {
            auto actorTexture = std::make_shared<urpg::Texture>();
            const auto actorPixels = makeBattleActorPixels();
            if (!actorTexture->loadFromMemory(actorPixels, 144, 192)) {
                throw std::runtime_error("Failed to load in-memory sprite frame-command texture.");
            }

            if (registerTextureHandle) {
                if (!renderer.registerTextureHandle("hero_sprite", actorTexture)) {
                    throw std::runtime_error("Failed to register preloaded sprite frame-command texture handle.");
                }
            }

            urpg::SpriteCommand spriteCmd;
            spriteCmd.textureId = "hero_sprite";
            spriteCmd.srcX = 48;
            spriteCmd.srcY = 96;
            spriteCmd.width = 48;
            spriteCmd.height = 48;
            spriteCmd.x = 20.0f;
            spriteCmd.y = 18.0f;
            spriteCmd.zOrder = 2;

            renderer.processFrameCommands({urpg::toFrameRenderCommand(spriteCmd)});
        },
        96,
        96,
        &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    return *snapshot;
}

SceneSnapshot captureTileFrameCommandSnapshot(bool registerTextureHandle) {
    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLScene(
        [&](urpg::OpenGLRenderer& renderer) {
            auto tilesetTexture = std::make_shared<urpg::Texture>();
            const auto tilesetPixels = makeMapSceneTilesetPixels();
            if (!tilesetTexture->loadFromMemory(tilesetPixels, 96, 96)) {
                throw std::runtime_error("Failed to load in-memory tile frame-command texture.");
            }

            if (registerTextureHandle) {
                if (!renderer.registerTextureHandle("default_tileset", tilesetTexture)) {
                    throw std::runtime_error("Failed to register preloaded tile frame-command texture handle.");
                }
            }

            urpg::TileCommand tileCmd;
            tileCmd.tilesetId = "default_tileset";
            tileCmd.tileIndex = 3;
            tileCmd.x = 0.0f;
            tileCmd.y = 0.0f;
            tileCmd.zOrder = 1;

            renderer.processFrameCommands({urpg::toFrameRenderCommand(tileCmd)});
        },
        48,
        48,
        &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    return *snapshot;
}

SceneSnapshot captureEngineShellMapSceneSnapshot(bool startDialogue) {
    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureEngineTick(
        CaptureBackend::OpenGL,
        [&](urpg::EngineShell& shell) {
            auto tilesetTexture = std::make_shared<urpg::Texture>();
            const auto tilesetPixels = makeMapSceneTilesetPixels();
            if (!tilesetTexture->loadFromMemory(tilesetPixels, 96, 96)) {
                throw std::runtime_error("Failed to load in-memory EngineShell tileset texture.");
            }

            auto actorTexture = std::make_shared<urpg::Texture>();
            const auto actorPixels = makeBattleActorPixels();
            if (!actorTexture->loadFromMemory(actorPixels, 144, 192)) {
                throw std::runtime_error("Failed to load in-memory EngineShell actor texture.");
            }

            auto* renderer = dynamic_cast<urpg::OpenGLRenderer*>(shell.getRenderer());
            if (renderer == nullptr) {
                throw std::runtime_error("EngineShell renderer is not an OpenGLRenderer.");
            }
            if (!renderer->registerTextureHandle("default_tileset", tilesetTexture)) {
                throw std::runtime_error("Failed to register EngineShell tileset texture handle.");
            }
            if (!renderer->registerTextureHandle("hero_sprite", actorTexture)) {
                throw std::runtime_error("Failed to register EngineShell player texture handle.");
            }

            auto map = std::make_shared<urpg::scene::MapScene>("EngineShellSnapshot", 2, 2);
            map->setAssetReferences({
                {"hero_sprite", {}},
                {"default_tileset", {}},
            });
            map->setTileset(tilesetTexture);
            map->setLayerData(0, {1, 2, 3, 1});
            map->setTile(0, 0, 1, true);
            map->setTile(1, 0, 2, true);
            map->setTile(0, 1, 3, true);
            map->setTile(1, 1, 1, true);
            if (startDialogue) {
                map->startDialogue({
                    {"page_1",
                     "Shell-owned dialogue overlay",
                     urpg::message::variantFromCompatRoute("speaker", "Guide", 1),
                     true,
                     {},
                     0},
                });
            }

            urpg::scene::SceneManager::getInstance().pushScene(map);
        },
        640,
        400,
        &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    return *snapshot;
}

SceneSnapshot captureEngineShellMenuSceneSnapshot() {
    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureEngineTick(
        CaptureBackend::OpenGL,
        [](urpg::EngineShell& /*shell*/) {
            auto menu = std::make_shared<urpg::scene::MenuScene>("EngineShellMenuSnapshot");
            urpg::scene::SceneManager::getInstance().pushScene(menu);
        },
        320,
        240,
        &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    return *snapshot;
}

SceneSnapshot captureEngineShellBattleSceneSnapshot() {
    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureEngineTick(
        CaptureBackend::OpenGL,
        [](urpg::EngineShell& /*shell*/) {
            auto actorTexture = std::make_shared<urpg::Texture>();
            const auto actorPixels = makeBattleActorPixels();
            if (!actorTexture->loadFromMemory(actorPixels, 144, 192)) {
                throw std::runtime_error("Failed to load in-memory EngineShell battle actor texture.");
            }

            auto enemyTexture = std::make_shared<urpg::Texture>();
            const auto enemyPixels = makeBattleEnemyPixels();
            if (!enemyTexture->loadFromMemory(enemyPixels, 128, 128)) {
                throw std::runtime_error("Failed to load in-memory EngineShell battle enemy texture.");
            }

            auto battle = std::make_shared<urpg::scene::BattleScene>(std::vector<std::string>{});
            battle->addActor("1", "Hero", 96, 28, {120.0f, 280.0f}, actorTexture);
            battle->addEnemy("1", "Slime", 44, 0, {540.0f, 170.0f}, enemyTexture);

            auto& participants =
                const_cast<std::vector<urpg::scene::BattleParticipant>&>(battle->getParticipants());
            if (!participants.empty()) {
                auto& actor = participants.front();
                actor.hp = 68;
                actor.mp = 14;
                actor.isGuarding = true;
                actor.states = {5, 6};
                actor.DamagePopupTimer = 0.75f;
                actor.DamagePopupValue = 42.0f;
                actor.DamagePopupColor = 0x00FF00FFu;
            }

            urpg::scene::SceneManager::getInstance().pushScene(battle);
        },
        800,
        600,
        &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    return *snapshot;
}

SceneSnapshot captureTexturedMapSceneBatchSnapshot() {
    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLScene(
        [](urpg::OpenGLRenderer& renderer) {
            auto tileset = std::make_shared<urpg::Texture>();
            const auto pixels = makeMapSceneTilesetPixels();
            if (!tileset->loadFromMemory(pixels, 96, 96)) {
                throw std::runtime_error("Failed to load in-memory tileset texture.");
            }

            urpg::scene::MapScene map("TexturedWorld", 2, 2);
            map.setTileset(tileset);
            map.setLayerData(0, {1, 2, 3, 1});

            urpg::SpriteBatcher batcher;
            batcher.begin();
            map.draw(batcher);
            batcher.end();

            renderer.renderBatches(batcher.getBatches());
        },
        96,
        96,
        &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    return *snapshot;
}

SceneSnapshot captureChatWindowSnapshot() {
    auto& layer = RenderLayer::getInstance();
    layer.flush();

    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLScene(
        [&](urpg::OpenGLRenderer& renderer) {
            auto windowskin = std::make_shared<urpg::Texture>();
            const auto pixels = makeWindowskinPixels();
            if (!windowskin->loadFromMemory(pixels, 192, 96)) {
                throw std::runtime_error("Failed to load in-memory windowskin texture.");
            }

            urpg::ui::ChatWindow chatWindow;
            chatWindow.setWindowskin(windowskin);
            chatWindow.addMessage("Guide", "Welcome back.");
            chatWindow.addMessage("Player", "Show me the next route.");
            chatWindow.setInputBuffer("Open the northern gate");
            chatWindow.update(0.25f);

            urpg::SpriteBatcher batcher;
            batcher.begin();
            chatWindow.draw(batcher);
            batcher.end();

            renderer.renderBatches(batcher.getBatches());
            renderer.processFrameCommands(layer.getFrameCommands());
        },
        800,
        480,
        &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    layer.flush();
    return cropSnapshot(*snapshot, 40, 40, 720, 400);
}

SceneSnapshot captureWindowStatusGaugeCrop(bool includeTpGauge) {
    auto& layer = RenderLayer::getInstance();
    layer.flush();

    auto& data = urpg::compat::DataManager::instance();
    REQUIRE(data.loadActors());

    urpg::compat::Window_Base::CreateParams params;
    params.rect = urpg::compat::Rect{0, 0, 220, 96};
    urpg::compat::Window_Base window(params);
    window.setPadding(8);
    window.setFontFace("TestFace");
    window.setFontSize(18);

    window.drawActorName(1, 8, 4, 100);
    window.drawActorHp(1, 8, 28, 128);
    window.drawActorMp(1, 8, 52, 128);
    if (includeTpGauge) {
        window.drawActorTp(1, 8, 76, 128);
    }

    VisualRegressionHarness harness;
    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame(layer.getFrameCommands(), 220, 96, &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    const auto cropped = cropSnapshot(*snapshot, 8, 4, 136, includeTpGauge ? 84 : 60);
    layer.flush();
    return cropped;
}

std::vector<FrameRenderCommand> makeRendererBackedCommands(bool includeText) {
    std::vector<FrameRenderCommand> commands;

    RectCommand rect;
    rect.x = 10.0f;
    rect.y = 12.0f;
    rect.w = 20.0f;
    rect.h = 10.0f;
    rect.r = 1.0f;
    rect.g = 0.0f;
    rect.b = 0.0f;
    rect.a = 1.0f;
    commands.push_back(toFrameRenderCommand(rect));

    if (includeText) {
        TextCommand text;
        text.x = 4.0f;
        text.y = 4.0f;
        text.text = "HP";
        text.fontSize = 12;
        text.r = 255;
        text.g = 255;
        text.b = 255;
        text.a = 255;
        commands.push_back(toFrameRenderCommand(text));
    }

    return commands;
}

std::vector<FrameRenderCommand> makeFullFrameRectCommands() {
    std::vector<FrameRenderCommand> commands;

    RectCommand rect;
    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.w = 4.0f;
    rect.h = 4.0f;
    rect.r = 1.0f;
    rect.g = 0.0f;
    rect.b = 0.0f;
    rect.a = 1.0f;
    commands.push_back(toFrameRenderCommand(rect));

    return commands;
}

std::vector<FrameRenderCommand> makeInsetRectCommands() {
    std::vector<FrameRenderCommand> commands;

    RectCommand rect;
    rect.x = 1.0f;
    rect.y = 1.0f;
    rect.w = 5.0f;
    rect.h = 4.0f;
    rect.r = 1.0f;
    rect.g = 0.0f;
    rect.b = 0.0f;
    rect.a = 1.0f;
    commands.push_back(toFrameRenderCommand(rect));

    return commands;
}

} // namespace

TEST_CASE("Snapshot: renderer-backed visual capture matches committed full-frame rect golden",
          "[snapshot][renderer][visual_capture]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame(makeFullFrameRectCommands(), 4, 4, &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    const auto result = harness.compareAgainstGolden("RendererBackedCapture", "rect_full_frame", *snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture matches committed clear-frame golden",
          "[snapshot][renderer][visual_capture]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame({}, 4, 4, &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    const auto result = harness.compareAgainstGolden("RendererBackedCapture", "clear_frame", *snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture matches committed inset-rect golden",
          "[snapshot][renderer][visual_capture]") {
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLFrame(makeInsetRectCommands(), 8, 8, &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());

    const auto result = harness.compareAgainstGolden("RendererBackedCapture", "rect_inset_clear_border", *snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture round-trips through golden comparison",
          "[snapshot][renderer][visual_capture]") {
    VisualRegressionHarness harness;

    std::string captureError;
    const auto withText = harness.captureOpenGLFrame(makeRendererBackedCommands(true), 40, 32, &captureError);
    INFO(captureError);
    REQUIRE(withText.has_value());

    const auto withoutText = harness.captureOpenGLFrame(makeRendererBackedCommands(false), 40, 32, &captureError);
    INFO(captureError);
    REQUIRE(withoutText.has_value());

    const std::filesystem::path tempRoot =
        std::filesystem::temp_directory_path() / "urpg_renderer_backed_visual_capture";
    std::filesystem::remove_all(tempRoot);
    std::filesystem::create_directories(tempRoot);

    harness.setGoldenRoot(tempRoot.string());
    REQUIRE(harness.saveGolden("RendererBackedCapture", "with_text", *withText));

    const auto identical = harness.compareAgainstGolden("RendererBackedCapture", "with_text", *withText);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto different = harness.compareAgainstGolden("RendererBackedCapture", "with_text", *withoutText);
    REQUIRE_FALSE(different.matches);
    REQUIRE(different.errorPercentage > 0.0f);

    std::filesystem::remove_all(tempRoot);
}

TEST_CASE("Snapshot: renderer-backed visual capture covers MapScene dialogue overlay crop",
          "[snapshot][renderer][visual_capture]") {
    const auto withDialogue = captureMapSceneDialogueOverlayCrop(true);
    const auto withoutDialogue = captureMapSceneDialogueOverlayCrop(false);

    REQUIRE(withDialogue.width == 200);
    REQUIRE(withDialogue.height == 80);
    REQUIRE(withDialogue.pixels.size() == static_cast<size_t>(withDialogue.width) * withDialogue.height);

    const auto changed = SnapshotValidator::compare(withDialogue, withoutDialogue);
    REQUIRE_FALSE(changed.matches);
    REQUIRE(changed.errorPercentage > 0.0f);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("RendererBackedCapture", "mapscene_dialogue_overlay_crop", withDialogue));
    }

    const auto identical =
        harness.compareAgainstGolden("RendererBackedCapture", "mapscene_dialogue_overlay_crop", withDialogue);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto different =
        harness.compareAgainstGolden("RendererBackedCapture", "mapscene_dialogue_overlay_crop", withoutDialogue);
    REQUIRE_FALSE(different.matches);
    REQUIRE(different.errorPercentage > 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture covers Window_Base status and gauge surface",
          "[snapshot][renderer][visual_capture]") {
    const auto withTpGauge = captureWindowStatusGaugeCrop(true);
    const auto withoutTpGauge = captureWindowStatusGaugeCrop(false);

    REQUIRE(withTpGauge.width == 136);
    REQUIRE(withTpGauge.height == 84);
    REQUIRE(withTpGauge.pixels.size() == static_cast<size_t>(withTpGauge.width) * withTpGauge.height);

    const auto changed = SnapshotValidator::compare(withTpGauge, withoutTpGauge);
    REQUIRE_FALSE(changed.matches);
    REQUIRE(changed.errorPercentage > 0.0f);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("RendererBackedCapture", "window_status_gauge_crop", withTpGauge));
    }

    const auto identical =
        harness.compareAgainstGolden("RendererBackedCapture", "window_status_gauge_crop", withTpGauge);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto different =
        harness.compareAgainstGolden("RendererBackedCapture", "window_status_gauge_crop", withoutTpGauge);
    REQUIRE_FALSE(different.matches);
    REQUIRE(different.errorPercentage > 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture covers MapScene world placeholder path",
          "[snapshot][renderer][visual_capture]") {
    const auto world = captureMapSceneWorldSnapshot();
    const auto dialogueOverlay = captureMapSceneDialogueOverlayCrop(true);

    REQUIRE(world.width == 96);
    REQUIRE(world.height == 96);
    REQUIRE(world.pixels.size() == static_cast<size_t>(world.width) * world.height);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("RendererBackedCapture", "mapscene_world_placeholder", world));
    }

    const auto identical =
        harness.compareAgainstGolden("RendererBackedCapture", "mapscene_world_placeholder", world);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto changed = SnapshotValidator::compare(
        cropSnapshot(world, 0, 0, dialogueOverlay.width, dialogueOverlay.height),
        dialogueOverlay);
    REQUIRE_FALSE(changed.matches);
    REQUIRE(changed.errorPercentage > 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture covers textured MapScene batch path",
          "[snapshot][renderer][visual_capture]") {
    const auto texturedWorld = captureTexturedMapSceneBatchSnapshot();
    const auto placeholderWorld = captureMapSceneWorldSnapshot();

    REQUIRE(texturedWorld.width == 96);
    REQUIRE(texturedWorld.height == 96);
    REQUIRE(texturedWorld.pixels.size() == static_cast<size_t>(texturedWorld.width) * texturedWorld.height);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("RendererBackedCapture", "mapscene_textured_batch_path", texturedWorld));
    }

    const auto identical =
        harness.compareAgainstGolden("RendererBackedCapture", "mapscene_textured_batch_path", texturedWorld);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto changed = SnapshotValidator::compare(texturedWorld, placeholderWorld);
    REQUIRE_FALSE(changed.matches);
    REQUIRE(changed.errorPercentage > 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture covers ChatWindow mixed batch and text path",
          "[snapshot][renderer][visual_capture]") {
    const auto chatWindow = captureChatWindowSnapshot();
    const auto windowStatus = captureWindowStatusGaugeCrop(true);

    REQUIRE(chatWindow.width == 720);
    REQUIRE(chatWindow.height == 400);
    REQUIRE(chatWindow.pixels.size() == static_cast<size_t>(chatWindow.width) * chatWindow.height);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("RendererBackedCapture", "chat_window_mixed_path", chatWindow));
    }

    const auto identical =
        harness.compareAgainstGolden("RendererBackedCapture", "chat_window_mixed_path", chatWindow);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto changed = SnapshotValidator::compare(
        cropSnapshot(chatWindow, 0, 0, windowStatus.width, windowStatus.height),
        windowStatus);
    REQUIRE_FALSE(changed.matches);
    REQUIRE(changed.errorPercentage > 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture covers BattleScene textured runtime path",
          "[snapshot][renderer][visual_capture]") {
    const auto battleScene = captureBattleSceneSnapshot(true);
    const auto reducedBattleScene = captureBattleSceneSnapshot(false);

    REQUIRE(battleScene.width == 800);
    REQUIRE(battleScene.height == 600);
    REQUIRE(battleScene.pixels.size() == static_cast<size_t>(battleScene.width) * battleScene.height);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("RendererBackedCapture", "battle_scene_textured_runtime_path", battleScene));
    }

    const auto identical =
        harness.compareAgainstGolden("RendererBackedCapture", "battle_scene_textured_runtime_path", battleScene);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto changed = SnapshotValidator::compare(battleScene, reducedBattleScene);
    REQUIRE_FALSE(changed.matches);
    REQUIRE(changed.errorPercentage > 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture covers textured direct sprite frame commands",
          "[snapshot][renderer][visual_capture]") {
    const auto texturedSprite = captureSpriteFrameCommandSnapshot(true);
    const auto placeholderSprite = captureSpriteFrameCommandSnapshot(false);

    REQUIRE(texturedSprite.width == 96);
    REQUIRE(texturedSprite.height == 96);
    REQUIRE(texturedSprite.pixels.size() == static_cast<size_t>(texturedSprite.width) * texturedSprite.height);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("RendererBackedCapture", "sprite_frame_command_textured", texturedSprite));
    }

    const auto identical =
        harness.compareAgainstGolden("RendererBackedCapture", "sprite_frame_command_textured", texturedSprite);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto changed = SnapshotValidator::compare(texturedSprite, placeholderSprite);
    REQUIRE_FALSE(changed.matches);
    REQUIRE(changed.errorPercentage > 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture covers textured direct tile frame commands",
          "[snapshot][renderer][visual_capture]") {
    const auto texturedTile = captureTileFrameCommandSnapshot(true);
    const auto placeholderTile = captureTileFrameCommandSnapshot(false);

    REQUIRE(texturedTile.width == 48);
    REQUIRE(texturedTile.height == 48);
    REQUIRE(texturedTile.pixels.size() == static_cast<size_t>(texturedTile.width) * texturedTile.height);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("RendererBackedCapture", "tile_frame_command_textured", texturedTile));
    }

    const auto identical =
        harness.compareAgainstGolden("RendererBackedCapture", "tile_frame_command_textured", texturedTile);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto changed = SnapshotValidator::compare(texturedTile, placeholderTile);
    REQUIRE_FALSE(changed.matches);
    REQUIRE(changed.errorPercentage > 0.0f);
}

TEST_CASE("Snapshot: renderer-backed visual capture covers EngineShell MapScene mixed runtime path",
          "[snapshot][renderer][visual_capture]") {
    const auto shellMixed = captureEngineShellMapSceneSnapshot(true);
    const auto shellWorldOnly = captureEngineShellMapSceneSnapshot(false);

    REQUIRE(shellMixed.width == 640);
    REQUIRE(shellMixed.height == 400);
    REQUIRE(shellMixed.pixels.size() == static_cast<size_t>(shellMixed.width) * shellMixed.height);

    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("RendererBackedCapture", "engine_shell_mapscene_mixed_runtime_path", shellMixed));
    }

    const auto identical =
        harness.compareAgainstGolden("RendererBackedCapture", "engine_shell_mapscene_mixed_runtime_path", shellMixed);
    REQUIRE(identical.matches);
    REQUIRE(identical.errorPercentage == 0.0f);

    const auto changed = SnapshotValidator::compare(shellMixed, shellWorldOnly);
    REQUIRE_FALSE(changed.matches);
    REQUIRE(changed.errorPercentage > 0.0f);
}
#endif

// ─── S29: Visual Regression Golden Coverage ──────────────────────────────────

TEST_CASE("Snapshot: MenuScene golden render produces deterministic full-frame output",
          "[snapshot][regression][s29]") {
#ifdef URPG_HEADLESS
    SUCCEED("Headless build: skipping renderer-backed MenuScene golden");
#else
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    std::string errorMessage;
    const auto snapshot = harness.captureOpenGLScene(
        [](urpg::OpenGLRenderer& renderer) {
            urpg::scene::MenuScene menu("MainMenu");
            menu.onCreate();
            menu.onStart();

            urpg::SpriteBatcher batcher;
            batcher.begin();
            menu.draw(batcher);
            batcher.end();
            renderer.renderBatches(batcher.getBatches());
        },
        320,
        240,
        &errorMessage);
    INFO(errorMessage);
    REQUIRE(snapshot.has_value());
    REQUIRE(snapshot->width == 320);
    REQUIRE(snapshot->height == 240);

    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("S29SceneGoldens", "menu_scene_full_frame", *snapshot));
    }

    const auto result = harness.compareAgainstGolden("S29SceneGoldens", "menu_scene_full_frame", *snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
#endif
}

TEST_CASE("Snapshot: EngineShell MenuScene golden render produces deterministic full-frame output",
          "[snapshot][regression][s1]") {
#ifdef URPG_HEADLESS
    SUCCEED("Headless build: skipping renderer-backed EngineShell MenuScene golden");
#else
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    const auto snapshot = captureEngineShellMenuSceneSnapshot();
    REQUIRE(snapshot.width == 320);
    REQUIRE(snapshot.height == 240);

    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("S1SceneGoldens", "engine_shell_menu_scene_full_frame", snapshot));
    }

    const auto result =
        harness.compareAgainstGolden("S1SceneGoldens", "engine_shell_menu_scene_full_frame", snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
#endif
}

TEST_CASE("Snapshot: EngineShell BattleScene golden render produces deterministic actor cue crop",
          "[snapshot][regression][s1]") {
#ifdef URPG_HEADLESS
    SUCCEED("Headless build: skipping renderer-backed EngineShell BattleScene golden");
#else
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    const auto fullFrame = captureEngineShellBattleSceneSnapshot();
    REQUIRE(fullFrame.width == 800);
    REQUIRE(fullFrame.height == 600);

    const auto snapshot = cropSnapshot(fullFrame, 96, 220, 160, 140);
    REQUIRE(snapshot.width == 160);
    REQUIRE(snapshot.height == 140);

    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("S1SceneGoldens", "engine_shell_battle_scene_actor_cue_crop", snapshot));
    }

    const auto result =
        harness.compareAgainstGolden("S1SceneGoldens", "engine_shell_battle_scene_actor_cue_crop", snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
#endif
}

TEST_CASE("Snapshot: MapScene golden matches expected full-frame output (regression lane)",
          "[snapshot][regression][s29]") {
#ifdef URPG_HEADLESS
    SUCCEED("Headless build: skipping renderer-backed MapScene regression golden");
#else
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    const auto snapshot = captureMapSceneWorldSnapshot();
    REQUIRE(snapshot.width == 96);
    REQUIRE(snapshot.height == 96);

    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("S29SceneGoldens", "map_scene_full_frame", snapshot));
    }

    const auto result = harness.compareAgainstGolden("S29SceneGoldens", "map_scene_full_frame", snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
#endif
}

TEST_CASE("Snapshot: BattleScene golden matches expected full-frame output (regression lane)",
          "[snapshot][regression][s29]") {
#ifdef URPG_HEADLESS
    SUCCEED("Headless build: skipping renderer-backed BattleScene regression golden");
#else
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    const auto snapshot = captureBattleSceneSnapshot(false);
    REQUIRE(snapshot.width == 800);
    REQUIRE(snapshot.height == 600);

    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("S29SceneGoldens", "battle_scene_full_frame", snapshot));
    }

    const auto result = harness.compareAgainstGolden("S29SceneGoldens", "battle_scene_full_frame", snapshot);
    REQUIRE(result.matches);
    REQUIRE(result.errorPercentage == 0.0f);
#endif
}

TEST_CASE("Snapshot: scene transition sequence produces deterministic golden pair",
          "[snapshot][regression][s29]") {
    // This test exercises the transition boundary: before-transition and after-transition
    // frames must be deterministic and independently stable.
#ifdef URPG_HEADLESS
    SUCCEED("Headless build: skipping renderer-backed transition golden");
#else
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    // Capture "before" frame: map scene at rest
    const auto before = captureMapSceneWorldSnapshot();
    REQUIRE(before.width == 96);
    REQUIRE(before.height == 96);

    // Capture "after" frame: battle entry overlay
    const auto after = captureBattleSceneSnapshot(false);
    REQUIRE(after.width == 800);
    REQUIRE(after.height == 600);

    // The two frames must differ (valid transition)
    const auto diff = SnapshotValidator::compare(before, after);
    REQUIRE_FALSE(diff.matches);

    // Each frame must individually match its committed golden
    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("S29TransitionGoldens", "pre_battle_map", before));
        REQUIRE(harness.saveGolden("S29TransitionGoldens", "post_battle_entry", after));
    }

    const auto preResult  = harness.compareAgainstGolden("S29TransitionGoldens", "pre_battle_map", before);
    const auto postResult = harness.compareAgainstGolden("S29TransitionGoldens", "post_battle_entry", after);
    REQUIRE(preResult.matches);
    REQUIRE(postResult.matches);
#endif
}

TEST_CASE("Snapshot: diff heatmap golden is actionable and stable",
          "[snapshot][regression][s29]") {
    // Ensures the heatmap diff artifact produced for CI reviewers is deterministic.
#ifdef URPG_HEADLESS
    SUCCEED("Headless build: skipping renderer-backed diff heatmap golden");
#else
    VisualRegressionHarness harness;
    harness.setGoldenRoot(getGoldenRoot().string());

    // Introduce a known synthetic difference to validate the heatmap pipeline.
    std::string errorMessage;
    const auto baseSnapshot = harness.captureOpenGLFrame(makeFullFrameRectCommands(), 4, 4, &errorMessage);
    INFO(errorMessage);
    REQUIRE(baseSnapshot.has_value());

    SceneSnapshot modified = *baseSnapshot;
    if (!modified.pixels.empty()) {
        modified.pixels[0].r = static_cast<uint8_t>((modified.pixels[0].r + 128) % 256);
    }

    const auto heatmap = VisualRegressionHarness::generateDiffHeatmap(*baseSnapshot, modified);
    REQUIRE(heatmap.width == baseSnapshot->width);
    REQUIRE(heatmap.height == baseSnapshot->height);
    REQUIRE(heatmap.pixels.size() == baseSnapshot->pixels.size());

    // The heatmap pixel at the modified location must be non-zero
    if (!heatmap.pixels.empty()) {
        const auto& p = heatmap.pixels[0];
        const bool hasDiffSignal = (p.r > 0 || p.g > 0 || p.b > 0);
        REQUIRE(hasDiffSignal);
    }

    if (shouldRegenerateRendererBackedGoldens()) {
        REQUIRE(harness.saveGolden("S29DiffGoldens", "heatmap_single_pixel_diff", heatmap));
    }

    const auto result = harness.compareAgainstGolden("S29DiffGoldens", "heatmap_single_pixel_diff", heatmap);
    REQUIRE(result.matches);
#endif
}
