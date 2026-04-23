#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <atomic>
#include "../global_state_hub.h"

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
        std::string asset_id;
        float volume = 1.0f;
        float pitch = 1.0f;
        bool isLooping = false;
        AudioCategory category = AudioCategory::System;
    };

    struct ActiveSourceSnapshot {
        AudioHandle handle = 0;
        std::string asset_id;
        float volume = 1.0f;
        float pitch = 1.0f;
        bool isLooping = false;
        AudioCategory category = AudioCategory::System;
    };

    AudioCore() {
        // Sync initial volumes and subscribe to changes
        m_stateHandle = GlobalStateHub::getInstance().subscribe("*", [this](const std::string& key, const GlobalStateHub::Value& value) {
            this->onConfigChanged(key, value);
        });
        
        syncVolumes();
    }

    ~AudioCore() {
        GlobalStateHub::getInstance().unsubscribe(m_stateHandle);
    }

    /**
     * @brief Plays a sound effect or system sound.
     */
    AudioHandle playSound(const std::string& assetId, AudioCategory category = AudioCategory::SE) {
        // In a real implementation, this would interface with OpenAL/SDL_Mixer/SoLoud
        AudioHandle handle = ++m_handleCounter;
        m_activeSources[handle] = { assetId, 1.0f, 1.0f, false, category };
        return handle;
    }

    /**
     * @brief Plays background music with crossfade support.
     */
    void playBGM(const std::string& assetId, float fadeSeconds = 1.0f) {
        (void)fadeSeconds;
        m_currentBGM = assetId;
        // Logic for crossfading would go here
    }

    const std::string& currentBGM() const {
        return m_currentBGM;
    }

    size_t activeSourceCount() const {
        return m_activeSources.size();
    }

    std::vector<ActiveSourceSnapshot> activeSources() const {
        std::vector<ActiveSourceSnapshot> snapshots;
        snapshots.reserve(m_activeSources.size());

        for (const auto& [handle, state] : m_activeSources) {
            snapshots.push_back({
                handle,
                state.asset_id,
                state.volume,
                state.pitch,
                state.isLooping,
                state.category,
            });
        }

        return snapshots;
    }

    void setCategoryVolume(AudioCategory category, float volume) {
        m_categoryVolumes[category] = volume;
        // In a real engine, this would update active channel gains
    }

    float getCategoryVolume(AudioCategory category) const {
        if (auto it = m_categoryVolumes.find(category); it != m_categoryVolumes.end()) {
            return it->second;
        }
        return 1.0f;
    }

    /**
     * @brief Applies SE-triggered BGM ducking parameters at runtime.
     *
     * When @p enabled is true the audio backend attenuates the BGM channel by
     * @p duckAmount (0.0 = silence, 1.0 = no attenuation) whenever an SE
     * channel is active.  Setting @p enabled to false restores the BGM channel
     * to its pre-duck volume immediately.
     *
     * @param enabled    Whether ducking is active.
     * @param duckAmount Attenuation factor in [0.0, 1.0].
     */
    void setDuckBGMOnSE(bool enabled, float duckAmount = 1.0f) {
        m_duckBGMOnSE = enabled;
        m_duckAmount = duckAmount;
        // In a real engine, this would be forwarded to the hardware mixer thread.
    }

    bool getDuckBGMOnSE() const { return m_duckBGMOnSE; }
    float getDuckAmount() const { return m_duckAmount; }

    void stopAll() {
        m_activeSources.clear();
        m_currentBGM.clear();
    }

private:
    void syncVolumes() {
        auto& hub = GlobalStateHub::getInstance();
        setCategoryVolume(AudioCategory::BGM, std::stof(hub.getConfig("audio.bgm_volume", "1.0")));
        setCategoryVolume(AudioCategory::BGS, std::stof(hub.getConfig("audio.bgs_volume", "1.0")));
        setCategoryVolume(AudioCategory::SE, std::stof(hub.getConfig("audio.se_volume", "1.0")));
        setCategoryVolume(AudioCategory::ME, std::stof(hub.getConfig("audio.me_volume", "1.0")));
        setCategoryVolume(AudioCategory::System, std::stof(hub.getConfig("audio.system_volume", "1.0")));
    }

    void onConfigChanged(const std::string& key, const GlobalStateHub::Value& value) {
        if (key.rfind("audio.", 0) != 0) {
            return;
        }
        if (!std::holds_alternative<std::string>(value)) return;
        const std::string& valStr = std::get<std::string>(value);
        float volume = std::stof(valStr);

        if (key == "audio.bgm_volume") setCategoryVolume(AudioCategory::BGM, volume);
        else if (key == "audio.bgs_volume") setCategoryVolume(AudioCategory::BGS, volume);
        else if (key == "audio.se_volume") setCategoryVolume(AudioCategory::SE, volume);
        else if (key == "audio.me_volume") setCategoryVolume(AudioCategory::ME, volume);
        else if (key == "audio.system_volume") setCategoryVolume(AudioCategory::System, volume);
    }

    std::atomic<AudioHandle> m_handleCounter{0};
    uint32_t m_stateHandle = 0;
    std::map<AudioHandle, ChannelState> m_activeSources;
    std::map<AudioCategory, float> m_categoryVolumes;
    std::string m_currentBGM;
    bool m_duckBGMOnSE = false;
    float m_duckAmount = 1.0f;
};

} // namespace urpg::audio
