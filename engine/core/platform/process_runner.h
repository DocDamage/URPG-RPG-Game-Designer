#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace urpg::platform {

struct ProcessCommand {
    std::filesystem::path executable;
    std::vector<std::string> arguments;
    std::filesystem::path workingDirectory;
    std::map<std::string, std::string> environment;
    std::chrono::milliseconds timeout{30000};
    bool captureStdout = true;
    bool captureStderr = true;
};

struct ProcessResult {
    int exitCode = -1;
    std::string stdoutText;
    std::string stderrText;
    bool timedOut = false;
    std::string error;
};

ProcessResult runProcess(const ProcessCommand& command);

} // namespace urpg::platform
