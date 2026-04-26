#pragma once

#include "audio_core.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace urpg::audio {

/**
 * @brief Panning/Attenuation models for spatial sound.
 * Part of Wave 4 Engine Polish.
 */
enum class RolloffModel { Linear, Inverse, Logarithmic, None };

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
    AudioHandle handle = 0;
    float lastGain = 0.0f;
    float lastPan = 0.0f;
};

struct AudioMixerDiagnostic {
    std::string code;
    std::string message;
    AudioHandle handle = 0;
    std::string assetPath;
};

/**
 * @brief Advanced Audio Mixer with Spatial/3D Panning.
 * Manages background music (BGM), sound effects (SE), and ambient loops.
 */
class AudioMixer {
  public:
    void bindCore(AudioCore* core) { m_core = core; }

    AudioCore* boundCore() const { return m_core; }

    /**
     * @brief Update all active sources based on listener position.
     */
    void update(float listenerX, float listenerY) {
        for (auto& source : m_sources) {
            if (!source.isPlaying)
                continue;

            // Calculate distance and attenuation
            float dx = source.x - listenerX;
            float dy = source.y - listenerY;
            float dist = std::sqrt(dx * dx + dy * dy);

            float gain = calculateGain(dist, source);
            float pan = calculatePan(dx, dist);

            applyHardwareMix(source, gain, pan);
        }
    }

    AudioHandle playSE(const std::string& path, float x, float y) {
        AudioSource source;
        source.assetPath = path;
        source.x = x;
        source.y = y;
        source.volume = 1.0f;
        source.pitch = 1.0f;
        source.referenceDistance = 1.0f;
        source.maxDistance = 500.0f;
        source.rolloff = RolloffModel::Inverse;
        source.loop = false;
        source.isPlaying = true;

        if (m_core) {
            source.handle = m_core->playSound(path, AudioCategory::SE, source.volume, source.pitch);
        } else {
            pushDiagnostic("audio_mixer.core_missing", "AudioMixer has no AudioCore binding.", 0, path);
        }

        const AudioHandle handle = source.handle;
        m_sources.push_back(std::move(source));
        return handle;
    }

    void setBGM(const std::string& path, float fadeSeconds = 1.0f) {
        m_targetBGM = path;
        if (!m_core) {
            pushDiagnostic("audio_mixer.core_missing", "AudioMixer has no AudioCore binding.", 0, path);
            return;
        }
        m_core->playBGM(path, fadeSeconds);
        m_currentBGM = path;
    }

    bool moveSource(AudioHandle handle, float x, float y) {
        auto* source = findSource(handle);
        if (!source) {
            pushDiagnostic("audio_mixer.source_missing", "AudioMixer source handle is not registered.", handle, {});
            return false;
        }
        source->x = x;
        source->y = y;
        return true;
    }

    std::optional<AudioSource> getSource(AudioHandle handle) const {
        for (const auto& source : m_sources) {
            if (source.handle == handle) {
                return source;
            }
        }
        return std::nullopt;
    }

    const std::vector<AudioMixerDiagnostic>& diagnostics() const { return m_diagnostics; }

    void clearDiagnostics() { m_diagnostics.clear(); }

  private:
    float calculateGain(float dist, const AudioSource& s) {
        if (dist <= s.referenceDistance)
            return s.volume;
        if (dist >= s.maxDistance)
            return 0.0f;

        switch (s.rolloff) {
        case RolloffModel::Linear:
            return s.volume * (1.0f - (dist - s.referenceDistance) / (s.maxDistance - s.referenceDistance));
        case RolloffModel::Inverse:
            return s.volume * (s.referenceDistance / dist);
        default:
            return s.volume;
        }
    }

    float calculatePan(float dx, float dist) {
        if (dist < 0.001f)
            return 0.0f;                             // Center
        return std::clamp(dx / 100.0f, -1.0f, 1.0f); // Simple linear pan
    }

    void applyHardwareMix(AudioSource& source, float gain, float pan) {
        source.lastGain = std::clamp(gain, 0.0f, source.volume);
        source.lastPan = std::clamp(pan, -1.0f, 1.0f);

        if (!m_core) {
            pushDiagnostic("audio_mixer.core_missing", "AudioMixer has no AudioCore binding.", source.handle,
                           source.assetPath);
            return;
        }
        if (source.handle == 0) {
            pushDiagnostic("audio_mixer.handle_missing", "AudioMixer source has no backend handle.", 0,
                           source.assetPath);
            return;
        }
        if (!m_core->setSpatialMix(source.handle, source.lastGain, source.lastPan)) {
            pushDiagnostic("audio_mixer.backend_source_missing", "AudioCore rejected spatial mix update for source.",
                           source.handle, source.assetPath);
        }
    }

    AudioSource* findSource(AudioHandle handle) {
        for (auto& source : m_sources) {
            if (source.handle == handle) {
                return &source;
            }
        }
        return nullptr;
    }

    void pushDiagnostic(std::string code, std::string message, AudioHandle handle, std::string assetPath) {
        m_diagnostics.push_back({std::move(code), std::move(message), handle, std::move(assetPath)});
    }

    std::vector<AudioSource> m_sources;
    std::vector<AudioMixerDiagnostic> m_diagnostics;
    std::string m_currentBGM;
    std::string m_targetBGM;
    AudioCore* m_core = nullptr;
};

} // namespace urpg::audio
