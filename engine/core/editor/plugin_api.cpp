#include "plugin_api.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/player_control_system.h"
#include "engine/core/ecs/world.h"
#include "engine/core/global_state_hub.h"
#include "runtimes/compat_js/input_manager.h"

#include <cmath>
#include <iostream>
#include <string>
#include <type_traits>

namespace {

urpg::World* g_plugin_api_world = nullptr;

float GlobalValueToFloat(const urpg::GlobalStateHub::Value& value) {
    return std::visit(
        [](const auto& current) -> float {
            using T = std::decay_t<decltype(current)>;
            if constexpr (std::is_same_v<T, bool>) {
                return current ? 1.0f : 0.0f;
            } else if constexpr (std::is_same_v<T, int32_t>) {
                return static_cast<float>(current);
            } else if constexpr (std::is_same_v<T, float>) {
                return current;
            } else if constexpr (std::is_same_v<T, std::string>) {
                try {
                    size_t consumed = 0;
                    const float parsed = std::stof(current, &consumed);
                    if (consumed == current.size()) {
                        return parsed;
                    }
                } catch (...) {
                }
                return 0.0f;
            }
        },
        value);
}

void AddNamedComponent(urpg::World& world, urpg::EntityID entity_id, const std::string& component_type) {
    if (component_type == "Transform") {
        world.AddComponent(entity_id, urpg::TransformComponent{});
    } else if (component_type == "Velocity") {
        world.AddComponent(entity_id, urpg::VelocityComponent{});
    } else if (component_type == "Visual") {
        world.AddComponent(entity_id, urpg::VisualComponent{});
    } else if (component_type == "Actor") {
        world.AddComponent(entity_id, urpg::ActorComponent{});
    } else if (component_type == "PlayerControl") {
        world.AddComponent(entity_id, urpg::PlayerControlComponent{});
    }
}

} // namespace

namespace urpg::editor {

void BindPluginAPIWorld(World* world) {
    g_plugin_api_world = world;
}

void UnbindPluginAPIWorld() {
    g_plugin_api_world = nullptr;
}

} // namespace urpg::editor

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
    if (g_plugin_api_world == nullptr) {
        return 0;
    }
    return static_cast<uint64_t>(g_plugin_api_world->CreateEntity());
}

void URPG_EntityDestroy(uint64_t entityId) {
    if (g_plugin_api_world == nullptr || entityId == 0) {
        return;
    }
    g_plugin_api_world->DestroyEntity(static_cast<urpg::EntityID>(entityId));
}

void URPG_EntityAddComponent(uint64_t entityId, const char* componentType) {
    if (g_plugin_api_world == nullptr || entityId == 0 || componentType == nullptr) {
        return;
    }
    AddNamedComponent(*g_plugin_api_world, static_cast<urpg::EntityID>(entityId), componentType);
}

void URPG_SetGlobalVariable(const char* key, float value) {
    if (!key) return;
    urpg::GlobalStateHub::getInstance().setVariable(key, value);
}

float URPG_GetGlobalVariable(const char* key) {
    if (!key) return 0.0f;
    return GlobalValueToFloat(urpg::GlobalStateHub::getInstance().getVariable(key));
}

void URPG_SetGlobalSwitch(const char* key, bool value) {
    if (!key) return;
    urpg::GlobalStateHub::getInstance().setSwitch(key, value);
}

bool URPG_GetGlobalSwitch(const char* key) {
    if (!key) return false;
    return urpg::GlobalStateHub::getInstance().getSwitch(key);
}

bool URPG_IsKeyPressed(int keyCode) {
    return urpg::compat::InputManager::instance().isPressed(keyCode);
}

void URPG_GetMousePosition(float* x, float* y) {
    auto& input = urpg::compat::InputManager::instance();
    if (x) *x = static_cast<float>(input.getMouseX());
    if (y) *y = static_cast<float>(input.getMouseY());
}

} // extern "C"
