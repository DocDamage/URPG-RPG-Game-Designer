#include "editor/project/project_external_change_coordinator.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

void write(const std::filesystem::path& path, const std::string& contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << contents;
}

} // namespace

TEST_CASE("External change coordinator preserves dirty work through compare and keep-local",
          "[project][external_change][session][pcq506]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_external_coordinator_keep_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto path = root / "map.json";
    write(path, R"({"value":"saved"})");
    std::string local = R"({"value":"local"})";
    bool dirty = true;
    urpg::editor::ProjectExternalChangeCoordinator coordinator;
    REQUIRE(coordinator.registerDocument({"map", {path, R"({"value":"saved"})", local, dirty, true},
        [&] { return local; }, [&] { return dirty; },
        [&](const std::string& contents, const std::filesystem::path&, std::string&) {
            local = contents; dirty = false; return true;
        }}));
    write(path, R"({"value":"external"})");
    REQUIRE(coordinator.inspect().size() == 1);
    const auto compared = coordinator.resolve("map", urpg::project::ProjectExternalResolution::Compare);
    REQUIRE(compared.success);
    REQUIRE(compared.local_content == R"({"value":"local"})");
    REQUIRE(compared.external_content == R"({"value":"external"})");
    REQUIRE(local == R"({"value":"local"})");
    REQUIRE(coordinator.resolve("map", urpg::project::ProjectExternalResolution::KeepLocal).success);
    REQUIRE(local == R"({"value":"local"})");
    REQUIRE(dirty);
    REQUIRE(coordinator.inspect().empty());
    write(path, local);
    REQUIRE(coordinator.acknowledgeSaved("map", local, path));
    REQUIRE(coordinator.inspect().empty());
    std::filesystem::remove_all(root, error);
}

TEST_CASE("External change coordinator reloads valid renamed files but refuses malformed reload",
          "[project][external_change][session]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_external_coordinator_reload_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto oldPath = root / "old.json";
    const auto newPath = root / "new.json";
    const std::string saved = R"({"value":"saved"})";
    write(oldPath, saved);
    std::string local = saved;
    bool dirty = false;
    urpg::editor::ProjectExternalChangeCoordinator coordinator;
    REQUIRE(coordinator.registerDocument({"dialogue", {oldPath, saved, local, dirty, true},
        [&] { return local; }, [&] { return dirty; },
        [&](const std::string& contents, const std::filesystem::path& path, std::string&) {
            local = contents; dirty = false; return path == newPath;
        }}));
    std::filesystem::rename(oldPath, newPath);
    REQUIRE(coordinator.inspect({{"dialogue", {newPath}}})[0].inspection.kind ==
            urpg::project::ProjectExternalChangeKind::Renamed);
    REQUIRE(coordinator.resolve("dialogue", urpg::project::ProjectExternalResolution::Reload).success);
    write(newPath, "{ malformed");
    REQUIRE(coordinator.inspect().size() == 1);
    const auto rejected = coordinator.resolve("dialogue", urpg::project::ProjectExternalResolution::Reload);
    REQUIRE_FALSE(rejected.success);
    REQUIRE(rejected.code == "project_external_resolution_unavailable");
    REQUIRE(local == saved);
    std::filesystem::remove_all(root, error);
}

TEST_CASE("External change coordinator routes renamed stable identity changes through impact review",
          "[project][external_change][reference_impact]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_external_coordinator_identity_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto oldPath = root / "old.json";
    const auto newPath = root / "new.json";
    const std::string saved = R"({"dialogue_id":"intro","nodes":[]})";
    const std::string changed = R"({"dialogue_id":"renamed_intro","nodes":[]})";
    write(oldPath, saved);
    std::string local = saved;
    bool reloaded = false;
    urpg::editor::ProjectExternalChangeCoordinator coordinator;
    REQUIRE(coordinator.registerDocument({"dialogue", {oldPath, saved, local, false, true},
        [&] { return local; }, [] { return false; },
        [&](const std::string&, const std::filesystem::path&, std::string&) { reloaded = true; return true; }}));
    std::filesystem::remove(oldPath);
    write(newPath, changed);
    REQUIRE(coordinator.inspect({{"dialogue", {newPath}}}).size() == 1);

    const auto result = coordinator.resolve("dialogue", urpg::project::ProjectExternalResolution::Reload);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "project_external_identity_impact_review_required");
    REQUIRE(result.previous_stable_id == "intro");
    REQUIRE(result.external_stable_id == "renamed_intro");
    REQUIRE_FALSE(reloaded);
    REQUIRE(coordinator.conflicts().size() == 1);
    std::filesystem::remove_all(root, error);
}
