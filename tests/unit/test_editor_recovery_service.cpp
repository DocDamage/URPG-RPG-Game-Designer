#include "editor/project/editor_recovery_service.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

namespace {

void writeTextFile(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << text;
}

std::string readTextFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

} // namespace

TEST_CASE("editor recovery service session markers", "[editor][recovery]") {
    const auto project_root = std::filesystem::temp_directory_path() / "urpg_recovery_test_proj";
    std::filesystem::remove_all(project_root);

    urpg::editor::EditorRecoveryService service;
    REQUIRE_FALSE(service.hasUncleanSessionMarker(project_root));

    REQUIRE(service.writeSessionMarker(project_root));
    REQUIRE(service.hasUncleanSessionMarker(project_root));
    std::ifstream markerInput(project_root / ".urpg" / "session_marker.json");
    const auto marker = nlohmann::json::parse(markerInput);
    REQUIRE(marker["schema_version"] == "urpg.session_marker.v1");
    REQUIRE(marker["pid"].get<uint64_t>() > 0);
    markerInput.close();

    REQUIRE(service.clearSessionMarker(project_root));
    REQUIRE_FALSE(service.hasUncleanSessionMarker(project_root));

    std::filesystem::remove_all(project_root);
}

TEST_CASE("editor recovery service snapshots life cycle", "[editor][recovery]") {
    const auto project_root = std::filesystem::temp_directory_path() / "urpg_recovery_snapshots_proj";
    std::filesystem::remove_all(project_root);

    // Create a mock project
    writeTextFile(project_root / "project.json", R"({"project_id":"my_test_proj"})");
    writeTextFile(project_root / "data" / "Map001.json", R"({"id":1})");

    urpg::editor::EditorRecoveryService service;
    std::vector<std::string> dirty_docs = {"map_doc_01"};

    REQUIRE(service.createRecoverySnapshot(project_root, "my_test_proj", dirty_docs));

    // List snapshots
    auto snaps = service.listSnapshots(project_root);
    REQUIRE(snaps.size() == 1);
    REQUIRE(snaps[0].project_id == "my_test_proj");
    REQUIRE(snaps[0].dirty_document_ids == dirty_docs);

    // Create more snapshots to verify pruning
    REQUIRE(service.createRecoverySnapshot(project_root, "my_test_proj", dirty_docs));
    REQUIRE(service.createRecoverySnapshot(project_root, "my_test_proj", dirty_docs));

    snaps = service.listSnapshots(project_root);
    REQUIRE(snaps.size() == 3);

    // Prune down to 2
    service.pruneSnapshots(project_root, 2, 100 * 1024 * 1024);
    snaps = service.listSnapshots(project_root);
    REQUIRE(snaps.size() == 2);

    // Restore newest snapshot
    const auto restore_dest = std::filesystem::temp_directory_path() / "urpg_recovery_restore_dest";
    std::filesystem::remove_all(restore_dest);

    REQUIRE(service.restoreRecoverySnapshot(snaps[0].path, restore_dest));
    REQUIRE(readTextFile(restore_dest / "project.json") == R"({"project_id":"my_test_proj"})");
    REQUIRE(readTextFile(restore_dest / "data" / "Map001.json") == R"({"id":1})");

    const auto replace_dest = std::filesystem::temp_directory_path() / "urpg_recovery_replace_dest";
    std::filesystem::remove_all(replace_dest);
    writeTextFile(replace_dest / "project.json", R"({"project_id":"old"})");
    REQUIRE(service.restoreRecoverySnapshot(snaps[0].path, replace_dest, true));
    REQUIRE(readTextFile(replace_dest / "project.json") == R"({"project_id":"my_test_proj"})");

    std::filesystem::remove_all(project_root);
    std::filesystem::remove_all(restore_dest);
    std::filesystem::remove_all(replace_dest);
    for (const auto& entry : std::filesystem::directory_iterator(replace_dest.parent_path())) {
        if (entry.path().filename().string().starts_with(replace_dest.filename().string() + ".pre-recovery-")) {
            std::filesystem::remove_all(entry.path());
        }
    }
}

TEST_CASE("editor recovery rejects a snapshot with corrupted payload", "[editor][recovery]") {
    const auto project_root = std::filesystem::temp_directory_path() / "urpg_recovery_corrupt_proj";
    std::filesystem::remove_all(project_root);
    writeTextFile(project_root / "project.json", R"({"project_id":"corrupt_test"})");

    urpg::editor::EditorRecoveryService service;
    REQUIRE(service.createRecoverySnapshot(project_root, "corrupt_test", {"project"}));
    const auto snapshots = service.listSnapshots(project_root);
    REQUIRE(snapshots.size() == 1);

    writeTextFile(snapshots[0].path / "project" / "project.json", "corrupted");
    REQUIRE(service.listSnapshots(project_root).empty());
    REQUIRE_FALSE(service.restoreRecoverySnapshot(
        snapshots[0].path, std::filesystem::temp_directory_path() / "urpg_corrupt_restore"));
    std::filesystem::remove_all(project_root);
}
