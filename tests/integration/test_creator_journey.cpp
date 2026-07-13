#include "editor/assets/asset_library_model.h"
#include "editor/assets/editor_asset_drag_payload.h"
#include "engine/core/editor/editor_shell.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/project/project_snapshot_store.h"
#include "engine/core/project/project_template_generator.h"
#include "engine/core/tools/export_packager.h"

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

nlohmann::json partialStep(const std::string& id, const std::string& diagnostic, const std::string& artifact) {
    return {{"id", id},
            {"status", "partial"},
            {"duration_ms", 0},
            {"diagnostic_codes", nlohmann::json::array({diagnostic})},
            {"artifact_paths", nlohmann::json::array({artifact})}};
}

void writeExternalCatalogFixture(const std::filesystem::path& catalogRoot) {
    std::filesystem::create_directories(catalogRoot);
    const auto shard = catalogRoot / "catalog-creator-00001.jsonl";
    {
        std::ofstream output(shard, std::ios::binary);
        output << R"({"asset_id":"local:creator-hero","virtual_path":"external/creator/hero.png","source_root":"external/creator","filename":"hero.png","extension":"png","media_kind":"image","archive_kind":"","size_bytes":128,"mtime_ns":1,"sha256":"","pack":"creator","category":"characters","tags":["hero"],"normalized_filename":"hero.png","normalized_virtual_path":"external/creator/hero.png","normalized_extension":"png","normalized_pack":"creator","normalized_category":"characters","normalized_tags":["hero"]})" << '\n';
    }
    const nlohmann::json metadata = {{"schema_version", "urpg.asset_catalog.v1"},
                                     {"generated_at", "2026-07-13T00:00:00Z"},
                                     {"scan_complete", true},
                                     {"counts", {{"asset_count", 1}, {"hash_pending_count", 1}, {"archive_count", 0}}},
                                     {"roots", nlohmann::json::array({{{"id", "external/creator"}, {"state", "complete"}, {"asset_count", 1}, {"hash_pending_count", 1}}})},
                                     {"shards", nlohmann::json::array({{{"path", shard.filename().generic_string()}, {"record_count", 1}}})}};
    std::ofstream output(catalogRoot / "catalog_meta.json", std::ios::binary);
    output << metadata.dump(2) << '\n';
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

    writeExternalCatalogFixture(project.root() / ".urpg" / "asset-index");
    urpg::editor::AssetLibraryModel assetLibrary;
    std::string catalogError;
    REQUIRE(assetLibrary.loadExternalCatalog(project.root() / ".urpg" / "asset-index", &catalogError));
    urpg::assets::LocalAssetCatalogQuery assetQuery;
    assetQuery.text = "hero";
    assetLibrary.setExternalCatalogQuery(assetQuery);
    REQUIRE(assetLibrary.snapshot().external_catalog["page"]["total_matches"] == 1);

    const urpg::editor::EditorAssetDragPayload rawAsset{"local:creator-hero", "", "image", 48, 48,
                                                        urpg::editor::EditorAssetProvenanceState::RawExternal};
    const auto rawDrop = urpg::editor::assessEditorAssetDrop(rawAsset, true);
    REQUIRE_FALSE(rawDrop.accepted);
    REQUIRE(rawDrop.code == "asset_drop_requires_attachment");
    const urpg::project::ProjectSnapshotStore snapshots;
    const auto snapshot = snapshots.createSnapshot(project.root(), project.snapshotRoot(), "before_playtest");
    REQUIRE(snapshot.success);

    const auto packageOutput = project.root() / "package-preview";
    std::filesystem::create_directories(packageOutput);
    urpg::tools::ExportConfig packageConfig{};
    packageConfig.target = urpg::tools::ExportTarget::Windows_x64;
    packageConfig.outputDir = packageOutput.generic_string();
    const auto packageValidation = urpg::tools::ExportPackager{}.validateBeforeExport(packageConfig);
    REQUIRE(packageValidation.passed);
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
        } else if (id == "discover_external_assets") {
            report["steps"].push_back(passedStep(id, "local_asset_catalog_page"));
        } else if (id == "attach_sprite") {
            report["steps"].push_back(partialStep(id, rawDrop.code, "editor_asset_drag_payload"));
        } else if (id == "package_project") {
            report["steps"].push_back(partialStep(id, "package_preview_validated", packageOutput.generic_string()));
        } else {
            report["steps"].push_back(partialStep(id, "workflow_not_integrated", ""));
        }
    }

    REQUIRE(report["steps"].size() == fixture["steps"].size());
    writeReport(report);
}
