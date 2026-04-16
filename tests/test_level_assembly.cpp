#include <catch2/catch_test_macros.hpp>
#include "level/level_assembly.h"
#include "level/level_block_importer.h"
#include <fstream>

using namespace urpg::level;

TEST_CASE("Snap logic validates directional opposites", "[level]") {
    SnapConnector n = {"Corridor", ConnectorSide::North, 0,0,0};
    SnapConnector s = {"Corridor", ConnectorSide::South, 0,0,0};
    SnapConnector e = {"Corridor", ConnectorSide::East, 0,0,0};
    SnapConnector w = {"Corridor", ConnectorSide::West, 0,0,0};
    SnapConnector wall = {"Wall", ConnectorSide::South, 0,0,0};

    SECTION("Opposites match") {
        REQUIRE(SnapLogic::canSnap(n, s));
        REQUIRE(SnapLogic::canSnap(s, n));
        REQUIRE(SnapLogic::canSnap(e, w));
        REQUIRE(SnapLogic::canSnap(w, e));
    }

    SECTION("Non-opposites fail") {
        REQUIRE_FALSE(SnapLogic::canSnap(n, e));
        REQUIRE_FALSE(SnapLogic::canSnap(n, n));
    }

    SECTION("Type mismatch fails") {
        REQUIRE_FALSE(SnapLogic::canSnap(s, wall));
    }
}

TEST_CASE("LevelBlockImporter loads JSON library", "[level]") {
    const char* testFile = "test_level_lib.json";
    {
        std::ofstream out(testFile);
        out << R"({
          "libraryName": "Test Lib",
          "blocks": [
            {
              "id": "b1",
              "prefabPath": "p1",
              "connectors": [
                { "side": "North", "type": "T1", "offset": [1, 2, 3] }
              ]
            }
          ]
        })";
    }

    auto blocks = LevelBlockImporter::importLibrary(testFile);
    REQUIRE(blocks.size() == 1);
    REQUIRE(blocks[0].getId() == "b1");
    REQUIRE(blocks[0].getConnectors().size() == 1);
    REQUIRE(blocks[0].getConnectors()[0].side == ConnectorSide::North);
    REQUIRE(blocks[0].getConnectors()[0].type == "T1");
    REQUIRE(blocks[0].getConnectors()[0].localX == 1.0f);
    
    std::remove(testFile);
}