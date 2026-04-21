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
    REQUIRE(snapshot["validationPassed"] == true);
    REQUIRE(snapshot["readyToExport"] == true);
    REQUIRE(snapshot["errors"].is_array());
    REQUIRE(snapshot["errors"].empty());

    std::filesystem::remove_all(base);
}
