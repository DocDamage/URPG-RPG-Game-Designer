// Unit tests for WindowCompat selectable and command surfaces.

#include "tests/unit/window_compat_test_helpers.h"


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
    REQUIRE(window.getIndex() == 9); // Clamped to maxItems - 1

    window.setIndex(-10);
    REQUIRE(window.getIndex() == -1); // Deselected
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
    REQUIRE(window.getIndex() == 2); // +maxCols

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
    REQUIRE(window.getIndex() == 3); // +numVisibleRows

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
    window.setIndex(10); // Beyond visible rows

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
    params.commands = {{"Item", "item", true, 0}, {"Skill", "skill", true, 1}};
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
    params.commands = {{"Item", "item", true, 0}, {"Skill", "skill", true, 1}};
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
    params.commands = {{"Item", "item", true, 0}, {"Skill", "skill", true, 1}, {"Equip", "equip", true, 2}};
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
        {"New Game", "newGame", true, 0}, {"Continue", "continue", true, 0}, {"Options", "options", true, 0}};

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
    params.commands = {{"Item", "item", true, 0}, {"Skill", "skill", true, 1}};
    Window_Command window(params);

    window.setIndex(1);

    auto& cmd = window.getCurrentCommand();
    REQUIRE(cmd.name == "Skill");
    REQUIRE(cmd.symbol == "skill");
}

TEST_CASE("Window_Command getCurrentSymbol", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {{"Item", "item", true, 0}, {"Skill", "skill", true, 1}};
    Window_Command window(params);

    window.setIndex(0);
    REQUIRE(window.getCurrentSymbol() == "item");

    window.setIndex(1);
    REQUIRE(window.getCurrentSymbol() == "skill");
}

TEST_CASE("Window_Command command enabled helpers", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {{"Item", "item", true, 0}, {"Locked", "locked", false, 1}};
    Window_Command window(params);

    REQUIRE(window.isCommandEnabled(0));
    REQUIRE_FALSE(window.isCommandEnabled(1));
    REQUIRE_FALSE(window.isCommandEnabled(99));

    window.setIndex(1);
    REQUIRE_FALSE(window.isCurrentItemEnabled());
}

TEST_CASE("Window_Command selectSymbol", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {{"Item", "item", true, 0}, {"Skill", "skill", true, 1}, {"Equip", "equip", true, 2}};
    Window_Command window(params);

    window.selectSymbol("skill");
    REQUIRE(window.getIndex() == 1);
    REQUIRE(window.getCurrentSymbol() == "skill");
}

TEST_CASE("Window_Command selectExt", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {{"Item", "item", true, 10}, {"Skill", "skill", true, 20}, {"Equip", "equip", true, 30}};
    Window_Command window(params);

    window.selectExt(20);
    REQUIRE(window.getIndex() == 1);
}

TEST_CASE("Window_Command findSymbol and findExt", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {{"Item", "item", true, 10}, {"Skill", "skill", true, 20}};
    Window_Command window(params);

    REQUIRE(window.findSymbol("skill") == 1);
    REQUIRE(window.findSymbol("missing") == -1);
    REQUIRE(window.findExt(10) == 0);
    REQUIRE(window.findExt(999) == -1);
}

TEST_CASE("Window_Command callOkHandler honors enabled state", "[compat][window]") {
    Window_Command::CreateParams params;
    params.commands = {{"Enabled", "ok", true, 0}, {"Disabled", "nope", false, 0}};
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
    params.commands = {{"Enabled", "ok", true, 0}, {"Disabled", "no", false, 1}};
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

