#include <catch2/catch_test_macros.hpp>
#include "editor/audio/audio_inspector_model.h"
#include "editor/audio/audio_inspector_panel.h"
#include "engine/core/audio/audio_core.h"
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
    }
}
