#include "editor/events/event_authoring_panel.h"
#include "editor/events/event_command_graph_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path eventAuthoringRepoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

nlohmann::json loadEventAuthoringJson(const std::filesystem::path& path) {
    std::ifstream stream(path);
    REQUIRE(stream.is_open());
    nlohmann::json json;
    stream >> json;
    return json;
}

} // namespace

TEST_CASE("EventAuthoringPanel renders document graph diagnostics and debugger summary", "[events][authoring][editor]") {
    using namespace urpg::events;
    urpg::editor::EventAuthoringPanel panel;

    EventDocument document;
    document.addMap(MapDefinition{"town", 10, 10});
    document.addEvent(EventDefinition{
        "evt",
        "town",
        1,
        1,
        {EventPage{"p", 0, EventTrigger::ActionButton, {}, {
            EventCommand{"switch", EventCommandKind::Switch, "s1", "true"}
        }}}
    });

    panel.model().load(std::move(document));
    panel.model().startDebugging("evt");
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().event_count == 1);
    REQUIRE(panel.lastRenderSnapshot().page_count == 1);
    REQUIRE(panel.lastRenderSnapshot().command_count == 1);
    REQUIRE(panel.lastRenderSnapshot().dependency_edge_count == 1);
    REQUIRE(panel.lastRenderSnapshot().diagnostic_count == 0);
    REQUIRE(panel.lastRenderSnapshot().has_active_page);
    REQUIRE(panel.lastRenderSnapshot().debugger_running);
}

TEST_CASE("EventAuthoringPanel renders empty document snapshot without crashing", "[events][authoring][editor]") {
    urpg::editor::EventAuthoringPanel panel;

    panel.model().load(urpg::events::EventDocument{});
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().event_count == 0);
    REQUIRE(panel.lastRenderSnapshot().page_count == 0);
    REQUIRE(panel.lastRenderSnapshot().command_count == 0);
    REQUIRE(panel.lastRenderSnapshot().dependency_edge_count == 0);
    REQUIRE(panel.lastRenderSnapshot().diagnostic_count == 0);
    REQUIRE_FALSE(panel.lastRenderSnapshot().has_active_page);
    REQUIRE_FALSE(panel.lastRenderSnapshot().debugger_running);
}

TEST_CASE("Event command graph is visually authorable, saved, and executable",
          "[events][authoring][command_graph][wysiwyg]") {
    const auto json = loadEventAuthoringJson(
        eventAuthoringRepoRoot() / "content" / "fixtures" / "event_command_graph_fixture.json");
    const auto document = urpg::events::EventCommandGraphDocument::fromJson(json);

    urpg::editor::EventCommandGraphPanel panel;
    panel.loadDocument(document);
    panel.selectNode("set_gate_open");
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().graph_id == "forest_gate_graph");
    REQUIRE(panel.snapshot().selected_node_id == "set_gate_open");
    REQUIRE(panel.snapshot().node_count == 3);
    REQUIRE(panel.snapshot().edge_count == 2);
    REQUIRE(panel.snapshot().runtime_command_count == 3);
    REQUIRE(panel.snapshot().diagnostic_count == 0);
    REQUIRE(panel.snapshot().dependency_edge_count == 2);
    REQUIRE(panel.snapshot().runtime_executed);
    REQUIRE(panel.snapshot().status_message == "Event command graph preview is ready.");
    REQUIRE_FALSE(panel.snapshot().saved_project_json.empty());

    const auto& runtime = panel.runtimePreview();
    REQUIRE(runtime.state.switches.at("gate_open"));
    REQUIRE(runtime.state.variables.at("crystals_spent") == 1);
    REQUIRE(runtime.timeline.Entries().size() == 2);
    REQUIRE(panel.runtimeDocument().events().size() == 1);
    REQUIRE(panel.runtimeDocument().events()[0].pages[0].commands.size() == 3);
}

TEST_CASE("Event command graph saved project data round-trips through runtime document",
          "[events][authoring][command_graph][wysiwyg]") {
    const auto json = loadEventAuthoringJson(
        eventAuthoringRepoRoot() / "content" / "fixtures" / "event_command_graph_fixture.json");
    const auto document = urpg::events::EventCommandGraphDocument::fromJson(json);
    const auto saved = document.toJson();
    const auto restored = urpg::events::EventCommandGraphDocument::fromJson(saved);

    REQUIRE(saved["schema"] == "urpg.event_command_graph.v1");
    REQUIRE(restored.id == document.id);
    REQUIRE(restored.orderedNodes().size() == 3);

    const auto runtime_document = restored.toEventDocument();
    REQUIRE(runtime_document.events().size() == 1);
    REQUIRE(runtime_document.events()[0].id == "evt_forest_gate");
    REQUIRE(runtime_document.events()[0].pages[0].commands[1].kind == urpg::events::EventCommandKind::Switch);
}

TEST_CASE("Event command graph diagnostics block false complete claims",
          "[events][authoring][command_graph][wysiwyg]") {
    urpg::events::EventCommandGraphDocument document;
    document.id = "broken_graph";
    document.map_id = "forest";
    document.event_id = "evt";
    document.page_id = "page";
    document.entry_node_id = "missing";
    document.nodes = {
        {"dup", "Unsupported", urpg::events::EventCommandKind::Unsupported, "", "", 0, 0, 0, {}},
        {"dup", "Switch without target", urpg::events::EventCommandKind::Switch, "", "true", 1, 100, 0, {}},
    };
    document.edges = {
        {"edge", "dup", "missing", "sequence"},
    };

    urpg::editor::EventCommandGraphPanel panel;
    panel.loadDocument(document);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().diagnostic_count >= 5);
    REQUIRE_FALSE(panel.snapshot().runtime_executed);
    REQUIRE(panel.snapshot().status_message == "Event command graph preview has diagnostics.");

    const auto& diagnostics = panel.runtimePreview().diagnostics;
    const auto hasCode = [&diagnostics](const std::string& code) {
        return std::any_of(diagnostics.begin(), diagnostics.end(), [&code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
    };
    REQUIRE(hasCode("duplicate_node_id"));
    REQUIRE(hasCode("unsupported_command_kind"));
    REQUIRE(hasCode("missing_command_target"));
    REQUIRE(hasCode("missing_entry_node"));
    REQUIRE(hasCode("missing_edge_target"));
}
