#include "engine/core/level/level_assembly.h"
#include "engine/core/level/level_block_importer.h"
#include <catch2/catch_test_macros.hpp>
#include <fstream>

using namespace urpg::level;

TEST_CASE("LevelAssembly: Snap Logic", "[level][assembly]") {
    SnapConnector hallNorth = {"Hall", ConnectorSide::North, 0, 0, 0};
    SnapConnector hallSouth = {"Hall", ConnectorSide::South, 0, 0, 0};
    SnapConnector wallNorth = {"Wall", ConnectorSide::North, 0, 0, 0};
    SnapConnector hallNorthOffset = {"Hall", ConnectorSide::North, 1, 0, 0};
    SnapConnector hallSouthOffsetMatch = {"Hall", ConnectorSide::South, 1, 0, 0};
    SnapConnector hallSouthOffsetMismatch = {"Hall", ConnectorSide::South, 0, 0, 0};

    SECTION("Opposite sides of same type can snap") {
        REQUIRE(SnapLogic::canSnap(hallNorth, hallSouth));
    }

    SECTION("Same sides cannot snap") {
        REQUIRE_FALSE(SnapLogic::canSnap(hallNorth, hallNorth));
    }

    SECTION("Different types cannot snap") {
        REQUIRE_FALSE(SnapLogic::canSnap(hallNorth, wallNorth));
    }

    SECTION("Matching connector metadata can snap") {
        REQUIRE(SnapLogic::canSnap(hallNorthOffset, hallSouthOffsetMatch));
    }

    SECTION("Different connector offsets cannot snap") {
        REQUIRE_FALSE(SnapLogic::canSnap(hallNorthOffset, hallSouthOffsetMismatch));
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

    SECTION("Registered blocks require matching connectors when placed against neighbors") {
        LevelBlock roomA("Room_A");
        roomA.addConnector({"Hall", ConnectorSide::East, 0, 0, 0});

        LevelBlock roomB("Room_B");
        roomB.addConnector({"Hall", ConnectorSide::West, 0, 0, 0});

        workspace.registerBlockDefinition(roomA);
        workspace.registerBlockDefinition(roomB);

        REQUIRE(workspace.placeBlock("Room_A", 0, 0, 0));

        const auto validation = workspace.validatePlacement("Room_B", 1, 0, 0);
        REQUIRE(validation.allowed);
        REQUIRE(validation.reason == "matched_connector");
        REQUIRE(validation.matching_connections == 1);

        REQUIRE(workspace.placeBlock("Room_B", 1, 0, 0));
    }

    SECTION("Registered blocks reject connector mismatches against occupied neighbors") {
        LevelBlock roomA("Room_A");
        roomA.addConnector({"Hall", ConnectorSide::East, 0, 0, 0});

        LevelBlock roomC("Room_C");
        roomC.addConnector({"Door", ConnectorSide::West, 0, 0, 0});

        workspace.registerBlockDefinition(roomA);
        workspace.registerBlockDefinition(roomC);

        REQUIRE(workspace.placeBlock("Room_A", 0, 0, 0));

        const auto validation = workspace.validatePlacement("Room_C", 1, 0, 0);
        REQUIRE_FALSE(validation.allowed);
        REQUIRE(validation.reason == "connector_mismatch");
        REQUIRE(validation.matching_connections == 0);

        REQUIRE_FALSE(workspace.placeBlock("Room_C", 1, 0, 0));
    }

    SECTION("Registered blocks reject neighbor placements when touching connector offsets do not match") {
        LevelBlock roomA("Room_A");
        roomA.addConnector({"Hall", ConnectorSide::East, 1, 0, 0});

        LevelBlock roomB("Room_B");
        roomB.addConnector({"Hall", ConnectorSide::West, 0, 0, 0});

        workspace.registerBlockDefinition(roomA);
        workspace.registerBlockDefinition(roomB);

        REQUIRE(workspace.placeBlock("Room_A", 0, 0, 0));

        const auto validation = workspace.validatePlacement("Room_B", 1, 0, 0);
        REQUIRE_FALSE(validation.allowed);
        REQUIRE(validation.reason == "connector_mismatch");
        REQUIRE(validation.matching_connections == 0);
        REQUIRE_FALSE(workspace.placeBlock("Room_B", 1, 0, 0));
    }

    SECTION("Registered blocks require a connector-backed attachment after the seed placement") {
        LevelBlock roomA("Room_A");
        roomA.addConnector({"Hall", ConnectorSide::East, 0, 0, 0});

        LevelBlock roomB("Room_B");
        roomB.addConnector({"Hall", ConnectorSide::West, 0, 0, 0});

        workspace.registerBlockDefinition(roomA);
        workspace.registerBlockDefinition(roomB);

        REQUIRE(workspace.placeBlock("Room_A", 0, 0, 0));

        const auto validation = workspace.validatePlacement("Room_B", 3, 0, 0);
        REQUIRE_FALSE(validation.allowed);
        REQUIRE(validation.reason == "detached_registered_block");
        REQUIRE(validation.matching_connections == 0);
        REQUIRE_FALSE(workspace.placeBlock("Room_B", 3, 0, 0));
    }

    SECTION("Unregistered blocks remain placeable for detached bootstrap workflows") {
        const auto validation = workspace.validatePlacement("Loose_Block", 3, 4, 0);
        REQUIRE(validation.allowed);
        REQUIRE(validation.reason == "unregistered_block");
        REQUIRE(workspace.placeBlock("Loose_Block", 3, 4, 0));
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
    REQUIRE(blocks[0].getPrefabPath() == "p1");
    REQUIRE(blocks[0].getConnectors().size() == 1);
    REQUIRE(blocks[0].getConnectors()[0].side == ConnectorSide::North);
    REQUIRE(blocks[0].getConnectors()[0].type == "T1");
    REQUIRE(blocks[0].getConnectors()[0].localX == 1.0f);

    std::remove(testFile);
}

TEST_CASE("LevelBlockLibrary builds deterministic thumbnails and registers block definitions", "[level][assembly]") {
    LevelBlock room("room_a");
    room.setPrefabPath("prefabs/room_a.scene");
    room.addConnector({"Hall", ConnectorSide::North, 0, 0, 0});
    room.addConnector({"Hall", ConnectorSide::South, 0, 0, 0});
    room.addConnector({"Hall", ConnectorSide::East, 0, 0, 0});
    room.addConnector({"Hall", ConnectorSide::West, 0, 0, 0});

    LevelBlock stair("stairs_up");
    stair.setPrefabPath("prefabs/stairs_up.scene");
    stair.addConnector({"Lift", ConnectorSide::Up, 0, 0, 0});
    stair.addConnector({"Lift", ConnectorSide::Down, 0, 0, 0});

    LevelBlockLibrary library("Dungeon Starters");
    library.addBlock(room);
    library.addBlock(stair);

    const auto thumbnails = library.buildThumbnails();
    REQUIRE(thumbnails.size() == 2);
    REQUIRE(thumbnails[0].blockId == "room_a");
    REQUIRE(thumbnails[0].prefabPath == "prefabs/room_a.scene");
    REQUIRE(thumbnails[0].connectorCount == 4);
    REQUIRE(thumbnails[0].rows.size() == 5);
    REQUIRE(thumbnails[0].rows[0][2] == 'N');
    REQUIRE(thumbnails[0].rows[2][0] == 'W');
    REQUIRE(thumbnails[0].rows[2][2] == 'R');
    REQUIRE(thumbnails[0].rows[2][4] == 'E');
    REQUIRE(thumbnails[0].rows[4][2] == 'S');
    REQUIRE(thumbnails[1].rows[1][2] == 'U');
    REQUIRE(thumbnails[1].rows[3][2] == 'D');

    LevelAssemblyWorkspace workspace;
    workspace.registerLibrary(library);
    REQUIRE(workspace.findBlockDefinition("room_a") != nullptr);
    REQUIRE(workspace.findBlockDefinition("stairs_up") != nullptr);
}

TEST_CASE("LevelBlockImporter loads library metadata and thumbnail-ready prefab paths", "[level][assembly]") {
    const char* testFile = "test_level_library_metadata.json";
    {
        std::ofstream out(testFile);
        out << R"({
          "libraryName": "Imported Library",
          "blocks": [
            {
              "id": "hub",
              "prefabPath": "prefabs/hub.scene",
              "connectors": [
                { "side": "North", "type": "Hall", "offset": [0, 0, 0] },
                { "side": "East", "type": "Hall", "offset": [0, 0, 0] }
              ]
            }
          ]
        })";
    }

    const auto library = LevelBlockImporter::importLibraryDefinition(testFile);
    REQUIRE(library.getName() == "Imported Library");
    REQUIRE(library.getBlocks().size() == 1);
    REQUIRE(library.getBlocks()[0].getId() == "hub");
    REQUIRE(library.getBlocks()[0].getPrefabPath() == "prefabs/hub.scene");

    const auto thumbnails = library.buildThumbnails();
    REQUIRE(thumbnails.size() == 1);
    REQUIRE(thumbnails[0].blockId == "hub");
    REQUIRE(thumbnails[0].rows[0][2] == 'N');
    REQUIRE(thumbnails[0].rows[2][4] == 'E');

    std::remove(testFile);
}
