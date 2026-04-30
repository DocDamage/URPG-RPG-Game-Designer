#include "engine/core/map/grid_part_document.h"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <utility>

using namespace urpg::map;

namespace {

PlacedPartInstance makePart(std::string instanceId, std::string partId = "prop.crate", int32_t x = 1, int32_t y = 2,
                            GridPartLayer layer = GridPartLayer::Object) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.layer = layer;
    part.grid_x = x;
    part.grid_y = y;
    return part;
}

} // namespace

TEST_CASE("GridPartDocument places unique parts", "[grid_part][document]") {
    GridPartDocument document("map001", 8, 6);
    auto part = makePart("map001:prop.crate:1:2");

    REQUIRE(document.placePart(part));

    REQUIRE(document.mapId() == "map001");
    REQUIRE(document.width() == 8);
    REQUIRE(document.height() == 6);
    REQUIRE(document.parts().size() == 1);
    REQUIRE(document.findPart("map001:prop.crate:1:2") != nullptr);
}

TEST_CASE("GridPartDocument rejects invalid placement identity", "[grid_part][document]") {
    GridPartDocument document("map001", 8, 6);

    auto missingInstance = makePart("");
    REQUIRE_FALSE(document.placePart(missingInstance));

    auto missingPart = makePart("map001:missing:1:2", "");
    REQUIRE_FALSE(document.placePart(missingPart));

    auto part = makePart("map001:prop.crate:1:2");
    REQUIRE(document.placePart(part));
    REQUIRE_FALSE(document.placePart(part));
    REQUIRE(document.parts().size() == 1);
}

TEST_CASE("GridPartDocument rejects out-of-bounds footprints", "[grid_part][document]") {
    GridPartDocument document("map001", 4, 4);

    auto negative = makePart("map001:prop.crate:-1:0", "prop.crate", -1, 0);
    REQUIRE_FALSE(document.placePart(negative));

    auto tooWide = makePart("map001:prop.banner:3:1", "prop.banner", 3, 1);
    tooWide.width = 2;
    REQUIRE_FALSE(document.placePart(tooWide));

    auto validLarge = makePart("map001:zone.cutscene:2:2", "zone.cutscene", 2, 2);
    validLarge.width = 2;
    validLarge.height = 2;
    REQUIRE(document.footprintInBounds(validLarge));
    REQUIRE(document.placePart(validLarge));
}

TEST_CASE("GridPartDocument moves and rejects locked moves", "[grid_part][document]") {
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:2")));

    REQUIRE(document.movePart("map001:prop.crate:1:2", 3, 4));
    const auto* moved = document.findPart("map001:prop.crate:1:2");
    REQUIRE(moved != nullptr);
    REQUIRE(moved->grid_x == 3);
    REQUIRE(moved->grid_y == 4);

    auto locked = makePart("map001:door.locked:2:2", "door.locked", 2, 2);
    locked.locked = true;
    REQUIRE(document.placePart(locked));
    REQUIRE_FALSE(document.movePart("map001:door.locked:2:2", 4, 4));
    const auto* unchanged = document.findPart("map001:door.locked:2:2");
    REQUIRE(unchanged != nullptr);
    REQUIRE(unchanged->grid_x == 2);
    REQUIRE(unchanged->grid_y == 2);
}

TEST_CASE("GridPartDocument resizes and rejects invalid resize", "[grid_part][document]") {
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:zone.cutscene:1:1", "zone.cutscene", 1, 1)));

    REQUIRE(document.resizePart("map001:zone.cutscene:1:1", 3, 2));
    const auto* resized = document.findPart("map001:zone.cutscene:1:1");
    REQUIRE(resized != nullptr);
    REQUIRE(resized->width == 3);
    REQUIRE(resized->height == 2);

    REQUIRE_FALSE(document.resizePart("map001:zone.cutscene:1:1", 0, 2));
    REQUIRE_FALSE(document.resizePart("map001:zone.cutscene:1:1", 8, 6));
    REQUIRE(resized->width == 3);
    REQUIRE(resized->height == 2);
}

TEST_CASE("GridPartDocument finds parts by coordinate and layer", "[grid_part][document]") {
    GridPartDocument document("map001", 8, 6);
    auto wall = makePart("map001:wall.stone:2:2", "wall.stone", 2, 2, GridPartLayer::Collision);
    wall.width = 2;
    REQUIRE(document.placePart(wall));
    REQUIRE(document.placePart(makePart("map001:prop.crate:3:2", "prop.crate", 3, 2, GridPartLayer::Object)));

    const auto partsAt = document.partsAt(3, 2);
    REQUIRE(partsAt.size() == 2);

    const auto collisionParts = document.partsAtLayer(3, 2, GridPartLayer::Collision);
    REQUIRE(collisionParts.size() == 1);
    REQUIRE(collisionParts.front()->instance_id == "map001:wall.stone:2:2");
}

TEST_CASE("GridPartDocument removes parts", "[grid_part][document]") {
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:2")));

    REQUIRE(document.removePart("map001:prop.crate:1:2"));

    REQUIRE(document.parts().empty());
    REQUIRE(document.findPart("map001:prop.crate:1:2") == nullptr);
    REQUIRE_FALSE(document.removePart("map001:prop.crate:1:2"));
}

TEST_CASE("GridPartDocument rejects destructive edits to locked parts", "[grid_part][document]") {
    GridPartDocument document("map001", 8, 6);
    auto locked = makePart("map001:door.locked:1:2", "door.locked", 1, 2);
    locked.locked = true;
    locked.properties["locked"] = "true";
    REQUIRE(document.placePart(locked));

    REQUIRE_FALSE(document.resizePart("map001:door.locked:1:2", 2, 1));
    REQUIRE(document.findPart("map001:door.locked:1:2")->width == 1);

    auto replacement = makePart("map001:door.locked:1:2", "door.open", 3, 3);
    REQUIRE_FALSE(document.replacePart(replacement));
    REQUIRE(document.findPart("map001:door.locked:1:2")->part_id == "door.locked");
    REQUIRE(document.findPart("map001:door.locked:1:2")->grid_x == 1);

    REQUIRE_FALSE(document.removePart("map001:door.locked:1:2"));
    REQUIRE(document.findPart("map001:door.locked:1:2") != nullptr);
}

TEST_CASE("GridPartDocument tracks dirty chunks for placement and edits", "[grid_part][document][chunks]") {
    GridPartDocument document("map001", 8, 8, 4);
    auto wide = makePart("map001:wall.stone:3:1", "wall.stone", 3, 1, GridPartLayer::Collision);
    wide.width = 3;

    REQUIRE(document.placePart(wide));

    REQUIRE(document.chunks().size() == 2);
    REQUIRE(document.dirtyChunks().size() == 2);
    REQUIRE(document.chunks()[0].coord.chunk_x == 0);
    REQUIRE(document.chunks()[0].coord.chunk_y == 0);
    REQUIRE(document.chunks()[0].instance_ids.size() == 1);
    REQUIRE(document.chunks()[1].coord.chunk_x == 1);
    REQUIRE(document.chunks()[1].coord.chunk_y == 0);

    document.clearDirtyChunks();
    REQUIRE(document.dirtyChunks().empty());

    REQUIRE(document.movePart("map001:wall.stone:3:1", 4, 5));
    const auto dirty = document.dirtyChunks();
    REQUIRE(dirty.size() == 3);
    REQUIRE(dirty[0].coord.chunk_x == 0);
    REQUIRE(dirty[0].coord.chunk_y == 0);
    REQUIRE(dirty[1].coord.chunk_x == 1);
    REQUIRE(dirty[1].coord.chunk_y == 0);
    REQUIRE(dirty[2].coord.chunk_x == 1);
    REQUIRE(dirty[2].coord.chunk_y == 1);
}

TEST_CASE("GridPartDocument keeps deleted chunks dirty until compile can consume them",
          "[grid_part][document][chunks]") {
    GridPartDocument document("map001", 8, 8, 4);
    REQUIRE(document.placePart(makePart("map001:prop.crate:5:5", "prop.crate", 5, 5)));
    document.clearDirtyChunks();

    REQUIRE(document.removePart("map001:prop.crate:5:5"));

    const auto dirty = document.dirtyChunks();
    REQUIRE(dirty.size() == 1);
    REQUIRE(dirty[0].coord.chunk_x == 1);
    REQUIRE(dirty[0].coord.chunk_y == 1);
    REQUIRE(dirty[0].instance_ids.empty());
}

TEST_CASE("GridPartDocument prunes empty chunks after dirty chunks are cleared",
          "[grid_part][document][chunks]") {
    GridPartDocument document("map001", 8, 8, 4);
    REQUIRE(document.placePart(makePart("map001:prop.crate:5:5", "prop.crate", 5, 5)));
    document.clearDirtyChunks();
    REQUIRE(document.chunks().size() == 1);

    REQUIRE(document.removePart("map001:prop.crate:5:5"));
    REQUIRE(document.dirtyChunks().size() == 1);

    document.clearDirtyChunks();

    REQUIRE(document.chunks().empty());
    REQUIRE(document.dirtyChunks().empty());
}
