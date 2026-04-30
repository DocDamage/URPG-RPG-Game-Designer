#include "engine/core/map/grid_part_validator.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using namespace urpg::map;

namespace {

PlacedPartInstance makePart(std::string instanceId, std::string partId = "prop.crate", int32_t x = 1, int32_t y = 2) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.grid_x = x;
    part.grid_y = y;
    return part;
}

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category = GridPartCategory::Prop,
                                  GridPartLayer layer = GridPartLayer::Object) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.category = category;
    definition.default_layer = layer;
    definition.footprint.width = 1;
    definition.footprint.height = 1;
    definition.supported_rulesets = {GridPartRuleset::TopDownJRPG};
    return definition;
}

bool hasCode(const std::vector<GridPartDiagnostic>& diagnostics, const std::string& code) {
    return std::any_of(diagnostics.begin(), diagnostics.end(),
                       [&](const GridPartDiagnostic& diagnostic) { return diagnostic.code == code; });
}

} // namespace

TEST_CASE("Grid part validator accepts a valid document", "[grid_part][validator]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:2")));

    const auto result = ValidateGridPartDocument(document, catalog);

    REQUIRE(result.ok);
    REQUIRE(result.diagnostics.empty());
    REQUIRE_FALSE(result.hasBlockers());
    REQUIRE_FALSE(result.hasErrors());
}

TEST_CASE("Grid part validator reports invalid document dimensions", "[grid_part][validator]") {
    GridPartCatalog catalog;
    GridPartDocument document("map001", 0, -1);

    const auto result = ValidateGridPartDocument(document, catalog);

    REQUIRE_FALSE(result.ok);
    REQUIRE(hasCode(result.diagnostics, "invalid_document_dimensions"));
}

TEST_CASE("Grid part validator reports missing and mismatched catalog definitions", "[grid_part][validator]") {
    GridPartCatalog catalog;
    auto doorDefinition = makeDefinition("door.locked", GridPartCategory::Door, GridPartLayer::Object);
    doorDefinition.footprint.width = 2;
    doorDefinition.footprint.height = 1;
    REQUIRE(catalog.addDefinition(doorDefinition));

    GridPartDocument document("map001", 8, 6);
    auto door = makePart("map001:door.locked:1:2", "door.locked", 1, 2);
    door.category = GridPartCategory::Prop;
    door.layer = GridPartLayer::Actor;
    REQUIRE(document.placePart(door));
    REQUIRE(document.placePart(makePart("map001:missing:3:3", "missing.part", 3, 3)));

    const auto result = ValidateGridPartDocument(document, catalog);

    REQUIRE_FALSE(result.ok);
    REQUIRE(hasCode(result.diagnostics, "part_definition_missing"));
    REQUIRE(hasCode(result.diagnostics, "part_category_mismatch"));
    REQUIRE(hasCode(result.diagnostics, "part_layer_mismatch"));
    REQUIRE(hasCode(result.diagnostics, "part_footprint_mismatch"));
}

TEST_CASE("Grid part validator reports out-of-bounds stored parts", "[grid_part][validator]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 4, 4);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:2")));
    auto* part = document.findPartMutable("map001:prop.crate:1:2");
    REQUIRE(part != nullptr);
    part->grid_x = 4;

    const auto result = ValidateGridPartDocument(document, catalog);

    REQUIRE_FALSE(result.ok);
    REQUIRE(hasCode(result.diagnostics, "part_footprint_out_of_bounds"));
}

TEST_CASE("Grid part validator reports incompatible requested ruleset", "[grid_part][validator]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));

    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:2")));

    GridPartValidationOptions options;
    options.ruleset = GridPartRuleset::SideScrollerAction;
    const auto result = ValidateGridPartDocument(document, catalog, options);

    REQUIRE_FALSE(result.ok);
    REQUIRE(hasCode(result.diagnostics, "part_ruleset_incompatible"));
}

TEST_CASE("Grid part validator reports non-overlap footprint conflicts", "[grid_part][validator]") {
    GridPartCatalog catalog;
    auto wall = makeDefinition("wall.stone", GridPartCategory::Wall, GridPartLayer::Collision);
    wall.footprint.blocks_navigation = true;
    wall.footprint.allow_overlap = false;
    REQUIRE(catalog.addDefinition(wall));

    GridPartDocument document("map001", 8, 6);
    auto first = makePart("map001:wall.stone:1:1", "wall.stone", 1, 1);
    first.category = GridPartCategory::Wall;
    first.layer = GridPartLayer::Collision;
    auto second = makePart("map001:wall.stone:1:1:b", "wall.stone", 1, 1);
    second.category = GridPartCategory::Wall;
    second.layer = GridPartLayer::Collision;

    REQUIRE(document.placePart(first));
    REQUIRE(document.placePart(second));

    const auto result = ValidateGridPartDocument(document, catalog);

    REQUIRE_FALSE(result.ok);
    REQUIRE(hasCode(result.diagnostics, "part_overlap_conflict"));
}

TEST_CASE("Grid part validator sorts diagnostics deterministically", "[grid_part][validator]") {
    GridPartCatalog catalog;
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:z:1:2", "missing.z", 1, 2)));
    REQUIRE(document.placePart(makePart("map001:a:1:2", "missing.a", 1, 2)));

    const auto result = ValidateGridPartDocument(document, catalog);

    REQUIRE(result.diagnostics.size() == 2);
    REQUIRE(result.diagnostics[0].instance_id == "map001:a:1:2");
    REQUIRE(result.diagnostics[1].instance_id == "map001:z:1:2");
}
