#include "audio_mix_backend_smoke.h"

namespace urpg::audio {

namespace {

std::string categoryToString(AudioCategory category) {
    switch (category) {
    case AudioCategory::BGM:
        return "BGM";
    case AudioCategory::BGS:
        return "BGS";
    case AudioCategory::SE:
        return "SE";
    case AudioCategory::ME:
        return "ME";
    case AudioCategory::System:
        return "System";
    }
    return "System";
}

nlohmann::json diagnosticsToJson(const std::vector<AudioBackendDiagnostic>& diagnostics) {
    auto j = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics) {
        j.push_back({
            {"code", diagnostic.code},
            {"message", diagnostic.message},
            {"assetId", diagnostic.asset_id},
        });
    }
    return j;
}

} // namespace

AudioMixBackendSmokeResult runAudioMixBackendSmoke(const AudioMixPresetBank& bank, AudioCore& core,
                                                   const AudioMixBackendSmokeRequest& request) {
    AudioMixBackendSmokeResult result;
    result.presetName = request.presetName;
    result.probeAssetId = request.probeAssetId;
    result.deviceOpenBefore = core.isBackendDeviceOpen();

    const auto preset = bank.loadPreset(request.presetName);
    result.presetFound = preset.has_value();
    if (!preset.has_value()) {
        result.diagnostics.push_back({
            "audio.mix_preset_missing",
            "Audio backend smoke could not apply the requested mix preset.",
            request.presetName,
        });
        result.deviceOpenAfter = core.isBackendDeviceOpen();
        return result;
    }

    result.presetApplied = bank.applyPreset(core, request.presetName);
    result.duckBGMOnSE = core.getDuckBGMOnSE();
    result.duckAmount = core.getDuckAmount();
    for (const auto& [category, volume] : preset->categoryVolumes) {
        result.categoryVolumes[category] = core.getCategoryVolume(category);
    }

    if (request.triggerProbePlayback) {
        result.probeAttempted = true;
        const auto handle = core.playSound(request.probeAssetId, request.probeCategory, request.probeVolume,
                                          request.probePitch);
        result.probeAccepted = handle != 0;
        if (handle != 0) {
            core.stopHandle(handle);
        }
    }

    result.deviceOpenAfter = core.isBackendDeviceOpen();
    result.diagnostics = core.audioDiagnostics();
    return result;
}

nlohmann::json audioMixBackendSmokeResultToJson(const AudioMixBackendSmokeResult& result) {
    nlohmann::json volumes = nlohmann::json::object();
    for (const auto& [category, volume] : result.categoryVolumes) {
        volumes[categoryToString(category)] = volume;
    }

    return {
        {"presetName", result.presetName},
        {"probeAssetId", result.probeAssetId},
        {"presetFound", result.presetFound},
        {"presetApplied", result.presetApplied},
        {"probeAttempted", result.probeAttempted},
        {"probeAccepted", result.probeAccepted},
        {"deviceOpenBefore", result.deviceOpenBefore},
        {"deviceOpenAfter", result.deviceOpenAfter},
        {"duckBGMOnSE", result.duckBGMOnSE},
        {"duckAmount", result.duckAmount},
        {"categoryVolumes", volumes},
        {"diagnostics", diagnosticsToJson(result.diagnostics)},
    };
}

std::vector<AudioBackendMatrixFixture> defaultAudioBackendMatrixFixtures() {
    return {
        {"null_backend", "null", true, false, 0, false, "Default"},
        {"sdl_backend_available", "sdl", true, true, 2, false, "Battle"},
        {"sdl_backend_unavailable", "sdl", false, false, 0, false, "Battle"},
        {"missing_output_device", "sdl", true, false, 0, true, "Default"},
        {"stereo_output", "sdl", true, true, 2, false, "Cinematic"},
        {"muted_release_fallback", "sdl", true, false, 0, true, "Battle"},
    };
}

std::vector<AudioBackendMatrixResult>
evaluateAudioBackendMatrix(const AudioMixPresetBank& bank, const std::vector<AudioBackendMatrixFixture>& fixtures) {
    std::vector<AudioBackendMatrixResult> results;
    results.reserve(fixtures.size());

    for (const auto& fixture : fixtures) {
        AudioBackendMatrixResult result;
        result.fixtureId = fixture.fixtureId;
        result.backendId = fixture.backendId;
        result.channelCount = fixture.channelCount;
        result.presetApplied = bank.loadPreset(fixture.presetName).has_value();

        if (fixture.backendId == "null") {
            result.deviceState = "not_applicable";
            result.fallbackPolicy = "silent_noop";
            result.releaseSafe = result.presetApplied;
            result.lastPlaybackDiagnostic = {
                "audio.null_backend",
                "Null backend accepted preset state without starting playback.",
                fixture.presetName,
            };
        } else if (!fixture.backendAvailable) {
            result.deviceState = "backend_unavailable";
            result.fallbackPolicy = "blocked_backend";
            result.releaseSafe = false;
            result.lastPlaybackDiagnostic = {
                "audio.backend_unavailable",
                "Requested audio backend is unavailable; playback was not marked active.",
                fixture.backendId,
            };
        } else if (!fixture.outputDevicePresent) {
            result.deviceState = "missing_output_device";
            result.fallbackPolicy =
                fixture.releaseMutedFallbackAllowed ? "muted_release_fallback" : "blocked_no_device";
            result.releaseSafe = result.presetApplied && fixture.releaseMutedFallbackAllowed;
            result.lastPlaybackDiagnostic = {
                fixture.releaseMutedFallbackAllowed ? "audio.muted_release_fallback" : "audio.output_device_missing",
                fixture.releaseMutedFallbackAllowed
                    ? "No output device is present; release-safe muted fallback is active."
                    : "No output device is present and no muted fallback policy is configured.",
                fixture.backendId,
            };
        } else if (fixture.channelCount == 2) {
            result.deviceState = fixture.fixtureId == "stereo_output" ? "stereo_output" : "available";
            result.fallbackPolicy = "normal_playback";
            result.playbackActive = result.presetApplied;
            result.releaseSafe = result.presetApplied;
            result.lastPlaybackDiagnostic = {
                "audio.stereo_output_ready",
                "Stereo output device accepted preset playback evidence.",
                fixture.backendId,
            };
        } else {
            result.deviceState = "unsupported_channel_layout";
            result.fallbackPolicy = "blocked_channel_layout";
            result.releaseSafe = false;
            result.lastPlaybackDiagnostic = {
                "audio.channel_layout_unsupported",
                "Audio output channel layout is not supported by the release matrix.",
                fixture.backendId,
            };
        }

        results.push_back(std::move(result));
    }

    return results;
}

nlohmann::json audioBackendMatrixResultToJson(const AudioBackendMatrixResult& result) {
    return {
        {"fixtureId", result.fixtureId},
        {"backendId", result.backendId},
        {"deviceState", result.deviceState},
        {"presetApplied", result.presetApplied},
        {"playbackActive", result.playbackActive},
        {"fallbackPolicy", result.fallbackPolicy},
        {"channelCount", result.channelCount},
        {"releaseSafe", result.releaseSafe},
        {"lastPlaybackDiagnostic",
         {
             {"code", result.lastPlaybackDiagnostic.code},
             {"message", result.lastPlaybackDiagnostic.message},
             {"assetId", result.lastPlaybackDiagnostic.asset_id},
         }},
    };
}

nlohmann::json audioBackendMatrixResultsToJson(const std::vector<AudioBackendMatrixResult>& results) {
    nlohmann::json rows = nlohmann::json::array();
    size_t mutedFallbackCount = 0;
    for (const auto& result : results) {
        if (result.releaseSafe && result.fallbackPolicy == "muted_release_fallback") {
            ++mutedFallbackCount;
        }
        rows.push_back(audioBackendMatrixResultToJson(result));
    }

    return {
        {"fixtureCount", results.size()},
        {"releaseSafeMutedFallbackCount", mutedFallbackCount},
        {"results", rows},
    };
}

} // namespace urpg::audio
