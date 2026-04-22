#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace urpg {

enum class ThreadRole : uint8_t {
    Render = 0,
    Logic = 1,
    Audio = 2,
    AssetStreaming = 3
};

enum class ScriptAccess : uint8_t {
    None = 0,
    CallbackOnly = 1,
    Full = 2
};

constexpr ScriptAccess ScriptAccessForRole(ThreadRole role) {
    switch (role) {
        case ThreadRole::Logic:
            return ScriptAccess::Full;
        case ThreadRole::AssetStreaming:
            return ScriptAccess::CallbackOnly;
        case ThreadRole::Render:
        case ThreadRole::Audio:
        default:
            return ScriptAccess::None;
    }
}

class ThreadRegistry {
public:
    void RegisterCurrentThread(ThreadRole role);
    bool IsCurrentThread(ThreadRole role) const;
    ScriptAccess CurrentScriptAccess() const;
    bool CanRunScriptDirectly() const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<ThreadRole, std::thread::id> role_threads_;
};

} // namespace urpg
