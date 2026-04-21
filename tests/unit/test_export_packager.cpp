#include <catch2/catch_test_macros.hpp>
#include "engine/core/tools/export_packager.h"
#include "engine/core/export/export_validator.h"

#include <filesystem>
#include <fstream>

using namespace urpg::tools;
using urpg::exporting::ExportValidator;

namespace {

void WriteFile(const std::filesystem::path& path, const std::string& content = "") {
    std::ofstream out(path, std::ios::binary);
    out << content;
}

} // namespace

TEST_CASE("ExportPackager::validateBeforeExport passes for a valid directory", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_valid";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "game.exe", "exe");
    WriteFile(base / "data.pck", "pck");

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();

    auto result = packager.validateBeforeExport(config);

    REQUIRE(result.passed);
    REQUIRE(result.errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager::validateBeforeExport fails when required files are missing", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_missing";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "data.pck", "pck");

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();

    auto result = packager.validateBeforeExport(config);

    REQUIRE_FALSE(result.passed);
    REQUIRE_FALSE(result.errors.empty());
    REQUIRE(result.errors[0].find("*.exe") != std::string::npos);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager::runExport result contains correct file list", "[export][packager]") {
    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = ".";
    config.compressAssets = true;

    auto result = packager.runExport(config);

    REQUIRE(result.success);
    REQUIRE_FALSE(result.generatedFiles.empty());
    REQUIRE(result.generatedFiles[0] == "data.pck");
}
