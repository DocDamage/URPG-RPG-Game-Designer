#include <catch2/catch_test_macros.hpp>

#include <algorithm>

#include "editor/accessibility/semantic_editor_command_surface.h"
#include "editor/dialogue/dialogue_graph_panel.h"
#include "editor/quest/quest_panel.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/map/project_world_graph.h"
#include "engine/core/map/tile_layer_document.h"
#include "engine/core/quest/quest_objective_graph.h"
#include "engine/core/ui/menu_authoring_document.h"

TEST_CASE("Semantic command surface provides wrapped ordered keyboard navigation and owner-backed menu edits",
          "[accessibility][semantic_editor][commands][pcq654]") {
    urpg::ui::MenuAuthoringDocument document;
    urpg::ui::MenuCanvasNode panel;
    panel.id = "panel";
    panel.label = "Panel";
    REQUIRE(document.addNode(panel));
    urpg::ui::MenuCanvasNode button;
    button.id = "button";
    button.kind = urpg::ui::MenuElementKind::Button;
    button.label = "Continue";
    REQUIRE(document.addNode(button));

    auto surface = urpg::editor::semanticCommandSurfaceForMenu(document);
    REQUIRE(surface.selectedId() == "panel");
    REQUIRE(surface.navigate(urpg::editor::SemanticEditorNavigation::Next).applied);
    REQUIRE(surface.selectedId() == "button");
    REQUIRE(surface.navigate(urpg::editor::SemanticEditorNavigation::Next).applied);
    REQUIRE(surface.selectedId() == "panel");
    REQUIRE(surface.select("button").applied);
    REQUIRE(surface.setSelectedProperty("accessible_label", "Continue the game").applied);
    REQUIRE(document.findNode("button")->accessible_label == "Continue the game");
    REQUIRE(surface.setSelectedProperty("x", "144").applied);
    REQUIRE(surface.setSelectedProperty("focus_order", "2").applied);
    REQUIRE(document.findNode("button")->layout.x == 144);
    REQUIRE(document.findNode("button")->layout.focus_order == 2);
    REQUIRE_FALSE(surface.setSelectedProperty("width", "not-a-number").applied);
    REQUIRE(surface.select("panel").applied);
    REQUIRE(surface.connectSelectedTo("button").applied);
    REQUIRE(document.findNode("button")->parent_id == "panel");

    const auto snapshot = surface.renderSnapshot();
    REQUIRE(snapshot["bound"] == true);
    REQUIRE(snapshot["rows"].size() == 2);
    REQUIRE(snapshot["property_editing"] == true);
    REQUIRE(snapshot["connection_creation"] == true);
    REQUIRE(snapshot["keyboard_commands"].size() == 6);
    REQUIRE(snapshot["controller_commands"].size() == 6);
    REQUIRE(snapshot["operation_routes"]["tree_list_navigation"] == true);
    REQUIRE(snapshot["operation_routes"]["controller_navigation"] == true);
}

TEST_CASE("Semantic command surfaces write dialogue and quest properties and connections through owners",
          "[accessibility][semantic_editor][commands][pcq654]") {
    urpg::dialogue::DialogueGraph dialogue;
    urpg::dialogue::DialogueNode start;
    start.id = "start";
    start.speaker_id = "guide";
    start.speaker_name = "Guide";
    start.localization_key = "dialogue.start";
    start.text_preview = "Old text";
    REQUIRE(dialogue.addNode(start));
    urpg::dialogue::DialogueNode end;
    end.id = "end";
    end.speaker_id = "guide";
    end.speaker_name = "Guide";
    end.localization_key = "dialogue.end";
    end.text_preview = "Done";
    end.ending = true;
    REQUIRE(dialogue.addNode(end));
    dialogue.setStartNode("start");

    auto dialogue_surface = urpg::editor::semanticCommandSurfaceForDialogue(dialogue);
    REQUIRE(dialogue_surface.select("start").applied);
    REQUIRE(dialogue_surface.setSelectedProperty("text_preview", "Welcome").applied);
    REQUIRE(dialogue.findNode("start")->text_preview == "Welcome");
    REQUIRE(dialogue_surface.connectSelectedTo("end").applied);
    REQUIRE(dialogue.findNode("start")->choices.size() == 1);
    REQUIRE(dialogue.findNode("start")->choices[0].target_node_id == "end");
    REQUIRE_FALSE(dialogue_surface.connectSelectedTo("end").applied);

    urpg::quest::QuestObjectiveGraphDocument quest;
    quest.quest_id = "quest";
    quest.title = "Quest";
    quest.nodes = {{"begin", "start", "Begin", {}, "quest.begin", {}, {}, 0, 0, false},
                   {"finish", "complete", "Finish", {}, "quest.finish", {}, {}, 0, 0, false}};
    auto quest_surface = urpg::editor::semanticCommandSurfaceForQuest(quest);
    REQUIRE(quest_surface.select("begin").applied);
    REQUIRE(quest_surface.setSelectedProperty("objective_id", "objective.begin").applied);
    REQUIRE(quest.nodes[0].objective_id == "objective.begin");
    REQUIRE(quest_surface.connectSelectedTo("finish").applied);
    REQUIRE(quest.links.size() == 1);
    REQUIRE(quest.links[0].from == "begin");
    REQUIRE(quest.links[0].to == "finish");
}

TEST_CASE("Semantic tile and world surfaces preserve owner validation and capability diagnostics",
          "[accessibility][semantic_editor][commands][pcq654]") {
    urpg::map::TileLayerDocument tile_map(2, 2);
    tile_map.addLayer({"ground", true, false, false, true, 0, {0, 0, 0, 0}});
    auto tile_surface = urpg::editor::semanticCommandSurfaceForTileMap(tile_map);
    REQUIRE(tile_surface.setSelectedProperty("visible", "false").applied);
    REQUIRE_FALSE(tile_map.layers()[0].visible);
    const auto unsupported = tile_surface.connectSelectedTo("ground");
    REQUIRE_FALSE(unsupported.applied);
    REQUIRE(unsupported.code == "semantic_connection_unsupported");

    urpg::map::ProjectWorldGraph world;
    REQUIRE(world.addMap({"map-a", "Map A", {}, {{{"exit-a", "Exit", 0, 0}}}, {}, {}}));
    REQUIRE(world.addMap({"map-b", "Map B", {{{"entry-b", "Entry", 0, 0}}}, {}, {}, {}}));
    auto world_surface = urpg::editor::semanticCommandSurfaceForWorldMap(world);
    REQUIRE(world_surface.select("map-a").applied);
    REQUIRE(world_surface.setSelectedProperty("label", "Map Alpha").applied);
    REQUIRE(world.maps()[0].label == "Map Alpha");
    REQUIRE(world_surface.connectSelectedTo("map-b").applied);
    REQUIRE(world.routes().size() == 1);
    REQUIRE(world.routes()[0].source_exit_id == "exit-a");
    REQUIRE(world.routes()[0].target_entrance_id == "entry-b");
    REQUIRE(world_surface.diagnostics().empty());
}

TEST_CASE("Semantic command diagnostics retain object links and focus the affected row",
          "[accessibility][semantic_editor][diagnostics][pcq654]") {
    urpg::ui::MenuAuthoringDocument menu;
    urpg::ui::MenuCanvasNode action;
    action.id = "required-action";
    action.label = "Required";
    action.focusable = true;
    action.required_action = true;
    action.layout = {0, 0, 80, 20, 0};
    REQUIRE(menu.addNode(action));

    auto surface = urpg::editor::semanticCommandSurfaceForMenu(menu);
    const auto linked = surface.linkedDiagnostics();
    REQUIRE_FALSE(linked.empty());
    const auto missing_label = std::find_if(linked.begin(), linked.end(), [](const auto& item) {
        return item.code == "missing_label";
    });
    REQUIRE(missing_label != linked.end());
    REQUIRE(missing_label->object_id == "required-action");
    const auto index = static_cast<std::size_t>(std::distance(linked.begin(), missing_label));
    REQUIRE(surface.focusDiagnostic(index).applied);
    REQUIRE(surface.selectedId() == "required-action");
    const auto snapshot = surface.renderSnapshot();
    REQUIRE(snapshot["linked_diagnostics"][index]["focusable"] == true);
    REQUIRE(snapshot["linked_diagnostics"][index]["object_id"] == "required-action");
    REQUIRE_FALSE(surface.focusDiagnostic(linked.size()).applied);

    urpg::map::TileLayerDocument tile_map(1, 1);
    tile_map.addLayer({"collision", true, false, true, false, 0, {0}});
    tile_map.addLayer({"navigation", true, false, false, true, 1, {0}});
    REQUIRE(tile_map.setTile("collision", 0, 0, 1));
    REQUIRE(tile_map.setTile("navigation", 0, 0, 1));
    auto tile_surface = urpg::editor::semanticCommandSurfaceForTileMap(tile_map);
    REQUIRE(tile_surface.linkedDiagnostics().size() == 1);
    REQUIRE(tile_surface.linkedDiagnostics()[0].object_id == "collision");
    REQUIRE(tile_surface.linkedDiagnostics()[0].related_object_id == "tile:0,0");
    REQUIRE(tile_surface.focusDiagnostic(0).applied);

    urpg::map::ProjectWorldGraph world;
    REQUIRE(world.addMap({"map-a", "Map A", {}, {{{"exit-a", "Exit", 0, 0}}}, {}, {}}));
    REQUIRE(world.addMap({"map-b", "Map B", {{{"entry-b", "Entry", 0, 0}}}, {}, {}, {}}));
    REQUIRE(world.addRoute({"bad-route", "Bad route", "map-a", "missing-exit",
                            "map-b", "entry-b", {}}));
    auto world_surface = urpg::editor::semanticCommandSurfaceForWorldMap(world);
    const auto world_diagnostics = world_surface.linkedDiagnostics();
    const auto broken_exit = std::find_if(world_diagnostics.begin(), world_diagnostics.end(), [](const auto& item) {
        return item.code == "route_source_exit_missing";
    });
    REQUIRE(broken_exit != world_diagnostics.end());
    REQUIRE(broken_exit->object_id == "map-a");
    REQUIRE(broken_exit->related_object_id == "bad-route");
    REQUIRE(world_surface.focusDiagnostic(
        static_cast<std::size_t>(std::distance(world_diagnostics.begin(), broken_exit))).applied);
    REQUIRE(world_surface.selectedId() == "map-a");
}

TEST_CASE("Dialogue and quest panels expose semantic command snapshots backed by their owned graphs",
          "[accessibility][semantic_editor][panel][pcq654]") {
    urpg::dialogue::DialogueGraph dialogue;
    urpg::dialogue::DialogueNode node;
    node.id = "line";
    node.speaker_id = "guide";
    node.speaker_name = "Guide";
    node.localization_key = "line";
    node.text_preview = "Before";
    node.ending = true;
    REQUIRE(dialogue.addNode(node));
    dialogue.setStartNode("line");
    urpg::editor::DialogueGraphPanel dialogue_panel;
    dialogue_panel.setGraph(std::move(dialogue));
    REQUIRE(dialogue_panel.setSemanticProperty("text_preview", "After").applied);
    dialogue_panel.render();
    REQUIRE(dialogue_panel.lastRenderSnapshot()["graph"]["nodes"][0]["text_preview"] == "After");
    REQUIRE(dialogue_panel.lastRenderSnapshot()["semantic_alternative"]["rows"].size() == 1);

    urpg::quest::QuestObjectiveGraphDocument quest;
    quest.quest_id = "quest";
    quest.title = "Quest";
    quest.nodes = {{"finish", "complete", "Old", {}, "quest.finish", {}, {}, 0, 0, false}};
    urpg::editor::QuestPanel quest_panel;
    quest_panel.bindObjectiveGraph(std::move(quest));
    REQUIRE(quest_panel.setSemanticProperty("title", "New").applied);
    REQUIRE(quest_panel.selectGraphNode("finish"));
    REQUIRE(quest_panel.navigateSemantic(urpg::editor::SemanticEditorNavigation::First).applied);
    quest_panel.render();
    REQUIRE(quest_panel.lastRenderSnapshot()["graph"]["nodes"][0]["title"] == "New");
    REQUIRE(quest_panel.lastRenderSnapshot()["graph"]["semantic_alternative"]["rows"].size() == 1);
}
