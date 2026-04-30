#include "engine/core/map/grid_part_commands.h"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <utility>

using namespace urpg::map;

namespace {

PlacedPartInstance makePart(std::string instanceId, std::string partId = "prop.crate", int32_t x = 1, int32_t y = 2) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.grid_x = x;
    part.grid_y = y;
    return part;
}

} // namespace

TEST_CASE("Grid part place command applies and undoes placement", "[grid_part][commands]") {
    GridPartDocument document("map001", 8, 6);
    PlacePartCommand command(makePart("map001:prop.crate:1:2"));

    REQUIRE(command.apply(document));
    REQUIRE(document.findPart("map001:prop.crate:1:2") != nullptr);
    REQUIRE(command.label() == "Place Part");

    REQUIRE(command.undo(document));
    REQUIRE(document.findPart("map001:prop.crate:1:2") == nullptr);
}

TEST_CASE("Grid part remove command restores removed part on undo", "[grid_part][commands]") {
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:2")));

    RemovePartCommand command("map001:prop.crate:1:2");
    REQUIRE(command.apply(document));
    REQUIRE(document.parts().empty());

    REQUIRE(command.undo(document));
    const auto* restored = document.findPart("map001:prop.crate:1:2");
    REQUIRE(restored != nullptr);
    REQUIRE(restored->part_id == "prop.crate");
}

TEST_CASE("Grid part move and resize commands restore previous geometry", "[grid_part][commands]") {
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:zone.cutscene:1:1", "zone.cutscene", 1, 1)));

    MovePartCommand move("map001:zone.cutscene:1:1", 4, 3, 1);
    REQUIRE(move.apply(document));
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->grid_x == 4);
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->grid_y == 3);
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->grid_z == 1);

    REQUIRE(move.undo(document));
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->grid_x == 1);
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->grid_y == 1);
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->grid_z == 0);

    ResizePartCommand resize("map001:zone.cutscene:1:1", 3, 2);
    REQUIRE(resize.apply(document));
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->width == 3);
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->height == 2);

    REQUIRE(resize.undo(document));
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->width == 1);
    REQUIRE(document.findPart("map001:zone.cutscene:1:1")->height == 1);
}

TEST_CASE("Grid part replace command swaps complete instance data", "[grid_part][commands]") {
    GridPartDocument document("map001", 8, 6);
    REQUIRE(document.placePart(makePart("map001:prop.crate:1:2")));

    auto replacement = makePart("map001:prop.crate:1:2", "prop.barrel", 5, 4);
    replacement.properties["lootTable"] = "barrel_loot";

    ReplacePartCommand command(replacement);
    REQUIRE(command.apply(document));
    REQUIRE(document.findPart("map001:prop.crate:1:2")->part_id == "prop.barrel");
    REQUIRE(document.findPart("map001:prop.crate:1:2")->properties.at("lootTable") == "barrel_loot");

    REQUIRE(command.undo(document));
    REQUIRE(document.findPart("map001:prop.crate:1:2")->part_id == "prop.crate");
    REQUIRE(document.findPart("map001:prop.crate:1:2")->properties.empty());
}

TEST_CASE("Grid part property command changes, removes, and restores properties", "[grid_part][commands]") {
    GridPartDocument document("map001", 8, 6);
    auto part = makePart("map001:door.locked:1:2", "door.locked");
    part.properties["locked"] = "true";
    REQUIRE(document.placePart(part));

    ChangePartPropertyCommand unlock("map001:door.locked:1:2", "locked", "false");
    REQUIRE(unlock.apply(document));
    REQUIRE(document.findPart("map001:door.locked:1:2")->properties.at("locked") == "false");
    REQUIRE(unlock.undo(document));
    REQUIRE(document.findPart("map001:door.locked:1:2")->properties.at("locked") == "true");

    ChangePartPropertyCommand remove("map001:door.locked:1:2", "locked", std::nullopt);
    REQUIRE(remove.apply(document));
    REQUIRE_FALSE(document.findPart("map001:door.locked:1:2")->properties.contains("locked"));
    REQUIRE(remove.undo(document));
    REQUIRE(document.findPart("map001:door.locked:1:2")->properties.at("locked") == "true");
}

TEST_CASE("Bulk grid part command applies atomically and rolls back failures", "[grid_part][commands]") {
    GridPartDocument document("map001", 8, 6);

    std::vector<std::unique_ptr<IGridPartCommand>> commands;
    commands.push_back(std::make_unique<PlacePartCommand>(makePart("map001:prop.crate:1:2")));
    commands.push_back(std::make_unique<PlacePartCommand>(makePart("map001:prop.crate:1:2")));

    BulkGridPartCommand bulk(std::move(commands), "Place Duplicate Parts");
    REQUIRE_FALSE(bulk.apply(document));
    REQUIRE(document.parts().empty());
    REQUIRE_FALSE(bulk.undo(document));
}

TEST_CASE("Bulk grid part command undoes successful commands in reverse order", "[grid_part][commands]") {
    GridPartDocument document("map001", 8, 6);

    std::vector<std::unique_ptr<IGridPartCommand>> commands;
    commands.push_back(std::make_unique<PlacePartCommand>(makePart("map001:prop.crate:1:2")));
    commands.push_back(std::make_unique<PlacePartCommand>(makePart("map001:prop.barrel:2:2", "prop.barrel", 2, 2)));

    BulkGridPartCommand bulk(std::move(commands), "Place Parts");
    REQUIRE(bulk.apply(document));
    REQUIRE(document.parts().size() == 2);

    REQUIRE(bulk.undo(document));
    REQUIRE(document.parts().empty());
}

TEST_CASE("Grid part command history tracks undo and redo stacks", "[grid_part][commands]") {
    GridPartDocument document("map001", 8, 6);
    GridPartCommandHistory history;

    REQUIRE_FALSE(history.canUndo());
    REQUIRE_FALSE(history.canRedo());

    REQUIRE(history.execute(document, std::make_unique<PlacePartCommand>(makePart("map001:prop.crate:1:2"))));
    REQUIRE(history.canUndo());
    REQUIRE_FALSE(history.canRedo());
    REQUIRE(history.undoLabel() == "Place Part");

    REQUIRE(history.undo(document));
    REQUIRE(document.parts().empty());
    REQUIRE_FALSE(history.canUndo());
    REQUIRE(history.canRedo());

    REQUIRE(history.redo(document));
    REQUIRE(document.findPart("map001:prop.crate:1:2") != nullptr);
    REQUIRE(history.canUndo());
    REQUIRE_FALSE(history.canRedo());
}

TEST_CASE("Grid part commands reject locked destructive edits", "[grid_part][commands]") {
    GridPartDocument document("map001", 8, 6);
    auto locked = makePart("map001:door.locked:1:2", "door.locked");
    locked.locked = true;
    locked.properties["locked"] = "true";
    REQUIRE(document.placePart(locked));

    RemovePartCommand remove("map001:door.locked:1:2");
    REQUIRE_FALSE(remove.apply(document));
    REQUIRE(document.findPart("map001:door.locked:1:2") != nullptr);

    ResizePartCommand resize("map001:door.locked:1:2", 2, 1);
    REQUIRE_FALSE(resize.apply(document));
    REQUIRE(document.findPart("map001:door.locked:1:2")->width == 1);

    auto replacement = makePart("map001:door.locked:1:2", "door.open");
    ReplacePartCommand replace(replacement);
    REQUIRE_FALSE(replace.apply(document));
    REQUIRE(document.findPart("map001:door.locked:1:2")->part_id == "door.locked");

    ChangePartPropertyCommand change("map001:door.locked:1:2", "locked", "false");
    REQUIRE_FALSE(change.apply(document));
    REQUIRE(document.findPart("map001:door.locked:1:2")->properties.at("locked") == "true");
}
