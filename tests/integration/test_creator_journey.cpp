#include "engine/core/editor/editor_shell.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/project/project_snapshot_store.h"
#include "engine/core/project/project_template_generator.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

namespace {

constexpr const char* kReportSchema = "urpg.creator_journey_report.v1";

class TempJourneyProject {
  public:
    TempJourneyProject() {
        const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        base_ = std::filesystem::temp_directory_path() / ("urpg_creator_journey_" + unique);
        root_ = base_ / "project";
        std::filesystem::create_directories(root_ / "content");
    }

    ~TempJourneyProject() {
        std::error_code error;
        std::filesystem::remove_all(base_, error);
    }

    const std::filesystem::path& root() const { return root_; }
    std::filesystem::path snapshotRoot() const { return base_ / "snapshots"; }

    void writeProject(const nlohmann::json& project) const {
        std::ofstream output(root_ / "project.json", std::ios::binary);
        output << project.dump(2) << '\n';
    }

  private:
    std::filesystem::path base_;
    std::filesystem::path root_;
};

nlohmann::json loadFixture() {
    std::ifstream input(std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" / "creator_journey_spec.json",
                        std::ios::binary);
    return nlohmann::json::parse(input);
}

void writeReport(const nlohmann::json& report) {
    const auto path = std::filesystem::path(URPG_BINARY_DIR) / "creator_journey_report.json";
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary);
    output << report.dump(2) << '\n';
    REQUIRE(output.good());
}

nlohmann::json passedStep(const std::string& id, const std::string& artifact) {
    return {{"id", id}, {"status", "passed"}, {"duration_ms", 0}, {"diagnostic_codes", nlohmann::json::array()},
            {"artifact_paths", nlohmann::json::array({artifact})}};
}

nlohmann::json deferredStep(const nlohmann::json& specification) {
    return {{"id", specification.at("id")},
            {"status", "deferred"},
            {"duration_ms", 0},
            {"diagnostic_codes", nlohmann::json::array({specification.at("reason")})},
            {"artifact_paths", nlohmann::json::array()}};
}

} // namespace

TEST_CASE("creator journey baseline emits an honest deterministic smoke report", "[integration][creator journey]") {
    const auto fixture = loadFixture();
    REQUIRE(fixture.value("schema", "") == "urpg.creator_journey.v1");
    REQUIRE(fixture.contains("steps"));
    REQUIRE(fixture["steps"].is_array());

    std::set<std::string> ids;
    for (const auto& step : fixture["steps"]) {
        REQUIRE(step.contains("id"));
        REQUIRE(ids.insert(step.at("id").get<std::string>()).second);
        REQUIRE(step.value("max_clicks", 0) > 0);
        REQUIRE(step.value("max_duration_ms", 0) > 0);
        if (step.value("baseline_status", "") == "deferred") {
            REQUIRE_FALSE(step.value("reason", "").empty());
            REQUIRE_FALSE(step.value("milestone", "").empty());
        }
    }

    urpg::editor::EditorShell shell;
    REQUIRE(shell.start(true));
    REQUIRE(shell.isRunning());

    urpg::project::ProjectTemplateGenerator generator;
    const auto created = generator.generate({"jrpg", "creator_journey", "Creator Journey"});
    REQUIRE(created.success);
    REQUIRE(generator.validateProjectDocument(created.project).empty());
    REQUIRE(created.project["subsystems"]["maps"][0].contains("spawn"));

    urpg::map::GridPartDocument map("creator_journey_start", 8, 8);
    urpg::map::PlacedPartInstance tile;
    tile.instance_id = "creator_journey_start:floor:2:3";
    tile.part_id = "floor";
    tile.category = urpg::map::GridPartCategory::Tile;
    tile.layer = urpg::map::GridPartLayer::Terrain;
    tile.grid_x = 2;
    tile.grid_y = 3;
    REQUIRE(map.placePart(tile));

    const TempJourneyProject project;
    project.writeProject(created.project);
    const urpg::project::ProjectSnapshotStore snapshots;
    const auto snapshot = snapshots.createSnapshot(project.root(), project.snapshotRoot(), "before_playtest");
    REQUIRE(snapshot.success);
    shell.shutdown();

    nlohmann::json report = {{"schema", kReportSchema},
                             {"journey_id", fixture.at("journey_id")},
                             {"status", "baseline"},
                             {"steps", nlohmann::json::array()}};
    for (const auto& step : fixture["steps"]) {
        const auto id = step.at("id").get<std::string>();
        if (step.value("baseline_status", "") == "deferred") {
            report["steps"].push_back(deferredStep(step));
        } else if (id == "launch_editor" || id == "return_to_editor") {
            report["steps"].push_back(passedStep(id, "editor_shell"));
        } else if (id == "create_project" || id == "validate_project" || id == "choose_spawn") {
            report["steps"].push_back(passedStep(id, "project_template_contract"));
        } else if (id == "paint_map") {
            report["steps"].push_back(passedStep(id, "grid_part_document"));
        } else if (id == "save_project") {
            report["steps"].push_back(passedStep(id, snapshot.snapshot_path.generic_string()));
        } else {
            report["steps"].push_back({{"id", id},
                                       {"status", "partial"},
                                       {"duration_ms", 0},
                                       {"diagnostic_codes", nlohmann::json::array({"workflow_not_integrated"})},
                                       {"artifact_paths", nlohmann::json::array()}});
        }
    }

    REQUIRE(report["steps"].size() == fixture["steps"].size());
    writeReport(report);
}
