#include <catch2/catch_test_macros.hpp>

#include "editor/accessibility/native_semantic_editor_accessibility.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/map/project_world_graph.h"
#include "engine/core/map/tile_layer_document.h"
#include "engine/core/quest/quest_objective_graph.h"
#include "engine/core/ui/menu_authoring_document.h"

#include <algorithm>

TEST_CASE("Native semantic projection routes dialogue selection properties and connections through its owner",
          "[accessibility][native_bridge][semantic_editor][pcq656]") {
    urpg::dialogue::DialogueGraph graph;
    urpg::dialogue::DialogueNode start;
    start.id = "start";
    start.speaker_id = "guide";
    start.speaker_name = "Guide";
    start.localization_key = "dialogue.start";
    start.text_preview = "Before";
    REQUIRE(graph.addNode(start));
    urpg::dialogue::DialogueNode finish;
    finish.id = "finish";
    finish.speaker_id = "guide";
    finish.speaker_name = "Guide";
    finish.localization_key = "dialogue.finish";
    finish.text_preview = "Done";
    finish.ending = true;
    REQUIRE(graph.addNode(finish));

    auto surface = urpg::editor::semanticCommandSurfaceForDialogue(graph);
    std::string selected = "start";
    urpg::editor::NativeAccessibilitySnapshot tree;
    urpg::editor::appendNativeSemanticEditorAccessibility(tree, "dialogue", "Dialogue", surface, selected);
    REQUIRE(std::any_of(tree.nodes.begin(), tree.nodes.end(), [](const auto& node) {
        return node.id == "semantic.dialogue.summary" && node.value == "2 rows";
    }));
    const auto connection = std::find_if(tree.nodes.begin(), tree.nodes.end(), [](const auto& node) {
        return node.id.starts_with("semantic.dialogue.connect.") && node.default_action == "Connect";
    });
    REQUIRE(connection != tree.nodes.end());
    const auto property = std::find_if(tree.nodes.begin(), tree.nodes.end(), [](const auto& node) {
        return node.name == "text_preview";
    });
    REQUIRE(property != tree.nodes.end());
    REQUIRE(property->editable);

    const auto edited = urpg::editor::setNativeSemanticEditorAccessibilityValue(
        "dialogue", property->id, "After", surface, &selected);
    REQUIRE(edited.applied);
    REQUIRE(edited.document_changed);
    REQUIRE(graph.findNode("start")->text_preview == "After");
    const auto connected = urpg::editor::activateNativeSemanticEditorAccessibilityNode(
        "dialogue", connection->id, surface, &selected);
    REQUIRE(connected.applied);
    REQUIRE(connected.document_changed);
    REQUIRE(graph.findNode("start")->choices.size() == 1);
    REQUIRE(graph.findNode("start")->choices.front().target_node_id == "finish");
    REQUIRE(urpg::editor::activateNativeSemanticEditorAccessibilityNode(
        "dialogue", "semantic.dialogue.navigate.next", surface, &selected).applied);
    REQUIRE(selected == "finish");
}

TEST_CASE("Native semantic projection covers menu quest tile and world owner capabilities",
          "[accessibility][native_bridge][semantic_editor][pcq656]") {
    urpg::editor::NativeAccessibilitySnapshot tree;

    urpg::ui::MenuAuthoringDocument menu;
    urpg::ui::MenuCanvasNode button;
    button.id = "continue";
    button.kind = urpg::ui::MenuElementKind::Button;
    button.label = "Continue";
    button.focusable = true;
    REQUIRE(menu.addNode(button));
    auto menu_surface = urpg::editor::semanticCommandSurfaceForMenu(menu);
    urpg::editor::appendNativeSemanticEditorAccessibility(tree, "menu", "Menu", menu_surface, "continue");

    urpg::quest::QuestObjectiveGraphDocument quest;
    quest.quest_id = "quest";
    quest.nodes = {{"start", "start", "Start", {}, "quest.start", {}, {}, 0, 0, false},
                   {"finish", "complete", "Finish", {}, "quest.finish", {}, {}, 0, 0, false}};
    auto quest_surface = urpg::editor::semanticCommandSurfaceForQuest(quest);
    urpg::editor::appendNativeSemanticEditorAccessibility(tree, "quest", "Quest", quest_surface, "start");

    urpg::map::TileLayerDocument tile(1, 1);
    tile.addLayer({"ground", true, false, false, true, 0, {0}});
    auto tile_surface = urpg::editor::semanticCommandSurfaceForTileMap(tile);
    urpg::editor::appendNativeSemanticEditorAccessibility(tree, "tile_map", "Tile map", tile_surface, "ground");

    urpg::map::ProjectWorldGraph world;
    REQUIRE(world.addMap({"a", "Map A", {}, {{{"exit", "Exit", 0, 0}}}, {}, {}}));
    REQUIRE(world.addMap({"b", "Map B", {{{"entry", "Entry", 0, 0}}}, {}, {}, {}}));
    auto world_surface = urpg::editor::semanticCommandSurfaceForWorldMap(world);
    urpg::editor::appendNativeSemanticEditorAccessibility(tree, "world_map", "World map", world_surface, "a");

    for (const auto* id : {"semantic.menu.summary", "semantic.quest.summary",
                           "semantic.tile_map.summary", "semantic.world_map.summary"}) {
        CHECK(std::any_of(tree.nodes.begin(), tree.nodes.end(), [id](const auto& node) { return node.id == id; }));
    }
    CHECK(std::any_of(tree.nodes.begin(), tree.nodes.end(), [](const auto& node) {
        return node.id == "semantic.quest.connect.0.1";
    }));
    CHECK_FALSE(std::any_of(tree.nodes.begin(), tree.nodes.end(), [](const auto& node) {
        return node.id.starts_with("semantic.tile_map.connect.");
    }));
    CHECK(std::any_of(tree.nodes.begin(), tree.nodes.end(), [](const auto& node) {
        return node.id == "semantic.world_map.connect.0.1";
    }));
}
