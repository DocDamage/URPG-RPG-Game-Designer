#include "engine/core/diagnostics/startup_diagnostics.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace {

std::filesystem::path tempRoot(const std::string& name) {
    const auto root = std::filesystem::temp_directory_path() / ("urpg_startup_diagnostics_" + name);
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    return root;
}

void writeText(const std::filesystem::path& path, const std::string& payload) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << payload;
}

} // namespace

TEST_CASE("Startup diagnostics write project-local JSONL records", "[diagnostics][startup]") {
    const auto root = tempRoot("project_local");

    const urpg::diagnostics::StartupDiagnosticRecord record{
        "runtime",
        urpg::diagnostics::StartupDiagnosticSeverity::Fatal,
        "project_root_missing",
        "Project root does not exist.",
        root,
        true,
    };

    const auto result = urpg::diagnostics::writeStartupDiagnostic(record);

    REQUIRE(result.written);
    REQUIRE(result.log_path == root / ".urpg" / "logs" / "runtime_startup.jsonl");
    REQUIRE(std::filesystem::exists(result.log_path));

    std::ifstream in(result.log_path, std::ios::binary);
    REQUIRE(in.good());
    std::string line;
    std::getline(in, line);
    const auto json = nlohmann::json::parse(line);
    REQUIRE(json["schema"] == "urpg.startup_diagnostic.v1");
    REQUIRE(json["app"] == "runtime");
    REQUIRE(json["severity"] == "fatal");
    REQUIRE(json["code"] == "project_root_missing");
    REQUIRE(json["headless"] == true);
}

TEST_CASE("Startup diagnostics fall back to per-user log path when project root is missing",
          "[diagnostics][startup]") {
    const auto missingRoot = std::filesystem::temp_directory_path() / "urpg_startup_diagnostics_missing_root";
    std::filesystem::remove_all(missingRoot);

    const urpg::diagnostics::StartupDiagnosticRecord record{
        "editor",
        urpg::diagnostics::StartupDiagnosticSeverity::Fatal,
        "project_root_missing",
        "Project root does not exist.",
        missingRoot,
        true,
    };

    const auto result = urpg::diagnostics::writeStartupDiagnostic(record);

    REQUIRE(result.written);
    REQUIRE(result.log_path.filename() == "editor_startup.jsonl");
    REQUIRE(result.log_path.string().find("startup_diagnostics") != std::string::npos);
    REQUIRE(std::filesystem::exists(result.log_path));
}

TEST_CASE("Startup input validation reports missing project roots and invalid window sizes",
          "[diagnostics][startup]") {
    const auto missingRoot = std::filesystem::temp_directory_path() / "urpg_startup_diagnostics_missing_validate";
    std::filesystem::remove_all(missingRoot);

    const auto missing = urpg::diagnostics::validateStartupInputs("runtime", missingRoot, 1280, 720, true);
    REQUIRE(missing.has_value());
    REQUIRE(missing->code == "project_root_missing");
    REQUIRE(missing->severity == urpg::diagnostics::StartupDiagnosticSeverity::Fatal);

    const auto root = tempRoot("invalid_size");
    const auto invalidSize = urpg::diagnostics::validateStartupInputs("editor", root, 0, 720, true);
    REQUIRE(invalidSize.has_value());
    REQUIRE(invalidSize->code == "invalid_window_size");

    const auto ok = urpg::diagnostics::validateStartupInputs("runtime", root, 1280, 720, true);
    REQUIRE_FALSE(ok.has_value());
}

TEST_CASE("Runtime project preflight accepts minimal manifest plus content root",
          "[diagnostics][startup][runtime][preflight]") {
    const auto root = tempRoot("runtime_preflight_valid");
    writeText(root / "project.json", R"({
  "_urpg_format_version": "1.0",
  "_engine_version_min": "0.1.0",
  "determinism": {
    "level": "A",
    "authoritative_math": "fixed32"
  }
})");
    std::filesystem::create_directories(root / "content");

    const auto result = urpg::diagnostics::validateRuntimeProjectPreflight("runtime", root, true);

    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("Runtime project preflight accepts dev content roots without a manifest",
          "[diagnostics][startup][runtime][preflight]") {
    const auto root = tempRoot("runtime_preflight_content_root");
    std::filesystem::create_directories(root / "content");

    const auto result = urpg::diagnostics::validateRuntimeProjectPreflight("runtime", root, true);

    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("Runtime project preflight rejects empty project roots",
          "[diagnostics][startup][runtime][preflight]") {
    const auto root = tempRoot("runtime_preflight_empty");

    const auto result = urpg::diagnostics::validateRuntimeProjectPreflight("runtime", root, true);

    REQUIRE(result.has_value());
    REQUIRE(result->code == "runtime_project_manifest_missing");
    REQUIRE(result->severity == urpg::diagnostics::StartupDiagnosticSeverity::Fatal);
}

TEST_CASE("Runtime project preflight rejects malformed manifests",
          "[diagnostics][startup][runtime][preflight]") {
    const auto root = tempRoot("runtime_preflight_invalid_manifest");
    writeText(root / "project.json", R"({"_urpg_format_version": "1.0",)");

    const auto result = urpg::diagnostics::validateRuntimeProjectPreflight("runtime", root, true);

    REQUIRE(result.has_value());
    REQUIRE(result->code == "runtime_project_manifest_invalid");
}

TEST_CASE("Runtime project preflight rejects manifest-only project roots without content",
          "[diagnostics][startup][runtime][preflight]") {
    const auto root = tempRoot("runtime_preflight_missing_content");
    writeText(root / "project.json", R"({
  "_urpg_format_version": "1.0",
  "_engine_version_min": "0.1.0",
  "determinism": {
    "level": "A",
    "authoritative_math": "fixed32"
  }
})");

    const auto result = urpg::diagnostics::validateRuntimeProjectPreflight("runtime", root, true);

    REQUIRE(result.has_value());
    REQUIRE(result->code == "runtime_project_content_missing");
}
