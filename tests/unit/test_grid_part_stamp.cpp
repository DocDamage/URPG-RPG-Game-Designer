#include "editor/spatial/grid_part_placement_panel.h"
#include "engine/core/map/grid_part_stamp.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

using namespace urpg::editor;
using namespace urpg::map;

namespace {

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category = GridPartCategory::Prop) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.description = "Stamp test part";
    definition.category = category;
    definition.default_layer = GridPartLayer::Object;
    definition.footprint.width = 1;
    definition.footprint.height = 1;
    return definition;
}

PlacedPartInstance makePart(std::string instanceId, std::string partId, int32_t x, int32_t y) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.category = GridPartCategory::Prop;
    part.layer = GridPartLayer::Object;
    part.grid_x = x;
    part.grid_y = y;
    return part;
}

GridPartCatalog makeCatalog() {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));
    REQUIRE(catalog.addDefinition(makeDefinition("prop.barrel")));
    return catalog;
}

GridPartStamp makeStamp() {
    GridPartStamp stamp;
    stamp.stamp_id = "crate_pair";
    stamp.display_name = "Crate Pair";
    stamp.width = 2;
    stamp.height = 1;
    stamp.tags = {"test"};
    stamp.parts.push_back(makePart("source:prop.crate:0:0", "prop.crate", 0, 0));
    stamp.parts.push_back(makePart("source:prop.barrel:1:0", "prop.barrel", 1, 0));
    return stamp;
}

} // namespace

TEST_CASE("Grid part stamp placement rebases coordinates and generates fresh instance ids", "[grid_part][stamp]") {
    const auto catalog = makeCatalog();
    GridPartDocument document("map001", 8, 6);
    const auto stamp = makeStamp();

    const auto result = PlaceGridPartStamp(document, catalog, stamp, 3, 2);

    REQUIRE(result.ok);
    REQUIRE(result.placed_instance_ids.size() == 2);
    REQUIRE(result.placed_instance_ids[0] == "map001:prop.crate:3:2");
    REQUIRE(result.placed_instance_ids[1] == "map001:prop.barrel:4:2");
    REQUIRE(document.findPart("map001:prop.crate:3:2")->grid_x == 3);
    REQUIRE(document.findPart("map001:prop.barrel:4:2")->grid_x == 4);
}

TEST_CASE("Grid part stamp placement suffixes duplicate target ids deterministically", "[grid_part][stamp]") {
    const auto catalog = makeCatalog();
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:3:2", "prop.crate", 3, 2)));

    const auto result = PlaceGridPartStamp(document, catalog, makeStamp(), 3, 2);

    REQUIRE(result.ok);
    REQUIRE(result.placed_instance_ids[0] == "map001:prop.crate:3:2:1");
    REQUIRE(document.findPart("map001:prop.crate:3:2") != nullptr);
    REQUIRE(document.findPart("map001:prop.crate:3:2:1") != nullptr);
}

TEST_CASE("Grid part stamp placement validates all parts atomically", "[grid_part][stamp]") {
    const auto catalog = makeCatalog();
    GridPartDocument document("map001", 4, 4);

    auto stamp = makeStamp();
    stamp.parts.push_back(makePart("source:missing:2:0", "missing.part", 2, 0));

    const auto result = PlaceGridPartStamp(document, catalog, stamp, 1, 1);

    REQUIRE_FALSE(result.ok);
    REQUIRE_FALSE(result.diagnostics.empty());
    REQUIRE(document.parts().empty());
}

TEST_CASE("Grid part placement panel fills rectangles atomically and supports undo redo", "[grid_part][stamp]") {
    const auto catalog = makeCatalog();
    GridPartDocument document("map001", 8, 6);

    GridPartPlacementPanel panel;
    panel.SetTargets(&document, &catalog, nullptr);
    REQUIRE(panel.SetSelectedPartId("prop.crate"));

    REQUIRE(panel.FillSelectedPartRectangle(1, 2, 3, 3));
    REQUIRE(document.parts().size() == 6);
    REQUIRE(document.findPart("map001:prop.crate:1:2") != nullptr);
    REQUIRE(document.findPart("map001:prop.crate:3:3") != nullptr);
    REQUIRE(panel.lastRenderSnapshot().can_undo);

    REQUIRE(panel.Undo());
    REQUIRE(document.parts().empty());
    REQUIRE(panel.lastRenderSnapshot().can_redo);

    REQUIRE(panel.Redo());
    REQUIRE(document.parts().size() == 6);
}
