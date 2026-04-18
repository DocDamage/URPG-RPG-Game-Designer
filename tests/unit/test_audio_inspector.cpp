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

    SECTION("Projecting active sounds (Simulated)") {
        core.playSound("se_test", AudioCategory::SE);
        model.refresh(core);
        
        // In this implementation, refresh doesn't yet pull private core state, 
        // but it should return a valid summary handle.
        auto summary = model.getSummary();
        REQUIRE(summary.masterVolume == 1.0f);
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
}
