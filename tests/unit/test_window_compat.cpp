// Unit tests for WindowCompat Core Surface
// Phase 2 - Compat Layer
//
// These tests verify the MZ Window API compatibility surface behavior.

#include "runtimes/compat_js/window_compat.h"
#include "runtimes/compat_js/input_manager.h"
#include "runtimes/compat_js/quickjs_runtime.h"
#include "runtimes/compat_js/data_manager.h"
#include "engine/core/render/render_layer.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

using namespace urpg::compat;

// ============================================================================
// Window_Base Tests
// ============================================================================

TEST_CASE("Window_Base creates with rect", "[compat][window]") {
    Window_Base::CreateParams params;
    params.rect = Rect{10, 20, 200, 100};
    
    Window_Base window(params);
    
    REQUIRE(window.getRect().x == 10);
    REQUIRE(window.getRect().y == 20);
    REQUIRE(window.getRect().width == 200);
    REQUIRE(window.getRect().height == 100);
}

TEST_CASE("Window_Base open/close state", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    REQUIRE_FALSE(window.isOpen());
    
    window.open();
    REQUIRE(window.isOpen());
    REQUIRE(window.isVisible());
    
    window.close();
    REQUIRE_FALSE(window.isOpen());
}

TEST_CASE("Window_Base show/hide", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    window.open();
    
    REQUIRE(window.isVisible());
    
    window.hide();
    REQUIRE_FALSE(window.isVisible());
    
    window.show();
    REQUIRE(window.isVisible());
}

TEST_CASE("Window_Base active state", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    REQUIRE(window.isActive());
    
    window.setActive(false);
    REQUIRE_FALSE(window.isActive());
    
    window.setActive(true);
    REQUIRE(window.isActive());
}

TEST_CASE("Window_Base opacity", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    REQUIRE(window.getOpacity() == 255);
    
    window.setOpacity(128);
    REQUIRE(window.getOpacity() == 128);
}

TEST_CASE("Window_Base background type", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    REQUIRE(window.getBackground() == 0);  // Default: window
    
    window.setBackground(1);
    REQUIRE(window.getBackground() == 1);  // Dim
    
    window.setBackground(2);
    REQUIRE(window.getBackground() == 2);  // Transparent
}

TEST_CASE("Window_Base padding", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    REQUIRE(window.getPadding() == 12);  // MZ default
    
    window.setPadding(16);
    REQUIRE(window.getPadding() == 16);
}

TEST_CASE("Window_Base transparent flag in constructor", "[compat][window]") {
    Window_Base::CreateParams params;
    params.transparent = true;
    
    Window_Base window(params);
    
    REQUIRE(window.getBackground() == 2);  // Transparent
}

TEST_CASE("Window_Base content rect calculation", "[compat][window]") {
    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    // Default padding is 12
    
    Window_Base window(params);
    
    auto content = window.getContentRect();
    REQUIRE(content.x == 12);
    REQUIRE(content.y == 12);
    REQUIRE(content.width == 176);  // 200 - 12*2
    REQUIRE(content.height == 76);  // 100 - 12*2
}

TEST_CASE("Window_Base getMethodStatus returns correct values", "[compat][window]") {
    REQUIRE(Window_Base::getMethodStatus("drawText") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawIcon") == CompatStatus::PARTIAL);
    REQUIRE(Window_Base::getMethodStatus("drawActorFace") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawItemName") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("unknownMethod") == CompatStatus::UNSUPPORTED);
}

TEST_CASE("Window_Base getMethodDeviation returns notes", "[compat][window]") {
    REQUIRE(Window_Base::getMethodDeviation("drawActorFace") == "");
    REQUIRE(Window_Base::getMethodDeviation("drawActorHp") == "");
    REQUIRE(Window_Base::getMethodDeviation("drawText") == "");  // FULL, no deviation
    REQUIRE(Window_Base::getMethodDeviation("drawIcon").find("SpriteCommand") != std::string::npos);
}

TEST_CASE("Window_Base drawActorFace records canonical source and destination rects", "[compat][window]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Base window(Window_Base::CreateParams{});

    window.drawActorFace(3, 10, 20, 200, 180);

    const auto drawInfo = window.getLastFaceDraw();
    REQUIRE(drawInfo.has_value());
    REQUIRE(drawInfo->actorId == 3);
    REQUIRE(drawInfo->faceIndex == 2);  // fallback mapping: actorId-1
    REQUIRE(drawInfo->faceName == "ActorFace_3");
    REQUIRE(drawInfo->sourceRect.x == 288);
    REQUIRE(drawInfo->sourceRect.y == 0);
    REQUIRE(drawInfo->sourceRect.width == 144);
    REQUIRE(drawInfo->sourceRect.height == 144);
    REQUIRE(drawInfo->destRect.x == 38);
    REQUIRE(drawInfo->destRect.y == 38);
    REQUIRE(drawInfo->destRect.width == 144);
    REQUIRE(drawInfo->destRect.height == 144);

    const auto& commands = layer.getCommands();
    REQUIRE_FALSE(commands.empty());
    REQUIRE(commands.back()->type == urpg::RenderCmdType::Sprite);

    auto spriteCmd = std::dynamic_pointer_cast<urpg::SpriteCommand>(commands.back());
    REQUIRE(spriteCmd != nullptr);
    REQUIRE(spriteCmd->textureId == "ActorFace_3");
    REQUIRE(spriteCmd->srcX == 288);
    REQUIRE(spriteCmd->srcY == 0);
    REQUIRE(spriteCmd->width == 144);
    REQUIRE(spriteCmd->height == 144);
    REQUIRE(spriteCmd->x == 50.0f);
    REQUIRE(spriteCmd->y == 50.0f);

    window.drawActorFace(3, 4, 8, 96, 120);
    const auto clippedInfo = window.getLastFaceDraw();
    REQUIRE(clippedInfo.has_value());
    REQUIRE(clippedInfo->sourceRect.x == 312); // col=2 base 288 + (144-96)/2
    REQUIRE(clippedInfo->sourceRect.y == 12);  // row=0 + (144-120)/2
    REQUIRE(clippedInfo->sourceRect.width == 96);
    REQUIRE(clippedInfo->sourceRect.height == 120);
    REQUIRE(clippedInfo->destRect.x == 4);
    REQUIRE(clippedInfo->destRect.y == 8);
    REQUIRE(clippedInfo->destRect.width == 96);
    REQUIRE(clippedInfo->destRect.height == 120);
}

TEST_CASE("Window_Base drawActorFace rejects invalid actor or size", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    window.drawActorFace(2, 0, 0, 144, 144);
    REQUIRE(window.getLastFaceDraw().has_value());

    window.drawActorFace(0, 0, 0, 144, 144);
    REQUIRE_FALSE(window.getLastFaceDraw().has_value());

    window.drawActorFace(1, 0, 0, 0, 144);
    REQUIRE_FALSE(window.getLastFaceDraw().has_value());
}

TEST_CASE("Window_Base setRect", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    window.setRect(Rect{50, 60, 300, 200});
    
    REQUIRE(window.getRect().x == 50);
    REQUIRE(window.getRect().y == 60);
    REQUIRE(window.getRect().width == 300);
    REQUIRE(window.getRect().height == 200);
}

// ============================================================================
// Window_Base Extended Methods Tests
// ============================================================================

TEST_CASE("Window_Base lineHeight returns default", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    REQUIRE(window.lineHeight() == 36);  // MZ default
}

TEST_CASE("Window_Base textWidth uses compat renderer metrics", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    REQUIRE(window.textWidth("Hello") > 0);
    REQUIRE(window.textWidth("HELLO") > window.textWidth("...."));
    REQUIRE(window.textWidth("") == 0);

    window.setFontSize(44);
    REQUIRE(window.textWidth("Hello") > 70);
}

TEST_CASE("Window_Base textSize uses compat layout metrics", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    Rect size = window.textSize("Test");
    REQUIRE(size.width > 0);
    REQUIRE(size.height == window.lineHeight());

    Rect multi = window.textSize("Line1\nLine2");
    REQUIRE(multi.height == window.lineHeight() * 2);

    Rect withFontEscape = window.textSize("\\{BIG\\}line");
    REQUIRE(withFontEscape.height >= window.lineHeight());
}

TEST_CASE("Window_Base drawText records centered and right-aligned pixel offsets", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});

    const std::string text = "Align";
    const int32_t maxWidth = 240;
    const int32_t measured = window.textWidth(text);

    window.drawText(text, 32, 18, maxWidth, "center");
    const auto centered = window.getLastTextDraw();
    REQUIRE(centered.has_value());
    REQUIRE(centered->measuredWidth == measured);
    REQUIRE(centered->resolvedX == 32 + ((maxWidth - measured) / 2));
    REQUIRE(centered->resolvedY == 18);

    window.drawText(text, 32, 18, maxWidth, "right");
    const auto right = window.getLastTextDraw();
    REQUIRE(right.has_value());
    REQUIRE(right->measuredWidth == measured);
    REQUIRE(right->resolvedX == 32 + (maxWidth - measured));
    REQUIRE(right->resolvedY == 18);
}

TEST_CASE("Window_Base drawText submits renderer TextCommand", "[compat][window]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Base::CreateParams params;
    params.rect = Rect{20, 30, 320, 180};
    Window_Base window(params);
    window.setPadding(12);
    window.setFontFace("TestFace");
    window.setFontSize(20);
    window.changeTextColor(Color{8, 16, 24, 255});

    window.drawText("Renderer call", 10, 18, 180, "center");
    const auto info = window.getLastTextDraw();
    REQUIRE(info.has_value());

    const auto& commands = layer.getCommands();
    REQUIRE_FALSE(commands.empty());
    REQUIRE(commands.back()->type == urpg::RenderCmdType::Text);
    auto textCmd = std::dynamic_pointer_cast<urpg::TextCommand>(commands.back());
    REQUIRE(textCmd != nullptr);
    REQUIRE(textCmd->text == "Renderer call");
    REQUIRE(textCmd->fontFace == "TestFace");
    REQUIRE(textCmd->fontSize == 20);
    REQUIRE(textCmd->maxWidth == 180);
    REQUIRE(textCmd->r == 8);
    REQUIRE(textCmd->g == 16);
    REQUIRE(textCmd->b == 24);
    REQUIRE(textCmd->a == 255);
    REQUIRE(textCmd->x == static_cast<float>(20 + 12 + info->resolvedX));
    REQUIRE(textCmd->y == static_cast<float>(30 + 12 + info->resolvedY));
}

TEST_CASE("Window_Base drawItemName emits icon + label draw calls", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});

    const uint32_t drawIconBefore = Window_Base::getMethodCallCount("drawIcon");
    const uint32_t drawTextBefore = Window_Base::getMethodCallCount("drawText");
    const uint32_t drawItemBefore = Window_Base::getMethodCallCount("drawItemName");

    window.drawItemName(7, 0, 0, 180);
    REQUIRE(Window_Base::getMethodCallCount("drawItemName") == drawItemBefore + 1);
    REQUIRE(Window_Base::getMethodCallCount("drawIcon") == drawIconBefore + 1);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == drawTextBefore + 1);

    window.drawItemName(0, 0, 0, 180);
    REQUIRE(Window_Base::getMethodCallCount("drawItemName") == drawItemBefore + 2);
    REQUIRE(Window_Base::getMethodCallCount("drawIcon") == drawIconBefore + 1);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == drawTextBefore + 1);
}

TEST_CASE("Window_Base drawActor gauges call drawGauge and drawText", "[compat][window]") {
    DataManager::instance().loadActors();
    Window_Base window(Window_Base::CreateParams{});
    window.clearTextDrawHistory();

    const uint32_t gaugeBefore = Window_Base::getMethodCallCount("drawGauge");
    const uint32_t textBefore = Window_Base::getMethodCallCount("drawText");

    window.drawActorHp(1, 0, 0, 128);
    window.drawActorMp(1, 0, 0, 128);
    window.drawActorTp(1, 0, 0, 128);

    REQUIRE(Window_Base::getMethodCallCount("drawGauge") == gaugeBefore + 3);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == textBefore + 6);

    const auto& history = window.getTextDrawHistory();
    REQUIRE(history.size() >= 6);
    REQUIRE(history[0].text == "HP");
    REQUIRE(history[1].text == "100");
    REQUIRE(history[2].text == "MP");
    REQUIRE(history[3].text == "30");
    REQUIRE(history[4].text == "TP");
    REQUIRE(history[5].text == "0");
}

TEST_CASE("Window_Base drawActorName emits actor name via drawText", "[compat][window]") {
    DataManager::instance().loadActors();
    Window_Base window(Window_Base::CreateParams{});
    window.clearTextDrawHistory();

    window.drawActorName(1, 10, 20, 150);

    const auto& history = window.getTextDrawHistory();
    REQUIRE_FALSE(history.empty());
    REQUIRE(history.back().text == "Hero");
}

TEST_CASE("Window_Base drawActorLevel emits level text via drawText", "[compat][window]") {
    DataManager::instance().loadActors();
    Window_Base window(Window_Base::CreateParams{});
    window.clearTextDrawHistory();

    window.drawActorLevel(1, 10, 20);

    const auto& history = window.getTextDrawHistory();
    REQUIRE_FALSE(history.empty());
    REQUIRE(history.back().text == "Lv 1");
}

TEST_CASE("Window_Base drawGauge submits RectCommands", "[compat][window]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    Window_Base window(params);

    window.drawGauge(10, 20, 100, 0.75, Color{255, 0, 0, 255}, Color{0, 255, 0, 255});

    const auto& commands = layer.getCommands();
    REQUIRE(commands.size() >= 2);
    REQUIRE(commands[commands.size() - 2]->type == urpg::RenderCmdType::Rect);
    REQUIRE(commands[commands.size() - 1]->type == urpg::RenderCmdType::Rect);

    auto fillCmd = std::dynamic_pointer_cast<urpg::RectCommand>(commands.back());
    REQUIRE(fillCmd != nullptr);
    REQUIRE(fillCmd->w == 75.0f);
}

TEST_CASE("Window_Base drawCharacter submits SpriteCommand", "[compat][window]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    Window_Base window(params);

    window.drawCharacter("Actor1", 2, 30, 40);

    const auto& commands = layer.getCommands();
    REQUIRE_FALSE(commands.empty());
    REQUIRE(commands.back()->type == urpg::RenderCmdType::Sprite);

    auto spriteCmd = std::dynamic_pointer_cast<urpg::SpriteCommand>(commands.back());
    REQUIRE(spriteCmd != nullptr);
    REQUIRE(spriteCmd->textureId == "Actor1");
    // MZ standard sheet: 4 cols × 2 rows of characters, 48×48 cells.
    // Character 2 standing frame: col=2, row=0, offset (+1, +0) within 3×4 block.
    REQUIRE(spriteCmd->srcX == ((2 % 4) * 3 + 1) * 48); // 336
    REQUIRE(spriteCmd->srcY == (2 / 4) * 4 * 48);       // 0
    REQUIRE(spriteCmd->width == 48);
    REQUIRE(spriteCmd->height == 48);
}

TEST_CASE("Window_Base drawIcon emits SpriteCommand with correct source rect", "[compat][window]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    Window_Base window(params);

    window.drawIcon(5, 10, 20);

    const auto& commands = layer.getCommands();
    REQUIRE_FALSE(commands.empty());
    REQUIRE(commands.back()->type == urpg::RenderCmdType::Sprite);

    auto spriteCmd = std::dynamic_pointer_cast<urpg::SpriteCommand>(commands.back());
    REQUIRE(spriteCmd != nullptr);
    REQUIRE(spriteCmd->textureId == "IconSet");
    REQUIRE(spriteCmd->srcX == (5 % 16) * 32);
    REQUIRE(spriteCmd->srcY == (5 / 16) * 32);
    REQUIRE(spriteCmd->width == 32);
    REQUIRE(spriteCmd->height == 32);
}

TEST_CASE("Window_Base registerAPI bindings route through default instance", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    Window_Base::setDefaultInstance(&window);

    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    Window_Base::registerAPI(ctx);

    urpg::Value textArg;
    textArg.v = std::string("Hello");
    auto result = ctx.callMethod("Window_Base", "drawText",
        std::vector<urpg::Value>{textArg, urpg::Value::Int(0), urpg::Value::Int(0)});

    REQUIRE(result.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.value.v));

    Window_Base::setDefaultInstance(nullptr);
}

TEST_CASE("Window_Base textColor change and reset", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    Color initial = window.textColor();
    REQUIRE(initial.r == 255);
    REQUIRE(initial.g == 255);
    REQUIRE(initial.b == 255);
    
    window.changeTextColor(Color{255, 0, 0, 255});
    Color red = window.textColor();
    REQUIRE(red.r == 255);
    REQUIRE(red.g == 0);
    REQUIRE(red.b == 0);
    
    window.resetTextColor();
    Color reset = window.textColor();
    REQUIRE(reset.r == 255);
    REQUIRE(reset.g == 255);
}

TEST_CASE("Window_Base systemColor returns valid colors", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    Color white = window.systemColor(1);
    REQUIRE(white.r == 255);
    REQUIRE(white.g == 255);
    REQUIRE(white.b == 255);
    
    Color black = window.systemColor(0);
    REQUIRE(black.r == 0);
    REQUIRE(black.g == 0);
    REQUIRE(black.b == 0);
    
    Color hp = window.systemColor(16);
    REQUIRE(hp.r == 255);
    REQUIRE(hp.g == 96);
}

TEST_CASE("Window_Base font settings", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    REQUIRE(window.fontFace() == "Microsoft YaHei");
    REQUIRE(window.fontSize() == 22);
    
    window.setFontFace("Arial");
    REQUIRE(window.fontFace() == "Arial");
    
    window.setFontSize(18);
    REQUIRE(window.fontSize() == 18);

    window.setFontSize(0);
    REQUIRE(window.fontSize() == 1);
    
    window.resetFontSettings();
    REQUIRE(window.fontFace() == "Microsoft YaHei");
    REQUIRE(window.fontSize() == 22);
}

TEST_CASE("Window_Base contents management", "[compat][window]") {
    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    Window_Base window(params);
    
    window.createContents();
    REQUIRE(window.contents() != INVALID_BITMAP);
    REQUIRE(window.getContentsBitmapInfo().has_value());
    REQUIRE(window.getContentsBitmapInfo()->handle == window.contents());
    REQUIRE(window.getContentsBitmapInfo()->width == 176);
    REQUIRE(window.getContentsBitmapInfo()->height == 76);
    
    window.destroyContents();
    REQUIRE(window.contents() == INVALID_BITMAP);
    REQUIRE_FALSE(window.getContentsBitmapInfo().has_value());
}

TEST_CASE("Window_Base contents lifecycle allocates and rotates deterministic handles", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});

    const auto before = window.contents();
    window.createContents();
    const auto firstCreated = window.contents();
    REQUIRE(firstCreated != 0);
    REQUIRE(firstCreated != before);

    window.destroyContents();
    REQUIRE(window.contents() == 0);

    window.createContents();
    const auto secondCreated = window.contents();
    REQUIRE(secondCreated != 0);
    REQUIRE(secondCreated != firstCreated);
}

TEST_CASE("Window_Base contents handles are unique per allocation", "[compat][window]") {
    Window_Base::CreateParams firstParams;
    firstParams.rect = Rect{0, 0, 200, 100};
    Window_Base::CreateParams secondParams;
    secondParams.rect = Rect{0, 0, 160, 90};
    Window_Base first(firstParams);
    Window_Base second(secondParams);

    first.createContents();
    second.createContents();

    REQUIRE(first.contents() != INVALID_BITMAP);
    REQUIRE(second.contents() != INVALID_BITMAP);
    REQUIRE(first.contents() != second.contents());
    REQUIRE(first.getContentsBitmapInfo().has_value());
    REQUIRE(second.getContentsBitmapInfo().has_value());
    REQUIRE(first.getContentsBitmapInfo()->width == 176);
    REQUIRE(first.getContentsBitmapInfo()->height == 76);
    REQUIRE(second.getContentsBitmapInfo()->width == 136);
    REQUIRE(second.getContentsBitmapInfo()->height == 66);
}

TEST_CASE("Window_Base contents bitmap dimensions stay in sync with rect and padding", "[compat][window]") {
    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    Window_Base window(params);

    window.createContents();
    REQUIRE(window.getContentsBitmapInfo().has_value());
    REQUIRE(window.getContentsBitmapInfo()->width == 176);
    REQUIRE(window.getContentsBitmapInfo()->height == 76);

    window.setRect(Rect{0, 0, 180, 140});
    REQUIRE(window.getContentsBitmapInfo().has_value());
    REQUIRE(window.getContentsBitmapInfo()->width == 156);
    REQUIRE(window.getContentsBitmapInfo()->height == 116);

    window.setPadding(20);
    REQUIRE(window.getContentsBitmapInfo().has_value());
    REQUIRE(window.getContentsBitmapInfo()->width == 140);
    REQUIRE(window.getContentsBitmapInfo()->height == 100);
}

TEST_CASE("Window_Base drawTextEx processes escape codes", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    DataManager::instance().setupNewGame();
    DataManager::instance().setVariable(2, 777);

    const uint32_t drawTextBefore = Window_Base::getMethodCallCount("drawText");
    const uint32_t drawIconBefore = Window_Base::getMethodCallCount("drawIcon");
    const uint32_t colorBefore = Window_Base::getMethodCallCount("changeTextColor");

    window.drawTextEx("\\C[2]HP\\I[5]\\V[2]\\G", 0, 0);

    REQUIRE(Window_Base::getMethodCallCount("drawText") >= drawTextBefore + 2);
    REQUIRE(Window_Base::getMethodCallCount("drawIcon") == drawIconBefore + 1);
    REQUIRE(Window_Base::getMethodCallCount("changeTextColor") == colorBefore + 1);

    REQUIRE(window.textWidth("A\\I[5]B") > window.textWidth("AB"));
    REQUIRE(window.textWidth("\\V[2]") > window.textWidth("0"));
}

TEST_CASE("Window_Message dialogue body supports centered and right alignment", "[compat][window][message]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Message::CreateParams params;
    params.rect = Rect{0, 0, 400, 220};
    params.messageX = 0;
    params.messageY = 0;
    params.messageWidth = 220;
    Window_Message messageWindow(params);
    messageWindow.setMessageText("Dialogue body");

    messageWindow.setMessageAlignment("center");
    messageWindow.drawMessageBody();
    const auto centerHistory = messageWindow.getTextDrawHistory();
    REQUIRE_FALSE(centerHistory.empty());
    const int32_t centerX = centerHistory.front().resolvedX;

    messageWindow.setMessageAlignment("right");
    messageWindow.drawMessageBody();
    const auto rightHistory = messageWindow.getTextDrawHistory();
    REQUIRE_FALSE(rightHistory.empty());
    const int32_t rightX = rightHistory.front().resolvedX;

    REQUIRE(centerX > 0);
    REQUIRE(rightX > centerX);
}

TEST_CASE("Window_Message drawMessageBody emits RenderLayer text commands", "[compat][window][message]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Message::CreateParams params;
    params.rect = Rect{0, 0, 400, 220};
    params.messageX = 10;
    params.messageY = 10;
    params.messageWidth = 380;
    Window_Message messageWindow(params);
    messageWindow.setMessageText("RenderLayer test body");

    messageWindow.drawMessageBody();

    const auto& commands = layer.getCommands();
    std::string accumulated;
    for (const auto& cmd : commands) {
        if (cmd->type == urpg::RenderCmdType::Text) {
            auto textCmd = std::dynamic_pointer_cast<urpg::TextCommand>(cmd);
            if (textCmd) {
                accumulated += textCmd->text;
            }
        }
    }
    REQUIRE(accumulated == "RenderLayer test body");
}

TEST_CASE("Snapshot: drawTextEx wrapped centered and right alignment remains stable", "[compat][window][snapshot]") {
    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 420, 240};
    Window_Base window(params);

    const std::string wrappedText = "Alpha beta gamma delta epsilon zeta eta theta";
    const int32_t x = 24;
    const int32_t y = 30;
    const int32_t width = 170;

    window.clearTextDrawHistory();
    window.setTextAlignment("center");
    window.drawTextEx(wrappedText, x, y, width);
    const auto centered = window.getTextDrawHistory();
    std::string centeredSnapshot;
    for (const auto& entry : centered) {
        centeredSnapshot += entry.text + "@" + std::to_string(entry.resolvedX) + "," + std::to_string(entry.resolvedY) + "|";
    }
    REQUIRE(centeredSnapshot == "Alpha @46,30|beta @116,30|gamma @41,66|delta @109,66|epsilon @35,102|zeta @127,102|eta @57,138|theta@101,138|");

    window.clearTextDrawHistory();
    window.setTextAlignment("right");
    window.drawTextEx(wrappedText, x, y, width);
    const auto right = window.getTextDrawHistory();
    std::string rightSnapshot;
    for (const auto& entry : right) {
        rightSnapshot += entry.text + "@" + std::to_string(entry.resolvedX) + "," + std::to_string(entry.resolvedY) + "|";
    }
    REQUIRE(rightSnapshot == "Alpha @68,30|beta @138,30|gamma @58,66|delta @126,66|epsilon @46,102|zeta @138,102|eta @90,138|theta@134,138|");
}

TEST_CASE("Window_Base getMethodStatus for extended methods", "[compat][window]") {
    REQUIRE(Window_Base::getMethodStatus("lineHeight") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawTextEx") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorHp") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorMp") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorTp") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("textWidth") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("textSize") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("contents") == CompatStatus::PARTIAL);
    REQUIRE(Window_Base::getMethodStatus("createContents") == CompatStatus::PARTIAL);
    REQUIRE(Window_Base::getMethodStatus("destroyContents") == CompatStatus::PARTIAL);
}

// ============================================================================
// Window_Selectable Tests
// ============================================================================

TEST_CASE("Window_Selectable creates with columns", "[compat][window]") {
    Window_Selectable::CreateParams params;
    params.maxCols = 2;
    params.numVisibleRows = 5;
    params.itemHeight = 48;
    
    Window_Selectable window(params);
    
    REQUIRE(window.getMaxCols() == 2);
    REQUIRE(window.getItemHeight() == 48);
}

TEST_CASE("Window_Selectable index management", "[compat][window]") {
    Window_Selectable window(Window_Selectable::CreateParams{});
    window.setMaxItems(10);
    
    REQUIRE(window.getIndex() == 0);
    
    window.setIndex(5);
    REQUIRE(window.getIndex() == 5);
    
    window.select(8);
    REQUIRE(window.getIndex() == 8);
    
    window.deselect();
    REQUIRE(window.getIndex() == -1);
}

TEST_CASE("Window_Selectable setIndex clamps to valid range", "[compat][window]") {
    Window_Selectable window(Window_Selectable::CreateParams{});
    window.setMaxItems(10);
    
    window.setIndex(100);
    REQUIRE(window.getIndex() == 9);  // Clamped to maxItems - 1
    
    window.setIndex(-10);
    REQUIRE(window.getIndex() == -1);  // Deselected
}

TEST_CASE("Window_Selectable max cols and max items are clamped", "[compat][window]") {
    Window_Selectable window(Window_Selectable::CreateParams{});

    window.setMaxCols(0);
    REQUIRE(window.getMaxCols() == 1);

    window.setMaxItems(-10);
    REQUIRE(window.getMaxItems() == 0);
    REQUIRE(window.getIndex() == -1);
}

TEST_CASE("Window_Selectable onSelect callback fires for index changes", "[compat][window]") {
    Window_Selectable window(Window_Selectable::CreateParams{});
    window.setMaxItems(5);

    int32_t callbackCount = 0;
    int32_t lastIndex = -999;
    window.setOnSelect([&](int32_t index) {
        ++callbackCount;
        lastIndex = index;
    });

    window.setIndex(3);
    REQUIRE(callbackCount == 1);
    REQUIRE(lastIndex == 3);

    // Same index should not trigger callback.
    window.setIndex(3);
    REQUIRE(callbackCount == 1);

    window.setIndex(-1);
    REQUIRE(callbackCount == 2);
    REQUIRE(lastIndex == -1);
}

TEST_CASE("Window_Selectable row and column calculation", "[compat][window]") {
    Window_Selectable::CreateParams params;
    params.maxCols = 3;
    Window_Selectable window(params);
    window.setMaxItems(20);
    
    // Index 7 in 3-column layout: row 2, col 1
    window.setIndex(7);
    REQUIRE(window.getRow() == 2);
    REQUIRE(window.getCol() == 1);
    
    // Index 0: row 0, col 0
    window.setIndex(0);
    REQUIRE(window.getRow() == 0);
    REQUIRE(window.getCol() == 0);
    
    // Index 5: row 1, col 2
    window.setIndex(5);
    REQUIRE(window.getRow() == 1);
    REQUIRE(window.getCol() == 2);
}

TEST_CASE("Window_Selectable cursor movement", "[compat][window]") {
    Window_Selectable::CreateParams params;
    params.maxCols = 2;
    Window_Selectable window(params);
    window.setMaxItems(10);
    
    window.setIndex(0);
    
    // Move down
    window.cursorDown(false);
    REQUIRE(window.getIndex() == 2);  // +maxCols
    
    // Move right
    window.cursorRight(false);
    REQUIRE(window.getIndex() == 3);
    
    // Move up
    window.cursorUp(false);
    REQUIRE(window.getIndex() == 1);
    
    // Move left
    window.cursorLeft(false);
    REQUIRE(window.getIndex() == 0);
}

TEST_CASE("Window_Selectable cursor wrapping", "[compat][window]") {
    Window_Selectable::CreateParams params;
    params.maxCols = 1;
    Window_Selectable window(params);
    window.setMaxItems(5);
    
    // At top, wrap up goes to bottom
    window.setIndex(0);
    window.cursorUp(true);
    REQUIRE(window.getIndex() == 4);
    
    // At bottom, wrap down goes to top
    window.cursorDown(true);
    REQUIRE(window.getIndex() == 0);
}

TEST_CASE("Window_Selectable paging", "[compat][window]") {
    Window_Selectable::CreateParams params;
    params.maxCols = 1;
    params.numVisibleRows = 3;
    Window_Selectable window(params);
    window.setMaxItems(20);
    
    window.setIndex(0);
    
    window.cursorPagedown();
    REQUIRE(window.getIndex() == 3);  // +numVisibleRows
    
    window.cursorPageup();
    REQUIRE(window.getIndex() == 0);
}

TEST_CASE("Window_Selectable top row management", "[compat][window]") {
    Window_Selectable::CreateParams params;
    params.maxCols = 1;
    params.numVisibleRows = 4;
    Window_Selectable window(params);
    window.setMaxItems(20);
    
    REQUIRE(window.getTopRow() == 0);
    
    window.setTopRow(5);
    REQUIRE(window.getTopRow() == 5);
    
    // setTopRow clamps to max
    window.setTopRow(100);
    REQUIRE(window.getTopRow() == window.getMaxTopRow());
}

TEST_CASE("Window_Selectable setIndex scrolls to visible", "[compat][window]") {
    Window_Selectable::CreateParams params;
    params.maxCols = 1;
    params.numVisibleRows = 4;
    Window_Selectable window(params);
    window.setMaxItems(20);
    
    window.setTopRow(0);
    window.setIndex(10);  // Beyond visible rows
    
    // Top row should adjust to make index visible
    REQUIRE(window.getTopRow() <= 10);
    REQUIRE(window.getTopRow() + 4 > 10);
}

TEST_CASE("Window_Selectable item width calculation", "[compat][window]") {
    Window_Selectable::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    params.maxCols = 2;
    params.numVisibleRows = 4;
    params.itemHeight = 24;
    Window_Selectable window(params);
    
    // Item width = (contentWidth - spacing) / cols
    // contentWidth = 200 - 12*2 = 176
    // spacing = 8
    // itemWidth = (176 - 8) / 2 = 84
    int32_t itemWidth = window.getItemWidth();
    REQUIRE(itemWidth > 0);
    REQUIRE(itemWidth < 100);
}

TEST_CASE("Window_Selectable update consumes InputManager cursor movement", "[compat][window]") {
    InputManager& input = InputManager::instance();
    input.initialize();
    input.clear();

    Window_Selectable::CreateParams params;
    params.maxCols = 1;
    params.numVisibleRows = 4;
    Window_Selectable window(params);
    window.setMaxItems(6);
    window.setIndex(0);
    window.setActive(true);
    window.open();

    input.setKeyPressed(InputKey::DOWN, true);
    window.update();
    REQUIRE(window.getIndex() == 1);

    input.update();
    window.update();
    REQUIRE(window.getIndex() == 2);

    input.setKeyPressed(InputKey::DOWN, false);
    input.update();
    input.setKeyPressed(InputKey::UP, true);
    window.update();
    REQUIRE(window.getIndex() == 1);

    input.shutdown();
}

TEST_CASE("Window_Command update dispatches ok and cancel input", "[compat][window]") {
    InputManager& input = InputManager::instance();
    input.initialize();
    input.clear();

    Window_Command::CreateParams params;
    params.commands = {
        {"Item", "item", true, 0},
        {"Skill", "skill", true, 1}
    };
    Window_Command window(params);
    window.setActive(true);
    window.open();
    window.setIndex(1);

    int32_t okCount = 0;
    std::string lastSymbol;
    window.setOnCommand([&](const std::string& symbol) {
        ++okCount;
        lastSymbol = symbol;
    });

    input.setKeyPressed(InputKey::DECISION, true);
    window.update();
    REQUIRE(okCount == 1);
    REQUIRE(lastSymbol == "skill");

    input.setKeyPressed(InputKey::DECISION, false);
    input.update();

    struct CancelTrackingWindow final : Window_Command {
        using Window_Command::Window_Command;
        int32_t cancelCount = 0;
        void processCancel() override { ++cancelCount; }
    };

    CancelTrackingWindow cancelWindow(params);
    cancelWindow.setActive(true);
    cancelWindow.open();

    input.setKeyPressed(InputKey::CANCEL, true);
    cancelWindow.update();
    REQUIRE(cancelWindow.cancelCount == 1);

    input.shutdown();
}

TEST_CASE("Window_Selectable update hit-tests touch input into selection", "[compat][window]") {
    InputManager& input = InputManager::instance();
    input.initialize();
    input.clear();

    Window_Selectable::CreateParams params;
    params.rect = Rect{10, 20, 180, 120};
    params.maxCols = 1;
    params.numVisibleRows = 4;
    params.itemHeight = 24;
    Window_Selectable window(params);
    window.setMaxItems(5);
    window.setIndex(0);
    window.setActive(true);
    window.open();

    const Rect contentRect = window.getContentRect();
    input.setTouchPosition(contentRect.x + 8, contentRect.y + (window.getItemHeight() * 2) + 5);
    input.setTouchPressed(true);
    window.update();
    REQUIRE(window.getIndex() == 2);

    input.shutdown();
}

TEST_CASE("Window_Selectable update consumes mouse wheel over content", "[compat][window]") {
    InputManager& input = InputManager::instance();
    input.initialize();
    input.clear();

    Window_Selectable::CreateParams params;
    params.rect = Rect{10, 20, 180, 120};
    params.maxCols = 1;
    params.numVisibleRows = 4;
    params.itemHeight = 24;
    Window_Selectable window(params);
    window.setMaxItems(8);
    window.setIndex(3);
    window.setActive(true);
    window.open();

    const Rect contentRect = window.getContentRect();
    input.setMousePosition(contentRect.x + 8, contentRect.y + 8);
    input.setMouseWheel(-1);
    REQUIRE(window.isCursorMovable());
    REQUIRE(input.getMouseWheel() == -1);
    REQUIRE(input.getMouseX() == contentRect.x + 8);
    REQUIRE(input.getMouseY() == contentRect.y + 8);
    window.update();
    REQUIRE(window.getIndex() == 4);
    REQUIRE(window.getTopRow() == 1);

    input.setMouseWheel(1);
    window.update();
    REQUIRE(window.getIndex() == 3);
    REQUIRE(window.getTopRow() == 1);

    input.shutdown();
}

TEST_CASE("Window_Selectable mouse wheel is ignored outside content", "[compat][window]") {
    InputManager& input = InputManager::instance();
    input.initialize();
    input.clear();

    Window_Selectable::CreateParams params;
    params.rect = Rect{10, 20, 180, 120};
    params.maxCols = 1;
    params.numVisibleRows = 4;
    params.itemHeight = 24;
    Window_Selectable window(params);
    window.setMaxItems(8);
    window.setIndex(3);
    window.setActive(true);
    window.open();

    input.setMousePosition(0, 0);
    input.setMouseWheel(-1);
    window.update();
    REQUIRE(window.getIndex() == 3);
    REQUIRE(window.getTopRow() == 0);

    input.shutdown();
}

TEST_CASE("Window_Command touch release on current item dispatches ok", "[compat][window]") {
    InputManager& input = InputManager::instance();
    input.initialize();
    input.clear();

    Window_Command::CreateParams params;
    params.rect = Rect{10, 20, 180, 120};
    params.itemHeight = 24;
    params.commands = {
        {"Item", "item", true, 0},
        {"Skill", "skill", true, 1}
    };
    Window_Command window(params);
    window.setActive(true);
    window.open();
    window.setIndex(0);

    int32_t okCount = 0;
    std::string lastSymbol;
    window.setOnCommand([&](const std::string& symbol) {
        ++okCount;
        lastSymbol = symbol;
    });

    const Rect contentRect = window.getContentRect();
    input.setTouchPosition(contentRect.x + 8, contentRect.y + window.getItemHeight() + 5);
    input.setTouchPressed(true);
    window.update();
    REQUIRE(window.getIndex() == 1);
    REQUIRE(okCount == 0);

    input.update();
    input.setTouchPressed(false);
    window.update();
    REQUIRE(okCount == 1);
    REQUIRE(lastSymbol == "skill");

    input.shutdown();
}

TEST_CASE("Window_Command touch drag retargets selection and suppresses ok until stable release", "[compat][window]") {
    InputManager& input = InputManager::instance();
    input.initialize();
    input.clear();

    Window_Command::CreateParams params;
    params.rect = Rect{10, 20, 180, 120};
    params.itemHeight = 24;
    params.commands = {
        {"Item", "item", true, 0},
        {"Skill", "skill", true, 1},
        {"Equip", "equip", true, 2}
    };
    Window_Command window(params);
    window.setActive(true);
    window.open();
    window.setIndex(0);

    int32_t okCount = 0;
    std::string lastSymbol;
    window.setOnCommand([&](const std::string& symbol) {
        ++okCount;
        lastSymbol = symbol;
    });

    const Rect contentRect = window.getContentRect();
    input.setTouchPosition(contentRect.x + 8, contentRect.y + 5);
    input.setTouchPressed(true);
    window.update();
    REQUIRE(window.getIndex() == 0);
    REQUIRE(okCount == 0);

    input.update();
    input.setTouchPosition(contentRect.x + 8, contentRect.y + (window.getItemHeight() * 2) + 5);
    window.update();
    REQUIRE(window.getIndex() == 2);
    REQUIRE(okCount == 0);

    input.update();
    input.setTouchPressed(false);
    window.update();
    REQUIRE(window.getIndex() == 2);
    REQUIRE(okCount == 0);

    input.setTouchPosition(contentRect.x + 8, contentRect.y + (window.getItemHeight() * 2) + 5);
    input.setTouchPressed(true);
    window.update();
    REQUIRE(window.getIndex() == 2);

    input.update();
    input.setTouchPressed(false);
    window.update();
    REQUIRE(okCount == 1);
    REQUIRE(lastSymbol == "equip");

    input.shutdown();
}

TEST_CASE("Window_Selectable touch drag below content scrolls downward", "[compat][window]") {
    InputManager& input = InputManager::instance();
    input.initialize();
    input.clear();

    Window_Selectable::CreateParams params;
    params.rect = Rect{10, 20, 180, 120};
    params.maxCols = 1;
    params.numVisibleRows = 4;
    params.itemHeight = 24;
    Window_Selectable window(params);
    window.setMaxItems(10);
    window.setIndex(3);
    window.setActive(true);
    window.open();

    const Rect contentRect = window.getContentRect();
    input.setTouchPosition(contentRect.x + 8, contentRect.y + (window.getItemHeight() * 3) + 5);
    input.setTouchPressed(true);
    window.update();
    REQUIRE(window.getIndex() == 3);
    REQUIRE(window.getTopRow() == 0);

    input.update();
    input.setTouchPosition(contentRect.x + 8, contentRect.y + contentRect.height + 6);
    window.update();
    REQUIRE(window.getIndex() == 4);
    REQUIRE(window.getTopRow() == 1);

    input.shutdown();
}

// ============================================================================
// Window_Command Tests
// ============================================================================

TEST_CASE("Window_Command creates with commands", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {
        {"New Game", "newGame", true, 0},
        {"Continue", "continue", true, 0},
        {"Options", "options", true, 0}
    };
    
    Window_Command window(params);
    
    REQUIRE(window.getCommandCount() == 3);
    REQUIRE(window.getMaxItems() == 3);
}

TEST_CASE("Window_Command addCommand", "[compat][window]") {
    Window_Command window(Window_Command::CreateParams{});
    
    window.addCommand("Item", "item", true, 0);
    window.addCommand("Skill", "skill", true, 0);
    window.addCommand("Equip", "equip", true, 0);
    
    REQUIRE(window.getCommandCount() == 3);
}

TEST_CASE("Window_Command clearCommands", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {{"Test", "test", true, 0}};
    Window_Command window(params);
    
    REQUIRE(window.getCommandCount() == 1);
    
    window.clearCommands();
    
    REQUIRE(window.getCommandCount() == 0);
    REQUIRE(window.getMaxItems() == 0);
}

TEST_CASE("Window_Command getCurrentCommand", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {
        {"Item", "item", true, 0},
        {"Skill", "skill", true, 1}
    };
    Window_Command window(params);
    
    window.setIndex(1);
    
    auto& cmd = window.getCurrentCommand();
    REQUIRE(cmd.name == "Skill");
    REQUIRE(cmd.symbol == "skill");
}

TEST_CASE("Window_Command getCurrentSymbol", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {
        {"Item", "item", true, 0},
        {"Skill", "skill", true, 1}
    };
    Window_Command window(params);
    
    window.setIndex(0);
    REQUIRE(window.getCurrentSymbol() == "item");
    
    window.setIndex(1);
    REQUIRE(window.getCurrentSymbol() == "skill");
}

TEST_CASE("Window_Command command enabled helpers", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {
        {"Item", "item", true, 0},
        {"Locked", "locked", false, 1}
    };
    Window_Command window(params);

    REQUIRE(window.isCommandEnabled(0));
    REQUIRE_FALSE(window.isCommandEnabled(1));
    REQUIRE_FALSE(window.isCommandEnabled(99));

    window.setIndex(1);
    REQUIRE_FALSE(window.isCurrentItemEnabled());
}

TEST_CASE("Window_Command selectSymbol", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {
        {"Item", "item", true, 0},
        {"Skill", "skill", true, 1},
        {"Equip", "equip", true, 2}
    };
    Window_Command window(params);
    
    window.selectSymbol("skill");
    REQUIRE(window.getIndex() == 1);
    REQUIRE(window.getCurrentSymbol() == "skill");
}

TEST_CASE("Window_Command selectExt", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {
        {"Item", "item", true, 10},
        {"Skill", "skill", true, 20},
        {"Equip", "equip", true, 30}
    };
    Window_Command window(params);
    
    window.selectExt(20);
    REQUIRE(window.getIndex() == 1);
}

TEST_CASE("Window_Command findSymbol and findExt", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {
        {"Item", "item", true, 10},
        {"Skill", "skill", true, 20}
    };
    Window_Command window(params);

    REQUIRE(window.findSymbol("skill") == 1);
    REQUIRE(window.findSymbol("missing") == -1);
    REQUIRE(window.findExt(10) == 0);
    REQUIRE(window.findExt(999) == -1);
}

TEST_CASE("Window_Command callOkHandler honors enabled state", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {
        {"Enabled", "ok", true, 0},
        {"Disabled", "nope", false, 0}
    };
    Window_Command window(params);

    int32_t calledCount = 0;
    std::string lastSymbol;
    window.setOnCommand([&](const std::string& symbol) {
        ++calledCount;
        lastSymbol = symbol;
    });

    window.setIndex(0);
    window.callOkHandler();
    REQUIRE(calledCount == 1);
    REQUIRE(lastSymbol == "ok");

    window.setIndex(1);
    window.callOkHandler();
    REQUIRE(calledCount == 1);
}

TEST_CASE("Window_Command getCommand bounds check", "[compat][window]") {
    Window_Command window(Window_Command::CreateParams{});
    window.addCommand("Test", "test", true, 0);
    
    // Valid index
    auto& cmd = window.getCommand(0);
    REQUIRE(cmd.name == "Test");
    
    // Invalid index returns empty command
    auto& invalid = window.getCommand(100);
    REQUIRE(invalid.name.empty());
    REQUIRE_FALSE(invalid.enabled);
}

TEST_CASE("Window_Command drawItem calls drawText", "[compat][window]") {
    Window_Command::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    params.commands = {
        {"Enabled", "ok", true, 0},
        {"Disabled", "no", false, 1}
    };
    Window_Command window(params);

    const uint32_t drawTextBefore = Window_Base::getMethodCallCount("drawText");
    const uint32_t changeColorBefore = Window_Base::getMethodCallCount("changeTextColor");

    window.drawItem(0);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == drawTextBefore + 1);
    REQUIRE(Window_Base::getMethodCallCount("changeTextColor") == changeColorBefore + 1);

    window.drawItem(1);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == drawTextBefore + 2);
    REQUIRE(Window_Base::getMethodCallCount("changeTextColor") == changeColorBefore + 2);
}

// ============================================================================
// Sprite_Character Tests
// ============================================================================

TEST_CASE("Sprite_Character creates with params", "[compat][sprite]") {
    Sprite_Character::CreateParams params;
    params.characterName = "Actor1";
    params.characterIndex = 2;
    params.x = 100;
    params.y = 200;
    
    Sprite_Character sprite(params);
    
    REQUIRE(sprite.getCharacterName() == "Actor1");
    REQUIRE(sprite.getCharacterIndex() == 2);
    REQUIRE(sprite.getX() == 100);
    REQUIRE(sprite.getY() == 200);
}

TEST_CASE("Sprite_Character position setters", "[compat][sprite]") {
    Sprite_Character sprite(Sprite_Character::CreateParams{});
    
    sprite.setX(50);
    sprite.setY(75);
    
    REQUIRE(sprite.getX() == 50);
    REQUIRE(sprite.getY() == 75);
}

TEST_CASE("Sprite_Character direction", "[compat][sprite]") {
    Sprite_Character sprite(Sprite_Character::CreateParams{});
    
    REQUIRE(sprite.getDirection() == 2);  // Default: down
    
    sprite.setDirection(4);  // Left
    REQUIRE(sprite.getDirection() == 4);
    
    sprite.setDirection(6);  // Right
    REQUIRE(sprite.getDirection() == 6);
    
    sprite.setDirection(8);  // Up
    REQUIRE(sprite.getDirection() == 8);
}

TEST_CASE("Sprite_Character pattern", "[compat][sprite]") {
    Sprite_Character sprite(Sprite_Character::CreateParams{});
    
    REQUIRE(sprite.getPattern() == 0);
    
    sprite.setPattern(1);
    REQUIRE(sprite.getPattern() == 1);
}

TEST_CASE("Sprite_Character visibility", "[compat][sprite]") {
    Sprite_Character sprite(Sprite_Character::CreateParams{});
    
    REQUIRE(sprite.isVisible());
    
    sprite.setVisible(false);
    REQUIRE_FALSE(sprite.isVisible());
}

TEST_CASE("Sprite_Character blend mode", "[compat][sprite]") {
    Sprite_Character sprite(Sprite_Character::CreateParams{});
    
    REQUIRE(sprite.getBlendMode() == 0);  // Normal
    
    sprite.setBlendMode(1);  // Additive
    REQUIRE(sprite.getBlendMode() == 1);
}

TEST_CASE("Sprite_Character opacity", "[compat][sprite]") {
    Sprite_Character sprite(Sprite_Character::CreateParams{});
    
    REQUIRE(sprite.getOpacity() == 255);
    
    sprite.setOpacity(128);
    REQUIRE(sprite.getOpacity() == 128);
}

TEST_CASE("Sprite_Character scale", "[compat][sprite]") {
    Sprite_Character sprite(Sprite_Character::CreateParams{});
    
    REQUIRE(sprite.getScaleX() == 1.0);
    REQUIRE(sprite.getScaleY() == 1.0);
    
    sprite.setScale(2.0, 1.5);
    REQUIRE(sprite.getScaleX() == 2.0);
    REQUIRE(sprite.getScaleY() == 1.5);
}

// ============================================================================
// Sprite_Actor Tests
// ============================================================================

TEST_CASE("Sprite_Actor creates with params", "[compat][sprite]") {
    Sprite_Actor::CreateParams params;
    params.actorId = 5;
    params.x = 150;
    params.y = 250;
    
    Sprite_Actor sprite(params);
    
    REQUIRE(sprite.getActorId() == 5);
    REQUIRE(sprite.getX() == 150);
    REQUIRE(sprite.getY() == 250);
}

TEST_CASE("Sprite_Actor position setters", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    
    sprite.setX(100);
    sprite.setY(200);
    
    REQUIRE(sprite.getX() == 100);
    REQUIRE(sprite.getY() == 200);
}

TEST_CASE("Sprite_Actor motion", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    
    REQUIRE(sprite.getMotion() == 0);
    
    sprite.setMotion(5);  // Skill motion
    REQUIRE(sprite.getMotion() == 5);
}

TEST_CASE("Sprite_Actor startMotion", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    
    sprite.startMotion(3);  // Guard motion
    
    REQUIRE(sprite.getMotion() == 3);
}

TEST_CASE("Sprite_Actor startEffect", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    
    REQUIRE_FALSE(sprite.isEffecting());
    
    sprite.startEffect("collapse");
    
    REQUIRE(sprite.isEffecting());

    for (int i = 0; i < 31; ++i) {
        REQUIRE(sprite.isEffecting());
        sprite.update();
    }
    REQUIRE(sprite.isEffecting());
    sprite.update();
    REQUIRE_FALSE(sprite.isEffecting());
}

TEST_CASE("Sprite_Actor startAnimation runs deterministic lifecycle", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    REQUIRE_FALSE(sprite.isAnimationPlaying());
    REQUIRE(sprite.getAnimationId() == 0);

    sprite.startAnimation(7);
    REQUIRE(sprite.isAnimationPlaying());
    REQUIRE(sprite.getAnimationId() == 7);

    // Duration = 24 + ((id % 5) * 6). For id=7 => 36 frames.
    for (int i = 0; i < 35; ++i) {
        REQUIRE(sprite.isAnimationPlaying());
        sprite.update();
    }

    REQUIRE(sprite.isAnimationPlaying());
    sprite.update();
    REQUIRE_FALSE(sprite.isAnimationPlaying());
    REQUIRE(sprite.getAnimationId() == 0);
}

TEST_CASE("Sprite_Actor startAnimation ignores invalid ids", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    sprite.startAnimation(0);
    REQUIRE_FALSE(sprite.isAnimationPlaying());
    REQUIRE(sprite.getAnimationId() == 0);
}

TEST_CASE("Sprite_Actor unknown effects are ignored deterministically", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    sprite.startEffect("unknownEffect");
    REQUIRE_FALSE(sprite.isEffecting());
}

TEST_CASE("Sprite_Actor visibility and opacity", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    
    REQUIRE(sprite.isVisible());
    REQUIRE(sprite.getOpacity() == 255);
    
    sprite.setVisible(false);
    sprite.setOpacity(100);
    
    REQUIRE_FALSE(sprite.isVisible());
    REQUIRE(sprite.getOpacity() == 100);
}

// ============================================================================
// WindowCompatManager Tests
// ============================================================================

TEST_CASE("WindowCompatManager creates Window_Base", "[compat][manager]") {
    WindowCompatManager manager;
    
    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 100, 50};
    
    uint32_t id = manager.createWindowBase(params);
    
    REQUIRE(id != 0);
    REQUIRE(manager.getWindow(id) != nullptr);
}

TEST_CASE("WindowCompatManager creates Window_Selectable", "[compat][manager]") {
    WindowCompatManager manager;
    
    uint32_t id = manager.createWindowSelectable(Window_Selectable::CreateParams{});
    
    REQUIRE(id != 0);
    REQUIRE(manager.getWindow(id) != nullptr);
}

TEST_CASE("WindowCompatManager creates Window_Command", "[compat][manager]") {
    WindowCompatManager manager;
    
    uint32_t id = manager.createWindowCommand(Window_Command::CreateParams{});
    
    REQUIRE(id != 0);
    REQUIRE(manager.getWindow(id) != nullptr);
}

TEST_CASE("WindowCompatManager creates Sprite_Character", "[compat][manager]") {
    WindowCompatManager manager;
    
    uint32_t id = manager.createSpriteCharacter(Sprite_Character::CreateParams{});
    
    REQUIRE(id != 0);
    REQUIRE(manager.getSpriteCharacter(id) != nullptr);
}

TEST_CASE("WindowCompatManager creates Sprite_Actor", "[compat][manager]") {
    WindowCompatManager manager;
    
    uint32_t id = manager.createSpriteActor(Sprite_Actor::CreateParams{});
    
    REQUIRE(id != 0);
    REQUIRE(manager.getSpriteActor(id) != nullptr);
}

TEST_CASE("WindowCompatManager destroyWindow", "[compat][manager]") {
    WindowCompatManager manager;
    
    uint32_t id = manager.createWindowBase(Window_Base::CreateParams{});
    REQUIRE(manager.getWindow(id) != nullptr);
    
    manager.destroyWindow(id);
    REQUIRE(manager.getWindow(id) == nullptr);
}

TEST_CASE("WindowCompatManager destroySprite", "[compat][manager]") {
    WindowCompatManager manager;
    
    uint32_t id = manager.createSpriteCharacter(Sprite_Character::CreateParams{});
    REQUIRE(manager.getSpriteCharacter(id) != nullptr);
    
    manager.destroySprite(id);
    REQUIRE(manager.getSpriteCharacter(id) == nullptr);
}

TEST_CASE("WindowCompatManager destroyAll", "[compat][manager]") {
    WindowCompatManager manager;
    
    manager.createWindowBase(Window_Base::CreateParams{});
    manager.createSpriteCharacter(Sprite_Character::CreateParams{});
    manager.createSpriteActor(Sprite_Actor::CreateParams{});
    
    manager.destroyAll();
    
    // All should be null after destroyAll
    // Note: IDs are sequential, so we can check 1, 2, 3
    REQUIRE(manager.getWindow(1) == nullptr);
    REQUIRE(manager.getSpriteCharacter(2) == nullptr);
    REQUIRE(manager.getSpriteActor(3) == nullptr);
}

TEST_CASE("WindowCompatManager generates unique IDs", "[compat][manager]") {
    WindowCompatManager manager;
    
    auto id1 = manager.createWindowBase(Window_Base::CreateParams{});
    auto id2 = manager.createWindowBase(Window_Base::CreateParams{});
    auto id3 = manager.createSpriteCharacter(Sprite_Character::CreateParams{});
    
    REQUIRE(id1 != id2);
    REQUIRE(id2 != id3);
    REQUIRE(id1 != id3);
}

TEST_CASE("WindowCompatManager getCompatReport", "[compat][manager]") {
    WindowCompatManager manager;
    
    auto report = manager.getCompatReport();
    
    REQUIRE_FALSE(report.empty());
    
    // Check that Window_Base methods are in report
    bool hasDrawText = false;
    for (const auto& entry : report) {
        if (entry.methodName == "drawText") {
            hasDrawText = true;
            REQUIRE(entry.className == "Window_Base");
            REQUIRE(entry.status == CompatStatus::FULL);
        }
    }
    REQUIRE(hasDrawText);
}

TEST_CASE("WindowCompatManager getCompatReport tracks call counts", "[compat][manager]") {
    WindowCompatManager manager;

    auto id = manager.createWindowBase(Window_Base::CreateParams{});
    auto* window = manager.getWindow(id);
    REQUIRE(window != nullptr);

    const uint32_t before = Window_Base::getMethodCallCount("drawText");
    window->drawText("Hello", 0, 0, 100, "left");

    auto report = manager.getCompatReport();
    uint32_t drawTextCountFromReport = 0;
    for (const auto& entry : report) {
        if (entry.methodName == "drawText") {
            drawTextCountFromReport = entry.callCount;
            break;
        }
    }

    REQUIRE(drawTextCountFromReport == before + 1);
}

TEST_CASE("WindowCompatManager registerAllAPIs", "[compat][manager]") {
    WindowCompatManager manager;
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    // Should not throw
    manager.registerAllAPIs(ctx);
}

TEST_CASE("Window_Selectable JS bindings return non-nil values", "[compat][window]") {
    Window_Selectable window(Window_Selectable::CreateParams{});
    window.setMaxItems(5);
    Window_Base::setDefaultInstance(&window);

    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    Window_Selectable::registerAPI(ctx);

    auto result = ctx.callMethod("Window_Selectable", "index", {});
    REQUIRE(result.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.value.v));

    result = ctx.callMethod("Window_Selectable", "maxItems", {});
    REQUIRE(result.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.value.v));

    result = ctx.callMethod("Window_Selectable", "select", {urpg::Value::Int(2)});
    REQUIRE(result.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.value.v));
    REQUIRE(window.getIndex() == 2);

    result = ctx.callMethod("Window_Selectable", "topRow", {});
    REQUIRE(result.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.value.v));

    result = ctx.callMethod("Window_Selectable", "itemWidth", {});
    REQUIRE(result.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.value.v));

    Window_Base::setDefaultInstance(nullptr);
}

TEST_CASE("Window_Command JS bindings return non-nil values", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {
        {"Item", "item", true, 10}
    };
    Window_Command window(params);
    Window_Base::setDefaultInstance(&window);

    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    Window_Command::registerAPI(ctx);

    urpg::Value strArg;
    strArg.v = std::string("item");
    auto result = ctx.callMethod("Window_Command", "findSymbol", {strArg});
    REQUIRE(result.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.value.v));

    result = ctx.callMethod("Window_Command", "selectSymbol", {strArg});
    REQUIRE(result.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.value.v));

    result = ctx.callMethod("Window_Command", "ext", {});
    REQUIRE(result.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(result.value.v));

    Window_Base::setDefaultInstance(nullptr);
}

// ============================================================================
// Rect and Color Tests
// ============================================================================

TEST_CASE("Rect fromValues", "[compat][types]") {
    auto rect = Rect::fromValues(10, 20, 100, 50);
    
    REQUIRE(rect.x == 10);
    REQUIRE(rect.y == 20);
    REQUIRE(rect.width == 100);
    REQUIRE(rect.height == 50);
}

TEST_CASE("Color fromRGBA", "[compat][types]") {
    auto color = Color::fromRGBA(255, 128, 64, 200);
    
    REQUIRE(color.r == 255);
    REQUIRE(color.g == 128);
    REQUIRE(color.b == 64);
    REQUIRE(color.a == 200);
}

TEST_CASE("Color fromHex", "[compat][types]") {
    auto color = Color::fromHex(0xFF8040CC);
    
    REQUIRE(color.r == 0xFF);
    REQUIRE(color.g == 0x80);
    REQUIRE(color.b == 0x40);
    REQUIRE(color.a == 0xCC);
}

TEST_CASE("Color default alpha is 255", "[compat][types]") {
    auto color = Color::fromRGBA(100, 100, 100);
    
    REQUIRE(color.a == 255);
}
