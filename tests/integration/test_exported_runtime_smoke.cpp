#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

struct ProcessResult {
    int exitCode = -1;
    std::string stdoutText;
    std::string stderrText;
};

std::filesystem::path tempPath(const std::string& name) {
    return std::filesystem::temp_directory_path() /
           (name + "_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

#ifndef _WIN32
std::string quote(const std::filesystem::path& path) {
    return "\"" + path.string() + "\"";
}

std::string quote(const std::string& value) {
    return "\"" + value + "\"";
}
#endif

ProcessResult runProcess(const std::filesystem::path& executable, const std::vector<std::string>& args) {
    const auto captureRoot = tempPath("urpg_export_runtime_smoke_capture");
    std::filesystem::remove_all(captureRoot);
    std::filesystem::create_directories(captureRoot);

    const auto stdoutPath = captureRoot / "stdout.txt";
    const auto stderrPath = captureRoot / "stderr.txt";

    ProcessResult result;
#ifdef _WIN32
    auto toWide = [](const std::string& text) { return std::wstring(text.begin(), text.end()); };
    auto quoteWide = [](const std::wstring& text) { return L"\"" + text + L"\""; };

    std::wstring commandLine = quoteWide(executable.wstring());
    for (const auto& arg : args) {
        commandLine += L" ";
        commandLine += quoteWide(toWide(arg));
    }

    SECURITY_ATTRIBUTES securityAttributes{};
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;

    HANDLE stdoutHandle = CreateFileW(stdoutPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &securityAttributes,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(stdoutHandle != INVALID_HANDLE_VALUE);

    HANDLE stderrHandle = CreateFileW(stderrPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &securityAttributes,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(stderrHandle != INVALID_HANDLE_VALUE);

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = stdoutHandle;
    startupInfo.hStdError = stderrHandle;

    PROCESS_INFORMATION processInfo{};
    std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
    mutableCommandLine.push_back(L'\0');

    const BOOL launched = CreateProcessW(nullptr, mutableCommandLine.data(), nullptr, nullptr, TRUE, 0, nullptr,
                                         nullptr, &startupInfo, &processInfo);

    CloseHandle(stdoutHandle);
    CloseHandle(stderrHandle);

    REQUIRE(launched == TRUE);
    WaitForSingleObject(processInfo.hProcess, 30000);

    DWORD exitCode = 1;
    REQUIRE(GetExitCodeProcess(processInfo.hProcess, &exitCode) == TRUE);
    result.exitCode = static_cast<int>(exitCode);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
#else
    std::ostringstream command;
    command << quote(executable);
    for (const auto& arg : args) {
        command << " " << quote(arg);
    }
    command << " > " << quote(stdoutPath);
    command << " 2> " << quote(stderrPath);

    result.exitCode = std::system(command.str().c_str());
#endif
    if (std::filesystem::exists(stdoutPath)) {
        result.stdoutText = readFile(stdoutPath);
    }
    if (std::filesystem::exists(stderrPath)) {
        result.stderrText = readFile(stderrPath);
    }

    std::filesystem::remove_all(captureRoot);
    return result;
}

std::string hostTargetName() {
#if defined(_WIN32)
    return "Windows_x64";
#elif defined(__APPLE__)
    return "macOS_Universal";
#else
    return "Linux_x64";
#endif
}

std::filesystem::path hostRuntimePath(const std::filesystem::path& exportDir) {
#if defined(_WIN32)
    return exportDir / "game.exe";
#elif defined(__APPLE__)
    return exportDir / "MyGame.app" / "Contents" / "MacOS" / "MyGame";
#else
    return exportDir / "game";
#endif
}

void tamperBundlePayload(const std::filesystem::path& bundlePath) {
    std::fstream bundle(bundlePath, std::ios::binary | std::ios::in | std::ios::out);
    REQUIRE(bundle.good());
    bundle.seekg(-1, std::ios::end);
    char byte = 0;
    bundle.read(&byte, 1);
    byte = static_cast<char>(byte ^ 0x5A);
    bundle.seekp(-1, std::ios::end);
    bundle.write(&byte, 1);
    REQUIRE(bundle.good());
}

} // namespace

TEST_CASE("exported runtime smoke launches signed bundle and rejects tampered bundle", "[export][runtime_smoke]") {
#if defined(URPG_PACK_CLI_PATH) && defined(URPG_EXPORT_SMOKE_APP_PATH) && defined(URPG_SOURCE_DIR)
    const auto outputDir = tempPath("urpg_export_runtime_smoke");
    std::filesystem::remove_all(outputDir);

    const auto sourceDir = std::filesystem::path(URPG_SOURCE_DIR);
    const auto fixtureProject = sourceDir / "content" / "fixtures" / "export_runtime_smoke_project";
    const auto packCliPath = std::filesystem::path(URPG_PACK_CLI_PATH);
    const auto smokeAppPath = std::filesystem::path(URPG_EXPORT_SMOKE_APP_PATH);
    REQUIRE(std::filesystem::exists(packCliPath));
    REQUIRE(std::filesystem::exists(smokeAppPath));
    REQUIRE(std::filesystem::exists(fixtureProject / "project.json"));
    REQUIRE(std::filesystem::exists(fixtureProject / "asset_licenses.json"));

    const auto packageResult = runProcess(packCliPath, {
                                                           "--json",
                                                           "--target",
                                                           hostTargetName(),
                                                           "--output",
                                                           outputDir.string(),
                                                           "--runtime-binary",
                                                           smokeAppPath.string(),
                                                           "--asset-root",
                                                           fixtureProject.string(),
                                                       });

    INFO(packageResult.stdoutText);
    INFO(packageResult.stderrText);
    REQUIRE(packageResult.exitCode == 0);
    REQUIRE(std::filesystem::exists(outputDir / "data.pck"));
    REQUIRE(std::filesystem::exists(hostRuntimePath(outputDir)));

    const auto markerPath = outputDir / "runtime_smoke_marker.txt";
    const auto launchResult = runProcess(hostRuntimePath(outputDir), {
                                                                         "--write-marker",
                                                                         markerPath.string(),
                                                                         "--target",
                                                                         hostTargetName(),
                                                                     });

    INFO(launchResult.stdoutText);
    INFO(launchResult.stderrText);
    REQUIRE(launchResult.exitCode == 0);
    REQUIRE(std::filesystem::exists(markerPath));
    REQUIRE(readFile(markerPath).find("URPG_EXPORT_SMOKE_OK") != std::string::npos);

    tamperBundlePayload(outputDir / "data.pck");
    std::filesystem::remove(markerPath);

    const auto tamperedResult = runProcess(hostRuntimePath(outputDir), {
                                                                           "--write-marker",
                                                                           markerPath.string(),
                                                                           "--target",
                                                                           hostTargetName(),
                                                                       });

    INFO(tamperedResult.stdoutText);
    INFO(tamperedResult.stderrText);
    REQUIRE(tamperedResult.exitCode != 0);
    REQUIRE_FALSE(std::filesystem::exists(markerPath));
    REQUIRE(tamperedResult.stderrText.find("URPG export smoke bundle load rejected") != std::string::npos);

    std::error_code cleanupError;
    std::filesystem::remove_all(outputDir, cleanupError);
#else
    SUCCEED("Export runtime smoke tool paths are not available in this build.");
#endif
}
