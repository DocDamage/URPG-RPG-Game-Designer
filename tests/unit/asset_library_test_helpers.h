#pragma once

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

inline std::filesystem::path uniqueTempRoot(const std::string& prefix) {
    const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (prefix + "_" + std::to_string(tick));
}

inline void writeBinaryFile(const std::filesystem::path& path, std::string_view payload) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << payload;
}

class EnvironmentVariableGuard {
  public:
    explicit EnvironmentVariableGuard(const char* name) : name_(name) {
        if (const char* value = std::getenv(name_)) {
            original_ = value;
        }
    }

    ~EnvironmentVariableGuard() {
#ifdef _WIN32
        if (original_) {
            _putenv_s(name_, original_->c_str());
        } else {
            _putenv_s(name_, "");
        }
#else
        if (original_) {
            setenv(name_, original_->c_str(), 1);
        } else {
            unsetenv(name_);
        }
#endif
    }

    void set(const std::string& value) const {
#ifdef _WIN32
        _putenv_s(name_, value.c_str());
#else
        setenv(name_, value.c_str(), 1);
#endif
    }

  private:
    const char* name_;
    std::optional<std::string> original_;
};
