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

// A small non-blocking child-process owner for editor workflows. Unlike
// runProcess(), this keeps lifecycle control with the caller so an editor can
// return immediately while its runtime playtest continues in a separate
// process.
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

    const std::string& stdoutText() const { return stdout_text_; }
    const std::string& stderrText() const { return stderr_text_; }
    const std::string& error() const { return error_; }

  private:
    void cleanup();

#ifdef _WIN32
    void* process_handle_ = nullptr;
    void* thread_handle_ = nullptr;
    void* stdout_read_ = nullptr;
    void* stderr_read_ = nullptr;
#else
    int pid_ = -1;
    int stdout_fd_ = -1;
    int stderr_fd_ = -1;
#endif
    std::string stdout_text_;
    std::string stderr_text_;
    std::string error_;
};

} // namespace urpg::platform
