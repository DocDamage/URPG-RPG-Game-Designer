#include "editor/export/export_diagnostics_panel.h"
#include "editor/export/export_preview_panel.h"
#include "engine/core/tools/export_packager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

using namespace urpg::editor;
using urpg::tools::ExportConfig;
using urpg::tools::ExportTarget;

namespace {

void WriteFile(const std::filesystem::path& path, const std::string& content = "") {
    std::ofstream out(path, std::ios::binary);
    out << content;
}

void CreateRealExportFixture(const std::filesystem::path& base, ExportTarget target) {
    std::filesystem::remove_all(base);

    urpg::tools::ExportPackager packager;
    ExportConfig config{};
    config.target = target;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);
    REQUIRE(result.success);
}

std::filesystem::path ExportPanelRepoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

nlohmann::json LoadExportPanelJson(const std::filesystem::path& path) {
    std::ifstream stream(path);
    REQUIRE(stream.is_open());
    nlohmann::json json;
    stream >> json;
    return json;
}

std::filesystem::path ExportSmokeRuntimePath() {
#ifdef URPG_EXPORT_SMOKE_APP_PATH
    return std::filesystem::path(URPG_EXPORT_SMOKE_APP_PATH);
#else
    return {};
#endif
}

} // namespace

TEST_CASE("ExportDiagnosticsPanel produces empty snapshot when no config set", "[export][editor][panel]") {
    ExportDiagnosticsPanel panel;
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.is_object());
    REQUIRE(snapshot["status"] == "disabled");
    REQUIRE(snapshot["disabled_reason"] == "No ExportConfig is set.");
    REQUIRE(snapshot["owner"] == "editor/export");
    REQUIRE(snapshot["readyToExport"] == false);
}

TEST_CASE("ExportDiagnosticsPanel snapshot reflects preflight readiness on a fresh directory", "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_errors";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    ExportDiagnosticsPanel panel;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    panel.setExportConfig(config);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["status"] == "ready");
    REQUIRE(snapshot["target"] == "Windows_x64");
    REQUIRE(snapshot["outputDir"] == base.string());
    REQUIRE(snapshot["validationSource"] == "packager_preflight");
    REQUIRE(snapshot["validationPassed"] == true);
    REQUIRE(snapshot["readyToExport"] == true);
    REQUIRE(snapshot["errors"].is_array());
    REQUIRE(snapshot["errors"].empty());
    REQUIRE(snapshot["postExportValidationPassed"] == false);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportDiagnosticsPanel snapshot shows ready-to-export true when preflight passes", "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_ready";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    ExportDiagnosticsPanel panel;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    panel.setExportConfig(config);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["target"] == "Windows_x64");
    REQUIRE(snapshot["outputDir"] == base.string());
    REQUIRE(snapshot["validationSource"] == "packager_preflight");
    REQUIRE(snapshot["validationPassed"] == true);
    REQUIRE(snapshot["readyToExport"] == true);
    REQUIRE(snapshot["errors"].is_array());
    REQUIRE(snapshot["errors"].empty());
    REQUIRE(snapshot["postExportValidationPassed"] == false);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportDiagnosticsPanel mirrors packager preflight for web exports", "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_web";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    ExportDiagnosticsPanel panel;
    ExportConfig config{};
    config.target = ExportTarget::Web_WASM;
    config.outputDir = base.string();
    panel.setExportConfig(config);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["target"] == "Web_WASM");
    REQUIRE(snapshot["validationSource"] == "packager_preflight");
    REQUIRE(snapshot["validationPassed"] == true);
    REQUIRE(snapshot["readyToExport"] == true);
    REQUIRE(snapshot["errors"].is_array());
    REQUIRE(snapshot["errors"].empty());
    REQUIRE(snapshot["postExportValidationPassed"] == false);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportDiagnosticsPanel surfaces emittedArtifacts when post-export tree is present", "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_emitted";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    ExportDiagnosticsPanel panel;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    panel.setExportConfig(config);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["postExportValidationPassed"] == true);
    REQUIRE(snapshot["emittedArtifacts"].is_array());
    REQUIRE_FALSE(snapshot["emittedArtifacts"].empty());

    bool foundExe = false;
    bool foundPck = false;
    for (const auto& item : snapshot["emittedArtifacts"]) {
        if (item == "game.exe") foundExe = true;
        if (item == "data.pck") foundPck = true;
    }
    REQUIRE(foundExe);
    REQUIRE(foundPck);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportDiagnosticsPanel post-export validation fails when emitted tree is incomplete", "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_incomplete";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "data.pck", "pck");

    ExportDiagnosticsPanel panel;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    panel.setExportConfig(config);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["validationPassed"] == true); // preflight passes
    REQUIRE(snapshot["readyToExport"] == true);
    REQUIRE(snapshot["postExportValidationPassed"] == false);
    REQUIRE(snapshot.contains("emittedArtifacts") == false);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportDiagnosticsPanel can report failed preflight alongside a valid emitted tree",
          "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_divergent_state";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    ExportDiagnosticsPanel panel;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.runtimeBinaryPath = (base / "missing_runtime.exe").string();
    panel.setExportConfig(config);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["validationPassed"] == false);
    REQUIRE(snapshot["readyToExport"] == false);
    REQUIRE(snapshot["postExportValidationPassed"] == true);
    REQUIRE(snapshot["errors"].is_array());
    REQUIRE_FALSE(snapshot["errors"].empty());
    REQUIRE(snapshot["emittedArtifacts"].is_array());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPreviewPanel produces exact release shipping manifest",
          "[export][editor][preview][wysiwyg]") {
    const auto runtime = ExportSmokeRuntimePath();
    REQUIRE_FALSE(runtime.empty());
    REQUIRE(std::filesystem::exists(runtime));

    auto document = urpg::exporting::ExportPreviewDocument::fromJson(
        LoadExportPanelJson(ExportPanelRepoRoot() / "content" / "fixtures" / "export_preview_fixture.json"));
    document.runtime_binary_path = runtime.string();

    const auto workspace = std::filesystem::temp_directory_path() / "urpg_export_preview_exact_ship";
    std::filesystem::remove_all(workspace);

    urpg::editor::ExportPreviewPanel panel;
    panel.loadDocument(document, workspace);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.snapshot().disabled);
    REQUIRE(panel.snapshot().preview_id == "windows_release_preview");
    REQUIRE(panel.snapshot().target == "Windows_x64");
    REQUIRE(panel.snapshot().mode == "release");
    REQUIRE(panel.snapshot().preflight_passed);
    REQUIRE(panel.snapshot().export_success);
    REQUIRE(panel.snapshot().post_export_validation_passed);
    REQUIRE(panel.snapshot().exact_ship_preview);
    REQUIRE(panel.snapshot().generated_file_count >= 2);
    REQUIRE(panel.snapshot().emitted_artifact_count >= 2);
    REQUIRE(panel.snapshot().expected_artifact_count == 2);
    REQUIRE(panel.snapshot().missing_expected_artifact_count == 0);
    REQUIRE(panel.snapshot().runtime_trace_count >= 5);
    REQUIRE(panel.snapshot().diagnostic_count == 0);
    REQUIRE(panel.snapshot().status_message == "Export preview is exactly what will ship.");
    REQUIRE(panel.snapshot().shipping_manifest["schema"] == "urpg.export_preview_manifest.v1");
    REQUIRE(panel.snapshot().shipping_manifest["exact_ship_preview"] == true);
    REQUIRE(panel.snapshot().shipping_manifest["missing_expected_artifacts"].empty());

    bool foundPck = false;
    bool foundExe = false;
    for (const auto& artifact : panel.result().emitted_artifacts) {
        foundPck = foundPck || artifact == "data.pck";
        foundExe = foundExe || artifact == "game.exe";
    }
    REQUIRE(foundPck);
    REQUIRE(foundExe);

    std::filesystem::remove_all(workspace);
}

TEST_CASE("ExportPreviewPanel edits ship settings and blocks missing expected artifacts",
          "[export][editor][preview][wysiwyg]") {
    const auto runtime = ExportSmokeRuntimePath();
    REQUIRE_FALSE(runtime.empty());
    REQUIRE(std::filesystem::exists(runtime));

    auto document = urpg::exporting::ExportPreviewDocument::fromJson(
        LoadExportPanelJson(ExportPanelRepoRoot() / "content" / "fixtures" / "export_preview_fixture.json"));
    document.mode = urpg::tools::ExportMode::DevBootstrap;

    const auto workspace = std::filesystem::temp_directory_path() / "urpg_export_preview_panel_edits";
    std::filesystem::remove_all(workspace);

    urpg::editor::ExportPreviewPanel panel;
    panel.loadDocument(document, workspace);
    panel.setRuntimeBinaryPath(runtime.string());
    panel.setMode(urpg::tools::ExportMode::Release);
    panel.setTarget(urpg::tools::ExportTarget::Windows_x64);
    panel.setOutputDir((workspace / "edited_output").string());
    panel.setExpectedArtifacts({"data.pck", "game.exe", "missing.bin"});
    panel.render();

    REQUIRE(panel.snapshot().preflight_passed);
    REQUIRE(panel.snapshot().export_success);
    REQUIRE(panel.snapshot().post_export_validation_passed);
    REQUIRE_FALSE(panel.snapshot().exact_ship_preview);
    REQUIRE(panel.snapshot().expected_artifact_count == 3);
    REQUIRE(panel.snapshot().missing_expected_artifact_count == 1);
    REQUIRE(panel.result().missing_expected_artifacts[0] == "missing.bin");
    REQUIRE(panel.snapshot().shipping_manifest["expected_artifacts"].size() == 3);
    REQUIRE(panel.snapshot().shipping_manifest["runtime_trace"].is_array());
    REQUIRE(panel.snapshot().status_message == "Export preview has diagnostics.");

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Export preview saved project data round-trips and flags bootstrap output",
          "[export][editor][preview][wysiwyg]") {
    auto document = urpg::exporting::ExportPreviewDocument::fromJson(
        LoadExportPanelJson(ExportPanelRepoRoot() / "content" / "fixtures" / "export_preview_fixture.json"));
    document.id = "bootstrap_preview";
    document.mode = urpg::tools::ExportMode::DevBootstrap;

    const auto saved = document.toJson();
    const auto restored = urpg::exporting::ExportPreviewDocument::fromJson(saved);
    const auto workspace = std::filesystem::temp_directory_path() / "urpg_export_preview_bootstrap";
    std::filesystem::remove_all(workspace);
    const auto result = urpg::exporting::RunExportPreview(restored, workspace);

    REQUIRE(saved["schema"] == "urpg.export_preview.v1");
    REQUIRE(restored.expected_artifacts.size() == 2);
    REQUIRE(result.preflight_passed);
    REQUIRE(result.export_success);
    REQUIRE(result.post_export_validation_passed);
    REQUIRE_FALSE(result.exact_ship_preview);
    REQUIRE_FALSE(result.diagnostics.empty());
    REQUIRE(result.diagnostics.back().code == "dev_bootstrap_not_release");

    std::filesystem::remove_all(workspace);
}

TEST_CASE("Export preview diagnostics block false exact-ship claims",
          "[export][editor][preview][wysiwyg]") {
    urpg::exporting::ExportPreviewDocument document;
    document.id = "broken_release_preview";
    document.mode = urpg::tools::ExportMode::Release;
    document.target = urpg::tools::ExportTarget::Windows_x64;

    const auto workspace = std::filesystem::temp_directory_path() / "urpg_export_preview_broken";
    std::filesystem::remove_all(workspace);

    urpg::editor::ExportPreviewPanel panel;
    panel.loadDocument(document, workspace);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().diagnostic_count == 1);
    REQUIRE_FALSE(panel.snapshot().preflight_passed);
    REQUIRE_FALSE(panel.snapshot().export_success);
    REQUIRE_FALSE(panel.snapshot().exact_ship_preview);
    REQUIRE(panel.snapshot().status_message == "Export preview has diagnostics.");
    REQUIRE(panel.result().diagnostics[0].code == "missing_runtime_binary");

    std::filesystem::remove_all(workspace);
}
