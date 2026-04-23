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

        // 5. Unknown category names (populated during JSON load)
        for (const auto& unknownName : preset.unknownCategoryNames) {
            issues.push_back(AudioMixIssue{
                AudioMixIssueSeverity::Warning,
                AudioMixIssueCategory::UnknownCategory,
                name,
                "Unknown audio category '" + unknownName + "' silently absorbed as System"
            });
        }
    }

    // 6. Cross-preset duck-rule conflict: multiple presets with duckBGMOnSE enabled
    //    but different duckAmount values → ambiguous bank-level ducking behavior.
    std::vector<std::string> duckEnabledNames;
    for (const auto& [name, preset] : presets) {
        if (preset.duckBGMOnSE) {
            duckEnabledNames.push_back(name);
        }
    }
    if (duckEnabledNames.size() >= 2) {
        float refAmount = presets.at(duckEnabledNames.front()).duckAmount;
        bool allSame = true;
        for (const auto& n : duckEnabledNames) {
            if (presets.at(n).duckAmount != refAmount) {
                allSame = false;
                break;
            }
        }
        if (!allSame) {
            std::string nameList;
            for (std::size_t i = 0; i < duckEnabledNames.size(); ++i) {
                if (i > 0) nameList += ", ";
                nameList += duckEnabledNames[i];
            }
            issues.push_back(AudioMixIssue{
                AudioMixIssueSeverity::Warning,
                AudioMixIssueCategory::CrossPresetDuckConflict,
                "",
                "Multiple presets enable BGM ducking with conflicting duck amounts: " + nameList
            });
        }
    }

    return issues;
}

} // namespace urpg::audio
