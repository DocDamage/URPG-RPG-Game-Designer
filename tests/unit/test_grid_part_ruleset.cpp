#include "engine/core/map/grid_part_ruleset.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>
#include <vector>

using namespace urpg::map;

namespace {

GridPartDefinition makeDefinition(std::string partId, GridPartCategory category = GridPartCategory::Prop,
                                  std::vector<GridPartRuleset> rulesets = {GridPartRuleset::TopDownJRPG}) {
    GridPartDefinition definition;
    definition.part_id = std::move(partId);
    definition.display_name = definition.part_id;
    definition.description = "Ruleset test part";
    definition.category = category;
    definition.default_layer =
        category == GridPartCategory::Platform ? GridPartLayer::Collision : GridPartLayer::Object;
    definition.supported_rulesets = std::move(rulesets);
    return definition;
}

PlacedPartInstance makePart(std::string instanceId, std::string partId, GridPartCategory category, int32_t x = 1,
                            int32_t y = 1) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.category = category;
    part.layer = category == GridPartCategory::Platform ? GridPartLayer::Collision : GridPartLayer::Object;
    part.grid_x = x;
    part.grid_y = y;
    return part;
}

} // namespace

TEST_CASE("Grid ruleset default profiles expose deterministic authoring constraints", "[grid_part][ruleset]") {
    const auto jrpg = MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG);
    REQUIRE(jrpg.id == "top_down_jrpg");
    REQUIRE(jrpg.requires_player_spawn);
    REQUIRE_FALSE(jrpg.requires_exit);
    REQUIRE_FALSE(jrpg.allows_platforms);
    REQUIRE(jrpg.default_tile_size == 48);

    const auto sideScroller = MakeDefaultGridRulesetProfile(GridPartRuleset::SideScrollerAction);
    REQUIRE(sideScroller.id == "side_scroller_action");
    REQUIRE(sideScroller.allows_gravity);
    REQUIRE(sideScroller.allows_platforms);
    REQUIRE(sideScroller.requires_exit);
}

TEST_CASE("Grid ruleset validation reports missing spawn and exit requirements", "[grid_part][ruleset]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("prop.crate")));
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:1", "prop.crate", GridPartCategory::Prop)));

    auto profile = MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG);
    profile.requires_exit = true;

    const auto result = ValidateGridPartRuleset(document, catalog, profile);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.hasErrors());
    REQUIRE(result.diagnostics.size() == 2);
    REQUIRE(result.diagnostics[0].code == "ruleset_requires_exit");
    REQUIRE(result.diagnostics[1].code == "ruleset_requires_spawn");
}

TEST_CASE("Grid ruleset validation accepts spawn and exit signals from part properties", "[grid_part][ruleset]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(makeDefinition("trigger.spawn", GridPartCategory::Trigger)));
    REQUIRE(catalog.addDefinition(makeDefinition("door.exit", GridPartCategory::Door)));
    GridPartDocument document("map001", 8, 6);

    auto spawn = makePart("map001:trigger.spawn:1:1", "trigger.spawn", GridPartCategory::Trigger);
    spawn.properties["role"] = "player_spawn";
    REQUIRE(document.placePart(spawn));

    auto exit = makePart("map001:door.exit:6:4", "door.exit", GridPartCategory::Door, 6, 4);
    exit.properties["role"] = "exit";
    REQUIRE(document.placePart(exit));

    auto profile = MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG);
    profile.requires_exit = true;

    const auto result = ValidateGridPartRuleset(document, catalog, profile);
    REQUIRE(result.ok);
    REQUIRE(result.diagnostics.empty());
}

TEST_CASE("Grid ruleset validation reports profile size and incompatible part constraints", "[grid_part][ruleset]") {
    GridPartCatalog catalog;
    REQUIRE(catalog.addDefinition(
        makeDefinition("platform.grass", GridPartCategory::Platform, {GridPartRuleset::SideScrollerAction})));
    GridPartDocument document("map001", 12, 8);
    auto platform = makePart("map001:platform.grass:1:1", "platform.grass", GridPartCategory::Platform);
    platform.properties["gravity"] = "true";
    REQUIRE(document.placePart(platform));

    auto profile = MakeDefaultGridRulesetProfile(GridPartRuleset::TopDownJRPG);
    profile.requires_player_spawn = false;
    profile.max_width = 8;
    profile.max_height = 6;

    const auto result = ValidateGridPartRuleset(document, catalog, profile);

    REQUIRE_FALSE(result.ok);
    REQUIRE(result.diagnostics.size() == 4);
    REQUIRE(result.diagnostics[0].code == "gravity_part_not_allowed_in_ruleset");
    REQUIRE(result.diagnostics[1].code == "map_exceeds_ruleset_size");
    REQUIRE(result.diagnostics[2].code == "part_ruleset_incompatible");
    REQUIRE(result.diagnostics[3].code == "platform_not_allowed_in_ruleset");
}
