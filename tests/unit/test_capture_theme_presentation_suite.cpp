#include "editor/capture/capture_panel.h"
#include "editor/presentation/photo_mode_panel.h"
#include "editor/ui/theme_builder_panel.h"
#include "engine/core/capture/capture_session.h"
#include "engine/core/capture/trailer_capture_manifest.h"
#include "engine/core/presentation/photo_mode_state.h"
#include "engine/core/ui/theme_registry.h"
#include "engine/core/ui/theme_validator.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Capture returns explicit unsupported status in headless mode", "[capture][ffs15]") {
    urpg::capture::CaptureSession session({true, "captures"});

    const auto result = session.captureScreenshot({"scene.title", 7, 1280, 720});

    REQUIRE_FALSE(result.supported);
    REQUIRE(result.status == "unsupported_headless");
    REQUIRE(result.outputName == "scene.title_000007.png");
}

TEST_CASE("Trailer manifest produces deterministic frame and thumbnail outputs", "[capture][ffs15]") {
    urpg::capture::TrailerCaptureManifest manifest{"launch", 3, "store"};

    REQUIRE(manifest.frameName(2) == "launch_store_frame_000002.png");
    REQUIRE(manifest.thumbnailName() == "launch_store_thumbnail.png");
    REQUIRE(manifest.validate().empty());
}

TEST_CASE("Photo mode state does not leak back into gameplay state after exit", "[photo_mode][ffs15]") {
    urpg::presentation::PhotoModeState photo;
    photo.enter({"map", {0.0f, 0.0f, 1.0f}, false, "clear", {}});
    photo.moveCamera({2.0f, 3.0f, 4.0f});
    photo.hideUi(true);
    photo.overrideWeather("rain");
    const auto poseResult = photo.setPose("actor.hero", "victory");
    REQUIRE(poseResult.accepted);

    const auto exported = photo.exportScreenshotState("shot.png");
    photo.exit();

    REQUIRE(exported.outputName == "shot.png");
    REQUIRE(exported.weather == "rain");
    REQUIRE_FALSE(photo.isActive());
    REQUIRE(photo.gameplayState().weather == "clear");
    REQUIRE(photo.poseFor("actor.hero").empty());
}

TEST_CASE("Photo mode rejects poses for actors not present", "[photo_mode][ffs15]") {
    urpg::presentation::PhotoModeState photo;
    photo.enter({"map", {0.0f, 0.0f, 1.0f}, false, "clear", {"actor.hero"}});

    const auto result = photo.setPose("actor.missing", "wave");

    REQUIRE_FALSE(result.accepted);
    REQUIRE(result.code == "actor_not_present");
}

TEST_CASE("Theme validator catches missing font and missing sound", "[theme][ffs15]") {
    urpg::ui::ThemeRegistry registry;
    urpg::ui::UiTheme theme;
    theme.id = "theme.dark";
    theme.font = "missing.ttf";
    theme.cursor = "cursor.default";
    theme.menuSound = "missing.ogg";
    theme.screens = {"title", "menu", "battle"};
    registry.addTheme(theme);

    urpg::ui::ThemeValidator validator({{"default.ttf"}, {"ok.ogg"}});
    const auto diagnostics = validator.validate(registry.theme("theme.dark"));

    REQUIRE(diagnostics.size() == 2);
    REQUIRE(diagnostics[0].code == "missing_font");
    REQUIRE(diagnostics[1].code == "missing_sound");
}

TEST_CASE("UI skin preview renders all core screens into a snapshot", "[theme][ffs15]") {
    urpg::ui::ThemeRegistry registry;
    registry.addTheme({"theme.default", "default.ttf", "cursor.default", "ok.ogg", {"title", "menu", "battle"}});

    const auto snapshot = registry.previewScreens("theme.default", {"title", "menu", "battle"});

    REQUIRE(snapshot.themeId == "theme.default");
    REQUIRE(snapshot.screens == std::vector<std::string>{"battle", "menu", "title"});
    REQUIRE(snapshot.stableHash == "theme.default:battle|menu|title");
    REQUIRE(urpg::editor::ui::ThemeBuilderPanel::snapshotLabel(snapshot) == "theme:preview-ready");
}

TEST_CASE("ThemeBuilderPanel exposes attached project UI theme selector assets", "[ui][theme][editor][asset_library]") {
    urpg::editor::ui::ThemeBuilderPanel panel;
    panel.setProjectAssetOptions({
        {
            "asset.window",
            "Window Skin",
            "content/assets/imported/asset.window/window.png",
            "ui",
            "ui",
            {"ui_theme_selector"},
        },
        {
            "asset.hero",
            "Hero",
            "content/assets/imported/asset.hero/hero.png",
            "sprite",
            "sprite",
            {"sprite_selector"},
        },
    });
    REQUIRE(panel.selectProjectAsset("asset.window"));
    const auto snapshot = panel.renderProjectAssetSelector();

    REQUIRE(snapshot.project_asset_options.size() == 1);
    REQUIRE(snapshot.project_asset_options[0].asset_id == "asset.window");
    REQUIRE(snapshot.project_asset_options[0].picker_kind == "ui");
    REQUIRE(snapshot.selected_project_asset_id == "asset.window");
}

TEST_CASE("Capture and photo panels expose headless snapshot labels", "[capture][photo_mode][theme][ffs15]") {
    REQUIRE(urpg::editor::capture::CapturePanel::snapshotLabel({false, "unsupported_headless", "shot.png"}) == "capture:unsupported_headless");

    urpg::presentation::PhotoModeState photo;
    photo.enter({"map", {0.0f, 0.0f, 1.0f}, true, "clear", {}});
    REQUIRE(urpg::editor::presentation::PhotoModePanel::snapshotLabel(photo) == "photo:active-hidden-ui");
}
