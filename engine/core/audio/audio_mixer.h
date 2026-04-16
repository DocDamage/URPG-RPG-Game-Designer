#pragma once

#include <string>
#include <vector>
#include "math/vector2.h" // Assuming vector2 exists based on previous code

namespace urpg::audio {

    /**
     * @brief Panning/Attenuation models for spatial sound.
     * Part of Wave 4 Engine Polish.
     */
    enum class RolloffModel {
        Linear,
        Inverse,
        Logarithmic,
        None
    };

    /**
     * @brief A single localized audio emitter in world space.
     */
    struct AudioSource {
        std::string assetPath;
        float x, y;
        float volume = 1.0f;
        float pitch = 1.0f;
        float referenceDistance = 1.0f;
        float maxDistance = 500.0f;
        RolloffModel rolloff = RolloffModel::Inverse;
        bool loop = false;
        bool isPlaying = false;
    };

    /**
     * @brief Advanced Audio Mixer with Spatial/3D Panning.
     * Manages background music (BGM), sound effects (SE), and ambient loops.
     */
    class AudioMixer {
    public:
        /**
         * @brief Update all active sources based on listener position.
         */
        void update(float listenerX, float listenerY) {
            for (auto& source : m_sources) {
                if (!source.isPlaying) continue;

                // Calculate distance and attenuation
                float dx = source.x - listenerX;
                float dy = source.y - listenerY;
                float dist = std::sqrt(dx*dx + dy*dy);

                float gain = calculateGain(dist, source);
                float pan = calculatePan(dx, dist);

                applyHardwareMix(source, gain, pan);
            }
        }

        void playSE(const std::string& path, float x, float y) {
            m_sources.push_back({path, x, y, 1.0f, 1.0f, 1.0f, 500.0f, RolloffModel::Inverse, false, true});
        }

        void setBGM(const std::string& path, float fadeSeconds = 1.0f) {
            m_targetBGM = path;
            // Logic for fading current BGM and crossfading to target
        }

    private:
        float calculateGain(float dist, const AudioSource& s) {
            if (dist <= s.referenceDistance) return s.volume;
            if (dist >= s.maxDistance) return 0.0f;

            switch (s.rolloff) {
                case RolloffModel::Linear:
                    return s.volume * (1.0f - (dist - s.referenceDistance) / (s.maxDistance - s.referenceDistance));
                case RolloffModel::Inverse:
                    return s.volume * (s.referenceDistance / dist);
                default: return s.volume;
            }
        }

        float calculatePan(float dx, float dist) {
            if (dist < 0.001f) return 0.0f; // Center
            return std::clamp(dx / 100.0f, -1.0f, 1.0f); // Simple linear pan
        }

        void applyHardwareMix(const AudioSource& s, float gain, float pan) {
            // Internal call to SDL_Mixer or OpenAL to set gain/pan for the channel
        }

        std::vector<AudioSource> m_sources;
        std::string m_currentBGM;
        std::string m_targetBGM;
    };

} // namespace urpg::audio
