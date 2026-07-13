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

extern std::filesystem::path g_PlaytestOverlayDir;
std::filesystem::path resolvePlaytestPath(const std::filesystem::path& path);

class Process {
  public:
    Process();
    ~Process();
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    Process(Process&&) = delete;
    Process& operator=(Process&&) = delete;

    bool launch(const ProcessCommand& command);
    bool isRunning(int* exitCode = nullptr);
    void terminate();

    const std::string& getStdout() const { return stdoutText; }
    const std::string& getStderr() const { return stderrText; }
    const std::string& getError() const { return error; }

  private:
    void cleanup();

#ifdef _WIN32
    void* hProcess = nullptr;
    void* hThread = nullptr;
    void* stdoutRead = nullptr;
    void* stderrRead = nullptr;
#else
    int pid = -1;
    int stdoutFd = -1;
    int stderrFd = -1;
#endif
    std::string stdoutText;
    std::string stderrText;
    std::string error;
};

} // namespace urpg::platform
