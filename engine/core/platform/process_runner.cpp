#include "engine/core/platform/process_runner.h"

#include <algorithm>
#include <chrono>
#include <thread>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;
#endif

namespace urpg::platform {
namespace {

constexpr size_t kMaxCapturedOutputBytes = 1024 * 1024;

void boundCapturedOutput(std::string& output) {
    if (output.size() > kMaxCapturedOutputBytes) {
        output.erase(0, output.size() - kMaxCapturedOutputBytes);
    }
}

#ifdef _WIN32
std::wstring utf8ToWide(const std::string& value) {
    if (value.empty()) {
        return {};
    }
    const int size =
        MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    std::wstring out(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), out.data(), size);
    return out;
}

std::wstring pathToWide(const std::filesystem::path& path) {
    return path.wstring();
}

std::wstring quoteWindowsArg(const std::wstring& value) {
    if (value.empty()) {
        return L"\"\"";
    }
    const bool needsQuotes = value.find_first_of(L" \t\"") != std::wstring::npos;
    std::wstring out;
    if (needsQuotes) {
        out.push_back(L'"');
    }
    size_t backslashes = 0;
    for (const wchar_t ch : value) {
        if (ch == L'\\') {
            ++backslashes;
            continue;
        }
        if (ch == L'"') {
            out.append(backslashes * 2 + 1, L'\\');
            out.push_back(L'"');
            backslashes = 0;
            continue;
        }
        out.append(backslashes, L'\\');
        backslashes = 0;
        out.push_back(ch);
    }
    if (needsQuotes) {
        out.append(backslashes * 2, L'\\');
        out.push_back(L'"');
    } else {
        out.append(backslashes, L'\\');
    }
    return out;
}

std::wstring buildCommandLine(const ProcessCommand& command) {
    std::wstring line = quoteWindowsArg(pathToWide(command.executable));
    for (const auto& arg : command.arguments) {
        line.push_back(L' ');
        line += quoteWindowsArg(utf8ToWide(arg));
    }
    return line;
}

bool createPipe(HANDLE& readHandle, HANDLE& writeHandle, bool inheritWrite) {
    SECURITY_ATTRIBUTES security{};
    security.nLength = sizeof(security);
    security.bInheritHandle = TRUE;
    if (!CreatePipe(&readHandle, &writeHandle, &security, 0)) {
        return false;
    }
    SetHandleInformation(inheritWrite ? readHandle : writeHandle, HANDLE_FLAG_INHERIT, 0);
    return true;
}

std::string readPipe(HANDLE handle) {
    std::string out;
    char buffer[4096];
    DWORD read = 0;
    while (ReadFile(handle, buffer, sizeof(buffer), &read, nullptr) && read > 0) {
        out.append(buffer, buffer + read);
    }
    return out;
}
#else
void setCloseOnExec(int fd) {
    const int flags = fcntl(fd, F_GETFD);
    if (flags >= 0) {
        fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
    }
}

void setNonBlocking(int fd) {
    const int flags = fcntl(fd, F_GETFL);
    if (flags >= 0) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
}

void readAvailable(int fd, std::string& out) {
    char buffer[4096];
    for (;;) {
        const ssize_t count = read(fd, buffer, sizeof(buffer));
        if (count > 0) {
            out.append(buffer, buffer + count);
            continue;
        }
        break;
    }
}
#endif

} // namespace

ProcessResult runProcess(const ProcessCommand& command) {
    ProcessResult result;
    if (command.executable.empty()) {
        result.error = "process executable is empty";
        return result;
    }

#ifdef _WIN32
    HANDLE stdoutRead = nullptr;
    HANDLE stdoutWrite = nullptr;
    HANDLE stderrRead = nullptr;
    HANDLE stderrWrite = nullptr;
    if (command.captureStdout && !createPipe(stdoutRead, stdoutWrite, true)) {
        result.error = "failed to create stdout pipe";
        return result;
    }
    if (command.captureStderr && !createPipe(stderrRead, stderrWrite, true)) {
        if (stdoutRead != nullptr) {
            CloseHandle(stdoutRead);
            CloseHandle(stdoutWrite);
        }
        result.error = "failed to create stderr pipe";
        return result;
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    if (command.captureStdout || command.captureStderr) {
        startup.dwFlags |= STARTF_USESTDHANDLES;
        startup.hStdOutput = command.captureStdout ? stdoutWrite : GetStdHandle(STD_OUTPUT_HANDLE);
        startup.hStdError = command.captureStderr ? stderrWrite : GetStdHandle(STD_ERROR_HANDLE);
        startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    }

    PROCESS_INFORMATION process{};
    auto commandLine = buildCommandLine(command);
    auto executable = pathToWide(command.executable);
    const bool useExplicitApplicationName = command.executable.is_absolute() || command.executable.has_parent_path();
    auto workingDirectory = command.workingDirectory.empty() ? std::wstring{} : pathToWide(command.workingDirectory);
    const BOOL launched =
        CreateProcessW(useExplicitApplicationName ? executable.c_str() : nullptr, commandLine.data(), nullptr, nullptr,
                       TRUE, CREATE_NO_WINDOW, nullptr,
                       workingDirectory.empty() ? nullptr : workingDirectory.c_str(), &startup, &process);
    if (stdoutWrite != nullptr) {
        CloseHandle(stdoutWrite);
    }
    if (stderrWrite != nullptr) {
        CloseHandle(stderrWrite);
    }
    if (!launched) {
        if (stdoutRead != nullptr) {
            CloseHandle(stdoutRead);
        }
        if (stderrRead != nullptr) {
            CloseHandle(stderrRead);
        }
        result.error = "failed to launch process: " + std::to_string(GetLastError());
        return result;
    }

    const DWORD waitMs = command.timeout.count() <= 0 ? INFINITE : static_cast<DWORD>(command.timeout.count());
    const DWORD waitResult = WaitForSingleObject(process.hProcess, waitMs);
    if (waitResult == WAIT_TIMEOUT) {
        TerminateProcess(process.hProcess, 1);
        result.timedOut = true;
        result.error = "process timed out";
    }

    DWORD exitCode = 1;
    GetExitCodeProcess(process.hProcess, &exitCode);
    result.exitCode = static_cast<int>(exitCode);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (stdoutRead != nullptr) {
        result.stdoutText = readPipe(stdoutRead);
        CloseHandle(stdoutRead);
    }
    if (stderrRead != nullptr) {
        result.stderrText = readPipe(stderrRead);
        CloseHandle(stderrRead);
    }
    return result;
#else
    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    if (command.captureStdout && pipe(stdoutPipe) != 0) {
        result.error = "failed to create stdout pipe";
        return result;
    }
    if (command.captureStderr && pipe(stderrPipe) != 0) {
        if (stdoutPipe[0] >= 0) {
            close(stdoutPipe[0]);
            close(stdoutPipe[1]);
        }
        result.error = "failed to create stderr pipe";
        return result;
    }

    const pid_t pid = fork();
    if (pid == 0) {
        if (!command.workingDirectory.empty()) {
            chdir(command.workingDirectory.c_str());
        }
        if (command.captureStdout) {
            dup2(stdoutPipe[1], STDOUT_FILENO);
        }
        if (command.captureStderr) {
            dup2(stderrPipe[1], STDERR_FILENO);
        }
        if (stdoutPipe[0] >= 0) {
            close(stdoutPipe[0]);
            close(stdoutPipe[1]);
        }
        if (stderrPipe[0] >= 0) {
            close(stderrPipe[0]);
            close(stderrPipe[1]);
        }

        std::vector<std::string> argvStorage;
        argvStorage.push_back(command.executable.string());
        argvStorage.insert(argvStorage.end(), command.arguments.begin(), command.arguments.end());
        std::vector<char*> argv;
        for (auto& value : argvStorage) {
            argv.push_back(value.data());
        }
        argv.push_back(nullptr);

        if (command.environment.empty()) {
            execvp(argv[0], argv.data());
        } else {
            std::vector<std::string> envStorage;
            for (char** entry = environ; *entry != nullptr; ++entry) {
                envStorage.emplace_back(*entry);
            }
            for (const auto& [key, value] : command.environment) {
                const std::string prefix = key + "=";
                envStorage.erase(std::remove_if(envStorage.begin(), envStorage.end(), [&](const std::string& item) {
                                     return item.rfind(prefix, 0) == 0;
                                 }),
                                 envStorage.end());
                envStorage.push_back(prefix + value);
            }
            std::vector<char*> envp;
            for (auto& value : envStorage) {
                envp.push_back(value.data());
            }
            envp.push_back(nullptr);
            execve(argv[0], argv.data(), envp.data());
        }
        _exit(127);
    }
    if (pid < 0) {
        result.error = "failed to fork process";
        return result;
    }

    if (stdoutPipe[1] >= 0) {
        close(stdoutPipe[1]);
        setCloseOnExec(stdoutPipe[0]);
        setNonBlocking(stdoutPipe[0]);
    }
    if (stderrPipe[1] >= 0) {
        close(stderrPipe[1]);
        setCloseOnExec(stderrPipe[0]);
        setNonBlocking(stderrPipe[0]);
    }

    const auto deadline = std::chrono::steady_clock::now() + command.timeout;
    int status = 0;
    bool exited = false;
    while (!exited) {
        if (stdoutPipe[0] >= 0) {
            readAvailable(stdoutPipe[0], result.stdoutText);
        }
        if (stderrPipe[0] >= 0) {
            readAvailable(stderrPipe[0], result.stderrText);
        }
        const pid_t waited = waitpid(pid, &status, WNOHANG);
        if (waited == pid) {
            exited = true;
            break;
        }
        if (command.timeout.count() > 0 && std::chrono::steady_clock::now() >= deadline) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            result.timedOut = true;
            result.error = "process timed out";
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    if (stdoutPipe[0] >= 0) {
        readAvailable(stdoutPipe[0], result.stdoutText);
        close(stdoutPipe[0]);
    }
    if (stderrPipe[0] >= 0) {
        readAvailable(stderrPipe[0], result.stderrText);
        close(stderrPipe[0]);
    }
    if (WIFEXITED(status)) {
        result.exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        result.exitCode = 128 + WTERMSIG(status);
    }
    return result;
#endif
}

Process::Process() = default;

Process::~Process() {
    terminate();
}

#ifdef _WIN32
namespace {

void readAvailablePipe(HANDLE handle, std::string& output) {
    DWORD available = 0;
    while (PeekNamedPipe(handle, nullptr, 0, nullptr, &available, nullptr) && available > 0) {
        std::string chunk(available, '\0');
        DWORD read = 0;
        if (!ReadFile(handle, chunk.data(), available, &read, nullptr) || read == 0) {
            break;
        }
        output.append(chunk.data(), read);
    }
}

} // namespace

bool Process::launch(const ProcessCommand& command) {
    cleanup();
    error_.clear();
    stdout_text_.clear();
    stderr_text_.clear();
    if (command.executable.empty()) {
        error_ = "process executable is empty";
        return false;
    }

    HANDLE stdoutRead = nullptr;
    HANDLE stdoutWrite = nullptr;
    HANDLE stderrRead = nullptr;
    HANDLE stderrWrite = nullptr;
    if (command.captureStdout && !createPipe(stdoutRead, stdoutWrite, true)) {
        error_ = "failed to create stdout pipe";
        return false;
    }
    if (command.captureStderr && !createPipe(stderrRead, stderrWrite, true)) {
        if (stdoutRead != nullptr) {
            CloseHandle(stdoutRead);
            CloseHandle(stdoutWrite);
        }
        error_ = "failed to create stderr pipe";
        return false;
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    if (command.captureStdout || command.captureStderr) {
        startup.dwFlags |= STARTF_USESTDHANDLES;
        startup.hStdOutput = command.captureStdout ? stdoutWrite : GetStdHandle(STD_OUTPUT_HANDLE);
        startup.hStdError = command.captureStderr ? stderrWrite : GetStdHandle(STD_ERROR_HANDLE);
        startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    }
    PROCESS_INFORMATION process{};
    auto commandLine = buildCommandLine(command);
    const auto executable = pathToWide(command.executable);
    const auto workingDirectory = command.workingDirectory.empty() ? std::wstring{} : pathToWide(command.workingDirectory);
    const bool explicitApplication = command.executable.is_absolute() || command.executable.has_parent_path();
    const BOOL launched = CreateProcessW(explicitApplication ? executable.c_str() : nullptr, commandLine.data(), nullptr,
                                         nullptr, TRUE, CREATE_NO_WINDOW, nullptr,
                                         workingDirectory.empty() ? nullptr : workingDirectory.c_str(), &startup, &process);
    if (stdoutWrite != nullptr) CloseHandle(stdoutWrite);
    if (stderrWrite != nullptr) CloseHandle(stderrWrite);
    if (!launched) {
        if (stdoutRead != nullptr) CloseHandle(stdoutRead);
        if (stderrRead != nullptr) CloseHandle(stderrRead);
        error_ = "failed to launch process: " + std::to_string(GetLastError());
        return false;
    }
    process_handle_ = process.hProcess;
    thread_handle_ = process.hThread;
    stdout_read_ = stdoutRead;
    stderr_read_ = stderrRead;
    return true;
}

bool Process::isRunning(int* exitCode) {
    const auto process = static_cast<HANDLE>(process_handle_);
    if (process == nullptr) {
        if (exitCode) *exitCode = -1;
        return false;
    }
    if (stdout_read_ != nullptr) readAvailablePipe(static_cast<HANDLE>(stdout_read_), stdout_text_);
    if (stderr_read_ != nullptr) readAvailablePipe(static_cast<HANDLE>(stderr_read_), stderr_text_);
    boundCapturedOutput(stdout_text_);
    boundCapturedOutput(stderr_text_);
    DWORD code = 0;
    if (!GetExitCodeProcess(process, &code)) {
        if (exitCode) *exitCode = -1;
        cleanup();
        return false;
    }
    if (code == STILL_ACTIVE) return true;
    if (exitCode) *exitCode = static_cast<int>(code);
    if (stdout_read_ != nullptr) stdout_text_ += readPipe(static_cast<HANDLE>(stdout_read_));
    if (stderr_read_ != nullptr) stderr_text_ += readPipe(static_cast<HANDLE>(stderr_read_));
    boundCapturedOutput(stdout_text_);
    boundCapturedOutput(stderr_text_);
    cleanup();
    return false;
}

void Process::terminate() {
    if (process_handle_ != nullptr) {
        TerminateProcess(static_cast<HANDLE>(process_handle_), 1);
        (void)WaitForSingleObject(static_cast<HANDLE>(process_handle_), 5000);
    }
    cleanup();
}

void Process::cleanup() {
    if (process_handle_ != nullptr) CloseHandle(static_cast<HANDLE>(process_handle_));
    if (thread_handle_ != nullptr) CloseHandle(static_cast<HANDLE>(thread_handle_));
    if (stdout_read_ != nullptr) CloseHandle(static_cast<HANDLE>(stdout_read_));
    if (stderr_read_ != nullptr) CloseHandle(static_cast<HANDLE>(stderr_read_));
    process_handle_ = nullptr;
    thread_handle_ = nullptr;
    stdout_read_ = nullptr;
    stderr_read_ = nullptr;
}
#else
bool Process::launch(const ProcessCommand& command) {
    cleanup();
    error_.clear();
    stdout_text_.clear();
    stderr_text_.clear();
    if (command.executable.empty()) {
        error_ = "process executable is empty";
        return false;
    }
    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};
    if (command.captureStdout && pipe(stdoutPipe) != 0) {
        error_ = "failed to create stdout pipe";
        return false;
    }
    if (command.captureStderr && pipe(stderrPipe) != 0) {
        if (stdoutPipe[0] >= 0) {
            close(stdoutPipe[0]);
            close(stdoutPipe[1]);
        }
        error_ = "failed to create stderr pipe";
        return false;
    }
    pid_ = fork();
    if (pid_ == 0) {
        if (!command.workingDirectory.empty()) chdir(command.workingDirectory.c_str());
        if (command.captureStdout) dup2(stdoutPipe[1], STDOUT_FILENO);
        if (command.captureStderr) dup2(stderrPipe[1], STDERR_FILENO);
        if (stdoutPipe[0] >= 0) { close(stdoutPipe[0]); close(stdoutPipe[1]); }
        if (stderrPipe[0] >= 0) { close(stderrPipe[0]); close(stderrPipe[1]); }
        std::vector<std::string> argvStorage{command.executable.string()};
        argvStorage.insert(argvStorage.end(), command.arguments.begin(), command.arguments.end());
        std::vector<char*> argv;
        for (auto& value : argvStorage) argv.push_back(value.data());
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        _exit(127);
    }
    if (pid_ < 0) {
        if (stdoutPipe[0] >= 0) { close(stdoutPipe[0]); close(stdoutPipe[1]); }
        if (stderrPipe[0] >= 0) { close(stderrPipe[0]); close(stderrPipe[1]); }
        error_ = "failed to fork process";
        return false;
    }
    if (stdoutPipe[1] >= 0) { close(stdoutPipe[1]); setCloseOnExec(stdoutPipe[0]); setNonBlocking(stdoutPipe[0]); stdout_fd_ = stdoutPipe[0]; }
    if (stderrPipe[1] >= 0) { close(stderrPipe[1]); setCloseOnExec(stderrPipe[0]); setNonBlocking(stderrPipe[0]); stderr_fd_ = stderrPipe[0]; }
    return true;
}

bool Process::isRunning(int* exitCode) {
    if (pid_ < 0) {
        if (exitCode) *exitCode = -1;
        return false;
    }
    if (stdout_fd_ >= 0) readAvailable(stdout_fd_, stdout_text_);
    if (stderr_fd_ >= 0) readAvailable(stderr_fd_, stderr_text_);
    boundCapturedOutput(stdout_text_);
    boundCapturedOutput(stderr_text_);
    int status = 0;
    const pid_t waited = waitpid(pid_, &status, WNOHANG);
    if (waited == 0) return true;
    if (exitCode) *exitCode = waited == pid_ && WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    cleanup();
    return false;
}

void Process::terminate() {
    if (pid_ >= 0) {
        kill(pid_, SIGKILL);
        int status = 0;
        (void)waitpid(pid_, &status, 0);
    }
    cleanup();
}

void Process::cleanup() {
    if (stdout_fd_ >= 0) close(stdout_fd_);
    if (stderr_fd_ >= 0) close(stderr_fd_);
    pid_ = -1;
    stdout_fd_ = -1;
    stderr_fd_ = -1;
}
#endif

} // namespace urpg::platform
