#include "editor/spatial/grid_part_playtest_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <utility>

using namespace urpg::editor;
using namespace urpg::map;

namespace {

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category = GridPartCategory::Prop,
                                  bool blocksNavigation = false) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.description = "Playtest pipeline test part";
    definition.category = category;
    definition.default_layer = category == GridPartCategory::Tile ? GridPartLayer::Terrain : GridPartLayer::Object;
    definition.collision_policy = blocksNavigation ? GridPartCollisionPolicy::Solid : GridPartCollisionPolicy::None;
    definition.footprint.blocks_navigation = blocksNavigation;
    definition.asset_id = definition.part_id + ".asset";
    definition.tile_id = category == GridPartCategory::Tile ? 7 : 0;
    definition.supported_rulesets = {GridPartRuleset::TopDownJRPG, GridPartRuleset::DungeonRoomBuilder};
    return definition;
}

PlacedPartInstance makePart(std::string instanceId, std::string partId, GridPartCategory category, int32_t x, int32_t y,
                            int32_t width = 1, int32_t height = 1) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.category = category;
    part.layer = category == GridPartCategory::Tile ? GridPartLayer::Terrain : GridPartLayer::Object;
    part.grid_x = x;
    part.grid_y = y;
    part.width = width;
    part.height = height;
    return part;
}

GridPartCatalog makeCatalog() {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("tile.grass", GridPartCategory::Tile)));
    REQUIRE(catalog.addDefinition(makeDefinition("trigger.spawn", GridPartCategory::Trigger)));
    REQUIRE(catalog.addDefinition(makeDefinition("trigger.exit", GridPartCategory::Trigger)));
    REQUIRE(catalog.addDefinition(makeDefinition("treasure.chest", GridPartCategory::TreasureChest)));
    REQUIRE(catalog.addDefinition(makeDefinition("wall.stone", GridPartCategory::Wall, true)));
    return catalog;
}

PlacedPartInstance makeSpawn(int32_t x = 0, int32_t y = 0) {
    auto spawn = makePart("map001:trigger.spawn:0:0", "trigger.spawn", GridPartCategory::Trigger, x, y);
    spawn.properties["role"] = "player_spawn";
    return spawn;
}

MapObjective makeChestObjective() {
    MapObjective objective;
    objective.type = MapObjectiveType::OpenChest;
    objective.objective_id = "open_chest";
    objective.target_instance_id = "map001:treasure.chest:3:0";
    return objective;
}

} // namespace

TEST_CASE("Grid part playtest pipeline validates compiles and records a successful run", "[grid_part][playtest]") {
    const auto catalog = makeCatalog();
    GridPartDocument document("map001", 5, 5);
    REQUIRE(document.placePart(makeSpawn()));
    REQUIRE(document.placePart(
        makePart("map001:treasure.chest:3:0", "treasure.chest", GridPartCategory::TreasureChest, 3, 0)));

    auto ruleset = MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG);
    auto objective = makeChestObjective();

    GridPartPlaytestPanel panel;
    panel.SetTargets(&document, &catalog);
    panel.SetRulesetProfile(ruleset);
    panel.SetObjective(objective);

    REQUIRE(panel.PlaytestFromStart());

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.has_document);
    REQUIRE(snapshot.has_catalog);
    REQUIRE(snapshot.running);
    REQUIRE(snapshot.has_runtime);
    REQUIRE(snapshot.latest_result.map_id == "map001");
    REQUIRE(snapshot.latest_result.completed_objective);
    REQUIRE_FALSE(snapshot.latest_result.softlocked);
    REQUIRE(snapshot.latest_result.start_x == 0);
    REQUIRE(snapshot.latest_result.start_y == 0);
    REQUIRE(snapshot.latest_result.end_x == 3);
    REQUIRE(snapshot.latest_result.end_y == 0);
    REQUIRE(snapshot.latest_result.visited_instance_ids.size() == 2);
}

TEST_CASE("Grid part playtest pipeline refuses invalid publish blockers", "[grid_part][playtest]") {
    const auto catalog = makeCatalog();
    GridPartDocument document("map001", 5, 5);
    REQUIRE(document.placePart(makePart("map001:missing:1:1", "missing.part", GridPartCategory::Prop, 1, 1)));

    GridPartPlaytestPanel panel;
    panel.SetTargets(&document, &catalog);
    panel.SetRulesetProfile(MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG));
    panel.SetObjective(makeChestObjective());

    REQUIRE_FALSE(panel.PlaytestFromStart());

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE_FALSE(snapshot.running);
    REQUIRE_FALSE(snapshot.can_launch);
    REQUIRE(snapshot.latest_result.softlocked);
    REQUIRE(snapshot.latest_result.diagnostics.size() >= 3);
}

TEST_CASE("Grid part playtest pipeline supports playtest from here", "[grid_part][playtest]") {
    const auto catalog = makeCatalog();
    GridPartDocument document("map001", 5, 5);
    REQUIRE(document.placePart(makeSpawn()));
    REQUIRE(document.placePart(
        makePart("map001:treasure.chest:3:0", "treasure.chest", GridPartCategory::TreasureChest, 3, 0)));

    GridPartPlaytestPanel panel;
    panel.SetTargets(&document, &catalog);
    panel.SetRulesetProfile(MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG));
    panel.SetObjective(makeChestObjective());

    REQUIRE(panel.PlaytestFromHere(2, 0));

    const auto& result = panel.lastRenderSnapshot().latest_result;
    REQUIRE(result.start_x == 2);
    REQUIRE(result.start_y == 0);
    REQUIRE(result.end_x == 3);
    REQUIRE(result.end_y == 0);
    REQUIRE(result.completed_objective);
}

TEST_CASE("Grid part playtest pipeline reports objective path softlocks", "[grid_part][playtest]") {
    const auto catalog = makeCatalog();
    GridPartDocument document("map001", 5, 5);
    REQUIRE(document.placePart(makeSpawn()));
    REQUIRE(document.placePart(makePart("map001:wall.stone:1:0", "wall.stone", GridPartCategory::Wall, 1, 0, 1, 5)));
    REQUIRE(document.placePart(
        makePart("map001:treasure.chest:3:0", "treasure.chest", GridPartCategory::TreasureChest, 3, 0)));

    GridPartPlaytestPanel panel;
    panel.SetTargets(&document, &catalog);
    panel.SetRulesetProfile(MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG));
    panel.SetObjective(makeChestObjective());

    REQUIRE_FALSE(panel.PlaytestObjectivePath());

    const auto& result = panel.lastRenderSnapshot().latest_result;
    REQUIRE(result.softlocked);
    REQUIRE_FALSE(result.completed_objective);
    const auto unreachable =
        std::find_if(result.diagnostics.begin(), result.diagnostics.end(),
                     [](const auto& diagnostic) { return diagnostic.code == "objective_unreachable"; });
    REQUIRE(unreachable != result.diagnostics.end());
}

TEST_CASE("Grid part playtest pipeline returns to editor without discarding last result", "[grid_part][playtest]") {
    const auto catalog = makeCatalog();
    GridPartDocument document("map001", 5, 5);
    REQUIRE(document.placePart(makeSpawn()));
    REQUIRE(document.placePart(
        makePart("map001:treasure.chest:3:0", "treasure.chest", GridPartCategory::TreasureChest, 3, 0)));

    GridPartPlaytestPanel panel;
    panel.SetTargets(&document, &catalog);
    panel.SetRulesetProfile(MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG));
    panel.SetObjective(makeChestObjective());

    REQUIRE(panel.PlaytestFromStart());
    REQUIRE(panel.ReturnToEditor());

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE_FALSE(snapshot.running);
    REQUIRE(snapshot.returned_to_editor);
    REQUIRE(snapshot.latest_result.completed_objective);
}
