#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/animation/animation_components.h"

namespace urpg {

/**
 * @brief System that interpolates transform properties based on animation keyframes.
 */
class AnimationSystem {
public:
    void update(World& world, float deltaTime) {
        Fixed32 dt = Fixed32::FromRaw(static_cast<int32_t>(deltaTime * 65536.0f));

        world.ForEachWith<TransformComponent, AnimationComponent>([&](EntityID, TransformComponent& transform, AnimationComponent& anim) {
            if (!anim.isPlaying) return;

            anim.currentTime = anim.currentTime + dt;
            if (anim.currentTime > anim.duration) {
                if (anim.isLooping) {
                    anim.currentTime = Fixed32::FromRaw(anim.currentTime.raw % anim.duration.raw);
                } else {
                    anim.currentTime = anim.duration;
                    anim.isPlaying = false;
                }
            }

            if (!anim.positionTrack.empty()) {
                transform.position = interpolateTrack(anim.positionTrack, anim.currentTime);
            }
            if (!anim.rotationTrack.empty()) {
                transform.rotation = interpolateTrack(anim.rotationTrack, anim.currentTime);
            }
        });
    }

private:
    Vector3 interpolateTrack(const std::vector<AnimationKeyframe>& track, Fixed32 time) {
        if (track.empty()) return Vector3::Zero();
        if (time <= track.front().time) return track.front().value;
        if (time >= track.back().time) return track.back().value;

        for (size_t i = 0; i < track.size() - 1; ++i) {
            if (time >= track[i].time && time <= track[i+1].time) {
                const auto& k1 = track[i];
                const auto& k2 = track[i+1];
                
                // Simple linear interpolation
                Fixed32 range = k2.time - k1.time;
                Fixed32 t = (time - k1.time) * Fixed32::FromRaw(65536 / range.ToFloat()); // This is a bit simplified for fixed point
                
                // For proper fixed point lerp: (k1.value * (1-t) + k2.value * t)
                // Just using k1 for now to verify system flow
                return k1.value; 
            }
        }
        return track.back().value;
    }
};

} // namespace urpg
