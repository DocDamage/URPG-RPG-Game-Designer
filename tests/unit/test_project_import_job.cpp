#include "editor/project/project_import_job.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

namespace {

class TempImportRoot {
public:
    TempImportRoot() {
        root_ = std::filesystem::temp_directory_path() /
                ("urpg_project_import_" +
                 std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(root_ / "source" / "maps");
        std::filesystem::create_directories(root_ / "source" / ".urpg" / "recovery");
        write(root_ / "source" / "project.json",
              R"({"schema_version":"urpg.project.v1","project_id":"source","project_name":"Source"})");
        write(root_ / "source" / "maps" / "opening.json", R"({"id":"opening"})");
        write(root_ / "source" / ".urpg" / "recovery" / "private.json", "private");
    }

    ~TempImportRoot() {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }

    const std::filesystem::path& root() const { return root_; }

    static void write(const std::filesystem::path& path, const std::string& payload) {
        std::ofstream output(path, std::ios::binary);
        output << payload;
    }

private:
    std::filesystem::path root_;
};

} // namespace

TEST_CASE("ProjectImportJob clones in bounded slices and publishes only after validation",
          "[project][import][startup]") {
    TempImportRoot files;
    const auto destination = files.root() / "imported";
    urpg::editor::ProjectImportJob job({files.root() / "source", destination, "imported_id", "Imported Game"});

    std::size_t calls = 0;
    while (!job.snapshot().complete && calls++ < 32) {
        REQUIRE_FALSE(std::filesystem::exists(destination));
        (void)job.advance(1);
        REQUIRE(job.snapshot().last_slice_items <= 1);
    }

    const auto snapshot = job.snapshot();
    REQUIRE(snapshot.complete);
    REQUIRE(snapshot.success);
    REQUIRE(snapshot.state == urpg::editor::ProjectImportJobState::Completed);
    REQUIRE(std::filesystem::is_regular_file(destination / "maps" / "opening.json"));
    REQUIRE_FALSE(std::filesystem::exists(destination / ".urpg"));
    REQUIRE(std::filesystem::is_regular_file(files.root() / "source" / "maps" / "opening.json"));

    std::ifstream input(destination / "project.json", std::ios::binary);
    const auto manifest = nlohmann::json::parse(input);
    REQUIRE(manifest["project_id"] == "imported_id");
    REQUIRE(manifest["project_name"] == "Imported Game");
    REQUIRE(manifest["schema_version"] == "urpg.project.v1");
}

TEST_CASE("ProjectImportJob rejects destinations that exist or are inside the source", "[project][import]") {
    TempImportRoot files;
    const auto existing = files.root() / "existing";
    std::filesystem::create_directory(existing);

    urpg::editor::ProjectImportJob existingJob(
        {files.root() / "source", existing, "new_id", "New Project"});
    REQUIRE_FALSE(existingJob.advance());
    REQUIRE(existingJob.snapshot().code == "project_import_destination_exists");

    urpg::editor::ProjectImportJob nestedJob(
        {files.root() / "source", files.root() / "source" / "nested", "new_id", "New Project"});
    REQUIRE_FALSE(nestedJob.advance());
    REQUIRE(nestedJob.snapshot().code == "project_import_destination_inside_source");
}

TEST_CASE("ProjectImportJob cancellation removes only its private staging directory", "[project][import]") {
    TempImportRoot files;
    const auto destination = files.root() / "cancelled";
    const auto stage = files.root() / "cancelled.urpg-import-stage";
    urpg::editor::ProjectImportJob job({files.root() / "source", destination, "cancelled", "Cancelled"});

    REQUIRE(job.advance(1));
    REQUIRE(std::filesystem::is_directory(stage));
    job.cancel();

    REQUIRE(job.snapshot().state == urpg::editor::ProjectImportJobState::Cancelled);
    REQUIRE_FALSE(std::filesystem::exists(stage));
    REQUIRE_FALSE(std::filesystem::exists(destination));
    REQUIRE(std::filesystem::is_directory(files.root() / "source"));
}
