#pragma once

#include <string>
#include <vector>

/**
 * @file plugin_api.h
 * @brief Stubbed C-style plugin bridge exports for fixture-backed editor integration.
 * 
 * Wave 7.2 seeded the exported surface shape, but these APIs are not yet a
 * production-ready native plugin runtime. Several functions still route to
 * scratch-state storage, placeholder IDs, or no-op engine hooks.
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

// --- Entity Management (STUB: synthetic IDs / no live ECS wiring) ---
URPG_API uint64_t URPG_EntityCreate();
URPG_API void URPG_EntityDestroy(uint64_t entityId);
URPG_API void URPG_EntityAddComponent(uint64_t entityId, const char* componentType);

// --- Global State Hub (STUB: process-local scratch storage, not GlobalStateHub) ---
URPG_API void URPG_SetGlobalVariable(const char* key, float value);
URPG_API float URPG_GetGlobalVariable(const char* key);
URPG_API void URPG_SetGlobalSwitch(const char* key, bool value);
URPG_API bool URPG_GetGlobalSwitch(const char* key);

// --- Input Queries (STUB: fixed placeholder responses, not live input state) ---
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
