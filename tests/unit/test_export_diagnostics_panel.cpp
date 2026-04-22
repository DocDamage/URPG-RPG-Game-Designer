#include "editor/export/export_diagnostics_panel.h"

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

} // namespace

TEST_CASE("ExportDiagnosticsPanel produces empty snapshot when no config set", "[export][editor][panel]") {
    ExportDiagnosticsPanel panel;
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.is_object());
    REQUIRE(snapshot.empty());
}

TEST_CASE("ExportDiagnosticsPanel snapshot reflects validation errors for missing files", "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_errors";
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
    REQUIRE(snapshot["target"] == "Windows_x64");
    REQUIRE(snapshot["outputDir"] == base.string());
    REQUIRE(snapshot["validationSource"] == "packager_preflight");
    REQUIRE(snapshot["validationPassed"] == false);
    REQUIRE(snapshot["readyToExport"] == false);
    REQUIRE(snapshot["errors"].is_array());
    REQUIRE_FALSE(snapshot["errors"].empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportDiagnosticsPanel snapshot shows ready-to-export true when validation passes", "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_ready";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "game.exe", "exe");
    WriteFile(base / "data.pck", "pck");

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

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportDiagnosticsPanel mirrors packager preflight for web exports", "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_web";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "index.html", "html");
    WriteFile(base / "game.wasm", "wasm");
    WriteFile(base / "game.js", "js");
    WriteFile(base / "data.pck", "pck");

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

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportDiagnosticsPanel surfaces emittedArtifacts when post-export tree is present", "[export][editor][panel]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_diagnostics_emitted";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "game.exe", "exe");
    WriteFile(base / "data.pck", "pck");

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
    REQUIRE(snapshot["validationPassed"] == false); // preflight fails
    REQUIRE(snapshot["postExportValidationPassed"] == false);
    REQUIRE(snapshot.contains("emittedArtifacts") == false);

    std::filesystem::remove_all(base);
}
