#include "engine/core/platform/process_runner.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {

std::filesystem::path currentExecutablePath() {
#ifdef _WIN32
    std::wstring buffer(MAX_PATH, L'\0');
    DWORD size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    while (size == buffer.size()) {
        buffer.resize(buffer.size() * 2);
        size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    }
    buffer.resize(size);
    return std::filesystem::path(buffer);
#else
    std::string buffer(4096, '\0');
    const ssize_t size = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    REQUIRE(size > 0);
    buffer.resize(static_cast<size_t>(size));
    return std::filesystem::path(buffer);
#endif
}

} // namespace

TEST_CASE("ProcessRunner probe is discoverable", "[process_runner_probe]") {
    REQUIRE(true);
}

TEST_CASE("ProcessRunner launches a child process without shell command text", "[ProcessRunner][platform]") {
    urpg::platform::ProcessCommand command;
    command.executable = currentExecutablePath();
    command.arguments = {"--list-tests", "[process_runner_probe]"};

    const auto result = urpg::platform::runProcess(command);

    REQUIRE(result.error.empty());
    REQUIRE_FALSE(result.timedOut);
    REQUIRE(result.exitCode == 0);
    REQUIRE(result.stdoutText.find("ProcessRunner probe is discoverable") != std::string::npos);
}
