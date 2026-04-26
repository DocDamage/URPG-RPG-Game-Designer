#include "editor/export/export_diagnostics_panel.h"
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
