#pragma once

#include "audio_core.h"
#include "audio_mix_presets.h"

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::audio {

struct AudioMixBackendSmokeRequest {
    std::string presetName;
    std::string probeAssetId = "audio_mix_backend_smoke_probe";
    AudioCategory probeCategory = AudioCategory::SE;
    float probeVolume = 0.25f;
    float probePitch = 1.0f;
    bool triggerProbePlayback = true;
};

struct AudioMixBackendSmokeResult {
    std::string presetName;
    std::string probeAssetId;
    bool presetFound = false;
    bool presetApplied = false;
    bool probeAttempted = false;
    bool probeAccepted = false;
    bool deviceOpenBefore = false;
    bool deviceOpenAfter = false;
    bool duckBGMOnSE = false;
    float duckAmount = 1.0f;
    std::map<AudioCategory, float> categoryVolumes;
    std::vector<AudioBackendDiagnostic> diagnostics;
};

AudioMixBackendSmokeResult runAudioMixBackendSmoke(const AudioMixPresetBank& bank, AudioCore& core,
                                                   const AudioMixBackendSmokeRequest& request);

nlohmann::json audioMixBackendSmokeResultToJson(const AudioMixBackendSmokeResult& result);

} // namespace urpg::audio
