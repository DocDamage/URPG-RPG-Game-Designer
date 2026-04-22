#include "audio_mix_validator.h"

namespace urpg::audio {

std::vector<AudioMixIssue> AudioMixValidator::validate(const AudioMixPresetBank& bank) const {
    std::vector<AudioMixIssue> issues;
    const auto& presets = bank.presets();

    // 1. Missing default preset
    bool hasDefault = false;
    for (const auto& [name, preset] : presets) {
        if (name == "Default") {
            hasDefault = true;
            break;
        }
    }
    if (!hasDefault) {
        issues.push_back(AudioMixIssue{
            AudioMixIssueSeverity::Error,
            AudioMixIssueCategory::MissingDefaultPreset,
            "",
            "Preset bank is missing a 'Default' preset"
        });
    }

    for (const auto& [name, preset] : presets) {
        // 2. Conflicting duck rules
        if (preset.duckBGMOnSE && preset.duckAmount == 0.0f) {
            issues.push_back(AudioMixIssue{
                AudioMixIssueSeverity::Warning,
                AudioMixIssueCategory::ConflictingDuckRules,
                name,
                "Preset has duckBGMOnSE enabled but duckAmount is 0.0"
            });
        }
        if (!preset.duckBGMOnSE && preset.duckAmount > 0.0f) {
            issues.push_back(AudioMixIssue{
                AudioMixIssueSeverity::Warning,
                AudioMixIssueCategory::ConflictingDuckRules,
                name,
                "Preset has duckBGMOnSE disabled but duckAmount is greater than 0.0"
            });
        }

        // 3. Volume out of range
        for (const auto& [category, volume] : preset.categoryVolumes) {
            if (volume < 0.0f || volume > 2.0f) {
                issues.push_back(AudioMixIssue{
                    AudioMixIssueSeverity::Error,
                    AudioMixIssueCategory::VolumeOutOfRange,
                    name,
                    "Category volume " + std::to_string(volume) + " is outside valid range [0.0, 2.0]"
                });
            }
        }

        // 4. Empty category volumes
        if (preset.categoryVolumes.empty()) {
            issues.push_back(AudioMixIssue{
                AudioMixIssueSeverity::Warning,
                AudioMixIssueCategory::EmptyCategoryVolumes,
                name,
                "Preset has no category volume mappings defined"
            });
        }
    }

    return issues;
}

} // namespace urpg::audio
