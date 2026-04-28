#include "engine/core/npc/npc_schedule.h"
#include "editor/npc/npc_panel.h"

#include <catch2/catch_test_macros.hpp>
#include <fstream>

namespace {

nlohmann::json loadNpcScheduleFixture() {
    std::ifstream stream(std::string(URPG_SOURCE_DIR) + "/content/fixtures/npc_schedule_fixture.json");
    REQUIRE(stream.good());
    return nlohmann::json::parse(stream);
}

} // namespace

TEST_CASE("NPC routine fallback activates when map is missing", "[npc][simulation][ffs13]") {
    urpg::npc::NpcSchedule schedule;
    schedule.addRoutine({"npc.merchant", "market", 8, 17, {4, 6}, "idle", "dialogue.shop"});
    schedule.setFallback({"home", {1, 1}, "wait", "dialogue.closed"});

    const auto state = schedule.resolve("npc.merchant", 10, {"home"});

    REQUIRE(state.usedFallback);
    REQUIRE(state.mapId == "home");
    REQUIRE(state.dialogueState == "dialogue.closed");
}

TEST_CASE("NPC schedule document previews daily routines and editor state", "[npc][schedule][routine]") {
    const auto document = urpg::npc::NpcScheduleDocument::fromJson(loadNpcScheduleFixture());

    REQUIRE(document.validate({"market", "tavern", "home"}).empty());

    const auto preview = document.preview("npc.merchant", 10, {"market", "home"});
    REQUIRE_FALSE(preview.state.usedFallback);
    REQUIRE(preview.state.mapId == "market");
    REQUIRE(preview.state.dialogueState == "dialogue.shop");

    const auto fallback = document.preview("npc.merchant", 23, {"home"});
    REQUIRE(fallback.state.usedFallback);
    REQUIRE(fallback.state.mapId == "home");

    urpg::editor::npc::NpcSchedulePanel panel;
    panel.loadDocument(document);
    panel.setPreviewContext("npc.merchant", 18, {"tavern", "home"});
    panel.render();

    REQUIRE(panel.snapshot().map_id == "tavern");
    REQUIRE_FALSE(panel.snapshot().used_fallback);
    REQUIRE(panel.saveProjectData() == document.toJson());
    REQUIRE(urpg::npc::npcSchedulePreviewToJson(panel.preview())["state"]["map_id"] == "tavern");
}

TEST_CASE("NPC schedule document reports overlaps invalid hours and missing maps", "[npc][schedule][routine]") {
    urpg::npc::NpcScheduleDocument document;
    document.fallback = {"missing", {0, 0}, "wait", "closed"};
    document.routines.push_back({"npc.merchant", "market", 8, 18, {1, 1}, "idle", "shop"});
    document.routines.push_back({"npc.merchant", "market", 17, 12, {1, 1}, "idle", "shop"});

    const auto diagnostics = document.validate({"home"});
    REQUIRE(diagnostics.size() >= 4);

    const auto preview = document.preview("npc.merchant", 9, {"home"});
    REQUIRE(preview.state.usedFallback);
    REQUIRE_FALSE(preview.diagnostics.empty());
}
