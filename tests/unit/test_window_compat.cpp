// Unit tests for WindowCompat Core Surface
// Phase 2 - Compat Layer
//
// These tests verify the MZ Window API compatibility surface behavior.

#include "tests/unit/window_compat_test_helpers.h"

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

    REQUIRE(window.getBackground() == 0); // Default: window

    window.setBackground(1);
    REQUIRE(window.getBackground() == 1); // Dim

    window.setBackground(2);
    REQUIRE(window.getBackground() == 2); // Transparent
}

TEST_CASE("Window_Base padding", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});

    REQUIRE(window.getPadding() == 12); // MZ default

    window.setPadding(16);
    REQUIRE(window.getPadding() == 16);
}

TEST_CASE("Window_Base transparent flag in constructor", "[compat][window]") {
    Window_Base::CreateParams params;
    params.transparent = true;

    Window_Base window(params);

    REQUIRE(window.getBackground() == 2); // Transparent
}

TEST_CASE("Window_Base content rect calculation", "[compat][window]") {
    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    // Default padding is 12

    Window_Base window(params);

    auto content = window.getContentRect();
    REQUIRE(content.x == 12);
    REQUIRE(content.y == 12);
    REQUIRE(content.width == 176); // 200 - 12*2
    REQUIRE(content.height == 76); // 100 - 12*2
}

TEST_CASE("Window_Base getMethodStatus returns correct values", "[compat][window]") {
    REQUIRE(Window_Base::getMethodStatus("drawText") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawIcon") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorFace") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawItemName") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("unknownMethod") == CompatStatus::UNSUPPORTED);
}

TEST_CASE("Window_Base getMethodDeviation returns notes", "[compat][window]") {
    REQUIRE(Window_Base::getMethodDeviation("drawActorFace") == "");
    REQUIRE(Window_Base::getMethodDeviation("drawActorHp") == "");
    REQUIRE(Window_Base::getMethodDeviation("drawText") == ""); // FULL, no deviation
    REQUIRE(Window_Base::getMethodDeviation("drawIcon") == "");
}

TEST_CASE("Window_Base drawActorFace records canonical source and destination rects", "[compat][window]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Base window(Window_Base::CreateParams{});

    window.drawActorFace(3, 10, 20, 200, 180);

    const auto drawInfo = window.getLastFaceDraw();
    REQUIRE(drawInfo.has_value());
    REQUIRE(drawInfo->actorId == 3);
    REQUIRE(drawInfo->faceIndex == 2); // fallback mapping: actorId-1
    REQUIRE(drawInfo->faceName == "ActorFace_3");
    REQUIRE(drawInfo->sourceRect.x == 288);
    REQUIRE(drawInfo->sourceRect.y == 0);
    REQUIRE(drawInfo->sourceRect.width == 144);
    REQUIRE(drawInfo->sourceRect.height == 144);
    REQUIRE(drawInfo->destRect.x == 38);
    REQUIRE(drawInfo->destRect.y == 38);
    REQUIRE(drawInfo->destRect.width == 144);
    REQUIRE(drawInfo->destRect.height == 144);

    const auto& commands = renderFrameCommands(layer);
    REQUIRE_FALSE(commands.empty());
    REQUIRE(renderCommandType(commands.back()) == urpg::RenderCmdType::Sprite);

    const auto* spriteCmd = renderCommandAs<urpg::SpriteRenderData>(commands.back());
    REQUIRE(spriteCmd != nullptr);
    REQUIRE(spriteCmd->textureId == "ActorFace_3");
    REQUIRE(spriteCmd->srcX == 288);
    REQUIRE(spriteCmd->srcY == 0);
    REQUIRE(spriteCmd->width == 144);
    REQUIRE(spriteCmd->height == 144);
    REQUIRE(commands.back().x == 50.0f);
    REQUIRE(commands.back().y == 50.0f);

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
    REQUIRE(window.lineHeight() == 36); // MZ default
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

    const auto& commands = renderFrameCommands(layer);
    REQUIRE_FALSE(commands.empty());
    REQUIRE(renderCommandType(commands.back()) == urpg::RenderCmdType::Text);
    const auto* textCmd = renderCommandAs<urpg::TextRenderData>(commands.back());
    REQUIRE(textCmd != nullptr);
    REQUIRE(textCmd->text == "Renderer call");
    REQUIRE(textCmd->fontFace == "TestFace");
    REQUIRE(textCmd->fontSize == 20);
    REQUIRE(textCmd->maxWidth == 180);
    REQUIRE(textCmd->r == 8);
    REQUIRE(textCmd->g == 16);
    REQUIRE(textCmd->b == 24);
    REQUIRE(textCmd->a == 255);
    REQUIRE(commands.back().x == static_cast<float>(20 + 12 + info->resolvedX));
    REQUIRE(commands.back().y == static_cast<float>(30 + 12 + info->resolvedY));
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

    const auto& commands = renderFrameCommands(layer);
    REQUIRE(commands.size() == 9);
    REQUIRE(renderCommandType(commands.front()) == urpg::RenderCmdType::Rect);

    const auto* background = renderCommandAs<urpg::RectRenderData>(commands.front());
    REQUIRE(background != nullptr);
    REQUIRE(background->w == Catch::Approx(100.0f));
    REQUIRE(background->h == Catch::Approx(12.0f));

    float totalFillWidth = 0.0f;
    float expectedX = commands.front().x;
    const float expectedY = commands.front().y;
    for (std::size_t index = 1; index < commands.size(); ++index) {
        REQUIRE(renderCommandType(commands[index]) == urpg::RenderCmdType::Rect);
        const auto* segment = renderCommandAs<urpg::RectRenderData>(commands[index]);
        REQUIRE(segment != nullptr);
        REQUIRE(commands[index].x == Catch::Approx(expectedX));
        REQUIRE(commands[index].y == Catch::Approx(expectedY));
        REQUIRE(segment->h == Catch::Approx(12.0f));
        totalFillWidth += segment->w;
        expectedX += segment->w;
    }
    REQUIRE(totalFillWidth == Catch::Approx(75.0f));
    REQUIRE(expectedX == Catch::Approx(commands.front().x + 75.0f));

    const auto* firstFillCmd = renderCommandAs<urpg::RectRenderData>(commands[1]);
    REQUIRE(firstFillCmd != nullptr);
    REQUIRE(firstFillCmd->r == Catch::Approx(1.0f));
    REQUIRE(firstFillCmd->g == Catch::Approx(0.0f));

    const auto* fillCmd = renderCommandAs<urpg::RectRenderData>(commands.back());
    REQUIRE(fillCmd != nullptr);
    REQUIRE(fillCmd->r == Catch::Approx(0.0f));
    REQUIRE(fillCmd->g == Catch::Approx(1.0f));
}

TEST_CASE("Window_Base drawCharacter submits SpriteCommand", "[compat][window]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Base::CreateParams params;
    params.rect = Rect{0, 0, 200, 100};
    Window_Base window(params);

    window.drawCharacter("Actor1", 2, 30, 40);

    const auto& commands = renderFrameCommands(layer);
    REQUIRE_FALSE(commands.empty());
    REQUIRE(renderCommandType(commands.back()) == urpg::RenderCmdType::Sprite);

    const auto* spriteCmd = renderCommandAs<urpg::SpriteRenderData>(commands.back());
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

    const auto& commands = renderFrameCommands(layer);
    REQUIRE_FALSE(commands.empty());
    REQUIRE(renderCommandType(commands.back()) == urpg::RenderCmdType::Sprite);

    const auto* spriteCmd = renderCommandAs<urpg::SpriteRenderData>(commands.back());
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

TEST_CASE("Window_Base registerAPI parses color objects and strings", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    Window_Base::setDefaultInstance(&window);

    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    Window_Base::registerAPI(ctx);

    auto objectResult =
        ctx.callMethod("Window_Base", "changeTextColor", std::vector<urpg::Value>{colorObject(12, 34, 56, 128)});

    REQUIRE(objectResult.success);
    Color objectColor = window.textColor();
    REQUIRE(objectColor.r == 12);
    REQUIRE(objectColor.g == 34);
    REQUIRE(objectColor.b == 56);
    REQUIRE(objectColor.a == 128);

    auto stringResult =
        ctx.callMethod("Window_Base", "changeTextColor", std::vector<urpg::Value>{stringValue("#102030")});

    REQUIRE(stringResult.success);
    Color stringColor = window.textColor();
    REQUIRE(stringColor.r == 0x10);
    REQUIRE(stringColor.g == 0x20);
    REQUIRE(stringColor.b == 0x30);
    REQUIRE(stringColor.a == 255);

    Window_Base::setDefaultInstance(nullptr);
}

TEST_CASE("Window_Base registerAPI drawGauge uses parsed gradient colors", "[compat][window]") {
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    Window_Base window(Window_Base::CreateParams{});
    Window_Base::setDefaultInstance(&window);

    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    Window_Base::registerAPI(ctx);

    auto result = ctx.callMethod("Window_Base", "drawGauge",
                                 std::vector<urpg::Value>{
                                     urpg::Value::Int(0),
                                     urpg::Value::Int(0),
                                     urpg::Value::Int(80),
                                     [] {
                                         urpg::Value rate;
                                         rate.v = 0.5;
                                         return rate;
                                     }(),
                                     colorObject(200, 10, 20),
                                     stringValue("#0010ff80"),
                                 });

    REQUIRE(result.success);

    const auto& commands = renderFrameCommands(layer);
    REQUIRE(commands.size() >= 9);

    const auto* firstFillCmd = renderCommandAs<urpg::RectRenderData>(commands[1]);
    REQUIRE(firstFillCmd != nullptr);
    REQUIRE(firstFillCmd->r == Catch::Approx(200.0f / 255.0f));
    REQUIRE(firstFillCmd->g == Catch::Approx(10.0f / 255.0f));
    REQUIRE(firstFillCmd->b == Catch::Approx(20.0f / 255.0f));

    const auto* lastFillCmd = renderCommandAs<urpg::RectRenderData>(commands.back());
    REQUIRE(lastFillCmd != nullptr);
    REQUIRE(lastFillCmd->r == Catch::Approx(0.0f));
    REQUIRE(lastFillCmd->g == Catch::Approx(16.0f / 255.0f));
    REQUIRE(lastFillCmd->b == Catch::Approx(1.0f));
    REQUIRE(lastFillCmd->a == Catch::Approx(128.0f / 255.0f));

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
    REQUIRE(window.getContentsBitmapInfo()->pixels.size() == 176 * 76);

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
    REQUIRE(first.getContentsBitmapInfo()->pixels.size() == 176 * 76);
    REQUIRE(second.getContentsBitmapInfo()->width == 136);
    REQUIRE(second.getContentsBitmapInfo()->height == 66);
    REQUIRE(second.getContentsBitmapInfo()->pixels.size() == 136 * 66);
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
    REQUIRE(window.getContentsBitmapInfo()->pixels.size() == 156 * 116);

    window.setPadding(20);
    REQUIRE(window.getContentsBitmapInfo().has_value());
    REQUIRE(window.getContentsBitmapInfo()->width == 140);
    REQUIRE(window.getContentsBitmapInfo()->height == 100);
    REQUIRE(window.getContentsBitmapInfo()->pixels.size() == 140 * 100);
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

    const auto& commands = renderFrameCommands(layer);
    std::string accumulated;
    for (const auto& cmd : commands) {
        if (renderCommandType(cmd) == urpg::RenderCmdType::Text) {
            const auto* textCmd = renderCommandAs<urpg::TextRenderData>(cmd);
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
        centeredSnapshot +=
            entry.text + "@" + std::to_string(entry.resolvedX) + "," + std::to_string(entry.resolvedY) + "|";
    }
    REQUIRE(centeredSnapshot == "Alpha @46,30|beta @116,30|gamma @41,66|delta @109,66|epsilon @35,102|zeta "
                                "@127,102|eta @57,138|theta@101,138|");

    window.clearTextDrawHistory();
    window.setTextAlignment("right");
    window.drawTextEx(wrappedText, x, y, width);
    const auto right = window.getTextDrawHistory();
    std::string rightSnapshot;
    for (const auto& entry : right) {
        rightSnapshot +=
            entry.text + "@" + std::to_string(entry.resolvedX) + "," + std::to_string(entry.resolvedY) + "|";
    }
    REQUIRE(rightSnapshot == "Alpha @68,30|beta @138,30|gamma @58,66|delta @126,66|epsilon @46,102|zeta @138,102|eta "
                             "@90,138|theta@134,138|");
}

TEST_CASE("Window_Base getMethodStatus for extended methods", "[compat][window]") {
    REQUIRE(Window_Base::getMethodStatus("lineHeight") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawTextEx") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorHp") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorMp") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorTp") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("textWidth") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("textSize") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("systemColor") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("contents") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("createContents") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("destroyContents") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("update") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodDeviation("contents").empty());
    REQUIRE(Window_Base::getMethodDeviation("createContents").empty());
    REQUIRE(Window_Base::getMethodDeviation("destroyContents").empty());
    REQUIRE(Window_Base::getMethodDeviation("update").empty());
    REQUIRE(Window_Base::getMethodStatus("nonexistentMethod") == CompatStatus::UNSUPPORTED);
}
