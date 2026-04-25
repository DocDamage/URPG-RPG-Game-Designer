#include "editor/replay/replay_panel.h"
#include "editor/timeline/timeline_panel.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Timeline and replay panels expose deterministic headless snapshots", "[editor][timeline][replay][ffs11]") {
    urpg::timeline::TimelineDocument timeline;
    timeline.addActor("hero");
    timeline.addCommand(urpg::timeline::TimelineCommand{"move", urpg::timeline::TimelineCommandKind::Movement, 1, 4, "hero", "x:1,y:2"});

    urpg::editor::TimelinePanel timeline_panel;
    timeline_panel.setDocument(timeline);
    timeline_panel.render();

    REQUIRE(timeline_panel.hasRenderedFrame());
    REQUIRE(timeline_panel.lastRenderSnapshot().has_document);
    REQUIRE(timeline_panel.lastRenderSnapshot().command_count == 1);
    REQUIRE(timeline_panel.lastRenderSnapshot().diagnostics.empty());

    urpg::editor::ReplayPanel replay_panel;
    urpg::replay::ReplayArtifact z_replay;
    z_replay.id = "z_replay";
    z_replay.seed = 1;
    z_replay.project_version = "1.0.0";
    z_replay.labels = {"golden"};
    urpg::replay::ReplayArtifact a_replay;
    a_replay.id = "a_replay";
    a_replay.seed = 1;
    a_replay.project_version = "1.0.0";
    a_replay.labels = {"golden"};
    replay_panel.gallery().add(z_replay);
    replay_panel.gallery().add(a_replay);
    replay_panel.render();

    REQUIRE(replay_panel.lastRenderSnapshot().artifact_count == 2);
    REQUIRE(replay_panel.lastRenderSnapshot().artifact_ids[0] == "a_replay");
    REQUIRE(replay_panel.lastRenderSnapshot().artifact_ids[1] == "z_replay");
}
