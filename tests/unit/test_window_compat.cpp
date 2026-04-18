// Unit tests for WindowCompat Core Surface
// Phase 2 - Compat Layer
//
// These tests verify the MZ Window API compatibility surface behavior.

#include "runtimes/compat_js/window_compat.h"
#include "runtimes/compat_js/quickjs_runtime.h"
#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/input_manager.h"
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
    REQUIRE(Window_Base::getMethodStatus("drawIcon") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorFace") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawItemName") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("unknownMethod") == CompatStatus::UNSUPPORTED);
}

TEST_CASE("Window_Base getMethodDeviation returns notes", "[compat][window]") {
    REQUIRE(Window_Base::getMethodDeviation("drawActorFace") == "");
    REQUIRE(Window_Base::getMethodDeviation("drawActorHp") == "");
    REQUIRE(Window_Base::getMethodDeviation("drawText") == "");  // FULL, no deviation
}

TEST_CASE("Window_Base drawActorFace records canonical source and destination rects", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});

    const uint32_t drawIconBefore = Window_Base::getMethodCallCount("drawIcon");
    window.drawActorFace(3, 10, 20, 200, 180);
    REQUIRE(Window_Base::getMethodCallCount("drawIcon") == drawIconBefore + 1);

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

    const uint32_t drawIconBefore = Window_Base::getMethodCallCount("drawIcon");

    window.drawActorFace(0, 0, 0, 144, 144);
    REQUIRE_FALSE(window.getLastFaceDraw().has_value());
    REQUIRE(Window_Base::getMethodCallCount("drawIcon") == drawIconBefore);

    window.drawActorFace(1, 0, 0, 0, 144);
    REQUIRE_FALSE(window.getLastFaceDraw().has_value());
    REQUIRE(Window_Base::getMethodCallCount("drawIcon") == drawIconBefore);
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
    Window_Base window(Window_Base::CreateParams{});

    const uint32_t gaugeBefore = Window_Base::getMethodCallCount("drawGauge");
    const uint32_t textBefore = Window_Base::getMethodCallCount("drawText");

    window.drawActorHp(1, 0, 0, 128);
    window.drawActorMp(1, 0, 0, 128);
    window.drawActorTp(1, 0, 0, 128);

    REQUIRE(Window_Base::getMethodCallCount("drawGauge") == gaugeBefore + 3);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == textBefore + 3);
}

TEST_CASE("Window_Base drawActorName uses DataManager actor name", "[compat][window]") {
    DataManager::instance().clearDatabase();
#ifdef URPG_SOURCE_DIR
    DataManager::instance().setDataPath(URPG_SOURCE_DIR "/tests/data/mz_data");
#else
    DataManager::instance().setDataPath("tests/data/mz_data");
#endif
    DataManager::instance().loadDatabase();

    Window_Base window(Window_Base::CreateParams{});

    // Valid actorId - should not crash and record drawText
    const uint32_t drawTextBefore = Window_Base::getMethodCallCount("drawText");
    window.drawActorName(1, 0, 0, 128);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == drawTextBefore + 1);

    // Invalid actorId - should not crash and record drawText with fallback name
    window.drawActorName(999, 0, 0, 128);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == drawTextBefore + 2);

    DataManager::instance().clearDatabase();
}

TEST_CASE("Window_Base drawActorLevel uses DataManager actor level", "[compat][window]") {
    DataManager::instance().clearDatabase();
#ifdef URPG_SOURCE_DIR
    DataManager::instance().setDataPath(URPG_SOURCE_DIR "/tests/data/mz_data");
#else
    DataManager::instance().setDataPath("tests/data/mz_data");
#endif
    DataManager::instance().loadDatabase();

    Window_Base window(Window_Base::CreateParams{});

    // Valid actorId - should not crash and record drawText
    const uint32_t drawTextBefore = Window_Base::getMethodCallCount("drawText");
    window.drawActorLevel(1, 0, 0);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == drawTextBefore + 1);

    // Invalid actorId - should not crash and record drawText with fallback level
    window.drawActorLevel(999, 0, 0);
    REQUIRE(Window_Base::getMethodCallCount("drawText") == drawTextBefore + 2);

    DataManager::instance().clearDatabase();
}

TEST_CASE("Window_Base drawActorHp computes rate from actor params", "[compat][window]") {
    DataManager::instance().clearDatabase();
#ifdef URPG_SOURCE_DIR
    DataManager::instance().setDataPath(URPG_SOURCE_DIR "/tests/data/mz_data");
#else
    DataManager::instance().setDataPath("tests/data/mz_data");
#endif
    DataManager::instance().loadDatabase();
    DataManager::instance().setupNewGame();

    Window_Base window(Window_Base::CreateParams{});

    // Valid actorId - should not crash and call drawGauge
    const uint32_t gaugeBefore = Window_Base::getMethodCallCount("drawGauge");
    window.drawActorHp(1, 0, 0, 128);
    REQUIRE(Window_Base::getMethodCallCount("drawGauge") == gaugeBefore + 1);
    double fullRate = window.getLastGaugeRate();
    REQUIRE(fullRate == 1.0);

    // Reduce HP and verify rate changes
    DataManager::instance().setGameActorHp(1, DataManager::instance().getGameActor(1)->mhp / 2);
    window.drawActorHp(1, 0, 0, 128);
    double halfRate = window.getLastGaugeRate();
    REQUIRE(halfRate == 0.5);

    // Invalid actorId - should not crash and call drawGauge with default rate
    window.drawActorHp(999, 0, 0, 128);
    REQUIRE(Window_Base::getMethodCallCount("drawGauge") == gaugeBefore + 3);

    DataManager::instance().clearDatabase();
}

TEST_CASE("Window_Base drawActorTp uses default mtp of 100", "[compat][window]") {
    DataManager::instance().clearDatabase();
    auto& actor = DataManager::instance().addTestActor();
    actor.id = 1;
    actor.initialLevel = 1;
    actor.params = {{100, 100, 10, 10, 10, 10, 10, 10}};
    DataManager::instance().setupGameActors();
    DataManager::instance().setGameActorTp(1, 50);

    Window_Base window(Window_Base::CreateParams{});
    window.drawActorTp(1, 0, 0, 128);
    // Default mtp is 100, so rate = 50/100 = 0.5
    REQUIRE(window.getLastGaugeRate() == 0.5);

    DataManager::instance().clearDatabase();
}

TEST_CASE("Window_Base drawActorTp respects custom mtp", "[compat][window]") {
    DataManager::instance().clearDatabase();
    InputManager::instance().clear();
    auto& actor = DataManager::instance().addTestActor();
    actor.id = 1;
    actor.initialLevel = 1;
    actor.params = {{100, 100, 10, 10, 10, 10, 10, 10}};
    DataManager::instance().setupGameActors();
    DataManager::instance().setGameActorTp(1, 50);
    DataManager::instance().setGameActorMtp(1, 200);

    Window_Base window(Window_Base::CreateParams{});
    window.drawActorTp(1, 0, 0, 128);
    // lastGaugeRate_ should be 50/200 = 0.25
    REQUIRE(window.getLastGaugeRate() == 0.25);

    DataManager::instance().clearDatabase();
}

TEST_CASE("Window_Base drawActorTp clamps rate to 1.0 when tp exceeds mtp", "[compat][window]") {
    DataManager::instance().clearDatabase();
    auto& actor = DataManager::instance().addTestActor();
    actor.id = 1;
    actor.initialLevel = 1;
    actor.params = {{100, 100, 10, 10, 10, 10, 10, 10}};
    DataManager::instance().setupGameActors();
    DataManager::instance().setGameActorTp(1, 150);
    DataManager::instance().setGameActorMtp(1, 100);

    Window_Base window(Window_Base::CreateParams{});
    window.drawActorTp(1, 0, 0, 128);
    // rate should be clamped to 1.0
    REQUIRE(window.getLastGaugeRate() == 1.0);

    DataManager::instance().clearDatabase();
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

TEST_CASE("Window_Base color helpers return expected colors", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    Color normal = window.normalColor();
    REQUIRE(normal.r == window.systemColor(0).r);
    REQUIRE(normal.g == window.systemColor(0).g);
    REQUIRE(normal.b == window.systemColor(0).b);
    REQUIRE(normal.a == window.systemColor(0).a);
    
    Color dim = window.dimColor();
    REQUIRE(dim.r == 128);
    REQUIRE(dim.g == 128);
    REQUIRE(dim.b == 128);
    REQUIRE(dim.a == 255);
    
    Color death = window.deathColor();
    REQUIRE(death.r == 255);
    REQUIRE(death.g == 0);
    REQUIRE(death.b == 0);
    REQUIRE(death.a == 255);
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
    Window_Base window(Window_Base::CreateParams{});
    
    window.createContents();
    REQUIRE(window.contents() != INVALID_BITMAP);
    
    window.destroyContents();
    REQUIRE(window.contents() == INVALID_BITMAP);
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

TEST_CASE("Window_Base getMethodStatus for extended methods", "[compat][window]") {
    REQUIRE(Window_Base::getMethodStatus("lineHeight") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawTextEx") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorHp") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorMp") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("drawActorTp") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("textWidth") == CompatStatus::FULL);
    REQUIRE(Window_Base::getMethodStatus("textSize") == CompatStatus::FULL);
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

TEST_CASE("Window_Selectable processCursorMove reads InputManager dir4", "[compat][window]") {
    InputManager::instance().clear();

    Window_Selectable::CreateParams params;
    params.maxCols = 2;
    Window_Selectable window(params);
    window.setMaxItems(10);

    // DOWN: index 0 -> 2
    window.setIndex(0);
    InputManager::instance().setKeyPressed(InputKey::DOWN, true);
    InputManager::instance().update();
    window.update();
    REQUIRE(window.getIndex() == 2);

    // LEFT: index 1 -> 0
    InputManager::instance().clear();
    window.setIndex(1);
    InputManager::instance().setKeyPressed(InputKey::LEFT, true);
    InputManager::instance().update();
    window.update();
    REQUIRE(window.getIndex() == 0);

    // RIGHT: index 0 -> 1
    InputManager::instance().clear();
    window.setIndex(0);
    InputManager::instance().setKeyPressed(InputKey::RIGHT, true);
    InputManager::instance().update();
    window.update();
    REQUIRE(window.getIndex() == 1);

    // UP: index 2 -> 0
    InputManager::instance().clear();
    window.setIndex(2);
    InputManager::instance().setKeyPressed(InputKey::UP, true);
    InputManager::instance().update();
    window.update();
    REQUIRE(window.getIndex() == 0);

    InputManager::instance().clear();
}

TEST_CASE("Window_Selectable processHandling triggers on OK", "[compat][window]") {
    InputManager::instance().clear();

    Window_Selectable window(Window_Selectable::CreateParams{});
    window.setMaxItems(5);
    window.setIndex(2);

    int32_t callbackIndex = -999;
    window.setOnSelect([&](int32_t index) {
        callbackIndex = index;
    });

    InputManager::instance().setKeyPressed(InputKey::DECISION, true);
    InputManager::instance().update();
    window.update();

    REQUIRE(callbackIndex == 2);

    InputManager::instance().clear();
}

TEST_CASE("Window_Selectable hitTest maps coordinates to item index", "[compat][window]") {
    Window_Selectable::CreateParams params;
    params.rect = Rect{0, 0, 200, 200};
    params.maxCols = 2;
    params.itemHeight = 36;
    params.numVisibleRows = 4;
    Window_Selectable window(params);
    window.setMaxItems(10);

    // Padding is 12, content starts at local (12, 12)
    // itemWidth = (176 - 8) / 2 = 84
    // Item 0 at local (12, 12) -> rel (0, 0)
    REQUIRE(window.hitTest(12, 12) == 0);
    // Item 1 at local (104, 12) -> rel (92, 0), col=92/84=1
    REQUIRE(window.hitTest(104, 12) == 1);
    // Item 2 at local (12, 48) -> rel (0, 36), row=36/36=1
    REQUIRE(window.hitTest(12, 48) == 2);
    // Item 3 at local (104, 48) -> rel (92, 36)
    REQUIRE(window.hitTest(104, 48) == 3);

    // Out of bounds (inside padding)
    REQUIRE(window.hitTest(0, 0) == -1);
    REQUIRE(window.hitTest(11, 11) == -1);

    // Out of bounds (beyond content)
    REQUIRE(window.hitTest(200, 200) == -1);

    // No items
    window.setMaxItems(0);
    REQUIRE(window.hitTest(12, 12) == -1);
}

TEST_CASE("Window_Selectable touch tap sets cursor index", "[compat][window]") {
    InputManager::instance().clear();

    Window_Selectable::CreateParams params;
    params.rect = Rect{0, 0, 200, 200};
    params.maxCols = 2;
    params.itemHeight = 36;
    params.numVisibleRows = 4;
    Window_Selectable window(params);
    window.setMaxItems(10);
    window.setIndex(0);

    // Touch at coordinate mapping to index 3
    InputManager::instance().update(); // clear previous frame state
    InputManager::instance().setTouchPosition(104, 48);
    InputManager::instance().setTouchPressed(true);
    window.update();

    REQUIRE(window.getIndex() == 3);

    InputManager::instance().clear();
}

TEST_CASE("Window_Selectable mouse click sets cursor index", "[compat][window]") {
    InputManager::instance().clear();

    Window_Selectable::CreateParams params;
    params.rect = Rect{0, 0, 200, 200};
    params.maxCols = 2;
    params.itemHeight = 36;
    params.numVisibleRows = 4;
    Window_Selectable window(params);
    window.setMaxItems(10);
    window.setIndex(0);

    // Mouse click at coordinate mapping to index 3
    InputManager::instance().update(); // clear previous frame state
    InputManager::instance().setMousePosition(104, 48);
    InputManager::instance().setMousePressed(0, true);
    window.update();

    REQUIRE(window.getIndex() == 3);

    InputManager::instance().clear();
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

TEST_CASE("Window_Command onOk calls callOkHandler", "[compat][window]") {
    InputManager::instance().clear();

    Window_Command::CreateParams params;
    params.commands = {
        {"Item", "item", true, 0},
        {"Skill", "skill", true, 0}
    };
    Window_Command window(params);
    window.setIndex(1);

    std::string lastSymbol;
    window.setOnCommand([&](const std::string& symbol) {
        lastSymbol = symbol;
    });

    InputManager::instance().setKeyPressed(InputKey::DECISION, true);
    InputManager::instance().update();
    window.update();

    REQUIRE(lastSymbol == "skill");

    InputManager::instance().clear();
}

TEST_CASE("Window_Command drawItem draws command text with color", "[compat][window]") {
    Window_Command::CreateParams params;
    params.rect = Rect{0, 0, 200, 200};
    params.commands = {
        {"Enabled", "enabled", true, 0},
        {"Disabled", "disabled", false, 1}
    };
    Window_Command window(params);

    const uint32_t drawItemBefore = Window_Base::getMethodCallCount("drawItem");
    const uint32_t drawTextBefore = Window_Base::getMethodCallCount("drawText");

    window.drawItem(0);
    window.drawItem(1);

    REQUIRE(Window_Base::getMethodCallCount("drawItem") == drawItemBefore + 2);
    REQUIRE(Window_Base::getMethodCallCount("drawText") >= drawTextBefore + 2);
}

TEST_CASE("Window_Command drawAllItems draws all commands", "[compat][window]") {
    Window_Command::CreateParams params;
    params.rect = Rect{0, 0, 200, 200};
    params.commands = {
        {"One", "one", true, 0},
        {"Two", "two", true, 0},
        {"Three", "three", false, 0}
    };
    Window_Command window(params);

    const uint32_t drawItemBefore = Window_Base::getMethodCallCount("drawItem");

    window.drawAllItems();

    REQUIRE(Window_Base::getMethodCallCount("drawItem") == drawItemBefore + 3);
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

TEST_CASE("Sprite_Character setCharacterName manages bitmap lifecycle", "[compat][sprite]") {
    Sprite_Character sprite(Sprite_Character::CreateParams{});
    
    REQUIRE(sprite.getBitmap() == INVALID_BITMAP);
    
    sprite.setCharacterName("Actor1");
    REQUIRE(sprite.getBitmap() != INVALID_BITMAP);
    
    sprite.setCharacterName("");
    REQUIRE(sprite.getBitmap() == INVALID_BITMAP);
}

TEST_CASE("Sprite_Actor startMotion sets up animation state", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    
    sprite.startMotion(1); // walk
    
    REQUIRE(sprite.getMotion() == 1);
    REQUIRE(sprite.isMotionPlaying());
    REQUIRE(sprite.getMotionFramesRemaining() > 0);
    
    // Exhaust frames
    for (int i = 0; i < 100; ++i) {
        sprite.update();
    }
    REQUIRE_FALSE(sprite.isMotionPlaying());
}

TEST_CASE("Sprite_Actor update handles motion countdown", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    
    sprite.startMotion(4); // damage (24 frames)
    
    REQUIRE(sprite.getMotionFramesRemaining() == 24);
    
    for (int i = 0; i < 12; ++i) {
        sprite.update();
    }
    REQUIRE(sprite.getMotionFramesRemaining() == 12);
    
    for (int i = 0; i < 12; ++i) {
        sprite.update();
    }
    REQUIRE_FALSE(sprite.isMotionPlaying());
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

TEST_CASE("Window_Base contents bitmap lifecycle", "[compat][window]") {
    Window_Base window(Window_Base::CreateParams{});
    
    REQUIRE(window.contents() == INVALID_BITMAP);
    
    window.createContents();
    REQUIRE(window.contents() != INVALID_BITMAP);
    
    window.destroyContents();
    REQUIRE(window.contents() == INVALID_BITMAP);
}

TEST_CASE("Sprite_Character update advances pattern deterministically", "[compat][sprite]") {
    Sprite_Character sprite(Sprite_Character::CreateParams{});
    
    REQUIRE(sprite.getPattern() == 0);
    
    sprite.update();
    REQUIRE(sprite.getPattern() == 1);
    
    sprite.update();
    REQUIRE(sprite.getPattern() == 2);
    
    sprite.update();
    REQUIRE(sprite.getPattern() == 3);
    
    sprite.update();
    REQUIRE(sprite.getPattern() == 0);
    
    sprite.update();
    REQUIRE(sprite.getPattern() == 1);
}

TEST_CASE("Sprite_Actor update completes animation lifecycle", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    
    sprite.startAnimation(1);
    REQUIRE(sprite.isAnimationPlaying());
    
    // Duration = 24 + ((1 % 5) * 6) = 30 frames
    for (int i = 0; i < 29; ++i) {
        REQUIRE(sprite.isAnimationPlaying());
        sprite.update();
    }
    
    REQUIRE(sprite.isAnimationPlaying());
    sprite.update();
    REQUIRE_FALSE(sprite.isAnimationPlaying());
}

TEST_CASE("Sprite_Actor update completes effect lifecycle", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    
    sprite.startEffect("whiten");
    REQUIRE(sprite.isEffecting());
    
    // whiten = 16 frames
    for (int i = 0; i < 15; ++i) {
        REQUIRE(sprite.isEffecting());
        sprite.update();
    }
    
    REQUIRE(sprite.isEffecting());
    sprite.update();
    REQUIRE_FALSE(sprite.isEffecting());
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
