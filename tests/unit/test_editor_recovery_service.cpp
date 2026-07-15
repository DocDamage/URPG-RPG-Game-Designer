#include "editor/project/editor_recovery_service.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path makeProjectRoot() {
    const auto root = std::filesystem::temp_directory_path() /
                      ("urpg_editor_recovery_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root / "content");
    std::ofstream(root / "project.json") << R"({"project_id":"recovery","project_name":"Recovery","schema_version":"urpg.project.v1"})";
    std::ofstream(root / "content" / "story.json") << R"({"chapter":1})";
    return root;
}

} // namespace

TEST_CASE("EditorRecoveryService keeps recovery data outside project snapshots", "[editor][recovery]") {
    const auto root = makeProjectRoot();
    urpg::editor::EditorRecoveryService service;
    REQUIRE(service.writeSessionMarker(root));
    REQUIRE(service.hasUncleanSessionMarker(root));
    REQUIRE(service.createRecoverySnapshot(root, "recovery", {"map.grid_parts", "ability.draft"}));
    const auto snapshots = service.listSnapshots(root);
    REQUIRE(snapshots.size() == 1);
    REQUIRE(snapshots.front().project_id == "recovery");
    REQUIRE(snapshots.front().dirty_document_ids == std::vector<std::string>{"map.grid_parts", "ability.draft"});
    REQUIRE_FALSE(std::filesystem::exists(snapshots.front().path / "project" / ".urpg"));
    REQUIRE(service.clearSessionMarker(root));
    REQUIRE_FALSE(service.hasUncleanSessionMarker(root));
    std::filesystem::remove_all(root);
}
