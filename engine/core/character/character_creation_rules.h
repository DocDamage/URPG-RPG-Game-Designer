#pragma once

#include "engine/core/character/character_identity.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::character {

struct CharacterCreationRuleIssue {
    std::string code;
    std::string field;
    std::string value;
    std::string message;
};

struct CharacterCreationClassRule {
    std::string classId;
    std::vector<std::string> allowedSpeciesIds;
    std::vector<std::string> allowedOriginIds;
    std::vector<std::string> allowedBackgroundIds;
    std::vector<std::string> requiredAppearanceTokens;
    std::vector<std::string> forbiddenAppearanceTokens;
};

struct CharacterCreationRules {
    size_t minNameLength = 1;
    size_t maxNameLength = 24;
    bool allowFreeTextName = true;
    std::vector<std::string> allowedNameSuggestions;
    std::vector<std::string> speciesIds;
    std::vector<std::string> originIds;
    std::vector<std::string> backgroundIds;
    std::unordered_map<std::string, float> minAttributes;
    std::unordered_map<std::string, float> maxAttributes;
    std::optional<float> attributePointBudget;
    std::vector<CharacterCreationClassRule> classRules;
};

const CharacterCreationRules& defaultCharacterCreationRules();

std::vector<CharacterCreationRuleIssue> validateCharacterCreationRules(
    const CharacterIdentity& identity,
    const CharacterCreationRules& rules = defaultCharacterCreationRules());

nlohmann::json characterCreationRulesToJson(const CharacterCreationRules& rules);

} // namespace urpg::character
