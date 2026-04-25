#include "engine/core/project/project_snapshot_store.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>

namespace {

void writeText(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << text;
}

std::string readText(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

} // namespace

TEST_CASE("project snapshot store round trips project files", "[project][snapshot][ffs09]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_ffs09_snapshot";
    std::filesystem::remove_all(base);
    writeText(base / "project" / "project.json", R"({"name":"Demo"})");
    writeText(base / "project" / "data" / "Map001.json", R"({"id":1})");

    const urpg::project::ProjectSnapshotStore store;
    const auto snapshot = store.createSnapshot(base / "project", base / "snapshots", "before_migration");
    REQUIRE(snapshot.success);
    REQUIRE(snapshot.manifest["files"].size() == 2);

    const auto restored = store.restoreSnapshot(snapshot.snapshot_path, base / "restored");
    REQUIRE(restored.success);
    REQUIRE(readText(base / "restored" / "project.json") == R"({"name":"Demo"})");
    REQUIRE(readText(base / "restored" / "data" / "Map001.json") == R"({"id":1})");

    std::filesystem::remove_all(base);
}

TEST_CASE("project snapshot store rejects restore target that already exists", "[project][snapshot][ffs09]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_ffs09_snapshot_exists";
    std::filesystem::remove_all(base);
    writeText(base / "project" / "project.json", R"({"name":"Demo"})");
    std::filesystem::create_directories(base / "restore_target");

    const urpg::project::ProjectSnapshotStore store;
    const auto snapshot = store.createSnapshot(base / "project", base / "snapshots", "before_edit");
    REQUIRE(snapshot.success);

    const auto restored = store.restoreSnapshot(snapshot.snapshot_path, base / "restore_target");
    REQUIRE_FALSE(restored.success);
    REQUIRE(restored.errors == std::vector<std::string>{"restore_target_exists"});

    std::filesystem::remove_all(base);
}
