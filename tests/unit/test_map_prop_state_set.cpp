#include "engine/core/map/map_prop_state_set.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("map prop states validate governed references and apply selected state", "[map][prop_state]") {
    urpg::map::MapPropStateSet states("town_gate");
    REQUIRE(states.addState({"closed", "gate_closed", {1, 1, false, true}, urpg::map::GridPartLayer::Object, "open_gate"}));
    REQUIRE(states.addState({"open", "gate_open", {1, 1, false, false}, urpg::map::GridPartLayer::Decoration, ""}));
    REQUIRE_FALSE(states.validate({"gate_closed"}, {"open_gate"}).valid);
    REQUIRE(states.validate({"gate_closed", "gate_open"}, {"open_gate"}).valid);
    REQUIRE(states.setActiveState("open"));
    urpg::map::PlacedPartInstance part;
    part.instance_id = "town:gate:1";
    REQUIRE(states.applyTo(part));
    REQUIRE(part.properties.at("propState") == "open");
    REQUIRE(part.properties.at("blocksNavigation") == "false");
}
