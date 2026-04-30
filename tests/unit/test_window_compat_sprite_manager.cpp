// Unit tests for WindowCompat sprite, manager, and value type surfaces.

#include "tests/unit/window_compat_test_helpers.h"

// ============================================================================
// Sprite Tests
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

    REQUIRE(sprite.getDirection() == 2); // Default: down

    sprite.setDirection(4); // Left
    REQUIRE(sprite.getDirection() == 4);

    sprite.setDirection(6); // Right
    REQUIRE(sprite.getDirection() == 6);

    sprite.setDirection(8); // Up
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

    REQUIRE(sprite.getBlendMode() == 0); // Normal

    sprite.setBlendMode(1); // Additive
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

TEST_CASE("Sprite_Character source rect tracks character index direction and pattern", "[compat][sprite]") {
    Sprite_Character::CreateParams params;
    params.characterName = "Actor1";
    params.characterIndex = 5;

    Sprite_Character sprite(params);
    REQUIRE(sprite.getSourceRect().x == 144);
    REQUIRE(sprite.getSourceRect().y == 192);
    REQUIRE(sprite.getSourceRect().width == 48);
    REQUIRE(sprite.getSourceRect().height == 48);

    sprite.setDirection(6);
    REQUIRE(sprite.getSourceRect().x == 144);
    REQUIRE(sprite.getSourceRect().y == 288);

    sprite.setPattern(2);
    REQUIRE(sprite.getSourceRect().x == 240);
    REQUIRE(sprite.getSourceRect().y == 288);

    sprite.setCharacterIndex(7);
    REQUIRE(sprite.getSourceRect().x == 528);
    REQUIRE(sprite.getSourceRect().y == 288);
}

TEST_CASE("Sprite_Character bitmap handles reload and release deterministically", "[compat][sprite]") {
    Sprite_Character::CreateParams params;
    params.characterName = "Actor1";

    Sprite_Character sprite(params);
    const auto initial = sprite.getBitmapInfo();
    REQUIRE(initial.has_value());
    REQUIRE(initial->assetId == "Actor1");

    const BitmapHandle initialHandle = initial->handle;
    sprite.setCharacterIndex(7);
    REQUIRE(sprite.getBitmapInfo().has_value());
    REQUIRE(sprite.getBitmapInfo()->handle == initialHandle);

    sprite.setCharacterName("Actor2");
    const auto reloaded = sprite.getBitmapInfo();
    REQUIRE(reloaded.has_value());
    REQUIRE(reloaded->assetId == "Actor2");
    REQUIRE(reloaded->handle != initialHandle);
    REQUIRE_FALSE(Sprite_Character::lookupBitmapInfo(initialHandle).has_value());

    sprite.setCharacterName("");
    REQUIRE_FALSE(sprite.getBitmapInfo().has_value());
    REQUIRE_FALSE(Sprite_Character::lookupBitmapInfo(reloaded->handle).has_value());
}

TEST_CASE("Sprite_Character update advances deterministic pattern cycle without reloading bitmap", "[compat][sprite]") {
    Sprite_Character::CreateParams params;
    params.characterName = "Actor1";
    params.characterIndex = 1;

    Sprite_Character sprite(params);
    const auto initialBitmap = sprite.getBitmapInfo();
    REQUIRE(initialBitmap.has_value());
    REQUIRE(sprite.getPattern() == 0);
    REQUIRE(sprite.getSourceRect().x == 144);
    REQUIRE(sprite.getSourceRect().y == 0);

    for (int i = 0; i < 11; ++i) {
        sprite.update();
    }

    REQUIRE(sprite.getPattern() == 0);
    REQUIRE(sprite.getBitmapInfo()->handle == initialBitmap->handle);

    sprite.update();
    REQUIRE(sprite.getPattern() == 1);
    REQUIRE(sprite.getSourceRect().x == 192);
    REQUIRE(sprite.getSourceRect().y == 0);
    REQUIRE(sprite.getBitmapInfo()->handle == initialBitmap->handle);

    for (int i = 0; i < 24; ++i) {
        sprite.update();
    }

    REQUIRE(sprite.getPattern() == 0);
    REQUIRE(sprite.getSourceRect().x == 144);
    REQUIRE(sprite.getSourceRect().y == 0);
    REQUIRE(sprite.getBitmapInfo()->handle == initialBitmap->handle);
}

TEST_CASE("Sprite_Character destructor releases owned bitmap handles", "[compat][sprite]") {
    BitmapHandle trackedHandle = INVALID_BITMAP;
    {
        Sprite_Character::CreateParams params;
        params.characterName = "Actor1";
        Sprite_Character sprite(params);
        REQUIRE(sprite.getBitmapInfo().has_value());
        trackedHandle = sprite.getBitmapInfo()->handle;
        REQUIRE(Sprite_Character::lookupBitmapInfo(trackedHandle).has_value());
    }

    REQUIRE(trackedHandle != INVALID_BITMAP);
    REQUIRE_FALSE(Sprite_Character::lookupBitmapInfo(trackedHandle).has_value());
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

    sprite.setMotion(5); // Skill motion
    REQUIRE(sprite.getMotion() == 5);
}

TEST_CASE("Sprite_Actor startMotion", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});

    sprite.startMotion(3); // Guard motion

    REQUIRE(sprite.getMotion() == 3);
}

TEST_CASE("Sprite_Actor startMotion keeps looping motions active across updates", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    sprite.startMotion(3); // Guard motion

    for (int i = 0; i < 36; ++i) {
        sprite.update();
        REQUIRE(sprite.getMotion() == 3);
    }
}

TEST_CASE("Sprite_Actor startMotion returns non-looping motions to idle", "[compat][sprite]") {
    Sprite_Actor sprite(Sprite_Actor::CreateParams{});
    sprite.startMotion(7); // Swing motion
    REQUIRE(sprite.getMotion() == 7);

    for (int i = 0; i < 17; ++i) {
        sprite.update();
        REQUIRE(sprite.getMotion() == 7);
    }

    sprite.update();
    REQUIRE(sprite.getMotion() == 0);
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

TEST_CASE("Sprite_Actor bitmap handles follow battler identity and release cleanly", "[compat][sprite]") {
    DataManager::instance().loadActors();

    Sprite_Actor::CreateParams params;
    params.actorId = 1;
    Sprite_Actor sprite(params);

    const auto initial = sprite.getBitmapInfo();
    REQUIRE(initial.has_value());
    REQUIRE(initial->assetId == "Actor1_1");
    REQUIRE(sprite.getBattlerName() == "Actor1_1");

    const BitmapHandle initialHandle = initial->handle;
    sprite.setBattlerName("Actor1_2");
    const auto reloaded = sprite.getBitmapInfo();
    REQUIRE(reloaded.has_value());
    REQUIRE(reloaded->assetId == "Actor1_2");
    REQUIRE(reloaded->handle != initialHandle);
    REQUIRE_FALSE(Sprite_Actor::lookupBitmapInfo(initialHandle).has_value());

    sprite.setBattlerName("");
    REQUIRE_FALSE(sprite.getBitmapInfo().has_value());
    REQUIRE_FALSE(Sprite_Actor::lookupBitmapInfo(reloaded->handle).has_value());
}

TEST_CASE("Sprite_Actor destructor releases owned bitmap handles", "[compat][sprite]") {
    BitmapHandle trackedHandle = INVALID_BITMAP;
    {
        Sprite_Actor::CreateParams params;
        params.battlerName = "Actor1_1";
        Sprite_Actor sprite(params);
        REQUIRE(sprite.getBitmapInfo().has_value());
        trackedHandle = sprite.getBitmapInfo()->handle;
        REQUIRE(Sprite_Actor::lookupBitmapInfo(trackedHandle).has_value());
    }

    REQUIRE(trackedHandle != INVALID_BITMAP);
    REQUIRE_FALSE(Sprite_Actor::lookupBitmapInfo(trackedHandle).has_value());
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

TEST_CASE("WindowCompatManager release-required APIs are not registered as stubs", "[compat][manager][quickjs]") {
    WindowCompatManager manager;
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    manager.registerAllAPIs(ctx);

    const auto statuses = ctx.getAllAPIStatuses();
    REQUIRE_FALSE(statuses.empty());
    for (const auto& status : statuses) {
        INFO(status.apiName);
        REQUIRE(status.status != CompatStatus::STUB);
    }
}

TEST_CASE("Sprite QuickJS bindings mutate deterministic sprite instances", "[compat][manager][quickjs]") {
    WindowCompatManager manager;
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    manager.registerAllAPIs(ctx);

    const auto characterStatus = ctx.getAPIStatus("Sprite_Character.setDirection");
    REQUIRE(characterStatus.status == CompatStatus::FULL);

    const auto actorStatus = ctx.getAPIStatus("Sprite_Actor.startMotion");
    REQUIRE(actorStatus.status == CompatStatus::FULL);

    const auto animationStatus = ctx.getAPIStatus("Sprite_Actor.isAnimationPlaying");
    REQUIRE(animationStatus.status == CompatStatus::FULL);

    Sprite_Character character(Sprite_Character::CreateParams{});
    Sprite_Character::setDefaultInstance(&character);

    const auto characterResult = ctx.callMethod("Sprite_Character", "setDirection", {urpg::Value::Int(6)});
    REQUIRE(characterResult.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(characterResult.value.v));
    REQUIRE(character.getDirection() == 6);

    const auto patternResult = ctx.callMethod("Sprite_Character", "setPattern", {urpg::Value::Int(2)});
    REQUIRE(patternResult.success);
    REQUIRE(character.getPattern() == 2);

    Sprite_Actor actor(Sprite_Actor::CreateParams{});
    Sprite_Actor::setDefaultInstance(&actor);

    const auto actorResult = ctx.callMethod("Sprite_Actor", "startMotion", {urpg::Value::Int(3)});
    REQUIRE(actorResult.success);
    REQUIRE_FALSE(std::holds_alternative<std::monostate>(actorResult.value.v));
    REQUIRE(actor.getMotion() == 3);

    const auto startAnimationResult = ctx.callMethod("Sprite_Actor", "startAnimation", {urpg::Value::Int(7)});
    REQUIRE(startAnimationResult.success);
    REQUIRE(actor.isAnimationPlaying());

    const auto animationResult = ctx.callMethod("Sprite_Actor", "isAnimationPlaying", {});
    REQUIRE(animationResult.success);
    const auto* animationPlaying = std::get_if<bool>(&animationResult.value.v);
    REQUIRE(animationPlaying != nullptr);
    REQUIRE(*animationPlaying);

    Sprite_Character::setDefaultInstance(nullptr);
    Sprite_Actor::setDefaultInstance(nullptr);
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
    params.commands = {{"Item", "item", true, 10}};
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
