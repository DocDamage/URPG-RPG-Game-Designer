#include "editor/spatial/map_authoring_context.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MapAuthoringContext preserves document ownership through shared history", "[spatial][map_authoring]") {
    urpg::editor::MapAuthoringContext context;
    context.setProjectRoot("C:/projects/creator-demo");
    context.setActiveMapId("map_intro");
    context.setSelection({"ground", "prop.tree", "event.guard", "part.bridge", "canvas", "paint"});
    context.setValidation({3, 1, "Resolve the blocked event before playtest."});
    context.setPlaytestState("ready");
    context.setPackageState("needs_validation");
    context.recordCommand({"paint_tiles", urpg::editor::MapAuthoringDocumentOwner::Perspective2D, false, true});
    context.recordCommand({"place_bridge", urpg::editor::MapAuthoringDocumentOwner::GridParts, true, false});
    context.recordCommand({"bridge_selection", urpg::editor::MapAuthoringDocumentOwner::GridParts, true, true});

    const auto& snapshot = context.snapshot();
    REQUIRE(snapshot.projectRoot == "C:/projects/creator-demo");
    REQUIRE(snapshot.activeMapId == "map_intro");
    REQUIRE(snapshot.selection.eventId == "event.guard");
    REQUIRE(snapshot.gridPartsDirty);
    REQUIRE(snapshot.perspective2DDirty);
    REQUIRE(snapshot.canUndo);
    REQUIRE(snapshot.historyOwner == "grid_parts");

    urpg::editor::MapAuthoringHistoryEntry restored;
    REQUIRE(context.undo(&restored));
    REQUIRE(restored.commandId == "bridge_selection");
    REQUIRE(restored.affectsGridParts);
    REQUIRE(restored.affectsPerspective2D);
    REQUIRE(context.snapshot().canRedo);
    REQUIRE(context.redo(&restored));
    REQUIRE(restored.commandId == "bridge_selection");
    context.markSaved(urpg::editor::MapAuthoringDocumentOwner::GridParts);
    REQUIRE_FALSE(context.snapshot().gridPartsDirty);
    REQUIRE(context.snapshot().perspective2DDirty);

    context.setDocumentDirty(urpg::editor::MapAuthoringDocumentOwner::Perspective2D, false);
    REQUIRE_FALSE(context.snapshot().perspective2DDirty);
}
