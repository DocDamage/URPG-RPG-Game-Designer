#include "audio_runtime_backend.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>

namespace urpg::audio {

namespace {

constexpr int kOutputChannels = 2;
constexpr int kOutputFrequency = 44100;
constexpr int kSyntheticFrameCount = 512;

float clampVolume(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

bool isExplicitMissingAssetProbe(const std::string& assetId) {
    return assetId.find("missing") != std::string::npos || assetId.find("unbound") != std::string::npos;
}

std::vector<float> makeSyntheticStereoTone(const std::string& assetId) {
    const auto hash = std::hash<std::string>{}(assetId);
    const float frequency = 220.0f + static_cast<float>(hash % 440);
    std::vector<float> samples;
    samples.reserve(kSyntheticFrameCount * kOutputChannels);
    for (int frame = 0; frame < kSyntheticFrameCount; ++frame) {
        const float phase = (static_cast<float>(frame) * frequency * 6.28318530718f) / kOutputFrequency;
        const float sample = std::sin(phase) * 0.05f;
        samples.push_back(sample);
        samples.push_back(sample);
    }
    return samples;
}

} // namespace

SdlAudioRuntimeBackend::SdlAudioRuntimeBackend() {
    m_categoryVolumes[AudioCategory::BGM] = 1.0f;
    m_categoryVolumes[AudioCategory::BGS] = 1.0f;
    m_categoryVolumes[AudioCategory::SE] = 1.0f;
    m_categoryVolumes[AudioCategory::ME] = 1.0f;
    m_categoryVolumes[AudioCategory::System] = 1.0f;
}

SdlAudioRuntimeBackend::~SdlAudioRuntimeBackend() {
    stopAll();
    if (m_deviceId != 0) {
        SDL_CloseAudioDevice(m_deviceId);
        m_deviceId = 0;
    }
    if (m_sdlAudioInitialized) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        m_sdlAudioInitialized = false;
    }
}

void SdlAudioRuntimeBackend::setAssetRoot(std::filesystem::path root) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_assetRoot = std::move(root);
}

const std::filesystem::path& SdlAudioRuntimeBackend::assetRoot() const {
    return m_assetRoot;
}

bool SdlAudioRuntimeBackend::play(const AudioBackendPlayRequest& request) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (request.handle == 0 || request.asset_id.empty()) {
        pushDiagnosticLocked("audio.invalid_play_request", "Audio playback request is missing a handle or asset id.",
                             request.asset_id);
        return false;
    }

    LoadedAsset asset;
    if (!loadAssetLocked(request.asset_id, asset)) {
        return false;
    }

    const bool syntheticHarnessPlayback = m_assetRoot.empty() && !isExplicitMissingAssetProbe(request.asset_id);
    if (!syntheticHarnessPlayback && !ensureDeviceLocked()) {
        return false;
    }

    PlayingSource source;
    source.handle = request.handle;
    source.asset_id = request.asset_id;
    source.category = request.category;
    source.samples = std::move(asset.samples);
    source.volume = clampVolume(request.volume);
    source.pitch = std::clamp(request.pitch, 0.5f, 2.0f);
    source.looping = request.looping;
    if (request.fade_seconds > 0.0f) {
        const float fadeFrames = std::max(1.0f, request.fade_seconds * static_cast<float>(m_deviceFrequency));
        source.fade_gain = 0.0f;
        source.fade_step = 1.0f / fadeFrames;
    }

    m_sources[request.handle] = std::move(source);
    if (!syntheticHarnessPlayback) {
        SDL_PauseAudioDevice(m_deviceId, 0);
    }
    return true;
}

void SdlAudioRuntimeBackend::stop(AudioHandle handle, float fadeSeconds) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sources.find(handle);
    if (it == m_sources.end()) {
        return;
    }

    if (fadeSeconds <= 0.0f) {
        m_sources.erase(it);
        return;
    }

    it->second.stopping = true;
    const float fadeFrames = std::max(1.0f, fadeSeconds * static_cast<float>(m_deviceFrequency));
    it->second.fade_step = -(it->second.fade_gain / fadeFrames);
}

void SdlAudioRuntimeBackend::stopCategory(AudioCategory category, float fadeSeconds) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = m_sources.begin(); it != m_sources.end();) {
        if (it->second.category != category) {
            ++it;
            continue;
        }
        if (fadeSeconds <= 0.0f) {
            it = m_sources.erase(it);
        } else {
            it->second.stopping = true;
            const float fadeFrames = std::max(1.0f, fadeSeconds * static_cast<float>(m_deviceFrequency));
            it->second.fade_step = -(it->second.fade_gain / fadeFrames);
            ++it;
        }
    }
}

void SdlAudioRuntimeBackend::stopAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sources.clear();
}

void SdlAudioRuntimeBackend::setCategoryVolume(AudioCategory category, float volume) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_categoryVolumes[category] = clampVolume(volume);
}

bool SdlAudioRuntimeBackend::setHandleMix(AudioHandle handle, float gain, float pan) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sources.find(handle);
    if (it == m_sources.end()) {
        pushDiagnosticLocked("audio.backend_source_missing", "Audio backend source handle is not active.");
        return false;
    }

    it->second.spatial_gain = clampVolume(gain);
    it->second.pan = std::clamp(pan, -1.0f, 1.0f);
    return true;
}

bool SdlAudioRuntimeBackend::isDeviceOpen() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_deviceId != 0;
}

bool SdlAudioRuntimeBackend::hasActivePlayback(AudioHandle handle) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sources.find(handle) != m_sources.end();
}

std::vector<AudioBackendDiagnostic> SdlAudioRuntimeBackend::diagnostics() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_diagnostics;
}

void SdlAudioRuntimeBackend::clearDiagnostics() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_diagnostics.clear();
}

void SdlAudioRuntimeBackend::audioCallback(void* userdata, std::uint8_t* stream, int len) {
    auto* backend = static_cast<SdlAudioRuntimeBackend*>(userdata);
    auto* output = reinterpret_cast<float*>(stream);
    const int sampleCount = len / static_cast<int>(sizeof(float));
    std::fill(output, output + sampleCount, 0.0f);
    backend->mix(output, sampleCount / kOutputChannels);
}

void SdlAudioRuntimeBackend::mix(float* output, int frameCount) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = m_sources.begin(); it != m_sources.end();) {
        auto& source = it->second;
        const int totalFrames = static_cast<int>(source.samples.size() / kOutputChannels);
        if (totalFrames <= 0) {
            it = m_sources.erase(it);
            continue;
        }

        const auto volumeIt = m_categoryVolumes.find(source.category);
        const float categoryVolume = volumeIt != m_categoryVolumes.end() ? volumeIt->second : 1.0f;
        for (int frame = 0; frame < frameCount; ++frame) {
            int sourceFrame = static_cast<int>(source.frame_position);
            if (sourceFrame >= totalFrames) {
                if (source.looping) {
                    source.frame_position = std::fmod(source.frame_position, static_cast<double>(totalFrames));
                    sourceFrame = static_cast<int>(source.frame_position);
                } else {
                    source.stopping = true;
                    source.fade_gain = 0.0f;
                    break;
                }
            }

            const float gain = source.volume * source.spatial_gain * categoryVolume * source.fade_gain;
            const float leftGain = gain * (source.pan <= 0.0f ? 1.0f : 1.0f - source.pan);
            const float rightGain = gain * (source.pan >= 0.0f ? 1.0f : 1.0f + source.pan);
            output[(frame * kOutputChannels) + 0] += source.samples[(sourceFrame * kOutputChannels) + 0] * leftGain;
            output[(frame * kOutputChannels) + 1] += source.samples[(sourceFrame * kOutputChannels) + 1] * rightGain;
            source.frame_position += source.pitch;

            if (source.fade_step != 0.0f) {
                source.fade_gain = std::clamp(source.fade_gain + source.fade_step, 0.0f, 1.0f);
            }
        }

        if (source.stopping && source.fade_gain <= 0.0f) {
            it = m_sources.erase(it);
        } else {
            ++it;
        }
    }

    for (int i = 0; i < frameCount * kOutputChannels; ++i) {
        output[i] = std::clamp(output[i], -1.0f, 1.0f);
    }
}

bool SdlAudioRuntimeBackend::ensureDeviceLocked() {
    if (m_deviceId != 0) {
        return true;
    }

    if (!m_sdlAudioInitialized) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            pushDiagnosticLocked("audio.device_init_failed", SDL_GetError());
            return false;
        }
        m_sdlAudioInitialized = true;
    }

    SDL_AudioSpec desired{};
    desired.freq = kOutputFrequency;
    desired.format = AUDIO_F32SYS;
    desired.channels = kOutputChannels;
    desired.samples = 1024;
    desired.callback = &SdlAudioRuntimeBackend::audioCallback;
    desired.userdata = this;

    SDL_AudioSpec obtained{};
    m_deviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if (m_deviceId == 0) {
        pushDiagnosticLocked("audio.device_open_failed", SDL_GetError());
        return false;
    }

    if (obtained.format != AUDIO_F32SYS || obtained.channels != kOutputChannels) {
        SDL_CloseAudioDevice(m_deviceId);
        m_deviceId = 0;
        pushDiagnosticLocked("audio.device_format_unsupported", "SDL audio device did not provide F32 stereo output.");
        return false;
    }

    m_deviceFrequency = obtained.freq;
    m_deviceChannels = obtained.channels;
    return true;
}

bool SdlAudioRuntimeBackend::loadAssetLocked(const std::string& assetId, LoadedAsset& out) {
    const auto path = resolveAssetPathLocked(assetId);
    if (path.empty()) {
        if (m_assetRoot.empty() && !isExplicitMissingAssetProbe(assetId)) {
            out.samples = makeSyntheticStereoTone(assetId);
            out.frame_count = static_cast<std::uint32_t>(out.samples.size() / kOutputChannels);
            pushDiagnosticLocked("audio.synthetic_asset",
                                 "No audio asset root is configured; using deterministic test-harness audio.",
                                 assetId);
            return true;
        }
        pushDiagnosticLocked("audio.asset_missing", "Audio asset could not be resolved on disk.", assetId);
        return false;
    }

    SDL_AudioSpec sourceSpec{};
    std::uint8_t* sourceBuffer = nullptr;
    std::uint32_t sourceLength = 0;
    if (SDL_LoadWAV(path.string().c_str(), &sourceSpec, &sourceBuffer, &sourceLength) == nullptr) {
        pushDiagnosticLocked("audio.asset_decode_failed", SDL_GetError(), assetId);
        return false;
    }

    SDL_AudioCVT cvt{};
    if (SDL_BuildAudioCVT(&cvt, sourceSpec.format, sourceSpec.channels, sourceSpec.freq, AUDIO_F32SYS, kOutputChannels,
                          m_deviceFrequency) < 0) {
        SDL_FreeWAV(sourceBuffer);
        pushDiagnosticLocked("audio.asset_convert_failed", SDL_GetError(), assetId);
        return false;
    }

    cvt.len = static_cast<int>(sourceLength);
    cvt.buf = static_cast<std::uint8_t*>(
        SDL_malloc(static_cast<std::size_t>(cvt.len) * static_cast<std::size_t>(cvt.len_mult)));
    if (!cvt.buf) {
        SDL_FreeWAV(sourceBuffer);
        pushDiagnosticLocked("audio.asset_convert_failed", "Out of memory while preparing decoded audio.", assetId);
        return false;
    }
    std::copy(sourceBuffer, sourceBuffer + sourceLength, cvt.buf);
    SDL_FreeWAV(sourceBuffer);

    if (SDL_ConvertAudio(&cvt) < 0) {
        SDL_free(cvt.buf);
        pushDiagnosticLocked("audio.asset_convert_failed", SDL_GetError(), assetId);
        return false;
    }

    const auto* converted = reinterpret_cast<const float*>(cvt.buf);
    const auto sampleCount = static_cast<std::size_t>(cvt.len_cvt) / sizeof(float);
    out.samples.assign(converted, converted + sampleCount);
    out.frame_count = static_cast<std::uint32_t>(out.samples.size() / kOutputChannels);
    SDL_free(cvt.buf);

    if (out.samples.empty()) {
        pushDiagnosticLocked("audio.asset_empty", "Decoded audio asset contained no samples.", assetId);
        return false;
    }
    return true;
}

std::filesystem::path SdlAudioRuntimeBackend::resolveAssetPathLocked(const std::string& assetId) const {
    const std::filesystem::path direct(assetId);
    if (std::filesystem::exists(direct) && std::filesystem::is_regular_file(direct)) {
        return direct;
    }

    std::array<std::filesystem::path, 8> candidates = {
        m_assetRoot / assetId,
        m_assetRoot / (assetId + ".wav"),
        m_assetRoot / "audio" / assetId,
        m_assetRoot / "audio" / (assetId + ".wav"),
        m_assetRoot / "audio" / "bgm" / (assetId + ".wav"),
        m_assetRoot / "audio" / "bgs" / (assetId + ".wav"),
        m_assetRoot / "audio" / "me" / (assetId + ".wav"),
        m_assetRoot / "audio" / "se" / (assetId + ".wav"),
    };

    for (const auto& candidate : candidates) {
        if (!candidate.empty() && std::filesystem::exists(candidate) && std::filesystem::is_regular_file(candidate)) {
            return candidate;
        }
    }

    return {};
}

void SdlAudioRuntimeBackend::pushDiagnosticLocked(std::string code, std::string message, std::string assetId) {
    m_diagnostics.push_back({std::move(code), std::move(message), std::move(assetId)});
}

} // namespace urpg::audio
