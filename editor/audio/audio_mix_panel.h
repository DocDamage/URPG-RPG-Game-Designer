#pragma once

#include <nlohmann/json.hpp>
#include "../../engine/core/audio/audio_mix_presets.h"

namespace urpg::editor {

/**
 * @brief Editor panel for inspecting and selecting audio mix presets.
 */
class AudioMixPanel {
public:
    void bindBank(urpg::audio::AudioMixPresetBank* bank);
    void render();

    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::audio::AudioMixPresetBank* m_bank = nullptr;
    std::string m_selectedPreset;
    nlohmann::json m_lastSnapshot;
};

} // namespace urpg::editor
