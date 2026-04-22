#include <catch2/catch_test_macros.hpp>
#include "engine/core/security/resource_protector.h"
#include "engine/core/tools/export_packager.h"
#include "engine/core/export/export_validator.h"

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

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

#ifdef URPG_EXPORT_SMOKE_APP_PATH
std::filesystem::path GetExportSmokeAppPath() {
    return std::filesystem::path(URPG_EXPORT_SMOKE_APP_PATH);
}
#endif

#ifdef _WIN32
bool LaunchProcessAndCaptureOutput(const std::filesystem::path& executable,
                                   const std::filesystem::path& markerPath,
                                   const std::filesystem::path& logPath) {
    SECURITY_ATTRIBUTES securityAttributes{};
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;

    HANDLE logHandle = CreateFileW(
        logPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &securityAttributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (logHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = logHandle;
    startupInfo.hStdError = logHandle;

    PROCESS_INFORMATION processInfo{};
    std::wstring commandLine =
        L"\"" + executable.wstring() + L"\" --write-marker \"" + markerPath.wstring() + L"\"";

    const BOOL launched = CreateProcessW(
        nullptr,
        commandLine.data(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo);

    CloseHandle(logHandle);

    if (!launched) {
        return false;
    }

    WaitForSingleObject(processInfo.hProcess, 30000);

    DWORD exitCode = 1;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    return exitCode == 0;
}
#endif

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

TEST_CASE("ExportPackager emits a valid export tree that passes post-export validation", "[export][packager]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_export_packager_emit_valid";
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

    // Verify emitted files exist on disk
    REQUIRE(std::filesystem::exists(base / "data.pck"));
    REQUIRE(std::filesystem::exists(base / "game.exe"));

    // Post-export validation must pass
    ExportValidator validator;
    auto errors = validator.validateExportDirectory(base.string(), ExportTarget::Windows_x64);
    REQUIRE(errors.empty());

    std::filesystem::remove_all(base);
}

TEST_CASE("ExportPackager emits valid export trees for all supported targets", "[export][packager]") {
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
            ("urpg_export_packager_emit_target_" + std::to_string(static_cast<int>(target)));
        CreateExportFixture(base, target);

        ExportConfig config{};
        config.target = target;
        config.outputDir = base.string();

        auto result = packager.runExport(config);
        INFO(result.log);
        REQUIRE(result.success);

        auto errors = validator.validateExportDirectory(base.string(), target);
        REQUIRE(errors.empty());

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

#ifdef _WIN32
TEST_CASE("ExportPackager stages a real Windows smoke export that launches successfully",
          "[export][packager][real_smoke]") {
#ifdef URPG_EXPORT_SMOKE_APP_PATH
    const auto outputDir = std::filesystem::temp_directory_path() /
        ("urpg_export_packager_real_smoke_" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto markerPath = outputDir / "smoke_marker.txt";
    const auto logPath = outputDir / "smoke_stdout.txt";
    const auto smokeAppPath = GetExportSmokeAppPath();

    REQUIRE(std::filesystem::exists(smokeAppPath));

    ExportPackager packager;
    ExportConfig config{};
    config.target = ExportTarget::Windows_x64;
    config.outputDir = outputDir.string();
    config.runtimeBinaryPath = smokeAppPath.string();
    config.compressAssets = true;

    const auto result = packager.runExport(config);

    INFO(result.log);
    REQUIRE(result.success);
    REQUIRE(std::filesystem::exists(outputDir / "game.exe"));
    REQUIRE(std::filesystem::exists(outputDir / "data.pck"));

    REQUIRE(LaunchProcessAndCaptureOutput(outputDir / "game.exe", markerPath, logPath));

    REQUIRE(std::filesystem::exists(markerPath));

    std::ifstream marker(markerPath);
    REQUIRE(marker.good());
    std::string markerContents;
    std::getline(marker, markerContents);
    REQUIRE(markerContents == "URPG_EXPORT_SMOKE_OK");

    std::error_code cleanupError;
    std::filesystem::remove_all(outputDir, cleanupError);
#else
    SUCCEED("URPG export smoke app path is not available in this build.");
#endif
}
#endif

TEST_CASE("ResourceProtector::compress is an explicit passthrough until real compression lands",
          "[export][security]") {
    urpg::security::ResourceProtector protector;
    const std::vector<uint8_t> rawData = {0x00, 0x10, 0x20, 0x20, 0xFF, 0x7A};

    REQUIRE_FALSE(urpg::security::ResourceProtector::compressionImplemented());
    REQUIRE(protector.compress(rawData) == rawData);
}

TEST_CASE("ResourceProtector obfuscation remains reversible XOR over the current passthrough bytes",
          "[export][security]") {
    urpg::security::ResourceProtector protector;
    const std::vector<uint8_t> rawData = {0x12, 0x34, 0x56, 0x78, 0x9A};
    auto protectedData = protector.compress(rawData);

    protector.obfuscate(protectedData, "urpg");
    REQUIRE(protectedData != rawData);

    protector.obfuscate(protectedData, "urpg");
    REQUIRE(protectedData == rawData);
}
