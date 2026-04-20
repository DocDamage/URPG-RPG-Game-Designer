#include "engine/core/level/procedural_toolkit.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::level;

namespace {

LevelBlock makeHallEast(const std::string& id) {
    LevelBlock block(id);
    block.addConnector({"Hall", ConnectorSide::East, 0, 0, 0});
    return block;
}

LevelBlock makeHallWest(const std::string& id) {
    LevelBlock block(id);
    block.addConnector({"Hall", ConnectorSide::West, 0, 0, 0});
    return block;
}

LevelBlock makeHallLine(const std::string& id) {
    LevelBlock block(id);
    block.addConnector({"Hall", ConnectorSide::East, 0, 0, 0});
    block.addConnector({"Hall", ConnectorSide::West, 0, 0, 0});
    return block;
}

bool layoutsMatch(const std::vector<GeneratedBlock>& lhs, const std::vector<GeneratedBlock>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (size_t index = 0; index < lhs.size(); ++index) {
        const auto& a = lhs[index];
        const auto& b = rhs[index];
        if (a.blockId != b.blockId || a.x != b.x || a.y != b.y || a.z != b.z) {
            return false;
        }
    }
    return true;
}

} // namespace

TEST_CASE("ProceduralToolkit generates deterministic seeded scenarios", "[procedural][level]") {
    const std::vector<LevelBlock> library = {
        makeHallEast("entry_east"),
        makeHallLine("corridor"),
        makeHallWest("exit_west")
    };

    SECTION("Same seed reproduces the same connected layout") {
        GenParams params;
        params.seed = 7;
        params.maxBlocks = 3;

        const auto first = ProceduralToolkit::generateDungeon(library, params);
        const auto second = ProceduralToolkit::generateDungeon(library, params);

        REQUIRE(layoutsMatch(first, second));
        REQUIRE(first.size() == 3);
        REQUIRE(first[0].x == 0);
        REQUIRE(first[0].y == 0);
        REQUIRE(first[0].z == 0);

        for (size_t index = 1; index < first.size(); ++index) {
            REQUIRE(first[index].x == static_cast<int32_t>(index));
            REQUIRE(first[index].y == 0);
            REQUIRE(first[index].z == 0);
        }
    }

    SECTION("Different seeds can select a different seeded scenario opener") {
        GenParams firstParams;
        firstParams.seed = 1;
        firstParams.maxBlocks = 3;

        GenParams secondParams;
        secondParams.seed = 2;
        secondParams.maxBlocks = 3;

        const auto first = ProceduralToolkit::generateDungeon(library, firstParams);
        const auto second = ProceduralToolkit::generateDungeon(library, secondParams);

        REQUIRE(first.size() == 3);
        REQUIRE(second.size() == 3);
        REQUIRE_FALSE(layoutsMatch(first, second));
        REQUIRE(first[0].blockId != second[0].blockId);
    }

    SECTION("Generation respects the requested block budget") {
        GenParams params;
        params.seed = 11;
        params.maxBlocks = 1;

        const auto layout = ProceduralToolkit::generateDungeon(library, params);
        REQUIRE(layout.size() == 1);
        REQUIRE(layout[0].x == 0);
        REQUIRE(layout[0].y == 0);
        REQUIRE(layout[0].z == 0);
    }
}
