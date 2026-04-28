#include "audio_mix_panel.h"

namespace urpg::editor {

void AudioMixPanel::bindBank(urpg::audio::AudioMixPresetBank* bank) {
    m_bank = bank;
}

void AudioMixPanel::bindCore(urpg::audio::AudioCore* core) {
    m_core = core;
}

bool AudioMixPanel::selectPreset(const std::string& name) {
    if (!m_bank) {
        return false;
    }
    auto presetOpt = m_bank->loadPreset(name);
    if (!presetOpt.has_value()) {
        return false;
    }
    m_selectedPreset = name;
    if (m_core) {
        m_bank->applyPreset(*m_core, name);
    }
    return true;
}

bool AudioMixPanel::runBackendSmoke(const std::string& presetName) {
    if (!m_bank || !m_core) {
        return false;
    }

    urpg::audio::AudioMixBackendSmokeRequest request;
    request.presetName = presetName;
    const auto result = urpg::audio::runAudioMixBackendSmoke(*m_bank, *m_core, request);
    m_lastBackendSmoke = urpg::audio::audioMixBackendSmokeResultToJson(result);
    if (result.presetApplied) {
        m_selectedPreset = presetName;
    }
    return result.presetApplied && (!result.probeAttempted || result.probeAccepted);
}

void AudioMixPanel::render() {
    nlohmann::json snapshot;
    snapshot["bankBound"] = m_bank != nullptr;
    snapshot["coreBound"] = m_core != nullptr;
    snapshot["statusMessages"] = nlohmann::json::array();
    snapshot["actions"] = {
        {"selectPreset", m_bank != nullptr},
        {"applyToLiveCore", m_bank != nullptr && m_core != nullptr},
        {"runBackendSmoke", m_bank != nullptr && m_core != nullptr},
    };

    if (m_bank) {
        auto presetNames = m_bank->listPresets();
        snapshot["presetCount"] = presetNames.size();
        snapshot["presetNames"] = presetNames;

        if (!m_selectedPreset.empty()) {
            auto presetOpt = m_bank->loadPreset(m_selectedPreset);
            if (presetOpt.has_value()) {
                nlohmann::json detail;
                detail["name"] = presetOpt->name;
                detail["duckBGMOnSE"] = presetOpt->duckBGMOnSE;
                detail["duckAmount"] = presetOpt->duckAmount;

                nlohmann::json volumes = nlohmann::json::object();
                for (const auto& [category, volume] : presetOpt->categoryVolumes) {
                    std::string key;
                    switch (category) {
                    case urpg::audio::AudioCategory::BGM:
                        key = "BGM";
                        break;
                    case urpg::audio::AudioCategory::BGS:
                        key = "BGS";
                        break;
                    case urpg::audio::AudioCategory::SE:
                        key = "SE";
                        break;
                    case urpg::audio::AudioCategory::ME:
                        key = "ME";
                        break;
                    case urpg::audio::AudioCategory::System:
                        key = "System";
                        break;
                    }
                    volumes[key] = volume;
                }
                detail["categoryVolumes"] = volumes;
                snapshot["selectedPreset"] = detail;
            } else {
                snapshot["selectedPreset"] = nullptr;
            }
        } else {
            snapshot["selectedPreset"] = nullptr;
        }
    } else {
        snapshot["presetCount"] = 0;
        snapshot["presetNames"] = nlohmann::json::array();
        snapshot["selectedPreset"] = nullptr;
        snapshot["statusMessages"].push_back("No audio mix preset bank is bound.");
    }

    // Surface live core duck state so the panel snapshot reflects runtime truth.
    if (m_core) {
        snapshot["liveCore"]["duckBGMOnSE"] = m_core->getDuckBGMOnSE();
        snapshot["liveCore"]["duckAmount"] = m_core->getDuckAmount();
    } else {
        snapshot["statusMessages"].push_back(
            "No live AudioCore is bound; preset selection will not affect runtime audio.");
    }

    snapshot["backendSmoke"] = m_lastBackendSmoke.is_null() ? nlohmann::json(nullptr) : m_lastBackendSmoke;

    m_lastSnapshot = snapshot;
}

nlohmann::json AudioMixPanel::lastRenderSnapshot() const {
    return m_lastSnapshot;
}

} // namespace urpg::editor
