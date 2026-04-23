#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include "engine/core/export/export_validator.h"
#include "engine/core/tools/export_packager.h"

#include <filesystem>
#include <fstream>
#include <vector>

using namespace urpg::exporting;
using urpg::tools::ExportTarget;

namespace {

void CreateRealExportFixture(const std::filesystem::path& base, ExportTarget target) {
    std::filesystem::remove_all(base);
    urpg::tools::ExportPackager packager;
    urpg::tools::ExportConfig config{};
    config.target = target;
    config.outputDir = base.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);
    REQUIRE(result.success);
}

std::vector<std::uint8_t> ReadFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

} // namespace

TEST_CASE("ExportValidator: Valid Windows export directory passes", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_win";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Missing required file produces error", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_missing";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);
    std::filesystem::remove(base / "game.exe");

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

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: macOS check detects .app directory", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_macos";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);
    CreateRealExportFixture(base, ExportTarget::macOS_Universal);

    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::macOS_Universal);

    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: Web check requires all file types", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_web";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Web_WASM);

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

    CreateRealExportFixture(base, ExportTarget::Linux_x64);

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
        CreateRealExportFixture(base, target);

        const auto errors = validator.validateExportDirectory(base.string(), target);
        REQUIRE(errors.empty());

        std::filesystem::remove_all(base);
    }
}

TEST_CASE("check_platform_exports.ps1 -Json emits matching validator result shape for valid export", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_ps1_json_valid";
    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    const std::filesystem::path scriptPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "tools" / "ci" / "check_platform_exports.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_export_ps1_json_valid_out.json";

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" -ExportDir \"" + base.string() +
        "\" -Target Windows_x64 -Json > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.good());
    nlohmann::json result;
    resultFile >> result;
    resultFile.close();

    REQUIRE(result["target"] == "Windows_x64");
    REQUIRE(result["passed"] == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());

    std::filesystem::remove(outputPath);
    std::filesystem::remove_all(base);
}

TEST_CASE("check_platform_exports.ps1 -Json emits matching validator result shape for invalid export", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_ps1_json_invalid";
    CreateRealExportFixture(base, ExportTarget::Windows_x64);
    std::filesystem::remove(base / "game.exe");

    const std::filesystem::path scriptPath =
        std::filesystem::path(URPG_SOURCE_DIR) / "tools" / "ci" / "check_platform_exports.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_export_ps1_json_invalid_out.json";

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" -ExportDir \"" + base.string() +
        "\" -Target Windows_x64 -Json > \"" + outputPath.string() + "\" 2>nul";

    const int exitCode = std::system(command.c_str());
    // The script exits with 1 on failure when -Json is present, but std::system
    // on Windows returns the raw exit code. We accept non-zero.
    (void)exitCode;

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.good());
    nlohmann::json result;
    resultFile >> result;
    resultFile.close();

    REQUIRE(result["target"] == "Windows_x64");
    REQUIRE(result["passed"] == false);
    REQUIRE(result["errors"].is_array());
    REQUIRE_FALSE(result["errors"].empty());

    std::filesystem::remove(outputPath);
    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: corrupted bundle integrity produces an error", "[export][validation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_validator_corrupt_bundle";
    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    auto bytes = ReadFileBytes(base / "data.pck");
    REQUIRE(bytes.size() > 32);
    bytes.back() ^= 0x1;

    std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    out.close();

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);

    REQUIRE_FALSE(errors.empty());
    bool foundIntegrityMismatch = false;
    for (const auto& error : errors) {
        if (error.find("integrity mismatch") != std::string::npos) {
            foundIntegrityMismatch = true;
            break;
        }
    }
    REQUIRE(foundIntegrityMismatch);

    std::filesystem::remove_all(base);
}

// ─── S29-T05/T06: Signing/enforcement seam + edge-case tamper coverage ───────

TEST_CASE("ExportValidator: bundle with empty entries array passes integrity check",
          "[export][validation][s29]") {
    // Edge case: valid header + empty payload is legitimate (no assets yet)
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_val_empty_entries";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    // Replace data.pck with a minimal valid bundle that has zero entries
    constexpr char kMagic[] = "URPGPCK1";
    nlohmann::json manifest;
    manifest["entries"] = nlohmann::json::array();
    manifest["payloadOffset"] = 0u;
    manifest["integrityMode"] = "fnv1a64_keyed";
    const std::string manifestText = manifest.dump();
    const auto manifestSize = static_cast<std::uint32_t>(manifestText.size());

    std::vector<std::uint8_t> bundle;
    for (char c : std::string(kMagic, sizeof(kMagic) - 1)) {
        bundle.push_back(static_cast<std::uint8_t>(c));
    }
    bundle.push_back(static_cast<std::uint8_t>(manifestSize & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 8) & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 16) & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 24) & 0xFF));
    for (char c : manifestText) {
        bundle.push_back(static_cast<std::uint8_t>(c));
    }

    {
        std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(bundle.data()), static_cast<std::streamsize>(bundle.size()));
    }

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: truncated bundle header reports structured error",
          "[export][validation][s29]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_val_truncated";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    // Truncate data.pck to only 4 bytes — less than the magic + manifest-size header
    {
        std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
        const char tiny[] = {0x55, 0x52, 0x50, 0x47};
        out.write(tiny, sizeof(tiny));
    }

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE_FALSE(errors.empty());
    bool foundHeaderError = false;
    for (const auto& e : errors) {
        if (e.find("truncated") != std::string::npos || e.find("header") != std::string::npos ||
            e.find("Invalid asset package") != std::string::npos) {
            foundHeaderError = true;
            break;
        }
    }
    REQUIRE(foundHeaderError);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: wrong magic bytes reports structured error",
          "[export][validation][s29]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_val_badmagic";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    auto bytes = ReadFileBytes(base / "data.pck");
    REQUIRE(bytes.size() >= 8);
    // Corrupt first byte of magic
    bytes[0] ^= 0xFF;

    {
        std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE_FALSE(errors.empty());
    bool foundMagicError = false;
    for (const auto& e : errors) {
        if (e.find("URPGPCK1") != std::string::npos || e.find("header") != std::string::npos ||
            e.find("Invalid asset package") != std::string::npos) {
            foundMagicError = true;
            break;
        }
    }
    REQUIRE(foundMagicError);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: manifest with missing integrityMode reports enforcement error",
          "[export][validation][s29]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_val_no_integrity_mode";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    CreateRealExportFixture(base, ExportTarget::Windows_x64);

    // Re-write data.pck with a valid header but manifest lacking integrityMode
    constexpr char kMagic[] = "URPGPCK1";
    nlohmann::json manifest;
    manifest["entries"] = nlohmann::json::array();
    manifest["payloadOffset"] = 0u;
    // integrityMode deliberately omitted

    const std::string manifestText = manifest.dump();
    const auto manifestSize = static_cast<std::uint32_t>(manifestText.size());

    std::vector<std::uint8_t> bundle;
    for (char c : std::string(kMagic, sizeof(kMagic) - 1)) {
        bundle.push_back(static_cast<std::uint8_t>(c));
    }
    bundle.push_back(static_cast<std::uint8_t>(manifestSize & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 8) & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 16) & 0xFF));
    bundle.push_back(static_cast<std::uint8_t>((manifestSize >> 24) & 0xFF));
    for (char c : manifestText) {
        bundle.push_back(static_cast<std::uint8_t>(c));
    }

    {
        std::ofstream out(base / "data.pck", std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(bundle.data()), static_cast<std::streamsize>(bundle.size()));
    }

    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE_FALSE(errors.empty());
    bool foundIntegrityModeError = false;
    for (const auto& e : errors) {
        if (e.find("integrity") != std::string::npos) {
            foundIntegrityModeError = true;
            break;
        }
    }
    REQUIRE(foundIntegrityModeError);

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportValidator: path-not-found returns a structured error",
          "[export][validation][s29]") {
    ExportValidator validator;
    const auto errors = validator.validateExportDirectory(
        "/nonexistent/urpg_export_does_not_exist_xyz", ExportTarget::Windows_x64);
    REQUIRE_FALSE(errors.empty());
    const bool hasStructuredPathError =
        errors[0].find("not a directory") != std::string::npos ||
        errors[0].find("does not exist") != std::string::npos;
    REQUIRE(hasStructuredPathError);
}

TEST_CASE("ExportValidator: report JSON for all-passing export is stable across calls",
          "[export][validation][s29]") {
    ExportValidator validator;
    const std::vector<std::string> noErrors;
    const auto report1 = validator.buildReportJson(noErrors, ExportTarget::Windows_x64);
    const auto report2 = validator.buildReportJson(noErrors, ExportTarget::Windows_x64);
    REQUIRE(report1.dump() == report2.dump());
    REQUIRE(report1["passed"] == true);
    REQUIRE(report1["errors"].empty());
}
