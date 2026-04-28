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

} // namespace urpg::audio
