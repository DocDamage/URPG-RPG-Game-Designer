#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/player_control_system.h"
#include "engine/core/ecs/ai_system.h"
#include "engine/core/ecs/movement_system.h"
#include "engine/core/ecs/collision_system.h"
#include "engine/core/animation/animation_system.h"
#include "engine/core/render/camera_system.h"
#include "engine/core/render/camera_follow_system.h"
#include "engine/core/render/camera_shake_system.h"
#include "engine/core/render/lighting_system.h"
#include "engine/core/events/trigger_system.h"
#include "engine/core/ecs/interaction_system.h"
#include "engine/core/ecs/health_system.h"
#include "engine/core/ecs/progression_system.h"
#include "engine/gameplay/status_effect_system.h"
#include "engine/core/ecs/economy_system.h"
#include "engine/core/render/particle_system.h"
#include "engine/core/ecs/lifetime_system.h"
#include "engine/core/gameplay/loot_system.h"
#include "engine/core/gameplay/quest_system.h"
#include "engine/core/ui/ui_state_sync_system.h"
#include "engine/core/input/input_core.h"

namespace urpg {

/**
 * @brief Orchestrates the execution order of all engine systems.
 */
class SystemOrchestrator {
public:
    void update(World& world, input::InputCore& input, float deltaTime) {
        // 1. Interaction & Player Control
        m_interaction.update(world, input);
        m_playerControl.update(world, input);

        // 2. Gameplay State & AI
        m_ai.update(world, deltaTime);
        m_statusEffects.update(world, deltaTime);
        m_health.update(world, deltaTime);
        m_quest.update(world);
        m_progression.update(world);
        m_economy.update(world);
        
        // 3. State Progression (Animations)
        m_animation.update(world, deltaTime);
        
        // 4. Physics & Collisions
        m_collision.update(world);
        m_lifetime.update(world, deltaTime);
        
        // 5. Movement Integration
        m_movement.update(world, deltaTime);

        // 6. Camera & View logic
        m_cameraFollow.update(world, deltaTime);
        m_cameraShake.update(world, deltaTime);

        // 7. UI and Visual Effects
        m_uiSync.update(world);
        m_particles.update(world, deltaTime);
        
        // 8. Render Preparation
        m_camera.update(world);
        m_lighting.update(world);
    }

    const CameraSystem& getCamera() const { return m_camera; }
    const TriggerSystem& getTrigger() const { return m_trigger; }

private:
    PlayerControlSystem m_playerControl;
    AISystem m_ai;
    StatusEffectSystem m_statusEffects;
    HealthSystem m_health;
    ProgressionSystem m_progression;
    QuestSystem m_quest;
    LootSystem m_loot;
    EconomySystem m_economy;
    InteractionSystem m_interaction;
    CameraFollowSystem m_cameraFollow;
    CameraShakeSystem m_cameraShake;
    AnimationSystem m_animation;
    CollisionSystem m_collision;
    MovementSystem m_movement;
    LifeTimeSystem m_lifetime;
    TriggerSystem m_trigger;
    UIStateSyncSystem m_uiSync;
    ParticleSystem m_particles;
    CameraSystem m_camera;
    LightingSystem m_lighting;
};

} // namespace urpg

