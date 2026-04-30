#include "engine/core/map/grid_part_reachability.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>
#include <vector>

using namespace urpg::map;

namespace {

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category = GridPartCategory::Prop,
                                  bool blocksNavigation = false) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.description = "Reachability test part";
    definition.category = category;
    definition.default_layer = category == GridPartCategory::Wall ? GridPartLayer::Collision : GridPartLayer::Object;
    definition.collision_policy = blocksNavigation ? GridPartCollisionPolicy::Solid : GridPartCollisionPolicy::None;
    definition.footprint.blocks_navigation = blocksNavigation;
    return definition;
}

PlacedPartInstance makePart(std::string instanceId, std::string partId, GridPartCategory category, int32_t x, int32_t y,
                            int32_t width = 1, int32_t height = 1) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.category = category;
    part.layer = category == GridPartCategory::Wall ? GridPartLayer::Collision : GridPartLayer::Object;
    part.grid_x = x;
    part.grid_y = y;
    part.width = width;
    part.height = height;
    return part;
}

GridPartCatalog makeCatalog() {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("trigger.spawn", GridPartCategory::Trigger)));
    REQUIRE(catalog.addDefinition(makeDefinition("trigger.exit", GridPartCategory::Trigger)));
    REQUIRE(catalog.addDefinition(makeDefinition("treasure.chest", GridPartCategory::TreasureChest)));
    REQUIRE(catalog.addDefinition(makeDefinition("quest.key", GridPartCategory::QuestItem)));
    REQUIRE(catalog.addDefinition(makeDefinition("door.locked", GridPartCategory::Door, true)));
    REQUIRE(catalog.addDefinition(makeDefinition("wall.stone", GridPartCategory::Wall, true)));
    return catalog;
}

PlacedPartInstance makeSpawn(int32_t x = 0, int32_t y = 0) {
    auto spawn = makePart("map001:trigger.spawn:0:0", "trigger.spawn", GridPartCategory::Trigger, x, y);
    spawn.properties["role"] = "player_spawn";
    return spawn;
}

} // namespace

TEST_CASE("Grid part reachability accepts reachable objective targets", "[grid_part][reachability]") {
    auto catalog = makeCatalog();
    GridPartDocument document("map001", 5, 5);
    REQUIRE(document.placePart(makeSpawn()));
    REQUIRE(document.placePart(
        makePart("map001:treasure.chest:3:0", "treasure.chest", GridPartCategory::TreasureChest, 3, 0)));

    auto ruleset = MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG);
    MapObjective objective;
    objective.type = MapObjectiveType::OpenChest;
    objective.objective_id = "open_chest";
    objective.target_instance_id = "map001:treasure.chest:3:0";

    const auto report = ValidateReachability(document, catalog, ruleset, objective);

    REQUIRE(report.ok);
    REQUIRE(report.diagnostics.empty());
    REQUIRE(report.reachable_cells.size() == 25);
}

TEST_CASE("Grid part reachability reports missing required spawn and objective", "[grid_part][reachability]") {
    auto catalog = makeCatalog();
    GridPartDocument document("map001", 5, 5);
    auto ruleset = MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG);
    MapObjective objective;

    const auto report = ValidateReachability(document, catalog, ruleset, objective);

    REQUIRE_FALSE(report.ok);
    REQUIRE(report.diagnostics.size() == 2);
    REQUIRE(report.diagnostics[0].code == "objective_missing");
    REQUIRE(report.diagnostics[1].code == "ruleset_requires_spawn");
}

TEST_CASE("Grid part reachability reports unreachable objective behind solid parts", "[grid_part][reachability]") {
    auto catalog = makeCatalog();
    GridPartDocument document("map001", 5, 5);
    REQUIRE(document.placePart(makeSpawn(0, 0)));
    REQUIRE(document.placePart(makePart("map001:wall.stone:1:0", "wall.stone", GridPartCategory::Wall, 1, 0, 1, 5)));
    REQUIRE(document.placePart(
        makePart("map001:treasure.chest:3:0", "treasure.chest", GridPartCategory::TreasureChest, 3, 0)));

    auto ruleset = MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG);
    MapObjective objective;
    objective.type = MapObjectiveType::OpenChest;
    objective.objective_id = "open_chest";
    objective.target_instance_id = "map001:treasure.chest:3:0";

    const auto report = ValidateReachability(document, catalog, ruleset, objective);

    REQUIRE_FALSE(report.ok);
    REQUIRE(report.diagnostics.size() == 1);
    REQUIRE(report.diagnostics[0].code == "objective_unreachable");
    REQUIRE(report.diagnostics[0].instance_id == "map001:treasure.chest:3:0");
}

TEST_CASE("Grid part reachability reports required exit when ruleset requires it", "[grid_part][reachability]") {
    auto catalog = makeCatalog();
    GridPartDocument document("map001", 5, 5);
    REQUIRE(document.placePart(makeSpawn()));
    REQUIRE(document.placePart(
        makePart("map001:treasure.chest:3:0", "treasure.chest", GridPartCategory::TreasureChest, 3, 0)));

    auto ruleset = MakeDefaultGridRulesetProfile(GridPartRuleset::DungeonRoomBuilder);
    MapObjective objective;
    objective.type = MapObjectiveType::OpenChest;
    objective.objective_id = "open_chest";
    objective.target_instance_id = "map001:treasure.chest:3:0";

    const auto report = ValidateReachability(document, catalog, ruleset, objective);

    REQUIRE_FALSE(report.ok);
    REQUIRE(report.diagnostics.size() == 1);
    REQUIRE(report.diagnostics[0].code == "ruleset_requires_exit");
}

TEST_CASE("Grid part reachability reports unreachable locked-door keys and key cycles", "[grid_part][reachability]") {
    auto catalog = makeCatalog();
    GridPartDocument document("map001", 6, 5);
    REQUIRE(document.placePart(makeSpawn(0, 0)));
    REQUIRE(document.placePart(makePart("map001:wall.stone:1:0", "wall.stone", GridPartCategory::Wall, 1, 0, 1, 5)));

    auto key = makePart("map001:quest.key:3:0", "quest.key", GridPartCategory::QuestItem, 3, 0);
    REQUIRE(document.placePart(key));

    auto door = makePart("map001:door.locked:4:0", "door.locked", GridPartCategory::Door, 4, 0);
    door.properties["locked"] = "true";
    door.properties["keyInstanceId"] = "map001:quest.key:3:0";
    REQUIRE(document.placePart(door));

    auto cyclicDoor = makePart("map001:door.locked:5:0", "door.locked", GridPartCategory::Door, 5, 0);
    cyclicDoor.properties["locked"] = "true";
    cyclicDoor.properties["keyInstanceId"] = "map001:door.locked:5:0";
    REQUIRE(document.placePart(cyclicDoor));

    auto ruleset = MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG);
    ruleset.requires_player_spawn = true;
    MapObjective objective;
    objective.type = MapObjectiveType::CollectQuestItem;
    objective.objective_id = "collect_key";
    objective.target_instance_id = "map001:quest.key:3:0";

    const auto report = ValidateReachability(document, catalog, ruleset, objective);

    REQUIRE_FALSE(report.ok);
    REQUIRE(report.diagnostics.size() == 3);
    REQUIRE(report.diagnostics[0].code == "door_key_cycle");
    REQUIRE(report.diagnostics[1].code == "objective_unreachable");
    REQUIRE(report.diagnostics[2].code == "required_key_unreachable");
}
