#include "audio_mix_presets.h"
#include <stdexcept>

namespace urpg::audio {

namespace {

std::string categoryToString(AudioCategory category) {
    switch (category) {
        case AudioCategory::BGM:    return "BGM";
        case AudioCategory::BGS:    return "BGS";
        case AudioCategory::SE:     return "SE";
        case AudioCategory::ME:     return "ME";
        case AudioCategory::System: return "System";
    }
    return "System";
}

AudioCategory stringToCategory(const std::string& s) {
    if (s == "BGM")    return AudioCategory::BGM;
    if (s == "BGS")    return AudioCategory::BGS;
    if (s == "SE")     return AudioCategory::SE;
    if (s == "ME")     return AudioCategory::ME;
    if (s == "System") return AudioCategory::System;
    return AudioCategory::System;
}

nlohmann::json presetToJson(const MixPreset& preset) {
    nlohmann::json j;
    j["name"] = preset.name;
    j["duckBGMOnSE"] = preset.duckBGMOnSE;
    j["duckAmount"] = preset.duckAmount;

    nlohmann::json volumes = nlohmann::json::object();
    for (const auto& [category, volume] : preset.categoryVolumes) {
        volumes[categoryToString(category)] = volume;
    }
    j["categoryVolumes"] = volumes;

    return j;
}

MixPreset presetFromJson(const nlohmann::json& j) {
    MixPreset preset;
    preset.name = j.value("name", "");
    preset.duckBGMOnSE = j.value("duckBGMOnSE", false);
    preset.duckAmount = j.value("duckAmount", 0.0f);

    if (j.contains("categoryVolumes") && j["categoryVolumes"].is_object()) {
        for (auto it = j["categoryVolumes"].begin(); it != j["categoryVolumes"].end(); ++it) {
            preset.categoryVolumes[stringToCategory(it.key())] = it.value().get<float>();
        }
    }

    return preset;
}

} // anonymous namespace

AudioMixPresetBank::AudioMixPresetBank() {
    MixPreset defaultPreset;
    defaultPreset.name = "Default";
    defaultPreset.categoryVolumes[AudioCategory::BGM] = 1.0f;
    defaultPreset.categoryVolumes[AudioCategory::BGS] = 1.0f;
    defaultPreset.categoryVolumes[AudioCategory::SE] = 1.0f;
    defaultPreset.categoryVolumes[AudioCategory::ME] = 1.0f;
    defaultPreset.categoryVolumes[AudioCategory::System] = 1.0f;
    defaultPreset.duckBGMOnSE = false;
    defaultPreset.duckAmount = 0.0f;
    m_presets[defaultPreset.name] = defaultPreset;

    MixPreset battlePreset;
    battlePreset.name = "Battle";
    battlePreset.categoryVolumes[AudioCategory::BGM] = 0.8f;
    battlePreset.categoryVolumes[AudioCategory::SE] = 1.2f;
    battlePreset.duckBGMOnSE = true;
    battlePreset.duckAmount = 0.5f;
    m_presets[battlePreset.name] = battlePreset;

    MixPreset cinematicPreset;
    cinematicPreset.name = "Cinematic";
    cinematicPreset.categoryVolumes[AudioCategory::BGM] = 1.0f;
    cinematicPreset.categoryVolumes[AudioCategory::BGS] = 0.0f;
    cinematicPreset.categoryVolumes[AudioCategory::ME] = 1.0f;
    cinematicPreset.categoryVolumes[AudioCategory::SE] = 0.6f;
    cinematicPreset.duckBGMOnSE = false;
    cinematicPreset.duckAmount = 0.0f;
    m_presets[cinematicPreset.name] = cinematicPreset;
}

std::optional<MixPreset> AudioMixPresetBank::loadPreset(const std::string& name) const {
    auto it = m_presets.find(name);
    if (it != m_presets.end()) {
        return it->second;
    }
    return std::nullopt;
}

void AudioMixPresetBank::savePreset(const MixPreset& preset) {
    m_presets[preset.name] = preset;
}

void AudioMixPresetBank::removePreset(const std::string& name) {
    m_presets.erase(name);
}

std::vector<std::string> AudioMixPresetBank::listPresets() const {
    std::vector<std::string> names;
    names.reserve(m_presets.size());
    for (const auto& [name, preset] : m_presets) {
        names.push_back(name);
    }
    return names;
}

bool AudioMixPresetBank::applyPreset(AudioCore& core, const std::string& name) const {
    auto preset = loadPreset(name);
    if (!preset.has_value()) {
        return false;
    }

    for (const auto& [category, volume] : preset->categoryVolumes) {
        core.setCategoryVolume(category, volume);
    }

    return true;
}

nlohmann::json AudioMixPresetBank::toJson() const {
    nlohmann::json j;
    j["version"] = "1.0.0";

    nlohmann::json presetsArray = nlohmann::json::array();
    for (const auto& [name, preset] : m_presets) {
        presetsArray.push_back(presetToJson(preset));
    }
    j["presets"] = presetsArray;

    return j;
}

void AudioMixPresetBank::fromJson(const nlohmann::json& j) {
    if (!j.contains("version") || !j["version"].is_string()) {
        throw std::runtime_error("AudioMixPresetBank JSON missing 'version' field");
    }

    std::string version = j["version"].get<std::string>();
    if (version != "1.0.0") {
        throw std::runtime_error("Unsupported AudioMixPresetBank JSON version: " + version);
    }

    if (!j.contains("presets") || !j["presets"].is_array()) {
        throw std::runtime_error("AudioMixPresetBank JSON missing 'presets' array");
    }

    m_presets.clear();
    for (const auto& presetJson : j["presets"]) {
        MixPreset preset = presetFromJson(presetJson);
        if (!preset.name.empty()) {
            m_presets[preset.name] = preset;
        }
    }
}

} // namespace urpg::audio
