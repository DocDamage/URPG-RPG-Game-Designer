#include "editor/world/world_panel.h"
#include "engine/core/world/world_map_graph.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("World route availability changes with flags", "[world][simulation][ffs13]") {
    urpg::world::WorldMapGraph graph;
    graph.addNode({"town", "Town"});
    graph.addNode({"ruins", "Ruins"});
    graph.addRoute({"town", "ruins", "bridge_repaired", "airship", true});

    REQUIRE_FALSE(graph.isRouteAvailable("town", "ruins", {}, {}));
    REQUIRE_FALSE(graph.isRouteAvailable("town", "ruins", {"bridge_repaired"}, {}));
    REQUIRE(graph.isRouteAvailable("town", "ruins", {"bridge_repaired"}, {"airship"}));
    REQUIRE(graph.validate().empty());
}

TEST_CASE("World route validator reports self-unlocking routes", "[world][simulation][ffs13]") {
    urpg::world::WorldMapGraph graph;
    graph.addNode({"gate", "Gate"});
    graph.addRoute({"gate", "gate", "gate", "", false});

    const auto diagnostics = graph.validate();
    REQUIRE_FALSE(diagnostics.empty());
    REQUIRE(diagnostics.front().code == "route_unlocks_itself");
}

TEST_CASE("Fast travel preview gates destinations by unlock flag and current location", "[world][simulation][fast-travel]") {
    urpg::world::WorldMapGraph graph;
    graph.addNode({"town", "Town"});
    graph.addNode({"tower", "Tower"});
    graph.addFastTravelDestination({"tower_waypoint",
                                    "tower",
                                    7,
                                    12,
                                    18,
                                    "assets/previews/tower.png",
                                    "Wizard tower",
                                    "tower_unlocked",
                                    "common_event:locked_tower",
                                    true});

    const auto locked = graph.previewFastTravel("town", "tower_waypoint", {});
    REQUIRE_FALSE(locked.available);
    REQUIRE(locked.locked_common_event == "common_event:locked_tower");
    REQUIRE(locked.diagnostics.front().code == "fast_travel_locked");

    const auto available = graph.previewFastTravel("town", "tower_waypoint", {"tower_unlocked"});
    REQUIRE(available.available);
    REQUIRE(available.command == "transfer:7:12:18");
    REQUIRE(available.preview_asset == "assets/previews/tower.png");
    REQUIRE(urpg::editor::world::WorldPanel::fastTravelSnapshot(available) ==
            "fast_travel:available:tower_waypoint:transfer:7:12:18:assets/previews/tower.png");

    const auto current = graph.previewFastTravel("tower", "tower_waypoint", {"tower_unlocked"});
    REQUIRE_FALSE(current.available);
    REQUIRE(current.hidden_current_location);
    REQUIRE(current.diagnostics.front().code == "fast_travel_current_location_hidden");
    REQUIRE(urpg::editor::world::WorldPanel::fastTravelSnapshot(current) ==
            "fast_travel:hidden_current:tower_waypoint");
}

TEST_CASE("Vehicle preview emits movement audio and interior transfer commands", "[world][simulation][vehicle]") {
    urpg::world::WorldMapGraph graph;
    graph.addVehicleProfile({"airship",
                             "Airship",
                             {1, 2},
                             "audio/bgm/airship.ogg",
                             "audio/bgs/wind.ogg",
                             1.75,
                             0.0,
                             42,
                             5,
                             9,
                             "#ffffff",
                             30});

    const auto blocked = graph.previewVehicle("airship", 4);
    REQUIRE_FALSE(blocked.available);
    REQUIRE(blocked.diagnostics.front().code == "vehicle_terrain_blocked");

    const auto preview = graph.previewVehicle("airship", 2);
    REQUIRE(preview.available);
    REQUIRE(preview.command == "vehicle:airship:speed:1.75:encounters:0:transition:#ffffff:30");
    REQUIRE(preview.audio_command == "vehicle_audio:audio/bgm/airship.ogg:audio/bgs/wind.ogg");
    REQUIRE(preview.interior_transfer_command == "vehicle_interior:42:5:9");
    REQUIRE(urpg::editor::world::WorldPanel::vehicleSnapshot(preview) ==
            "vehicle:available:airship:vehicle:airship:speed:1.75:encounters:0:transition:#ffffff:30:"
            "vehicle_audio:audio/bgm/airship.ogg:audio/bgs/wind.ogg:vehicle_interior:42:5:9");
}
