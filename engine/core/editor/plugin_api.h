#pragma once

#include <string>
#include <vector>
#include "bridge/value.h"

/**
 * @file plugin_api.h
 * @brief Stable C-style API exports for native URPG plugins.
 * 
 * Wave 7.2: Implement C++ Plugin API Exports.
 */

#ifdef _WIN32
    #define URPG_API __declspec(dllexport)
#else
    #define URPG_API __attribute__((visibility("default")))
#endif

extern "C" {

// --- Logging & Diagnostics ---
URPG_API void URPG_LogInfo(const char* message);
URPG_API void URPG_LogError(const char* message);

// --- Entity Management (ECS Bridge) ---
URPG_API uint64_t URPG_EntityCreate();
URPG_API void URPG_EntityDestroy(uint64_t entityId);
URPG_API void URPG_EntityAddComponent(uint64_t entityId, const char* componentType);

// --- Global State Hub ---
URPG_API void URPG_SetGlobalVariable(const char* key, float value);
URPG_API float URPG_GetGlobalVariable(const char* key);
URPG_API void URPG_SetGlobalSwitch(const char* key, bool value);
URPG_API bool URPG_GetGlobalSwitch(const char* key);

// --- Input Queries ---
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
