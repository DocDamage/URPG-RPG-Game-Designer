#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/input/input_core.h"

namespace urpg {

/**
 * @brief Component to mark an entity as being player-controlled.
 */
struct PlayerControlComponent {
    Fixed32 moveSpeed = Fixed32::FromInt(5);
};

class PlayerControlSystem {
public:
    void update(World& world, const input::InputCore& input) {
        world.ForEachWith<TransformComponent, VelocityComponent, PlayerControlComponent>(
            [&](EntityID, TransformComponent&, VelocityComponent& velocity, const PlayerControlComponent& control) {
                Fixed32 vx = Fixed32::FromRaw(0);
                Fixed32 vy = Fixed32::FromRaw(0);

                if (input.isActionActive(input::InputAction::MoveUp)) {
                    vy = vy - control.moveSpeed;
                }
                if (input.isActionActive(input::InputAction::MoveDown)) {
                    vy = vy + control.moveSpeed;
                }
                if (input.isActionActive(input::InputAction::MoveLeft)) {
                    vx = vx - control.moveSpeed;
                }
                if (input.isActionActive(input::InputAction::MoveRight)) {
                    vx = vx + control.moveSpeed;
                }

                velocity.linear.x = vx;
                velocity.linear.y = vy;
            }
        );
    }
};

} // namespace urpg
