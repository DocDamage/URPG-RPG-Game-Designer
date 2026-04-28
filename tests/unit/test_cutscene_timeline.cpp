#include "engine/core/timeline/cutscene_timeline.h"
#include "editor/timeline/cutscene_timeline_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <fstream>

namespace {

nlohmann::json loadCutsceneTimelineFixture() {
    std::ifstream stream(std::string(URPG_SOURCE_DIR) + "/content/fixtures/cutscene_timeline_fixture.json");
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

} // namespace

TEST_CASE("Cutscene timeline previews cues variables and runtime commands", "[timeline][cutscene]") {
    const auto document = urpg::timeline::CutsceneTimelineDocument::fromJson(loadCutsceneTimelineFixture());
    const std::set<std::string> localization_keys{"intro.guide.line", "intro.choice.ready"};

    REQUIRE(document.validate(localization_keys).empty());

    const auto runtime = document.toRuntimeTimeline();
    REQUIRE(runtime.commands().size() == 5);

    const auto preview = document.preview(50, 99, localization_keys);
    REQUIRE(preview.active_cues.size() == 3);
    REQUIRE(preview.played_commands.size() == 5);
    REQUIRE(preview.variable_writes.at("intro_seen") == "true");

    urpg::editor::CutsceneTimelinePanel panel;
    panel.loadDocument(document);
    panel.setPreviewContext(50, 99, localization_keys);
    panel.render();

    REQUIRE(panel.snapshot().active_cue_count == 3);
    REQUIRE(panel.snapshot().variable_write_count == 1);
    REQUIRE(panel.saveProjectData() == document.toJson());
    REQUIRE(urpg::timeline::cutsceneTimelinePreviewToJson(panel.preview())["active_cues"].size() == 3);
}

TEST_CASE("Cutscene timeline reports missing tracks actors localization and invalid times", "[timeline][cutscene]") {
    urpg::timeline::CutsceneTimelineDocument document;
    document.id = "bad";
    document.actors = {"hero"};
    document.tracks.push_back({"dialogue", urpg::timeline::CutsceneTrackKind::Dialogue, "missing_actor"});
    document.cues.push_back({"cue", "missing_track", -1, -2, "", "missing.key", {}});

    const auto diagnostics = document.validate({"known.key"});
    REQUIRE(diagnostics.size() >= 5);

    const auto preview = document.preview(0, 1, {"known.key"});
    REQUIRE_FALSE(preview.diagnostics.empty());
}
