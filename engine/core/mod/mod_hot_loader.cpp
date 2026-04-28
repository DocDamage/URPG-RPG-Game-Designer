#include "engine/core/mod/mod_hot_loader.h"

#include <fstream>

namespace urpg::mod {

ModHotLoader::ModHotLoader(ModLoader& loader, ModRegistry& registry) : m_loader(loader), m_registry(registry) {}

void ModHotLoader::setDebounceInterval(std::chrono::milliseconds interval) {
    m_debounceInterval = interval;
}

void ModHotLoader::trackRegisteredMods() {
    for (const auto& manifest : m_registry.listMods()) {
        (void)trackMod(manifest.id);
    }
}

bool ModHotLoader::trackMod(const std::string& modId) {
    const auto manifest = m_registry.getMod(modId);
    if (!manifest.has_value() || manifest->entryPoint.empty()) {
        return false;
    }

    WatchedMod watched;
    watched.entryPoint = resolveEntryPoint(*manifest);
    std::error_code ec;
    watched.lastWriteTime = std::filesystem::last_write_time(watched.entryPoint, ec);
    watched.hasTimestamp = !ec;
    if (watched.hasTimestamp) {
        watched.contentHash = hashFile(watched.entryPoint);
    }
    watched.lastObservedChange = std::chrono::steady_clock::now();
    m_watchedMods[modId] = std::move(watched);
    m_eventLog.push_back({ModHotLoadEventType::Registered, modId, m_watchedMods[modId].entryPoint, "Mod is tracked for hot-load."});
    return true;
}

void ModHotLoader::untrackMod(const std::string& modId) {
    m_watchedMods.erase(modId);
}

ModHotLoadPollResult ModHotLoader::poll() {
    ModHotLoadPollResult result;
    const auto now = std::chrono::steady_clock::now();

    for (auto& [modId, watched] : m_watchedMods) {
        std::error_code ec;
        const auto currentWriteTime = std::filesystem::last_write_time(watched.entryPoint, ec);
        if (ec) {
            recordEvent(result, {ModHotLoadEventType::MissingEntrypoint, modId, watched.entryPoint,
                                 "Tracked mod entrypoint is missing."});
            watched.hasTimestamp = false;
            continue;
        }

        if (!watched.hasTimestamp) {
            watched.lastWriteTime = currentWriteTime;
            watched.hasTimestamp = true;
            watched.lastObservedChange = now;
            recordEvent(result, {ModHotLoadEventType::Changed, modId, watched.entryPoint,
                                 "Tracked mod entrypoint became available."});
            continue;
        }

        const auto currentHash = hashFile(watched.entryPoint);
        if (currentWriteTime == watched.lastWriteTime && currentHash == watched.contentHash) {
            continue;
        }

        if (now - watched.lastObservedChange < m_debounceInterval) {
            continue;
        }

        watched.lastObservedChange = now;
        watched.lastWriteTime = currentWriteTime;
        watched.contentHash = currentHash;
        recordEvent(result, {ModHotLoadEventType::Changed, modId, watched.entryPoint,
                             "Tracked mod entrypoint changed."});

        if (!m_loader.getRuntimeSnapshot(modId).active) {
            continue;
        }

        const auto reload = m_loader.reloadMod(modId);
        if (reload.success) {
            result.anyReloaded = true;
            recordEvent(result, {ModHotLoadEventType::Reloaded, modId, watched.entryPoint,
                                 "Active mod reloaded after entrypoint change."});
        } else {
            recordEvent(result, {ModHotLoadEventType::Failed, modId, watched.entryPoint, reload.errorMessage});
        }
    }

    return result;
}

std::filesystem::path ModHotLoader::resolveEntryPoint(const ModManifest& manifest) const {
    const std::filesystem::path entry = manifest.entryPoint;
    if (entry.is_absolute() || std::filesystem::exists(entry)) {
        return entry;
    }
    return std::filesystem::current_path() / entry;
}

uint64_t ModHotLoader::hashFile(const std::filesystem::path& path) const {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return 0;
    }

    uint64_t hash = 1469598103934665603ull;
    char ch = 0;
    while (input.get(ch)) {
        hash ^= static_cast<unsigned char>(ch);
        hash *= 1099511628211ull;
    }
    return hash;
}

void ModHotLoader::recordEvent(ModHotLoadPollResult& result, ModHotLoadEvent event) {
    result.events.push_back(event);
    m_eventLog.push_back(std::move(event));
}

} // namespace urpg::mod
