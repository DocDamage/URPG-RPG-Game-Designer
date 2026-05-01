#include "editor/audio/audio_inspector_model.h"
#include "editor/audio/audio_inspector_panel.h"
#include "engine/core/audio/audio_core.h"
#include <catch2/catch_test_macros.hpp>
#include <memory>

using namespace urpg::editor;
using namespace urpg::audio;

TEST_CASE("AudioInspectorModel: Projection", "[editor][audio][inspector]") {
    AudioCore core;
    AudioInspectorModel model;

    SECTION("Initial state is empty") {
        model.refresh(core);
        auto summary = model.getSummary();
        REQUIRE(summary.activeCount == 0);
        REQUIRE(summary.issueCount == 0);
    }

    SECTION("Projects live active sounds into rows") {
        const auto handle = core.playSound("se_test", AudioCategory::SE);
        core.setCategoryVolume(AudioCategory::System, 0.65f);
        model.refresh(core);

        auto summary = model.getSummary();
        REQUIRE(summary.activeCount == 1);
        REQUIRE(summary.issueCount == 0);
        REQUIRE(summary.masterVolume == 0.65f);

        const auto rows = model.getRows();
        REQUIRE(rows.size() == 1);
        REQUIRE(rows[0].handle == handle);
        REQUIRE(rows[0].assetId == "se_test");
        REQUIRE(rows[0].category == AudioCategory::SE);
        REQUIRE(rows[0].volume == 1.0f);
        REQUIRE(rows[0].pitch == 1.0f);
        REQUIRE_FALSE(rows[0].isLooping);
        REQUIRE(rows[0].isActive);
    }
}

TEST_CASE("AudioInspectorPanel: Visibility", "[editor][audio][panel]") {
    AudioInspectorPanel panel;

    SECTION("Starts hidden") {
        REQUIRE_FALSE(panel.isVisible());
    }

    SECTION("Can be toggled") {
        panel.setVisible(true);
        REQUIRE(panel.isVisible());
    }

    SECTION("Visible render records a snapshot") {
        AudioCore core;
        panel.onRefreshRequested(core);
        panel.setVisible(true);

        panel.render();

        REQUIRE(panel.hasRenderedFrame());
        REQUIRE(panel.lastRenderSnapshot().active_count == 0);
        REQUIRE(panel.lastRenderSnapshot().issue_count == 0);
        REQUIRE(panel.lastRenderSnapshot().master_volume == 1.0f);
        REQUIRE_FALSE(panel.lastRenderSnapshot().has_data);
        REQUIRE(panel.lastRenderSnapshot().model_bound);
        REQUIRE(panel.lastRenderSnapshot().status_message ==
                "No live audio sources or diagnostics are available; refresh from AudioCore.");
        REQUIRE(panel.lastRenderSnapshot().live_rows.empty());
    }

    SECTION("Visible render includes live handle rows") {
        AudioCore core;
        const auto handle = core.playSound("se_ui_ping", AudioCategory::SE);
        panel.onRefreshRequested(core);
        panel.setVisible(true);

        panel.render();

        REQUIRE(panel.hasRenderedFrame());
        REQUIRE(panel.lastRenderSnapshot().active_count == 1);
        REQUIRE(panel.lastRenderSnapshot().has_data);
        REQUIRE(panel.lastRenderSnapshot().status_message.empty());
        REQUIRE(panel.lastRenderSnapshot().live_rows.size() == 1);
        REQUIRE(panel.lastRenderSnapshot().live_rows[0].handle == handle);
        REQUIRE(panel.lastRenderSnapshot().live_rows[0].assetId == "se_ui_ping");
        REQUIRE(panel.lastRenderSnapshot().live_rows[0].category == AudioCategory::SE);
        REQUIRE(panel.lastRenderSnapshot().live_rows[0].isActive);
    }

    SECTION("Visible render exposes attached project audio selector assets") {
        panel.setProjectAssetOptions({
            {
                "asset.click",
                "Click",
                "content/assets/imported/asset.click/click.wav",
                "audio",
                "audio/se",
                {"audio_selector"},
            },
            {
                "asset.hero",
                "Hero",
                "content/assets/imported/asset.hero/hero.png",
                "sprite",
                "sprite",
                {"level_builder", "sprite_selector"},
            },
        });
        REQUIRE(panel.selectProjectAsset("asset.click"));
        panel.setVisible(true);

        panel.render();

        REQUIRE(panel.lastRenderSnapshot().project_asset_options.size() == 1);
        REQUIRE(panel.lastRenderSnapshot().project_asset_options[0].asset_id == "asset.click");
        REQUIRE(panel.lastRenderSnapshot().project_asset_options[0].picker_kind == "audio");
        REQUIRE(panel.lastRenderSnapshot().selected_project_asset_id == "asset.click");
    }

    SECTION("Visible render carries selected live row workflow state") {
        AudioCore core;
        const auto first_handle = core.playSound("se_ui_ping", AudioCategory::SE);
        const auto second_handle = core.playSound("bgm_field", AudioCategory::BGM);
        panel.onRefreshRequested(core);
        REQUIRE(panel.getModel()->selectNextRow());
        panel.setVisible(true);

        panel.render();

        REQUIRE(panel.lastRenderSnapshot().selected_handle.has_value());
        REQUIRE(*panel.lastRenderSnapshot().selected_handle == first_handle);
        REQUIRE(panel.lastRenderSnapshot().selected_row.has_value());
        REQUIRE(panel.lastRenderSnapshot().selected_row->assetId == "se_ui_ping");
        REQUIRE(panel.lastRenderSnapshot().can_select_next_row);
        REQUIRE_FALSE(panel.lastRenderSnapshot().can_select_previous_row);

        REQUIRE(panel.getModel()->selectNextRow());
        panel.render();

        REQUIRE(panel.lastRenderSnapshot().selected_handle.has_value());
        REQUIRE(*panel.lastRenderSnapshot().selected_handle == second_handle);
        REQUIRE(panel.lastRenderSnapshot().selected_row.has_value());
        REQUIRE(panel.lastRenderSnapshot().selected_row->assetId == "bgm_field");
        REQUIRE_FALSE(panel.lastRenderSnapshot().can_select_next_row);
        REQUIRE(panel.lastRenderSnapshot().can_select_previous_row);
    }
}
