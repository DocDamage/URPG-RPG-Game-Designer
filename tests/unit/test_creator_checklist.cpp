#include "editor/project/creator_checklist.h"

#include <catch2/catch_test_macros.hpp>

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

    urpg::editor::CreatorChecklist checklist;
    const auto snapshot = checklist.inspect(root);
    REQUIRE_FALSE(snapshot.dismissed);
    REQUIRE(snapshot.items.size() == 8);
    for (const auto& item : snapshot.items) REQUIRE(item.complete);
    REQUIRE(checklist.setDismissed(root, true));
    REQUIRE(checklist.inspect(root).dismissed);
    std::ifstream stateInput(root / ".urpg/creator/checklist.json", std::ios::binary);
    const auto state = nlohmann::json::parse(stateInput);
    REQUIRE(state.value("schema", "") == "urpg.creator_checklist.v1");
    REQUIRE(state.value("dismissed", false));
    stateInput.close();
    REQUIRE(checklist.setDismissed(root, false));
    REQUIRE_FALSE(checklist.inspect(root).dismissed);
    std::filesystem::remove_all(root);
}
