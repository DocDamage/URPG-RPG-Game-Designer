#pragma once

#include "engine/core/animation/animation_clip.h"
#include "engine/core/animation/animation_components.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/world.h"

namespace urpg {

/**
 * @brief System that interpolates transform properties based on animation keyframes.
 */
class AnimationSystem {
  public:
    static AnimationComponent bindClip(const animation::AnimationClip& clip) {
        AnimationComponent component;
        component.duration = Fixed32::FromFloat(clip.duration);
        component.isLooping = false;
        component.isPlaying = true;

        const std::vector<float> times = clip.collectKeyframeTimes();
        for (float sampleTime : times) {
            AnimationKeyframe keyframe;
            keyframe.time = Fixed32::FromFloat(sampleTime);
            keyframe.value.x = Fixed32::FromFloat(clip.evaluate("positionX", sampleTime));
            keyframe.value.y = Fixed32::FromFloat(clip.evaluate("positionY", sampleTime));
            keyframe.value.z = Fixed32::FromFloat(clip.evaluate("positionZ", sampleTime));
            component.positionTrack.push_back(keyframe);
        }

        return component;
    }

    void update(World& world, float deltaTime) {
        Fixed32 dt = Fixed32::FromRaw(static_cast<int32_t>(deltaTime * 65536.0f));

        world.ForEachWith<TransformComponent, AnimationComponent>(
            [&](EntityID, TransformComponent& transform, AnimationComponent& anim) {
                if (!anim.isPlaying)
                    return;

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
        if (track.empty())
            return Vector3::Zero();
        if (time <= track.front().time)
            return track.front().value;
        if (time >= track.back().time)
            return track.back().value;

        for (size_t i = 0; i < track.size() - 1; ++i) {
            if (time >= track[i].time && time <= track[i + 1].time) {
                const auto& k1 = track[i];
                const auto& k2 = track[i + 1];

                // Simple linear interpolation
                Fixed32 range = k2.time - k1.time;
                if (range.raw == 0) {
                    return k2.value;
                }

                const float t = (time - k1.time).ToFloat() / range.ToFloat();
                Vector3 value;
                value.x = Fixed32::FromFloat(k1.value.x.ToFloat() + (k2.value.x.ToFloat() - k1.value.x.ToFloat()) * t);
                value.y = Fixed32::FromFloat(k1.value.y.ToFloat() + (k2.value.y.ToFloat() - k1.value.y.ToFloat()) * t);
                value.z = Fixed32::FromFloat(k1.value.z.ToFloat() + (k2.value.z.ToFloat() - k1.value.z.ToFloat()) * t);
                return value;
            }
        }
        return track.back().value;
    }
};

} // namespace urpg
