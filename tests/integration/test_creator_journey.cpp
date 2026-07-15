#include "editor/assets/asset_library_model.h"
#include "editor/assets/editor_asset_drag_payload.h"
#include "editor/playtest/playtest_session_controller.h"
#include "editor/project/editor_dirty_state_registry.h"
#include "editor/project/editor_project_session.h"
#include "engine/core/assets/project_asset_attachment_service.h"
#include "engine/core/editor/editor_shell.h"
#include "engine/core/map/grid_part_commands.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/project/project_creation_service.h"
#include "engine/core/project/project_snapshot_store.h"
#include "engine/core/project/project_template_generator.h"
#include "engine/core/security/sha256.h"
#include "engine/core/tools/export_packager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

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

std::string hashFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    const std::vector<std::uint8_t> bytes(std::istreambuf_iterator<char>(input), {});
    return urpg::security::Sha256::toHex(urpg::security::Sha256::compute(bytes));
}

std::filesystem::path writeQualificationArtifact(const std::filesystem::path& root,
                                                 const std::string& id,
                                                 const nlohmann::json& value) {
    const auto path = root / (id + ".json");
    std::ofstream output(path, std::ios::binary);
    output << value.dump(2) << '\n';
    REQUIRE(output.good());
    return path;
}

nlohmann::json qualificationEvidence(const std::string& kind,
                                     const std::filesystem::path& artifact,
                                     const std::string& sourceCommit) {
    return {{"kind", kind},
            {"artifact_path", artifact.generic_string()},
            {"sha256", hashFile(artifact)},
            {"source_commit", sourceCommit}};
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

TEST_CASE("creator journey qualification emits native target evidence when wrapper provenance is present",
          "[integration][creator journey][creator journey qualification]") {
    const auto buildRoot = std::filesystem::path(URPG_BINARY_DIR);
    const auto provenancePath = buildRoot / "creator_journey_qualification_provenance.json";
    if (!std::filesystem::is_regular_file(provenancePath)) {
        SUCCEED("Target qualification is emitted only by the clean PFU-I1 wrapper.");
        return;
    }

    std::ifstream provenanceInput(provenancePath, std::ios::binary);
    const auto provenance = nlohmann::json::parse(provenanceInput, nullptr, false);
    REQUIRE_FALSE(provenance.is_discarded());
    REQUIRE(provenance.value("schema", "") == "urpg.creator_journey_qualification_provenance.v1");
    const auto sourceCommit = provenance.value("source_commit", "");
    REQUIRE_FALSE(sourceCommit.empty());
    REQUIRE(provenance.value("clean_worktree", false));
    REQUIRE(provenance.contains("builds"));

    const auto evidenceRoot = buildRoot / "creator_journey_qualification_evidence";
    std::error_code error;
    std::filesystem::remove_all(evidenceRoot, error);
    std::filesystem::create_directories(evidenceRoot, error);
    REQUIRE_FALSE(error);

    urpg::editor::EditorShell shell;
    REQUIRE(shell.start(true));
    REQUIRE(shell.isRunning());
    const auto launchArtifact = writeQualificationArtifact(
        evidenceRoot, "launch_editor", {{"owner", "EditorShell"}, {"headless", true}, {"running", shell.isRunning()}});

    urpg::project::ProjectCreationRequest request;
    request.project_id = "qualification_project";
    request.project_name = "PFU I1 Qualification";
    request.destination = evidenceRoot / "project";
    request.starter_map = "qualification_map";
    const auto created = urpg::project::ProjectCreationService{}.createProject(request);
    REQUIRE(created.success);
    urpg::editor::EditorProjectSession session;
    REQUIRE(session.openProject(created.project_root).success);
    const auto projectArtifact = writeQualificationArtifact(
        evidenceRoot, "create_project",
        {{"owner", "ProjectCreationService"}, {"result", created.code}, {"project_root", created.project_root.generic_string()},
         {"session", session.lastDiagnostic().code}});

    const auto promotedPayload = evidenceRoot / "promoted" / "hero.png";
    std::filesystem::create_directories(promotedPayload.parent_path());
    std::ofstream promotedOutput(promotedPayload, std::ios::binary);
    promotedOutput << "qualification-hero";
    REQUIRE(promotedOutput.good());
    urpg::assets::AssetPromotionManifest manifest;
    manifest.assetId = "qualification.hero";
    manifest.sourcePath = "reviewed/qualification/hero.png";
    manifest.promotedPath = promotedPayload.generic_string();
    manifest.licenseId = "user_license_note";
    manifest.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    manifest.preview.kind = "image";
    manifest.preview.thumbnailPath = promotedPayload.generic_string();
    manifest.preview.width = 48;
    manifest.preview.height = 48;
    manifest.package.includeInRuntime = true;
    const auto attached = urpg::assets::ProjectAssetAttachmentService{}.attachPromotedAsset(manifest, created.project_root);
    REQUIRE(attached.success);

    urpg::map::GridPartDocument map("qualification_map", 8, 8);
    urpg::map::PlacedPartInstance tile;
    tile.instance_id = "qualification_map:floor:2:3";
    tile.part_id = "floor";
    tile.category = urpg::map::GridPartCategory::Tile;
    tile.layer = urpg::map::GridPartLayer::Terrain;
    tile.grid_x = 2;
    tile.grid_y = 3;
    urpg::map::GridPartCommandHistory history;
    REQUIRE(history.execute(map, std::make_unique<urpg::map::PlacePartCommand>(tile)));
    REQUIRE(history.undo(map));
    REQUIRE(history.redo(map));
    urpg::editor::EditorDirtyStateRegistry dirtyRegistry;
    urpg::editor::EditorDirtySurface dirtySurface;
    dirtySurface.document_id = "qualification_map";
    dirtySurface.focus_route = "map_authoring";
    dirtySurface.save = [] { return urpg::editor::EditorDirtySaveResult{true, "map_saved", "saved"}; };
    REQUIRE(dirtyRegistry.registerSurface(std::move(dirtySurface)));
    REQUIRE(dirtyRegistry.markDirty("qualification_map"));
    REQUIRE(dirtyRegistry.save("qualification_map").success);
    const auto mapArtifact = writeQualificationArtifact(
        evidenceRoot, "author_map",
        {{"owner", "GridPartDocument"}, {"parts", static_cast<int>(map.parts().size())},
         {"history", {{"undo", history.canUndo()}, {"redo", history.canRedo()}}},
         {"dirty_state", "map_saved"}});
    const auto assetArtifact = writeQualificationArtifact(
        evidenceRoot, "attach_asset",
        {{"owner", "ProjectAssetAttachmentService"}, {"result", attached.code},
         {"payload", attached.payloadPath.generic_string()}, {"manifest", attached.manifestPath.generic_string()},
         {"history_owner", "GridPartCommandHistory"}});

    urpg::editor::PlaytestSessionController playtest(URPG_RUNTIME_PATH);
    REQUIRE(playtest.start(created.project_root, "qualification_map", "4,6", "{\"grid\":true}\n", "{\"p2d\":true}\n"));
    const auto playtestSession = playtest.sessionDirectory();
    REQUIRE(std::filesystem::is_regular_file(playtestSession / "session.json"));
    playtest.returnToEditor();
    REQUIRE(playtest.state() == urpg::editor::PlaytestSessionState::Returned);
    const auto playtestArtifact = writeQualificationArtifact(
        evidenceRoot, "playtest_and_return",
        {{"owner", "PlaytestSessionController"}, {"session", playtestSession.generic_string()},
         {"state", "returned"}});

    const auto snapshot = urpg::project::ProjectSnapshotStore{}.createSnapshot(
        created.project_root, evidenceRoot / "snapshots", "qualification_save");
    REQUIRE(snapshot.success);
    urpg::tools::ExportConfig packageConfig{};
    packageConfig.target = urpg::tools::ExportTarget::Windows_x64;
    packageConfig.outputDir = (evidenceRoot / "package").generic_string();
    REQUIRE(urpg::tools::ExportPackager{}.validateBeforeExport(packageConfig).passed);
    const auto packageEvidence = buildRoot / "creator_journey_qualification_package_smoke.json";
    REQUIRE(std::filesystem::is_regular_file(packageEvidence));
    const auto saveArtifact = writeQualificationArtifact(
        evidenceRoot, "save_validate_and_package",
        {{"owner", "ProjectSnapshotStore"}, {"snapshot", snapshot.snapshot_path.generic_string()},
         {"validation", "passed"}, {"package_smoke", packageEvidence.generic_string()}});

    nlohmann::json report = {{"schema", "urpg.creator_journey_qualification_report.v1"},
                             {"qualification_id", "pfu_i1_native_creator_baseline"},
                             {"status", "passed"},
                             {"provenance", provenance},
                             {"steps", nlohmann::json::array()}};
    report["steps"] = {
        {{"id", "launch_editor"}, {"status", "passed"}, {"duration_ms", 0}, {"diagnostic_codes", nlohmann::json::array()},
         {"evidence", nlohmann::json::array({qualificationEvidence("editor_startup", launchArtifact, sourceCommit)})}},
        {{"id", "create_project"}, {"status", "passed"}, {"duration_ms", 0}, {"diagnostic_codes", nlohmann::json::array()},
         {"evidence", nlohmann::json::array({qualificationEvidence("native_project_creation", projectArtifact, sourceCommit), qualificationEvidence("project_validation", projectArtifact, sourceCommit)})}},
        {{"id", "attach_project_asset"}, {"status", "passed"}, {"duration_ms", 0}, {"diagnostic_codes", nlohmann::json::array()},
         {"evidence", nlohmann::json::array({qualificationEvidence("governed_asset_attachment", assetArtifact, sourceCommit), qualificationEvidence("undo_history", assetArtifact, sourceCommit)})}},
        {{"id", "author_map"}, {"status", "passed"}, {"duration_ms", 0}, {"diagnostic_codes", nlohmann::json::array()},
         {"evidence", nlohmann::json::array({qualificationEvidence("map_authoring", mapArtifact, sourceCommit), qualificationEvidence("native_dirty_state", mapArtifact, sourceCommit)})}},
        {{"id", "playtest_and_return"}, {"status", "passed"}, {"duration_ms", 0}, {"diagnostic_codes", nlohmann::json::array()},
         {"evidence", nlohmann::json::array({qualificationEvidence("editor_playtest_session", playtestArtifact, sourceCommit), qualificationEvidence("editor_return", playtestArtifact, sourceCommit)})}},
        {{"id", "save_validate_and_package"}, {"status", "passed"}, {"duration_ms", 0}, {"diagnostic_codes", nlohmann::json::array()},
         {"evidence", nlohmann::json::array({qualificationEvidence("native_save", saveArtifact, sourceCommit), qualificationEvidence("project_validation", saveArtifact, sourceCommit), qualificationEvidence("package_smoke", packageEvidence, sourceCommit)})}},
    };
    const auto reportPath = buildRoot / "creator_journey_qualification_report.json";
    std::ofstream output(reportPath, std::ios::binary);
    output << report.dump(2) << '\n';
    REQUIRE(output.good());
    shell.shutdown();
}
