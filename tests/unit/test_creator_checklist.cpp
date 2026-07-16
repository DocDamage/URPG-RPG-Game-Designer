#include "editor/project/creator_checklist.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace {
void writeJson(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary);
    output << text;
}
}

TEST_CASE("CreatorChecklist derives completion from durable project state", "[project][creator checklist]") {
    const auto root = std::filesystem::temp_directory_path() /
        ("urpg_creator_checklist_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    writeJson(root / "content/maps/intro.json", R"({"spawn":{"x":1,"y":2}})");
    writeJson(root / "content/assets/manifests/hero.json", "{}");
    writeJson(root / "content/events/guide.json", "{}");
    writeJson(root / "content/dialogue/guide.json", "{}");
    writeJson(root / ".urpg/playtest/last_completed.json", "{}");
    writeJson(root / ".urpg/creator/last_manual_save.json", "{}");
    writeJson(root / ".urpg/reports/validation.json", "{}");
    writeJson(root / "project.json", R"({"creator_sample":true})");

    urpg::editor::CreatorChecklist checklist;
    const auto snapshot = checklist.inspect(root);
    REQUIRE_FALSE(snapshot.dismissed);
    REQUIRE(snapshot.items.size() == 8);
    REQUIRE(snapshot.sample_project);
    REQUIRE(snapshot.completed);
    REQUIRE(snapshot.next_item_id.empty());
    for (const auto& item : snapshot.items) REQUIRE(item.complete);
    for (const auto& item : snapshot.items) {
        REQUIRE_FALSE(item.route.empty());
        REQUIRE_FALSE(item.action_label.empty());
        REQUIRE_FALSE(item.coaching.empty());
    }
    REQUIRE(checklist.setDismissed(root, true));
    REQUIRE(checklist.inspect(root).dismissed);
    std::ifstream stateInput(root / ".urpg/creator/checklist.json", std::ios::binary);
    const auto state = nlohmann::json::parse(stateInput);
    REQUIRE(state.value("schema", "") == "urpg.creator_checklist.v2");
    REQUIRE(state.value("dismissed", false));
    stateInput.close();
    REQUIRE(checklist.setDismissed(root, false));
    REQUIRE_FALSE(checklist.inspect(root).dismissed);
    std::filesystem::remove_all(root);
}

TEST_CASE("CreatorChecklist persists completion and can replay contextual coaching", "[project][creator checklist][onboarding]") {
    const auto root = std::filesystem::temp_directory_path() /
        ("urpg_creator_checklist_replay_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    writeJson(root / "project.json", R"({"creator_sample":true})");
    urpg::editor::CreatorChecklist checklist;
    auto snapshot = checklist.inspect(root);
    REQUIRE(snapshot.sample_project);
    REQUIRE_FALSE(snapshot.completed);
    REQUIRE(snapshot.next_item_id == "hero_art");
    REQUIRE(checklist.setCompleted(root, true));
    REQUIRE(checklist.inspect(root).completed);
    REQUIRE(checklist.setDismissed(root, true));
    REQUIRE(checklist.replay(root));
    snapshot = checklist.inspect(root);
    REQUIRE_FALSE(snapshot.dismissed);
    REQUIRE_FALSE(snapshot.completed);
    REQUIRE(snapshot.replay_count == 1);
    REQUIRE(snapshot.next_item_id == "hero_art");
    std::filesystem::remove_all(root);
}

TEST_CASE("CreatorChecklist recognizes events persisted by the native Map document", "[project][creator checklist][events]") {
    const auto root = std::filesystem::temp_directory_path() /
        ("urpg_creator_checklist_map_event_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    writeJson(root / "content/maps/willow_village.p2d.json",
              R"({"events":[{"event_id":"elder_mira_intro","pages":[{"page_id":"main"}]}]})");

    urpg::editor::CreatorChecklist checklist;
    const auto snapshot = checklist.inspect(root);
    const auto event = std::find_if(snapshot.items.begin(), snapshot.items.end(), [](const auto& item) {
        return item.id == "npc_event";
    });
    REQUIRE(event != snapshot.items.end());
    REQUIRE(event->complete);
    std::filesystem::remove_all(root);
}
