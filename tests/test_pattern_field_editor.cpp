#include <catch2/catch_test_macros.hpp>

#include "editor/ability/pattern_field_model.h"
#include "editor/ability/pattern_field_panel.h"
#include "engine/core/ability/pattern_field.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace urpg;
using namespace urpg::editor;

TEST_CASE("Pattern Field Editor authoring", "[pattern][editor]") {
    PatternFieldModel model;
    PatternFieldPanel panel;
    panel.bindModel(model);

    SECTION("Initial empty grid") {
        std::ostringstream capturedOutput;
        auto* originalOutput = std::cout.rdbuf(capturedOutput.rdbuf());

        panel.render();
        std::cout.rdbuf(originalOutput);

        const auto& snapshot = panel.getRenderSnapshot();
        REQUIRE(snapshot.has_rendered_frame);
        REQUIRE(snapshot.name == "New Pattern");
        REQUIRE(snapshot.viewport_size == 5);
        REQUIRE(snapshot.grid_rows.size() == 5);
        REQUIRE(snapshot.controls.size() == 5);
        const auto clearControl = std::find_if(snapshot.controls.begin(), snapshot.controls.end(), [](const auto& control) {
            return control.id == "clear_pattern";
        });
        REQUIRE(clearControl != snapshot.controls.end());
        REQUIRE_FALSE(clearControl->enabled);
        REQUIRE(clearControl->disabled_reason == "No selected pattern points to clear.");
        REQUIRE(capturedOutput.str().empty());
    }

    SECTION("Painting cross pattern") {
        REQUIRE(panel.togglePoint(0, 0));
        REQUIRE(panel.togglePoint(1, 0));
        REQUIRE(panel.togglePoint(-1, 0));
        REQUIRE(panel.togglePoint(0, 1));
        REQUIRE(panel.togglePoint(0, -1));
        panel.render();

        REQUIRE(model.isPointSelected(0, 0));
        REQUIRE(model.isPointSelected(1, 0));
        REQUIRE(model.isPointSelected(-1, 0));
        REQUIRE_FALSE(model.isPointSelected(1, 1));
    }

    SECTION("Removing a point") {
        REQUIRE(panel.applyPreset("cross_small"));
        REQUIRE(panel.togglePoint(1, 0));
        panel.render();

        REQUIRE_FALSE(model.isPointSelected(1, 0));
    }

    SECTION("7x7 viewport") {
        REQUIRE(panel.resizeViewport(7));
        panel.render();

        const auto& snapshot = panel.getRenderSnapshot();
        REQUIRE(snapshot.viewport_size == 7);
        REQUIRE(snapshot.grid_rows.size() == 7);
    }

    SECTION("Name and clear actions persist into the bound model") {
        REQUIRE(panel.setPatternName("Skill Burst"));
        REQUIRE(panel.applyPreset("cross_small"));
        REQUIRE(panel.clearPattern());

        const auto pattern = model.getCurrentPattern();
        REQUIRE(pattern);
        REQUIRE(pattern->getName() == "Cross Small");
        REQUIRE(pattern->getPoints().empty());
    }
}
