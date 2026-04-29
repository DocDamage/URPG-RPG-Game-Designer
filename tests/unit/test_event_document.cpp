#include "engine/core/events/event_document.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

namespace {

urpg::events::EventDocument makeDocument() {
    using namespace urpg::events;
    EventDocument document;
    document.addMap(MapDefinition{"town", 20, 15});
    document.addCommonEvent(CommonEventDefinition{"ce_reward", {EventCommand{"gold", EventCommandKind::Gold, {}, {}, 25}}});
    document.addEvent(EventDefinition{
        "evt_npc",
        "town",
        4,
        5,
        {
            EventPage{"base", 0, EventTrigger::ActionButton, {}, {EventCommand{"msg", EventCommandKind::Message, {}, "hello"}}},
            EventPage{"quest", 10, EventTrigger::ActionButton, EventCondition{{{"quest_started", true}}, {}, {}}, {EventCommand{"ce", EventCommandKind::CommonEvent, "ce_reward", {}}}}
        }
    });
    return document;
}

} // namespace

TEST_CASE("EventDocument resolves overlapping pages deterministically", "[events][authoring][document]") {
    auto document = makeDocument();
    urpg::events::EventWorldState state;
    state.switches["quest_started"] = true;

    const auto page = document.resolveActivePage("evt_npc", state);

    REQUIRE(page.has_value());
    REQUIRE(page->id == "quest");
}

TEST_CASE("EventDocument preserves unsupported commands in compat fallback payloads", "[events][authoring][document]") {
    const auto json = nlohmann::json::parse(R"({
        "maps": [{"id": "town", "width": 10, "height": 10}],
        "events": [{
            "id": "evt_plugin",
            "map_id": "town",
            "x": 1,
            "y": 1,
            "pages": [{
                "id": "p1",
                "priority": 0,
                "trigger": "action_button",
                "commands": [{
                    "id": "raw_1",
                    "kind": "mz_unknown_777",
                    "payload": {"code": 777, "parameters": ["raw"]}
                }]
            }]
        }]
    })");

    const auto document = urpg::events::EventDocument::fromJson(json);
    const auto round_trip = document.toJson();

    REQUIRE(round_trip["events"][0]["pages"][0]["commands"][0]["kind"] == "unsupported");
    REQUIRE(round_trip["events"][0]["pages"][0]["commands"][0]["_compat_command_fallbacks"]["payload"]["code"] == 777);
}

TEST_CASE("EventDocument round-trips native draggable event metadata", "[events][authoring][document][drag]") {
    const auto json = nlohmann::json::parse(R"({
        "maps": [{"id": "town", "width": 20, "height": 15}],
        "events": [{
            "id": "evt_crate",
            "map_id": "town",
            "x": 3,
            "y": 4,
            "drag": {
                "enabled": true,
                "axis": "horizontal",
                "snap_to_grid": true,
                "require_passable_target": true,
                "required_switch": "can_push_crates",
                "min_x": 2,
                "min_y": 4,
                "max_x": 12,
                "max_y": 4
            },
            "pages": [{
                "id": "p1",
                "priority": 0,
                "trigger": "action_button",
                "commands": []
            }]
        }]
    })");

    auto document = urpg::events::EventDocument::fromJson(json);
    document.setKnownSwitches({"can_push_crates"});
    const auto diagnostics = document.validate();
    const auto saved = document.toJson();

    REQUIRE(diagnostics.empty());
    REQUIRE(document.events()[0].drag.enabled);
    REQUIRE(document.events()[0].drag.axis == "horizontal");
    REQUIRE(document.events()[0].drag.required_switch == "can_push_crates");
    REQUIRE(saved["events"][0]["drag"]["enabled"] == true);
    REQUIRE(saved["events"][0]["drag"]["max_x"] == 12);
}

TEST_CASE("EventDocument validates draggable event configuration", "[events][authoring][document][drag]") {
    using namespace urpg::events;
    EventDocument document;
    document.addMap(MapDefinition{"town", 10, 10});
    document.setKnownSwitches({"can_drag"});
    EventDefinition event;
    event.id = "evt_bad_drag";
    event.map_id = "town";
    event.x = 1;
    event.y = 1;
    event.drag.enabled = true;
    event.drag.axis = "diagonal";
    event.drag.required_switch = "missing_switch";
    event.drag.min_x = 8;
    event.drag.max_x = 2;
    event.drag.max_y = 99;
    event.pages.push_back(EventPage{"p", 0, EventTrigger::ActionButton, {}, {}});
    document.addEvent(std::move(event));

    const auto diagnostics = document.validate();

    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "invalid_drag_axis"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "invalid_drag_bounds"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "drag_bounds_out_of_map"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "missing_switch_reference"; }));
}

TEST_CASE("EventDocument validation reports duplicate ids, missing references, disabled plugins, and cycles", "[events][authoring][document]") {
    using namespace urpg::events;
    EventDocument document;
    document.addMap(MapDefinition{"town", 5, 5});
    document.addCommonEvent(CommonEventDefinition{"a", {EventCommand{"call_b", EventCommandKind::CommonEvent, "b", {}}}});
    document.addCommonEvent(CommonEventDefinition{"b", {EventCommand{"call_a", EventCommandKind::CommonEvent, "a", {}}}});
    document.addEvent(EventDefinition{"evt", "missing", 0, 0, {EventPage{"p", 0, EventTrigger::ActionButton, {}, {EventCommand{"plugin", EventCommandKind::Plugin, "DisabledPlugin", {}}}}}});
    document.addEvent(EventDefinition{"evt", "town", 9, 9, {EventPage{"p", 0, EventTrigger::ActionButton, {}, {EventCommand{"common", EventCommandKind::CommonEvent, "missing_ce", {}}}}}});

    const auto diagnostics = document.validate();

    REQUIRE(diagnostics.size() >= 5);
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "duplicate_event_id"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "missing_target_map"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "missing_common_event"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "disabled_plugin_command"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "cyclic_common_event"; }));
}

TEST_CASE("EventDocument validation reports unreachable pages and deleted authoring references", "[events][authoring][document]") {
    using namespace urpg::events;
    EventDocument document;
    document.addMap(MapDefinition{"town", 10, 10});
    document.setKnownSwitches({"quest_started"});
    document.setKnownVariables({"party_level"});
    document.setKnownSaveFields({"party.gold"});
    document.addEvent(EventDefinition{
        "evt_shadowed",
        "town",
        1,
        1,
        {
            EventPage{
                "higher",
                10,
                EventTrigger::ActionButton,
                EventCondition{{{"quest_started", true}}, {}, {}},
                {EventCommand{"write_save", EventCommandKind::Gold, {}, {}, 0, {}, {"party.gold"}}}
            },
            EventPage{
                "lower",
                0,
                EventTrigger::ActionButton,
                EventCondition{{{"quest_started", true}, {"deleted_switch", true}}, {{"deleted_variable", 1}}, {}},
                {
                    EventCommand{"set_deleted_switch", EventCommandKind::Switch, "deleted_switch", "true"},
                    EventCommand{"set_deleted_variable", EventCommandKind::Variable, "deleted_variable", {}, 4},
                    EventCommand{"read_deleted_save", EventCommandKind::Message, {}, {}, 0, {"missing.save"}, {}}
                }
            }
        }
    });

    const auto diagnostics = document.validate();

    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "unreachable_page"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "missing_switch_reference"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "missing_variable_reference"; }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) { return diagnostic.code == "missing_save_field_reference"; }));
}
