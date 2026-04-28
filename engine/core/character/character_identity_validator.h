#pragma once

#include "engine/core/character/character_identity.h"

#include <string>
#include <vector>

namespace urpg::character {

enum class CharacterIdentityIssueSeverity {
    Warning,
    Error
};

enum class CharacterIdentityIssueCategory {
    MissingName,
    MissingClass,
    UnknownClass,
    UnknownPortrait,
    UnknownBodySprite,
    UnknownAppearanceToken,
    DuplicateAppearanceToken,
    CreationRuleViolation
};

struct CharacterIdentityCatalog {
    std::vector<std::string> classIds;
    std::vector<std::string> portraitIds;
    std::vector<std::string> bodySpriteIds;
    std::vector<std::string> appearanceTokens;
};

struct CharacterIdentityIssue {
    CharacterIdentityIssueSeverity severity = CharacterIdentityIssueSeverity::Error;
    CharacterIdentityIssueCategory category = CharacterIdentityIssueCategory::MissingName;
    std::string field;
    std::string value;
    std::string message;
};

/**
 * @brief Validates character identity data against the bounded native catalog contract.
 */
class CharacterIdentityValidator {
public:
    std::vector<CharacterIdentityIssue> validate(const CharacterIdentity& identity,
                                                 const CharacterIdentityCatalog& catalog) const;
};

} // namespace urpg::character
