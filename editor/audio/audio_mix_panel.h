#pragma once

#include <nlohmann/json.hpp>
#include "../../engine/core/audio/audio_mix_backend_smoke.h"
#include "../../engine/core/audio/audio_mix_presets.h"
#include "../../engine/core/audio/audio_core.h"

namespace urpg::editor {

/**
 * @brief Editor panel for inspecting and selecting audio mix presets.
 *
 * Call bindCore() with a live AudioCore to make selectPreset() apply the
 * chosen preset parameters (category volumes and duck-BGM-on-SE) to the
 * running audio backend immediately.
 */
class AudioMixPanel {
public:
    void bindBank(urpg::audio::AudioMixPresetBank* bank);
    /**
     * @brief Bind a live AudioCore so that selectPreset() applies parameters
     *        to the running backend.  Pass nullptr to detach.
     */
    void bindCore(urpg::audio::AudioCore* core);
    /**
     * @brief Select the named preset as the active preset and, when a live
     *        core is bound, immediately apply its parameters to that core.
     * @return true if the preset exists and was applied (or selected when no
     *         core is bound), false if the preset is unknown.
     */
    bool selectPreset(const std::string& name);
    /**
     * @brief Apply a preset to the live core and run a deterministic backend
     *        playback smoke probe. The latest result is included in render().
     */
    bool runBackendSmoke(const std::string& presetName);
    void setBackendMatrixEvidence(const std::vector<urpg::audio::AudioBackendMatrixResult>& results);
    void render();

    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::audio::AudioMixPresetBank* m_bank = nullptr;
    urpg::audio::AudioCore*          m_core = nullptr;
    std::string m_selectedPreset;
    nlohmann::json m_lastBackendSmoke;
    nlohmann::json m_backendMatrixEvidence;
    nlohmann::json m_lastSnapshot;
};

} // namespace urpg::editor
