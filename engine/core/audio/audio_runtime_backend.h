#pragma once

#include "audio_core_fwd.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace urpg::audio {

struct AudioBackendDiagnostic {
    std::string code;
    std::string message;
    std::string asset_id;
};

struct AudioBackendPlayRequest {
    AudioHandle handle = 0;
    std::string asset_id;
    AudioCategory category = AudioCategory::System;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool looping = false;
    float fade_seconds = 0.0f;
};

class SdlAudioRuntimeBackend {
  public:
    SdlAudioRuntimeBackend();
    ~SdlAudioRuntimeBackend();

    SdlAudioRuntimeBackend(const SdlAudioRuntimeBackend&) = delete;
    SdlAudioRuntimeBackend& operator=(const SdlAudioRuntimeBackend&) = delete;

    void setAssetRoot(std::filesystem::path root);
    const std::filesystem::path& assetRoot() const;

    bool play(const AudioBackendPlayRequest& request);
    void stop(AudioHandle handle, float fadeSeconds = 0.0f);
    void stopCategory(AudioCategory category, float fadeSeconds = 0.0f);
    void stopAll();
    void setCategoryVolume(AudioCategory category, float volume);
    bool setHandleMix(AudioHandle handle, float gain, float pan);

    bool isDeviceOpen() const;
    bool hasActivePlayback(AudioHandle handle) const;
    std::vector<AudioBackendDiagnostic> diagnostics() const;
    void clearDiagnostics();

  private:
    struct LoadedAsset {
        std::vector<float> samples;
        std::uint32_t frame_count = 0;
    };

    struct PlayingSource {
        AudioHandle handle = 0;
        std::string asset_id;
        AudioCategory category = AudioCategory::System;
        std::vector<float> samples;
        double frame_position = 0.0;
        float volume = 1.0f;
        float spatial_gain = 1.0f;
        float pan = 0.0f;
        float pitch = 1.0f;
        bool looping = false;
        bool stopping = false;
        float fade_gain = 1.0f;
        float fade_step = 0.0f;
    };

    static void audioCallback(void* userdata, std::uint8_t* stream, int len);
    void mix(float* output, int frameCount);

    bool ensureDeviceLocked();
    bool loadAssetLocked(const std::string& assetId, LoadedAsset& out);
    std::filesystem::path resolveAssetPathLocked(const std::string& assetId) const;
    void pushDiagnosticLocked(std::string code, std::string message, std::string assetId = {});

    mutable std::mutex m_mutex;
    std::filesystem::path m_assetRoot;
    std::map<AudioCategory, float> m_categoryVolumes;
    std::map<AudioHandle, PlayingSource> m_sources;
    std::vector<AudioBackendDiagnostic> m_diagnostics;
    std::uint32_t m_deviceId = 0;
    int m_deviceFrequency = 44100;
    int m_deviceChannels = 2;
    bool m_sdlAudioInitialized = false;
};

} // namespace urpg::audio
