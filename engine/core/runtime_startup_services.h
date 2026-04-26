#pragma once

#include "engine/core/audio/audio_core.h"
#include "engine/core/input/input_core.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg {

enum class RuntimeStartupSubsystemStatus {
    Initialized,
    Skipped,
    Warning,
    Error,
};

struct RuntimeStartupSubsystemReport {
    std::string subsystem;
    RuntimeStartupSubsystemStatus status = RuntimeStartupSubsystemStatus::Initialized;
    std::string code;
    std::string message;
};

struct RuntimeStartupReport {
    std::filesystem::path project_root;
    std::vector<RuntimeStartupSubsystemReport> subsystems;
    std::string locale_code;
    size_t locale_key_count = 0;
    size_t input_mapping_count = 0;

    bool hasErrors() const;
    bool hasSubsystem(const std::string& subsystem) const;
    bool hasSubsystemCode(const std::string& subsystem, const std::string& code) const;
};

class RuntimeStartupServices {
  public:
    static RuntimeStartupReport initialize(const std::filesystem::path& project_root, audio::AudioCore& audio,
                                           input::InputCore& input, bool audio_compat_bound);
};

const char* toString(RuntimeStartupSubsystemStatus status);

} // namespace urpg
