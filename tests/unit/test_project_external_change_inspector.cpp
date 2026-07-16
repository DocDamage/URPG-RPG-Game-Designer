#include "engine/core/project/project_external_change_inspector.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

using urpg::project::ProjectExternalChangeKind;
using urpg::project::ProjectExternalDocumentBaseline;
using urpg::project::ProjectExternalResolution;
using urpg::project::inspectProjectExternalChange;

namespace {

void write(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary | std::ios::trunc) << content;
}

} // namespace

TEST_CASE("External change inspector distinguishes unchanged and dirty modified documents",
          "[project][external_change]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_external_change_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto path = root / "dialogue.json";
    const std::string original = R"({"schema_version":"v1","text":"hello"})";
    write(path, original);
    auto baseline = ProjectExternalDocumentBaseline{path, original, original, false, true};
    REQUIRE(inspectProjectExternalChange(baseline).kind == ProjectExternalChangeKind::Unchanged);
    baseline.local_content = R"({"schema_version":"v1","text":"local"})";
    baseline.dirty = true;
    write(path, R"({"schema_version":"v1","text":"external"})");
    const auto conflict = inspectProjectExternalChange(baseline);
    REQUIRE(conflict.success);
    REQUIRE(conflict.kind == ProjectExternalChangeKind::Modified);
    REQUIRE(conflict.has_unsaved_conflict);
    REQUIRE(conflict.code == "project_external_change_dirty_conflict");
    REQUIRE(conflict.available_resolutions ==
            std::vector<ProjectExternalResolution>{ProjectExternalResolution::Compare,
                                                   ProjectExternalResolution::KeepLocal,
                                                   ProjectExternalResolution::Reload});
    std::filesystem::remove_all(root, error);
}

TEST_CASE("External change inspector detects rename and deletion without discarding local state",
          "[project][external_change]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_external_rename_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto oldPath = root / "old.json";
    const auto newPath = root / "new.json";
    const std::string original = R"({"schema_version":"v1"})";
    write(newPath, original);
    const ProjectExternalDocumentBaseline baseline{oldPath, original, "dirty local", true, true};
    const auto renamed = inspectProjectExternalChange(baseline, {newPath});
    REQUIRE(renamed.kind == ProjectExternalChangeKind::Renamed);
    REQUIRE(renamed.observed_path == newPath);
    REQUIRE(renamed.has_unsaved_conflict);
    std::filesystem::remove(newPath, error);
    const auto deleted = inspectProjectExternalChange(baseline, {newPath});
    REQUIRE(deleted.kind == ProjectExternalChangeKind::Deleted);
    REQUIRE(deleted.has_unsaved_conflict);
    REQUIRE(deleted.available_resolutions ==
            std::vector<ProjectExternalResolution>{ProjectExternalResolution::Compare,
                                                   ProjectExternalResolution::KeepLocal});
    std::filesystem::remove_all(root, error);
}

TEST_CASE("External change inspector rejects malformed JSON reload", "[project][external_change]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_external_malformed_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto path = root / "menu.json";
    write(path, "{not-json");
    const auto malformed = inspectProjectExternalChange(
        ProjectExternalDocumentBaseline{path, R"({"schema":"v1"})", R"({"schema":"v1"})", false, true});
    REQUIRE(malformed.success);
    REQUIRE(malformed.kind == ProjectExternalChangeKind::Malformed);
    REQUIRE(malformed.available_resolutions ==
            std::vector<ProjectExternalResolution>{ProjectExternalResolution::Compare,
                                                   ProjectExternalResolution::KeepLocal});
    REQUIRE_FALSE(malformed.diagnostics.empty());
    std::filesystem::remove_all(root, error);
}
