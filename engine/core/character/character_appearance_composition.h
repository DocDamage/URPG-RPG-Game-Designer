#pragma once

#include "engine/core/character/character_identity.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::character {

enum class CharacterCompositionSurface {
    Portrait,
    Field,
    Battle,
};

struct CharacterAppearanceLayer {
    std::string id;
    std::string source_asset_id;
    std::string slot;
    int32_t order = 0;
    bool required = false;
};

struct CharacterSurfaceComposition {
    CharacterCompositionSurface surface = CharacterCompositionSurface::Portrait;
    std::string base_asset_id;
    std::vector<CharacterAppearanceLayer> layers;
};

struct CharacterAppearanceComposition {
    CharacterSurfaceComposition portrait;
    CharacterSurfaceComposition field;
    CharacterSurfaceComposition battle;
    std::vector<std::string> diagnostics;
    bool complete = false;
};

CharacterAppearanceComposition composeCharacterAppearance(const CharacterIdentity& identity);

nlohmann::json characterAppearanceCompositionToJson(const CharacterAppearanceComposition& composition);

} // namespace urpg::character
