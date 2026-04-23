#include "editor/accessibility/accessibility_audio_adapter.h"

namespace urpg::editor {

using namespace urpg::accessibility;

std::vector<UiElementSnapshot> AccessibilityAudioAdapter::ingest(
    const urpg::audio::AudioMixPresetBank& bank) {
    std::vector<UiElementSnapshot> elements;

    const auto& presets = bank.presets();
    int32_t focusIndex = 1;

    for (const auto& [name, preset] : presets) {
        UiElementSnapshot el;
        el.id = "audio.preset." + name;
        // Presets with unknown category names are given an empty label so the
        // accessibility auditor surfaces a MissingLabel issue for author review.
        el.label = preset.unknownCategoryNames.empty() ? name : std::string{};
        el.hasFocus = true;   // every preset is navigable in the audio mix panel
        el.focusOrder = focusIndex++;
        el.contrastRatio = 0.0f;
        el.sourceContext = "engine/core/audio/audio_mix_presets.h";
        elements.push_back(std::move(el));
    }

    return elements;
}

} // namespace urpg::editor
