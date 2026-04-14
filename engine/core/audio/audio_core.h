#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <atomic>

namespace urpg::audio {

/**
 * @brief Handle to an active audio source/stream.
 */
using AudioHandle = uint64_t;

/**
 * @brief Native representation of audio categories for mixing.
 */
enum class AudioCategory : uint8_t {
    BGM,
    BGS,
    SE,
    ME,
    System
};

/**
 * @brief Authoritative Mixer for native audio playback.
 */
class AudioCore {
public:
    struct ChannelState {
        float volume = 1.0f;
        float pitch = 1.0f;
        bool isLooping = false;
        AudioCategory category = AudioCategory::System;
    };

    /**
     * @brief Plays a sound effect or system sound.
     */
    AudioHandle playSound(const std::string& assetId, AudioCategory category = AudioCategory::SE) {
        // In a real implementation, this would interface with OpenAL/SDL_Mixer/SoLoud
        AudioHandle handle = ++m_handleCounter;
        m_activeSources[handle] = { 1.0f, 1.0f, false, category };
        return handle;
    }

    /**
     * @brief Plays background music with crossfade support.
     */
    void playBGM(const std::string& assetId, float fadeSeconds = 1.0f) {
        m_currentBGM = assetId;
        // Logic for crossfading would go here
    }

    void setCategoryVolume(AudioCategory category, float volume) {
        m_categoryVolumes[category] = volume;
    }

    float getCategoryVolume(AudioCategory category) const {
        if (auto it = m_categoryVolumes.find(category); it != m_categoryVolumes.end()) {
            return it->second;
        }
        return 1.0f;
    }

    void stopAll() {
        m_activeSources.clear();
        m_currentBGM.clear();
    }

private:
    std::atomic<AudioHandle> m_handleCounter{0};
    std::map<AudioHandle, ChannelState> m_activeSources;
    std::map<AudioCategory, float> m_categoryVolumes;
    std::string m_currentBGM;
};

} // namespace urpg::audio
