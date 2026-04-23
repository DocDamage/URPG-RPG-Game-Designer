#pragma once

#include "audio_mix_presets.h"
#include <string>
#include <vector>

namespace urpg::audio {

enum class AudioMixIssueSeverity {
    Warning,
    Error
};

enum class AudioMixIssueCategory {
    MissingDefaultPreset,
    ConflictingDuckRules,
    VolumeOutOfRange,
    EmptyCategoryVolumes,
    UnknownCategory,
    CrossPresetDuckConflict
};

struct AudioMixIssue {
    AudioMixIssueSeverity severity;
    AudioMixIssueCategory category;
    std::string presetName;
    std::string message;
};

/**
 * @brief Validates an AudioMixPresetBank for data-quality issues.
 */
class AudioMixValidator {
public:
    std::vector<AudioMixIssue> validate(const AudioMixPresetBank& bank) const;
};

} // namespace urpg::audio
