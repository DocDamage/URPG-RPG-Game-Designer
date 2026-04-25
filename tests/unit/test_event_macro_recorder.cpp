#include "engine/core/events/event_macro_recorder.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Event macro recorder emits editable event and timeline draft commands", "[events][timeline][ffs11]") {
    urpg::events::EventMacroRecorder recorder;
    recorder.record({0, "message", "npc", "Welcome"});
    recorder.record({12, "move", "hero", "x:5,y:6"});
    recorder.record({20, "mz_raw_355", "PluginCommand", "raw payload"});

    const auto draft = recorder.finishDraft("intro_scene");

    REQUIRE(draft.event_id == "intro_scene");
    REQUIRE(draft.event_commands.size() == 3);
    REQUIRE(draft.event_commands[0].kind == urpg::events::EventCommandKind::Message);
    REQUIRE(draft.timeline.commands().size() == 3);
    REQUIRE(draft.timeline.commands()[1].kind == urpg::timeline::TimelineCommandKind::Movement);
    REQUIRE(draft.event_commands[2].kind == urpg::events::EventCommandKind::Unsupported);
    REQUIRE(draft.event_commands[2].compat_fallback["action"] == "mz_raw_355");
}
