#pragma once

#include "engine/core/accessibility/accessibility_auditor.h"
#include "engine/core/audio/audio_mix_presets.h"
#include <vector>

namespace urpg::editor {

/**
 * @brief Adapts a live AudioMixPresetBank into UiElementSnapshot elements
 *        for accessibility auditing.
 *
 * Each named preset becomes one UI element. Presets with unknown category names
 * (populated during JSON load) are exposed with an empty label so MissingLabel
 * issues surface for author attention.
 */
class AccessibilityAudioAdapter {
public:
    static std::vector<urpg::accessibility::UiElementSnapshot> ingest(
        const urpg::audio::AudioMixPresetBank& bank);
};

} // namespace urpg::editor
