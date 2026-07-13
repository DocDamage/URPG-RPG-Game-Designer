#include "editor/ui/editor_context_action.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("editor context actions preserve map selection and explain deferred routes", "[editor][context_action]") {
    urpg::editor::EditorContextActionStack actions;
    urpg::editor::EditorContextAction valid;
    valid.route = "event_authoring";
    valid.objectKind = "event";
    valid.objectId = "village_elder";
    valid.projectRoot = "project";
    valid.selection.objectId = "village_elder";
    valid.selection.viewportFocus = "tile:4,6";

    const auto opened = actions.open(valid);
    REQUIRE(opened.success);
    REQUIRE(actions.active()->selection.viewportFocus == "tile:4,6");
    REQUIRE(actions.returnToPrevious().success);
    REQUIRE(actions.depth() == 0);

    valid.route = "character_creator";
    const auto character = actions.open(valid);
    REQUIRE(character.success);
    REQUIRE(actions.returnToPrevious().success);

    valid.route = "battle_preview";
    const auto deferred = actions.open(valid);
    REQUIRE_FALSE(deferred.success);
    REQUIRE(deferred.code == "context_action_route_unavailable");
    REQUIRE_FALSE(deferred.remediation.empty());
}
