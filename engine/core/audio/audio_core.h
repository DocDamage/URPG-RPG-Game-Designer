#pragma once

#include "audio_core_fwd.h"
#include "audio_runtime_backend.h"

#include "../global_state_hub.h"
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace urpg::audio {

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
        float spatialGain = 1.0f;
        float pan = 0.0f;
    };

    struct ActiveSourceSnapshot {
        AudioHandle handle = 0;
        std::string asset_id;
        float volume = 1.0f;
        float pitch = 1.0f;
        bool isLooping = false;
        AudioCategory category = AudioCategory::System;
        bool backendActive = false;
        float spatialGain = 1.0f;
        float pan = 0.0f;
    };

    AudioCore() : m_backend(std::make_unique<SdlAudioRuntimeBackend>()) {
        // Sync initial volumes and subscribe to changes
        m_stateHandle = GlobalStateHub::getInstance().subscribe(
            "*",
            [this](const std::string& key, const GlobalStateHub::Value& value) { this->onConfigChanged(key, value); });

        syncVolumes();
    }

    ~AudioCore() { GlobalStateHub::getInstance().unsubscribe(m_stateHandle); }

    /**
     * @brief Plays a sound effect or system sound.
     */
    AudioHandle playSound(const std::string& assetId, AudioCategory category = AudioCategory::SE, float volume = 1.0f,
                          float pitch = 1.0f) {
        AudioHandle handle = ++m_handleCounter;
        const float normalizedVolume = clampVolume(volume);
        const float normalizedPitch = std::clamp(pitch, 0.5f, 2.0f);
        m_activeSources[handle] = {assetId, normalizedVolume, normalizedPitch, false, category, 1.0f, 0.0f};
        m_backend->play({handle, assetId, category, normalizedVolume, normalizedPitch, false, 0.0f});
        return handle;
    }

    /**
     * @brief Plays background music with crossfade support.
     */
    void playBGM(const std::string& assetId, float fadeSeconds = 1.0f) {
        if (m_bgmHandle != 0) {
            m_backend->stop(m_bgmHandle, fadeSeconds);
            m_activeSources.erase(m_bgmHandle);
        }
        AudioHandle handle = ++m_handleCounter;
        m_currentBGM = assetId;
        m_bgmHandle = handle;
        m_activeSources[handle] = {assetId, 1.0f, 1.0f, true, AudioCategory::BGM, 1.0f, 0.0f};
        m_backend->play({handle, assetId, AudioCategory::BGM, 1.0f, 1.0f, true, fadeSeconds});
    }

    void playBGS(const std::string& assetId, float fadeSeconds = 1.0f) {
        if (m_bgsHandle != 0) {
            m_backend->stop(m_bgsHandle, fadeSeconds);
            m_activeSources.erase(m_bgsHandle);
        }
        AudioHandle handle = ++m_handleCounter;
        m_currentBGS = assetId;
        m_bgsHandle = handle;
        m_activeSources[handle] = {assetId, 1.0f, 1.0f, true, AudioCategory::BGS, 1.0f, 0.0f};
        m_backend->play({handle, assetId, AudioCategory::BGS, 1.0f, 1.0f, true, fadeSeconds});
    }

    void playME(const std::string& assetId, float volume = 1.0f, float pitch = 1.0f) {
        if (m_meHandle != 0) {
            m_backend->stop(m_meHandle);
            m_activeSources.erase(m_meHandle);
        }
        AudioHandle handle = ++m_handleCounter;
        m_meHandle = handle;
        const float normalizedVolume = clampVolume(volume);
        const float normalizedPitch = std::clamp(pitch, 0.5f, 2.0f);
        m_activeSources[handle] = {assetId, normalizedVolume, normalizedPitch, false, AudioCategory::ME, 1.0f, 0.0f};
        m_backend->play({handle, assetId, AudioCategory::ME, normalizedVolume, normalizedPitch, false, 0.0f});
    }

    const std::string& currentBGM() const { return m_currentBGM; }

    const std::string& currentBGS() const { return m_currentBGS; }

    size_t activeSourceCount() const {
        return static_cast<size_t>(std::count_if(m_activeSources.begin(), m_activeSources.end(), [](const auto& entry) {
            return entry.second.category != AudioCategory::BGM && entry.second.category != AudioCategory::BGS;
        }));
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
                m_backend->hasActivePlayback(handle),
                state.spatialGain,
                state.pan,
            });
        }

        return snapshots;
    }

    void setCategoryVolume(AudioCategory category, float volume) {
        m_categoryVolumes[category] = volume;
        if (category == AudioCategory::BGM && m_duckBGMOnSE) {
            applyRuntimeDucking();
        } else {
            m_backend->setCategoryVolume(category, clampVolume(volume));
        }
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
        m_duckAmount = clampVolume(duckAmount);
        applyRuntimeDucking();
    }

    bool getDuckBGMOnSE() const { return m_duckBGMOnSE; }
    float getDuckAmount() const { return m_duckAmount; }

    void stopAll() {
        m_backend->stopAll();
        m_activeSources.clear();
        m_currentBGM.clear();
        m_currentBGS.clear();
        m_bgmHandle = 0;
        m_bgsHandle = 0;
        m_meHandle = 0;
    }

    void stopHandle(AudioHandle handle, float fadeSeconds = 0.0f) {
        m_backend->stop(handle, fadeSeconds);
        if (handle == m_bgmHandle) {
            m_currentBGM.clear();
            m_bgmHandle = 0;
        }
        if (handle == m_bgsHandle) {
            m_currentBGS.clear();
            m_bgsHandle = 0;
        }
        if (handle == m_meHandle) {
            m_meHandle = 0;
        }
        m_activeSources.erase(handle);
    }

    void stopCategory(AudioCategory category, float fadeSeconds = 0.0f) {
        m_backend->stopCategory(category, fadeSeconds);
        for (auto it = m_activeSources.begin(); it != m_activeSources.end();) {
            if (it->second.category == category) {
                it = m_activeSources.erase(it);
            } else {
                ++it;
            }
        }
        if (category == AudioCategory::BGM) {
            m_currentBGM.clear();
            m_bgmHandle = 0;
        } else if (category == AudioCategory::BGS) {
            m_currentBGS.clear();
            m_bgsHandle = 0;
        } else if (category == AudioCategory::ME) {
            m_meHandle = 0;
        }
    }

    bool isBackendDeviceOpen() const { return m_backend->isDeviceOpen(); }

    void setAssetRoot(std::filesystem::path root) { m_backend->setAssetRoot(std::move(root)); }

    std::vector<AudioBackendDiagnostic> audioDiagnostics() const {
        auto diagnostics = m_backend->diagnostics();
        diagnostics.insert(diagnostics.end(), m_diagnostics.begin(), m_diagnostics.end());
        return diagnostics;
    }

    void clearAudioDiagnostics() {
        m_backend->clearDiagnostics();
        m_diagnostics.clear();
    }

    bool setSpatialMix(AudioHandle handle, float gain, float pan) {
        auto it = m_activeSources.find(handle);
        if (it == m_activeSources.end()) {
            m_diagnostics.push_back({
                "audio.source_missing",
                "AudioCore source handle is not active.",
                std::to_string(handle),
            });
            return false;
        }

        it->second.spatialGain = clampVolume(gain);
        it->second.pan = std::clamp(pan, -1.0f, 1.0f);
        if (!m_backend->setHandleMix(handle, it->second.spatialGain, it->second.pan)) {
            m_diagnostics.push_back({
                "audio.backend_source_missing",
                "AudioCore source is tracked, but the runtime backend has no active source for that handle.",
                it->second.asset_id,
            });
            return false;
        }
        return true;
    }

  private:
    static float clampVolume(float volume) { return std::clamp(volume, 0.0f, 1.0f); }

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
        if (!std::holds_alternative<std::string>(value))
            return;
        const std::string& valStr = std::get<std::string>(value);
        float volume = std::stof(valStr);

        if (key == "audio.bgm_volume")
            setCategoryVolume(AudioCategory::BGM, volume);
        else if (key == "audio.bgs_volume")
            setCategoryVolume(AudioCategory::BGS, volume);
        else if (key == "audio.se_volume")
            setCategoryVolume(AudioCategory::SE, volume);
        else if (key == "audio.me_volume")
            setCategoryVolume(AudioCategory::ME, volume);
        else if (key == "audio.system_volume")
            setCategoryVolume(AudioCategory::System, volume);
    }

    void applyRuntimeDucking() {
        const float bgmVolume = getCategoryVolume(AudioCategory::BGM);
        m_backend->setCategoryVolume(AudioCategory::BGM, m_duckBGMOnSE ? bgmVolume * m_duckAmount : bgmVolume);
    }

    std::atomic<AudioHandle> m_handleCounter{0};
    uint32_t m_stateHandle = 0;
    std::map<AudioHandle, ChannelState> m_activeSources;
    std::map<AudioCategory, float> m_categoryVolumes;
    std::string m_currentBGM;
    std::string m_currentBGS;
    AudioHandle m_bgmHandle = 0;
    AudioHandle m_bgsHandle = 0;
    AudioHandle m_meHandle = 0;
    bool m_duckBGMOnSE = false;
    float m_duckAmount = 1.0f;
    std::unique_ptr<SdlAudioRuntimeBackend> m_backend;
    std::vector<AudioBackendDiagnostic> m_diagnostics;
};

} // namespace urpg::audio
