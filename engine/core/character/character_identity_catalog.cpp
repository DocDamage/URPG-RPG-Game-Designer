#include "engine/core/character/character_identity_catalog.h"

#include <algorithm>

namespace urpg::character {

const std::vector<CharacterClassPreset>& defaultCharacterClassPresets() {
    static const std::vector<CharacterClassPreset> kPresets = {
        {
            "class_warrior",
            "Warrior",
            "portrait_warrior_01",
            "sprite_warrior_body",
            {"armor_steel", "beard_short"},
            {{"STR", 14.0f}, {"VIT", 12.0f}, {"INT", 6.0f}, {"AGI", 8.0f}},
            0.72f,
            0.26f,
            0.18f,
        },
        {
            "class_mage",
            "Mage",
            "portrait_mage_01",
            "sprite_mage_body",
            {"hat_wizard", "cloak_travel"},
            {{"STR", 6.0f}, {"VIT", 8.0f}, {"INT", 14.0f}, {"AGI", 10.0f}},
            0.24f,
            0.38f,
            0.74f,
        },
        {
            "class_ranger",
            "Ranger",
            "portrait_ranger_01",
            "sprite_ranger_body",
            {"cloak_travel", "hair_long"},
            {{"STR", 9.0f}, {"VIT", 9.0f}, {"INT", 8.0f}, {"AGI", 14.0f}},
            0.18f,
            0.52f,
            0.28f,
        },
        {
            "class_rogue",
            "Rogue",
            "portrait_rogue_01",
            "sprite_rogue_body",
            {"hair_short", "cloak_travel"},
            {{"STR", 8.0f}, {"VIT", 8.0f}, {"INT", 9.0f}, {"AGI", 15.0f}},
            0.36f,
            0.36f,
            0.42f,
        },
    };
    return kPresets;
}

const CharacterIdentityCatalog& defaultCharacterIdentityCatalog() {
    static const CharacterIdentityCatalog kCatalog = [] {
        CharacterIdentityCatalog catalog;
        catalog.classIds.reserve(defaultCharacterClassPresets().size());
        catalog.portraitIds.reserve(defaultCharacterClassPresets().size());
        catalog.bodySpriteIds.reserve(defaultCharacterClassPresets().size());

        for (const auto& preset : defaultCharacterClassPresets()) {
            catalog.classIds.push_back(preset.id);
            catalog.portraitIds.push_back(preset.portraitId);
            catalog.bodySpriteIds.push_back(preset.bodySpriteId);
        }

        catalog.appearanceTokens = {
            "hair_short",
            "hair_long",
            "beard_short",
            "armor_steel",
            "cloak_travel",
            "hat_wizard",
        };
        return catalog;
    }();

    return kCatalog;
}

const std::vector<std::string>& defaultCharacterNameSuggestions() {
    static const std::vector<std::string> kNames = {
        "Ayla",
        "Kara",
        "Lyra",
        "Nova",
        "Orin",
        "Tarin",
    };
    return kNames;
}

const CharacterClassPreset* findCharacterClassPreset(std::string_view classId) {
    const auto& presets = defaultCharacterClassPresets();
    const auto it = std::find_if(
        presets.begin(),
        presets.end(),
        [classId](const CharacterClassPreset& preset) { return preset.id == classId; });
    return it != presets.end() ? &(*it) : nullptr;
}

bool applyCharacterClassPreset(CharacterIdentity& identity,
                               std::string_view classId,
                               bool applyVisualDefaults,
                               bool resetAppearanceTokens) {
    const auto* preset = findCharacterClassPreset(classId);
    if (!preset) {
        return false;
    }

    identity.setClassId(preset->id);
    identity.setBaseAttributes(preset->baseAttributes);
    if (identity.getSpeciesId().empty()) {
        identity.setSpeciesId("human");
    }
    if (identity.getOriginId().empty()) {
        identity.setOriginId(preset->id == "class_mage" || preset->id == "class_rogue" ? "capital" : "frontier");
    }
    if (identity.getBackgroundId().empty()) {
        if (preset->id == "class_mage") {
            identity.setBackgroundId("scholar");
        } else if (preset->id == "class_ranger" || preset->id == "class_rogue") {
            identity.setBackgroundId("wanderer");
        } else {
            identity.setBackgroundId("soldier");
        }
    }

    if (applyVisualDefaults) {
        identity.setPortraitId(preset->portraitId);
        identity.setBodySpriteId(preset->bodySpriteId);
    }

    if (resetAppearanceTokens) {
        identity.setAppearanceTokens(preset->defaultAppearanceTokens);
    }

    return true;
}

} // namespace urpg::character
