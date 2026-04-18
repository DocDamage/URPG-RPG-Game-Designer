#include "plugin_api.h"
#include <iostream>
#include <map>
#include <string>

// Internal placeholder for engine-side implementations of the Plugin API.
// This file currently provides fixture-backed scratch state and no-op hooks,
// not a production-ready bridge into ECSWorld, input, or editor runtime systems.

namespace urpg::editor {
    // Scratch storage used to keep plugin fixture tests deterministic.
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
    // STUB: returns synthetic IDs only; no entity is registered with a live ECS world yet.
    static uint64_t s_nextId = 1000;
    return s_nextId++;
}

void URPG_EntityDestroy(uint64_t entityId) {
    // STUB: entity destruction is not wired to a live ECS world yet.
}

void URPG_EntityAddComponent(uint64_t entityId, const char* componentType) {
    // STUB: component attachment is not wired to reflection or ECS storage yet.
}

void URPG_SetGlobalVariable(const char* key, float value) {
    if (!key) return;
    // STUB: stores values in process-local scratch state rather than GlobalStateHub.
    s_globals[key] = value;
}

float URPG_GetGlobalVariable(const char* key) {
    if (!key || s_globals.find(key) == s_globals.end()) return 0.0f;
    return s_globals[key];
}

void URPG_SetGlobalSwitch(const char* key, bool value) {
    if (!key) return;
    // STUB: stores values in process-local scratch state rather than engine save/runtime state.
    s_switches[key] = value;
}

bool URPG_GetGlobalSwitch(const char* key) {
    if (!key || s_switches.find(key) == s_switches.end()) return false;
    return s_switches[key];
}

bool URPG_IsKeyPressed(int keyCode) {
    // STUB: input is not bridged to InputManager yet.
    return false;
}

void URPG_GetMousePosition(float* x, float* y) {
    // STUB: pointer position is not bridged to the active platform window yet.
    if (x) *x = 0.0f;
    if (y) *y = 0.0f;
}

} // extern "C"
