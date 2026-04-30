#include "engine/core/map/grid_part_objective.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

using namespace urpg::map;

namespace {

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.description = "Objective test part";
    definition.category = category;
    definition.default_layer = GridPartLayer::Object;
    return definition;
}

PlacedPartInstance makePart(std::string instanceId, std::string partId, GridPartCategory category) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.category = category;
    part.layer = GridPartLayer::Object;
    part.grid_x = 1;
    part.grid_y = 1;
    return part;
}

} // namespace

TEST_CASE("Map objective validation reports missing required objective", "[grid_part][objective]") {
    GridPartCatalog catalog;
    GridPartDocument document("map001", 8, 6);

    MapObjective objective;
    objective.required_for_publish = true;

    const auto result = ValidateMapObjective(document, catalog, objective);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.diagnostics.size() == 1);
    REQUIRE(result.diagnostics[0].code == "objective_missing");
}

TEST_CASE("Map objective validation reports missing target instance", "[grid_part][objective]") {
    GridPartCatalog catalog;
    GridPartDocument document("map001", 8, 6);

    MapObjective objective;
    objective.type = MapObjectiveType::OpenChest;
    objective.objective_id = "open_first_chest";
    objective.target_instance_id = "map001:treasure.chest:2:2";

    const auto result = ValidateMapObjective(document, catalog, objective);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.diagnostics.size() == 1);
    REQUIRE(result.diagnostics[0].code == "objective_target_missing");
    REQUIRE(result.diagnostics[0].target == "map001:treasure.chest:2:2");
}

TEST_CASE("Map objective validation reports wrong target category", "[grid_part][objective]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate", GridPartCategory::Prop)));
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate", GridPartCategory::Prop)));

    MapObjective objective;
    objective.type = MapObjectiveType::OpenChest;
    objective.objective_id = "open_first_chest";
    objective.target_instance_id = "map001:prop.crate:1:1";

    const auto result = ValidateMapObjective(document, catalog, objective);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.diagnostics.size() == 1);
    REQUIRE(result.diagnostics[0].code == "objective_target_wrong_category");
    REQUIRE(result.diagnostics[0].instance_id == "map001:prop.crate:1:1");
}

TEST_CASE("Map objective validation accepts category-compatible completion targets", "[grid_part][objective]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("treasure.chest", GridPartCategory::TreasureChest)));
    GridPartDocument document("map001", 8, 6);
    REQUIRE(
        document.placePart(makePart("map001:treasure.chest:2:2", "treasure.chest", GridPartCategory::TreasureChest)));

    MapObjective objective;
    objective.type = MapObjectiveType::OpenChest;
    objective.objective_id = "open_first_chest";
    objective.target_instance_id = "map001:treasure.chest:2:2";

    const auto result = ValidateMapObjective(document, catalog, objective);

    REQUIRE(result.ok);
    REQUIRE(result.diagnostics.empty());
}

TEST_CASE("Map objective validation requires a completion signal when no target is needed", "[grid_part][objective]") {
    GridPartCatalog catalog;
    GridPartDocument document("map001", 8, 6);

    MapObjective objective;
    objective.type = MapObjectiveType::SurviveWaves;
    objective.objective_id = "survive_three_waves";

    const auto missingSignal = ValidateMapObjective(document, catalog, objective);
    REQUIRE_FALSE(missingSignal.ok);
    REQUIRE(missingSignal.diagnostics[0].code == "objective_has_no_completion_signal");

    objective.required_flag = "waves_cleared";
    const auto withFlag = ValidateMapObjective(document, catalog, objective);
    REQUIRE(withFlag.ok);
}
