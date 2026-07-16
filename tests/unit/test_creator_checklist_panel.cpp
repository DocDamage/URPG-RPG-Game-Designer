#include "editor/project/creator_checklist_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {
std::filesystem::path uniqueChecklistPanelRoot() {
    return std::filesystem::temp_directory_path() /
           ("urpg_creator_checklist_panel_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
}
}

TEST_CASE("CreatorChecklistPanel persists dismissal and restoration per project", "[project][creator_checklist]") {
    const auto root = uniqueChecklistPanelRoot();
    std::filesystem::create_directories(root / "content" / "maps");
    urpg::editor::CreatorChecklistPanel panel;
    panel.setProjectRoot(root);
    REQUIRE_FALSE(panel.snapshot().dismissed);
    std::string error;
    REQUIRE(panel.dismiss(&error));
    REQUIRE(error.empty());
    REQUIRE(panel.snapshot().dismissed);
    REQUIRE(panel.restore(&error));
    REQUIRE_FALSE(panel.snapshot().dismissed);
    REQUIRE(panel.complete(&error));
    REQUIRE(panel.snapshot().completed);
    REQUIRE(panel.replay(&error));
    REQUIRE_FALSE(panel.snapshot().completed);
    REQUIRE(panel.snapshot().replay_count == 1);
    REQUIRE(panel.isVisible());
    std::filesystem::remove_all(root);
}
