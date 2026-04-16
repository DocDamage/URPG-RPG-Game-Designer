#include "plugin_api.h"
#include <iostream>
#include <map>
#include <string>

// Internal placeholder for engine-side implementations of the Plugin API.
// In a full implementation, these would route directly to GlobalStateHub, ECSWorld, etc.

namespace urpg::editor {
    // Hidden storage for global simulation
    static std::map<std::string, float> s_globals;
    static std::map<std::string, bool> s_switches;
}

using namespace urpg::editor;

extern "C" {

void URPG_LogInfo(const char* message) {
    if (!message) return;
    std::cout << "[URPG Plugin Info] " << message << std::endl;
}

void URPG_LogError(const char* message) {
    if (!message) return;
    std::cerr << "[URPG Plugin Error] " << message << std::endl;
}

uint64_t URPG_EntityCreate() {
    static uint64_t s_nextId = 1000;
    return s_nextId++;
}

void URPG_EntityDestroy(uint64_t entityId) {
    // Route to ECSWorld::destroyEntity
}

void URPG_EntityAddComponent(uint64_t entityId, const char* componentType) {
    // Route to ECSWorld::addComponent with type-lookup reflection
}

void URPG_SetGlobalVariable(const char* key, float value) {
    if (!key) return;
    s_globals[key] = value;
}

float URPG_GetGlobalVariable(const char* key) {
    if (!key || s_globals.find(key) == s_globals.end()) return 0.0f;
    return s_globals[key];
}

void URPG_SetGlobalSwitch(const char* key, bool value) {
    if (!key) return;
    s_switches[key] = value;
}

bool URPG_GetGlobalSwitch(const char* key) {
    if (!key || s_switches.find(key) == s_switches.end()) return false;
    return s_switches[key];
}

bool URPG_IsKeyPressed(int keyCode) {
    // Route to InputManager
    return false;
}

void URPG_GetMousePosition(float* x, float* y) {
    if (x) *x = 0.0f;
    if (y) *y = 0.0f;
}

} // extern "C"
