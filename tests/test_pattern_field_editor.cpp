#include <catch2/catch_test_macros.hpp>

#include "editor/ability/pattern_field_model.h"
#include "editor/ability/pattern_field_panel.h"
#include "engine/core/ability/pattern_field.h"

using namespace urpg;
using namespace urpg::editor;

TEST_CASE("Pattern Field Editor authoring", "[pattern][editor]") {
    PatternFieldModel model;
    PatternFieldPanel panel;

    SECTION("Initial empty grid") {
        panel.update(model);
        panel.render();
    }

    SECTION("Painting cross pattern") {
        model.togglePoint(0, 0); // Origin
        model.togglePoint(1, 0);
        model.togglePoint(-1, 0);
        model.togglePoint(0, 1);
        model.togglePoint(0, -1);

        panel.update(model);
        panel.render();

        REQUIRE(model.isPointSelected(0, 0));
        REQUIRE(model.isPointSelected(1, 0));
        REQUIRE(model.isPointSelected(-1, 0));
        REQUIRE_FALSE(model.isPointSelected(1, 1));
    }

    SECTION("Removing a point") {
        model.togglePoint(0, 0);
        model.togglePoint(1, 0);
        model.togglePoint(-1, 0);
        model.togglePoint(0, 1);
        model.togglePoint(0, -1);

        model.togglePoint(1, 0);
        panel.update(model);
        panel.render();

        REQUIRE_FALSE(model.isPointSelected(1, 0));
    }

    SECTION("7x7 viewport") {
        model.resizeViewport(7);
        panel.update(model);
        panel.render();
    }
}
