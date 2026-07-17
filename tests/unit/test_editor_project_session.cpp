#include "editor/project/editor_project_session.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

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

    void writeDocument(const std::filesystem::path& relative, const std::string& payload) const {
        std::filesystem::create_directories((root_ / relative).parent_path());
        std::ofstream output(root_ / relative, std::ios::binary);
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
    std::string closed_id;
    session.addSwitchListener([&notified_id](const urpg::editor::EditorProjectIdentity& identity) {
        notified_id = identity.project_id;
    });
    session.addCloseListener([&closed_id](const urpg::editor::EditorProjectIdentity& identity) {
        closed_id = identity.project_id;
    });

    const auto result = session.openProject(project.root());
    REQUIRE(result.success);
    REQUIRE(result.code == "project_opened");
    REQUIRE(session.isOpen());
    REQUIRE(session.activeProject().root == std::filesystem::weakly_canonical(project.root()));
    REQUIRE(session.activeProject().display_name == "Creator Demo");
    REQUIRE(notified_id == "creator_demo");
    session.setDirtySurfaceSummaries({"map:opening", "ability:fire"});
    REQUIRE(session.dirtySurfaceSummaries().size() == 2);
    REQUIRE(session.closeProject().success);
    REQUIRE(closed_id == "creator_demo");
    REQUIRE(session.dirtySurfaceSummaries().empty());
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

TEST_CASE("EditorProjectSession pre-open inspection validates identity without switching session",
          "[project][project session][preflight]") {
    TempProjectRoot project;
    project.writeManifest(
        R"({"schema_version":"urpg.project.v1","project_id":"health_check","project_name":"Health Check"})");
    urpg::editor::EditorProjectSession session;
    int notifications = 0;
    session.addSwitchListener([&](const auto&) { ++notifications; });
    const auto inspection = session.inspectProject(project.root());
    REQUIRE(inspection.result.success);
    REQUIRE(inspection.identity.project_id == "health_check");
    REQUIRE(inspection.identity.display_name == "Health Check");
    REQUIRE_FALSE(session.isOpen());
    REQUIRE(notifications == 0);
    REQUIRE(session.lastDiagnostic().code.empty());
}

TEST_CASE("EditorProjectSession migrates durable legacy documents before owner notification",
          "[project][project session][migration]") {
    TempProjectRoot project;
    project.writeManifest(
        R"({"schema_version":"urpg.project.v1","project_id":"legacy","project_name":"Legacy"})");
    project.writeDocument("content/abilities/fire.json", R"({"ability_id":"fire"})");
    urpg::editor::EditorProjectSession session;
    bool listenerSawCurrentSchema = false;
    session.addSwitchListener([&](const auto&) {
        std::ifstream input(project.root() / "content" / "abilities" / "fire.json", std::ios::binary);
        const auto value = nlohmann::json::parse(input);
        listenerSawCurrentSchema = value.value("schema", "") == "urpg.ability.v1";
    });

    const auto result = session.openProject(project.root());
    REQUIRE(result.success);
    REQUIRE(result.code == "project_opened_after_migration");
    REQUIRE(listenerSawCurrentSchema);
}
