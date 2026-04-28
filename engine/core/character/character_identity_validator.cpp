#include "engine/core/character/character_identity_validator.h"

#include "engine/core/character/character_creation_rules.h"

#include <algorithm>

namespace urpg::character {

namespace {

bool containsValue(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

} // namespace

std::vector<CharacterIdentityIssue> CharacterIdentityValidator::validate(
    const CharacterIdentity& identity,
    const CharacterIdentityCatalog& catalog) const {
    std::vector<CharacterIdentityIssue> issues;

    if (identity.getName().empty()) {
        issues.push_back({
            CharacterIdentityIssueSeverity::Error,
            CharacterIdentityIssueCategory::MissingName,
            "name",
            "",
            "Character name is required.",
        });
    }

    if (identity.getClassId().empty()) {
        issues.push_back({
            CharacterIdentityIssueSeverity::Error,
            CharacterIdentityIssueCategory::MissingClass,
            "classId",
            "",
            "Character class is required.",
        });
    } else if (!containsValue(catalog.classIds, identity.getClassId())) {
        issues.push_back({
            CharacterIdentityIssueSeverity::Error,
            CharacterIdentityIssueCategory::UnknownClass,
            "classId",
            identity.getClassId(),
            "Character class is not in the current class catalog.",
        });
    }

    if (!identity.getPortraitId().empty() && !containsValue(catalog.portraitIds, identity.getPortraitId())) {
        issues.push_back({
            CharacterIdentityIssueSeverity::Error,
            CharacterIdentityIssueCategory::UnknownPortrait,
            "portraitId",
            identity.getPortraitId(),
            "Portrait id is not in the current portrait catalog.",
        });
    }

    if (!identity.getBodySpriteId().empty() &&
        !containsValue(catalog.bodySpriteIds, identity.getBodySpriteId())) {
        issues.push_back({
            CharacterIdentityIssueSeverity::Error,
            CharacterIdentityIssueCategory::UnknownBodySprite,
            "bodySpriteId",
            identity.getBodySpriteId(),
            "Body sprite id is not in the current body-sprite catalog.",
        });
    }

    std::vector<std::string> seenTokens;
    for (const auto& token : identity.getAppearanceTokens()) {
        if (!containsValue(catalog.appearanceTokens, token)) {
            issues.push_back({
                CharacterIdentityIssueSeverity::Error,
                CharacterIdentityIssueCategory::UnknownAppearanceToken,
                "appearanceTokens",
                token,
                "Appearance token '" + token + "' is not in the current appearance catalog.",
            });
        }

        if (containsValue(seenTokens, token)) {
            issues.push_back({
                CharacterIdentityIssueSeverity::Error,
                CharacterIdentityIssueCategory::DuplicateAppearanceToken,
                "appearanceTokens",
                token,
                "Appearance token '" + token + "' is duplicated.",
            });
        } else {
            seenTokens.push_back(token);
        }
    }

    for (const auto& ruleIssue : validateCharacterCreationRules(identity)) {
        issues.push_back({
            CharacterIdentityIssueSeverity::Error,
            CharacterIdentityIssueCategory::CreationRuleViolation,
            ruleIssue.field,
            ruleIssue.value,
            ruleIssue.message,
        });
    }

    return issues;
}

} // namespace urpg::character
