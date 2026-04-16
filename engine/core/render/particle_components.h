#pragma once

#include "engine/core/math/fixed32.h"
#include "engine/core/math/vector3.h"

namespace urpg {

/**
 * @brief Component responsible for spawning a particle effect.
 */
struct ParticleEmitterComponent {
    uint32_t maxParticles = 100;
    Fixed32 rate = Fixed32::FromInt(10); // Particles per second
    Fixed32 lifetime = Fixed32::FromInt(2);
    Vector3 startColor = {Fixed32::FromInt(1), Fixed32::FromInt(1), Fixed32::FromInt(1)};
    Vector3 endColor = {Fixed32::FromInt(0), Fixed32::FromInt(0), Fixed32::FromInt(0)};
    Vector3 initialVelocityMin = {Fixed32::FromInt(-1), Fixed32::FromInt(1), Fixed32::FromInt(-1)};
    Vector3 initialVelocityMax = {Fixed32::FromInt(1), Fixed32::FromInt(2), Fixed32::FromInt(1)};
    bool isLooping = true;
};

} // namespace urpg
