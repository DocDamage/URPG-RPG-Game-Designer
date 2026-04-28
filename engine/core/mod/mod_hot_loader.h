#pragma once

#include "engine/core/mod/mod_loader.h"

#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::mod {

enum class ModHotLoadEventType {
    Registered,
    Changed,
    Reloaded,
    Failed,
    MissingEntrypoint
};

struct ModHotLoadEvent {
    ModHotLoadEventType type = ModHotLoadEventType::Registered;
    std::string modId;
    std::filesystem::path entryPoint;
    std::string message;
};

struct ModHotLoadPollResult {
    bool anyReloaded = false;
    std::vector<ModHotLoadEvent> events;
};

class ModHotLoader {
public:
    explicit ModHotLoader(ModLoader& loader, ModRegistry& registry);

    void setDebounceInterval(std::chrono::milliseconds interval);
    void trackRegisteredMods();
    bool trackMod(const std::string& modId);
    void untrackMod(const std::string& modId);

    ModHotLoadPollResult poll();
    const std::vector<ModHotLoadEvent>& eventLog() const { return m_eventLog; }
    void clearEventLog() { m_eventLog.clear(); }

private:
    struct WatchedMod {
        std::filesystem::path entryPoint;
        std::filesystem::file_time_type lastWriteTime{};
        uint64_t contentHash = 0;
        std::chrono::steady_clock::time_point lastObservedChange{};
        bool hasTimestamp = false;
    };

    std::filesystem::path resolveEntryPoint(const ModManifest& manifest) const;
    uint64_t hashFile(const std::filesystem::path& path) const;
    void recordEvent(ModHotLoadPollResult& result, ModHotLoadEvent event);

    ModLoader& m_loader;
    ModRegistry& m_registry;
    std::chrono::milliseconds m_debounceInterval{50};
    std::unordered_map<std::string, WatchedMod> m_watchedMods;
    std::vector<ModHotLoadEvent> m_eventLog;
};

} // namespace urpg::mod
