#include "engine/core/threading/thread_roles.h"

namespace urpg {

void ThreadRegistry::RegisterCurrentThread(ThreadRole role) {
    std::lock_guard lock(mutex_);
    role_threads_[role] = std::this_thread::get_id();
}

bool ThreadRegistry::IsCurrentThread(ThreadRole role) const {
    std::lock_guard lock(mutex_);
    auto iter = role_threads_.find(role);
    if (iter == role_threads_.end()) {
        return false;
    }
    return iter->second == std::this_thread::get_id();
}

ScriptAccess ThreadRegistry::CurrentScriptAccess() const {
    std::lock_guard lock(mutex_);
    for (const auto& [role, tid] : role_threads_) {
        if (tid == std::this_thread::get_id()) {
            return ScriptAccessForRole(role);
        }
    }
    return ScriptAccess::None;
}

bool ThreadRegistry::CanRunScriptDirectly() const {
    return CurrentScriptAccess() == ScriptAccess::Full;
}

} // namespace urpg
