#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>
#include "audio_core.h"
#include <nlohmann/json.hpp>

namespace urpg::audio {

/**
 * @brief A named mix preset with per-category volume overrides and optional BGM ducking.
 */
struct MixPreset {
    std::string name;
    std::map<AudioCategory, float> categoryVolumes;
    bool duckBGMOnSE = false;
    float duckAmount = 0.0f;
    /** Category name strings encountered during JSON load that did not map to a known AudioCategory. */
    std::vector<std::string> unknownCategoryNames;
};

/**
 * @brief In-memory bank of mix presets with JSON serialization.
 */
class AudioMixPresetBank {
public:
    AudioMixPresetBank();

    void loadDefaults();
    std::optional<MixPreset> loadPreset(const std::string& name) const;
    void savePreset(const MixPreset& preset);
    void removePreset(const std::string& name);
    std::vector<std::string> listPresets() const;
    bool applyPreset(AudioCore& core, const std::string& name) const;

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
    void loadFromJsonString(const std::string& json_text);

    const std::map<std::string, MixPreset>& presets() const { return m_presets; }

private:
    std::map<std::string, MixPreset> m_presets;
};

} // namespace urpg::audio
