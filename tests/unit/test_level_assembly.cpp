#include "engine/core/level/level_assembly.h"
#include "engine/core/level/level_block_importer.h"
#include <catch2/catch_test_macros.hpp>
#include <fstream>

using namespace urpg::level;

TEST_CASE("LevelAssembly: Snap Logic", "[level][assembly]") {
    SnapConnector hallNorth = {"Hall", ConnectorSide::North, 0, 0, 0};
    SnapConnector hallSouth = {"Hall", ConnectorSide::South, 0, 0, 0};
    SnapConnector wallNorth = {"Wall", ConnectorSide::North, 0, 0, 0};

    SECTION("Opposite sides of same type can snap") {
        REQUIRE(SnapLogic::canSnap(hallNorth, hallSouth));
    }

    SECTION("Same sides cannot snap") {
        REQUIRE_FALSE(SnapLogic::canSnap(hallNorth, hallNorth));
    }

    SECTION("Different types cannot snap") {
        REQUIRE_FALSE(SnapLogic::canSnap(hallNorth, wallNorth));
    }
}

TEST_CASE("LevelAssembly: Workspace Placement", "[level][assembly]") {
    LevelAssemblyWorkspace workspace;

    SECTION("Placing a block in empty space succeeds") {
        REQUIRE(workspace.placeBlock("Room_A", 0, 0));
        REQUIRE(workspace.hasBlockAt(0, 0));
    }

    SECTION("Placing a block in occupied space fails") {
        workspace.placeBlock("Room_A", 5, 5);
        REQUIRE_FALSE(workspace.placeBlock("Room_B", 5, 5));
        REQUIRE(workspace.getPlacedBlocks().size() == 1);
    }

    SECTION("Placing blocks at different Z levels is allowed") {
        REQUIRE(workspace.placeBlock("Floor_1", 0, 0, 0));
        REQUIRE(workspace.placeBlock("Floor_2", 0, 0, 1));
        REQUIRE(workspace.hasBlockAt(0, 0, 0));
        REQUIRE(workspace.hasBlockAt(0, 0, 1));
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
