#include "audio_mix_panel.h"

namespace urpg::editor {

void AudioMixPanel::bindBank(urpg::audio::AudioMixPresetBank* bank) {
    m_bank = bank;
}

void AudioMixPanel::render() {
    nlohmann::json snapshot;

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
                        case urpg::audio::AudioCategory::BGM:    key = "BGM"; break;
                        case urpg::audio::AudioCategory::BGS:    key = "BGS"; break;
                        case urpg::audio::AudioCategory::SE:     key = "SE"; break;
                        case urpg::audio::AudioCategory::ME:     key = "ME"; break;
                        case urpg::audio::AudioCategory::System: key = "System"; break;
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
    }

    m_lastSnapshot = snapshot;
}

nlohmann::json AudioMixPanel::lastRenderSnapshot() const {
    return m_lastSnapshot;
}

} // namespace urpg::editor
