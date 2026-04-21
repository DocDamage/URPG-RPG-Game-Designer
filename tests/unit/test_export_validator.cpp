#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include "engine/core/export/export_validator.h"
#include "engine/core/tools/export_packager.h"

#include <filesystem>
#include <fstream>

using namespace urpg::exporting;
using urpg::tools::ExportTarget;

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

TEST_CASE("ExportValidator: Valid Windows export directory passes", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_win";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "game.exe", "exe");
    WriteFile(base / "data.pck", "pck");

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Missing required file produces error", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_missing";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "data.pck", "pck");

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE_FALSE(errors.empty());
    REQUIRE(errors[0].find("*.exe") != std::string::npos);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Missing optional file does not produce error", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_optional";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "game.exe", "exe");
    WriteFile(base / "data.pck", "pck");

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: macOS check detects .app directory", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_macos";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);
    std::filesystem::create_directories(base / "MyGame.app");

    WriteFile(base / "data.pck", "pck");

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::macOS_Universal);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Web check requires all file types", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_web";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "index.html", "html");
    WriteFile(base / "game.wasm", "wasm");
    WriteFile(base / "game.js", "js");
    WriteFile(base / "data.pck", "pck");

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Web_WASM);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Report JSON contains errors array and target name", "[export][validation]") {
    ExportValidator validator;
    std::vector<std::string> errors = { "Missing required file: index.html" };
    auto report = validator.buildReportJson(errors, ExportTarget::Web_WASM);

    REQUIRE(report["target"] == "Web_WASM");
    REQUIRE(report["passed"] == false);
    REQUIRE(report["errors"].is_array());
    REQUIRE(report["errors"].size() == 1);
    REQUIRE(report["errors"][0] == "Missing required file: index.html");
}

TEST_CASE("ExportValidator: Linux check detects executable without extension", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_linux";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    WriteFile(base / "mygame", "elf");
    WriteFile(base / "data.pck", "pck");

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Linux_x64);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: supported target fixtures satisfy all required artifacts", "[export][validation]") {
    const std::vector<ExportTarget> targets = {
        ExportTarget::Windows_x64,
        ExportTarget::Linux_x64,
        ExportTarget::macOS_Universal,
        ExportTarget::Web_WASM,
    };

    ExportValidator validator;

    for (const auto target : targets) {
        const auto base = std::filesystem::temp_directory_path() /
            ("urpg_export_validator_target_" + std::to_string(static_cast<int>(target)));
        CreateExportFixture(base, target);

        const auto errors = validator.validateExportDirectory(base.string(), target);
        REQUIRE(errors.empty());

        std::filesystem::remove_all(base);
    }
}
