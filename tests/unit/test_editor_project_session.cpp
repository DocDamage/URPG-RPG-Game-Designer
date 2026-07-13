#include "editor/project/editor_project_session.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TempProjectRoot {
  public:
    TempProjectRoot() {
        root_ = std::filesystem::temp_directory_path() /
                ("urpg_editor_project_session_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(root_);
    }

    ~TempProjectRoot() {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }

    const std::filesystem::path& root() const { return root_; }

    void writeManifest(const std::string& payload) const {
        std::ofstream output(root_ / "project.json", std::ios::binary);
        output << payload;
    }

  private:
    std::filesystem::path root_;
};

} // namespace

TEST_CASE("EditorProjectSession commits validated projects before notifying panels", "[project][project session]") {
    TempProjectRoot project;
    project.writeManifest(R"({"schema_version":"urpg.project.v1","project_id":"creator_demo","project_name":"Creator Demo"})");

    urpg::editor::EditorProjectSession session;
    std::string notified_id;
    session.addSwitchListener([&notified_id](const urpg::editor::EditorProjectIdentity& identity) {
        notified_id = identity.project_id;
    });

    const auto result = session.openProject(project.root());
    REQUIRE(result.success);
    REQUIRE(result.code == "project_opened");
    REQUIRE(session.isOpen());
    REQUIRE(session.activeProject().root == std::filesystem::weakly_canonical(project.root()));
    REQUIRE(session.activeProject().display_name == "Creator Demo");
    REQUIRE(notified_id == "creator_demo");
}

TEST_CASE("EditorProjectSession preserves the active project after a failed switch", "[project][project session]") {
    TempProjectRoot project;
    project.writeManifest(R"({"schema_version":"urpg.project.v1","project_id":"stable","project_name":"Stable"})");

    urpg::editor::EditorProjectSession session;
    REQUIRE(session.openProject(project.root()).success);
    const auto original_root = session.activeProject().root;

    const auto result = session.openProject(project.root() / "missing");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.code == "project_root_missing");
    REQUIRE(session.isOpen());
    REQUIRE(session.activeProject().root == original_root);
    REQUIRE(session.lastDiagnostic().code == "project_root_missing");
}

TEST_CASE("EditorProjectSession rejects malformed and incomplete manifests", "[project][project session]") {
    TempProjectRoot malformed;
    malformed.writeManifest("{ not json");
    urpg::editor::EditorProjectSession session;
    REQUIRE(session.openProject(malformed.root()).code == "project_manifest_invalid");

    TempProjectRoot incomplete;
    incomplete.writeManifest(R"({"project_id":"missing_fields"})");
    REQUIRE(session.openProject(incomplete.root()).code == "project_manifest_incomplete");
    REQUIRE_FALSE(session.isOpen());
}
