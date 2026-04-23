#pragma once

#include "engine/core/character/character_identity.h"
#include "engine/core/character/character_identity_validator.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace urpg::character {

struct CharacterClassPreset {
    std::string id;
    std::string displayName;
    std::string portraitId;
    std::string bodySpriteId;
    std::vector<std::string> defaultAppearanceTokens;
    std::unordered_map<std::string, float> baseAttributes;
    float previewR = 0.2f;
    float previewG = 0.2f;
    float previewB = 0.2f;
};

/**
 * @brief Returns the bounded in-tree character catalog shared by editor/runtime flows.
 */
const CharacterIdentityCatalog& defaultCharacterIdentityCatalog();

/**
 * @brief Returns deterministic class presets for the bounded creator workflow.
 */
const std::vector<CharacterClassPreset>& defaultCharacterClassPresets();

/**
 * @brief Returns deterministic suggested player names for the runtime creator flow.
 */
const std::vector<std::string>& defaultCharacterNameSuggestions();

/**
 * @brief Finds a bounded class preset by id.
 */
const CharacterClassPreset* findCharacterClassPreset(std::string_view classId);

/**
 * @brief Applies the bounded class preset data onto the provided identity.
 */
bool applyCharacterClassPreset(CharacterIdentity& identity,
                               std::string_view classId,
                               bool applyVisualDefaults = true,
                               bool resetAppearanceTokens = true);

} // namespace urpg::character
