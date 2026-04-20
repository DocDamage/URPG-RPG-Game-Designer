#pragma once

#include <string>
#include <vector>
#include <variant>

namespace urpg::animation {

/**
 * @brief Simple keyframe for numeric properties.
 */
struct Keyframe {
    float time;
    float value;
    // std::string easing; // In a full implementation
};

/**
 * @brief Definition for an animation track on a specific property.
 */
struct AnimationTrack {
    std::string propertyName; // "positionX", "opacity", "rotation"
    std::vector<Keyframe> keyframes;
};

/**
 * @brief A full animation clip containing multiple tracks.
 */
class AnimationClip {
public:
    std::string id;
    float duration;
    std::vector<AnimationTrack> tracks;

    std::vector<float> collectKeyframeTimes() const {
        std::vector<float> times;
        for (const auto& track : tracks) {
            for (const auto& keyframe : track.keyframes) {
                if (std::find(times.begin(), times.end(), keyframe.time) == times.end()) {
                    times.push_back(keyframe.time);
                }
            }
        }

        std::sort(times.begin(), times.end());
        return times;
    }

    float evaluate(const std::string& propertyName, float time) const {
        for (const auto& track : tracks) {
            if (track.propertyName == propertyName) {
                // Return simple interpolated value (baseline)
                if (track.keyframes.empty()) return 0.0f;
                if (time <= track.keyframes.front().time) return track.keyframes.front().value;
                if (time >= track.keyframes.back().time) return track.keyframes.back().value;

                for (size_t i = 0; i < track.keyframes.size() - 1; ++i) {
                    const auto& k1 = track.keyframes[i];
                    const auto& k2 = track.keyframes[i+1];
                    if (time >= k1.time && time < k2.time) {
                        float t = (time - k1.time) / (k2.time - k1.time);
                        return k1.value + (k2.value - k1.value) * t;
                    }
                }
            }
        }
        return 0.0f;
    }
};

} // namespace urpg::animation
