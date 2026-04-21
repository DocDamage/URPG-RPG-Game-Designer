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

void CreateExportFixture(const std::filesystem::path& base, ExportTarget target) {
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    switch (target) {
    case ExportTarget::Windows_x64:
        WriteFile(base / "game.exe", "exe");
        WriteFile(base / "data.pck", "pck");
        break;
    case ExportTarget::Linux_x64:
        WriteFile(base / "game", "elf");
        WriteFile(base / "data.pck", "pck");
        break;
    case ExportTarget::macOS_Universal:
        std::filesystem::create_directories(base / "MyGame.app");
        WriteFile(base / "data.pck", "pck");
        break;
    case ExportTarget::Web_WASM:
        WriteFile(base / "index.html", "html");
        WriteFile(base / "game.wasm", "wasm");
        WriteFile(base / "game.js", "js");
        WriteFile(base / "data.pck", "pck");
        break;
    }
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
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_run_valid";
    CreateExportFixture(base, ExportTarget::Windows_x64);

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();
    config.compressAssets = true;

    auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);
    REQUIRE_FALSE(result.generatedFiles.empty());
    REQUIRE(result.generatedFiles[0] == "data.pck");

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager::validateBeforeExport matches validator contract across supported targets",
          "[export][packager]") {
    const std::vector<ExportTarget> targets = {
        ExportTarget::Windows_x64,
        ExportTarget::Linux_x64,
        ExportTarget::macOS_Universal,
        ExportTarget::Web_WASM,
    };

    ExportPackager packager;
    ExportValidator validator;

    for (const auto target : targets) {
        const auto base = std::filesystem::temp_directory_path() /
            ("urpg_export_packager_target_" + std::to_string(static_cast<int>(target)));
        CreateExportFixture(base, target);

        ExportConfig config{};
        config.target = target;
        config.outputDir = base.string();

        const auto validation = packager.validateBeforeExport(config);
        const auto validatorErrors = validator.validateExportDirectory(base.string(), target);

        REQUIRE(validation.passed == validatorErrors.empty());
        REQUIRE(validation.errors == validatorErrors);

        std::filesystem::remove_all(base);
    }
}

TEST_CASE("ExportPackager::runExport fails when pre-export validation fails", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_run_invalid";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "data.pck", "pck");

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = base.string();

    const auto result = packager.runExport(config);

    REQUIRE_FALSE(result.success);
    REQUIRE(result.generatedFiles.empty());
    REQUIRE(result.log.find("Pre-export validation failed.") != std::string::npos);
    REQUIRE(result.log.find("Missing required file: *.exe") != std::string::npos);

    std::filesystem::remove_all(base);
}
