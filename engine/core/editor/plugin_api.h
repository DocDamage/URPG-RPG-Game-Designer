#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg {
class World;
}

/**
 * @file plugin_api.h
 * @brief C-style plugin bridge exports for bounded native editor/runtime integration.
 *
 * Global state and compat input queries route into live engine-owned state.
 * Entity lifecycle calls operate on a caller-bound ECS world when one is
 * available; without a bound world they fail closed rather than inventing
 * synthetic entities.
 */

#ifdef _WIN32
    #define URPG_API __declspec(dllexport)
#else
    #define URPG_API __attribute__((visibility("default")))
#endif

extern "C" {

// --- Logging & Diagnostics ---
// Routed: emits log lines only; does not integrate with a structured editor log sink yet.
URPG_API void URPG_LogInfo(const char* message);
URPG_API void URPG_LogError(const char* message);

// --- Entity Management (routes into a caller-bound ECS world) ---
URPG_API uint64_t URPG_EntityCreate();
URPG_API void URPG_EntityDestroy(uint64_t entityId);
URPG_API void URPG_EntityAddComponent(uint64_t entityId, const char* componentType);

// --- Global State Hub (routes into GlobalStateHub) ---
URPG_API void URPG_SetGlobalVariable(const char* key, float value);
URPG_API float URPG_GetGlobalVariable(const char* key);
URPG_API void URPG_SetGlobalSwitch(const char* key, bool value);
URPG_API bool URPG_GetGlobalSwitch(const char* key);

// --- Input Queries (routes into compat InputManager state) ---
URPG_API bool URPG_IsKeyPressed(int keyCode);
URPG_API void URPG_GetMousePosition(float* x, float* y);

// --- Plugin Metadata ---
struct URPG_PluginHeader {
    const char* id;
    const char* name;
    const char* version;
};

// Function pointer types for plugin entry points
typedef void (*URPG_InitFn)();
typedef void (*URPG_UpdateFn)(float deltaTime);
typedef void (*URPG_ShutdownFn)();

} // extern "C"

namespace urpg::editor {

class ScopedPluginAPIWorldBinding {
public:
    explicit ScopedPluginAPIWorldBinding(World* world);
    ~ScopedPluginAPIWorldBinding();

    ScopedPluginAPIWorldBinding(const ScopedPluginAPIWorldBinding&) = delete;
    ScopedPluginAPIWorldBinding& operator=(const ScopedPluginAPIWorldBinding&) = delete;

    ScopedPluginAPIWorldBinding(ScopedPluginAPIWorldBinding&& other) noexcept;
    ScopedPluginAPIWorldBinding& operator=(ScopedPluginAPIWorldBinding&& other) noexcept = delete;

private:
    bool m_active = false;
};

/**
 * @brief Pushes a world binding used by the C plugin API entity exports.
 *
 * Bindings are thread-local and nest safely. A matching
 * `UnbindPluginAPIWorld()` removes only the most recently pushed binding on
 * the current thread.
 */
void BindPluginAPIWorld(World* world);

/**
 * @brief Pops the most recently pushed world binding on the current thread.
 */
void UnbindPluginAPIWorld();

} // namespace urpg::editor
