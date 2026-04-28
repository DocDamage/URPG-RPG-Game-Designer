#include "engine/core/character/character_creation_rules.h"

#include <algorithm>
#include <numeric>

namespace urpg::character {

namespace {

bool containsValue(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

const CharacterCreationClassRule* findClassRule(const CharacterCreationRules& rules, const std::string& classId) {
    const auto it = std::find_if(rules.classRules.begin(),
                                 rules.classRules.end(),
                                 [&classId](const CharacterCreationClassRule& rule) {
                                     return rule.classId == classId;
                                 });
    return it == rules.classRules.end() ? nullptr : &(*it);
}

void pushCatalogIssue(std::vector<CharacterCreationRuleIssue>& issues,
                      const std::string& field,
                      const std::string& value,
                      const std::string& label) {
    issues.push_back({
        "unknown_" + field,
        field,
        value,
        label + " is not allowed by the active character creation rules.",
    });
}

} // namespace

const CharacterCreationRules& defaultCharacterCreationRules() {
    static const CharacterCreationRules kRules = [] {
        CharacterCreationRules rules;
        rules.minNameLength = 2;
        rules.maxNameLength = 24;
        rules.allowFreeTextName = true;
        rules.allowedNameSuggestions = {"Ayla", "Kara", "Lyra", "Nova", "Orin", "Tarin"};
        rules.speciesIds = {"human", "elf", "dwarf"};
        rules.originIds = {"frontier", "capital", "wilds"};
        rules.backgroundIds = {"soldier", "scholar", "wanderer", "artisan"};
        rules.minAttributes = {{"STR", 4.0f}, {"VIT", 4.0f}, {"INT", 4.0f}, {"AGI", 4.0f}};
        rules.maxAttributes = {{"STR", 18.0f}, {"VIT", 18.0f}, {"INT", 18.0f}, {"AGI", 18.0f}};
        rules.attributePointBudget = 40.0f;
        rules.classRules = {
            {"class_warrior", {"human", "dwarf"}, {"frontier", "capital"}, {"soldier", "wanderer"}, {}, {"hat_wizard"}},
            {"class_mage", {"human", "elf"}, {"capital", "wilds"}, {"scholar", "artisan"}, {}, {"armor_steel"}},
            {"class_ranger", {"human", "elf"}, {"frontier", "wilds"}, {"wanderer", "artisan"}, {}, {}},
            {"class_rogue", {"human", "elf", "dwarf"}, {"capital", "frontier"}, {"wanderer", "artisan"}, {}, {}},
        };
        return rules;
    }();
    return kRules;
}

std::vector<CharacterCreationRuleIssue> validateCharacterCreationRules(const CharacterIdentity& identity,
                                                                       const CharacterCreationRules& rules) {
    std::vector<CharacterCreationRuleIssue> issues;

    const auto name = identity.getName();
    if (!name.empty() && name.size() < rules.minNameLength) {
        issues.push_back({"name_too_short", "name", name, "Character name is shorter than the rule minimum."});
    }
    if (name.size() > rules.maxNameLength) {
        issues.push_back({"name_too_long", "name", name, "Character name exceeds the rule maximum."});
    }
    if (!rules.allowFreeTextName && !name.empty() && !containsValue(rules.allowedNameSuggestions, name)) {
        issues.push_back({"name_not_suggested", "name", name, "Character name must come from the allowed name list."});
    }

    if (!identity.getSpeciesId().empty() && !containsValue(rules.speciesIds, identity.getSpeciesId())) {
        pushCatalogIssue(issues, "speciesId", identity.getSpeciesId(), "Species");
    }
    if (!identity.getOriginId().empty() && !containsValue(rules.originIds, identity.getOriginId())) {
        pushCatalogIssue(issues, "originId", identity.getOriginId(), "Origin");
    }
    if (!identity.getBackgroundId().empty() && !containsValue(rules.backgroundIds, identity.getBackgroundId())) {
        pushCatalogIssue(issues, "backgroundId", identity.getBackgroundId(), "Background");
    }

    if (const auto* classRule = findClassRule(rules, identity.getClassId())) {
        if (!identity.getSpeciesId().empty() && !classRule->allowedSpeciesIds.empty() &&
            !containsValue(classRule->allowedSpeciesIds, identity.getSpeciesId())) {
            issues.push_back({"class_species_restricted", "speciesId", identity.getSpeciesId(),
                              "Selected species cannot use the selected class."});
        }
        if (!identity.getOriginId().empty() && !classRule->allowedOriginIds.empty() &&
            !containsValue(classRule->allowedOriginIds, identity.getOriginId())) {
            issues.push_back({"class_origin_restricted", "originId", identity.getOriginId(),
                              "Selected origin cannot use the selected class."});
        }
        if (!identity.getBackgroundId().empty() && !classRule->allowedBackgroundIds.empty() &&
            !containsValue(classRule->allowedBackgroundIds, identity.getBackgroundId())) {
            issues.push_back({"class_background_restricted", "backgroundId", identity.getBackgroundId(),
                              "Selected background cannot use the selected class."});
        }
        for (const auto& required : classRule->requiredAppearanceTokens) {
            if (!containsValue(identity.getAppearanceTokens(), required)) {
                issues.push_back({"appearance_required", "appearanceTokens", required,
                                  "Selected class requires this appearance token."});
            }
        }
        for (const auto& forbidden : classRule->forbiddenAppearanceTokens) {
            if (containsValue(identity.getAppearanceTokens(), forbidden)) {
                issues.push_back({"appearance_forbidden", "appearanceTokens", forbidden,
                                  "Selected class forbids this appearance token."});
            }
        }
    }

    float total = 0.0f;
    for (const auto& [attribute, value] : identity.getBaseAttributes()) {
        total += value;
        if (const auto minIt = rules.minAttributes.find(attribute); minIt != rules.minAttributes.end() &&
            value < minIt->second) {
            issues.push_back({"attribute_below_min", "baseAttributes." + attribute, std::to_string(value),
                              "Attribute is below the rule minimum."});
        }
        if (const auto maxIt = rules.maxAttributes.find(attribute); maxIt != rules.maxAttributes.end() &&
            value > maxIt->second) {
            issues.push_back({"attribute_above_max", "baseAttributes." + attribute, std::to_string(value),
                              "Attribute is above the rule maximum."});
        }
    }
    if (rules.attributePointBudget.has_value() && total > *rules.attributePointBudget) {
        issues.push_back({"attribute_budget_exceeded", "baseAttributes", std::to_string(total),
                          "Allocated attributes exceed the character creation point budget."});
    }

    return issues;
}

nlohmann::json characterCreationRulesToJson(const CharacterCreationRules& rules) {
    nlohmann::json classRules = nlohmann::json::array();
    for (const auto& rule : rules.classRules) {
        classRules.push_back({
            {"classId", rule.classId},
            {"allowedSpeciesIds", rule.allowedSpeciesIds},
            {"allowedOriginIds", rule.allowedOriginIds},
            {"allowedBackgroundIds", rule.allowedBackgroundIds},
            {"requiredAppearanceTokens", rule.requiredAppearanceTokens},
            {"forbiddenAppearanceTokens", rule.forbiddenAppearanceTokens},
        });
    }

    return {
        {"minNameLength", rules.minNameLength},
        {"maxNameLength", rules.maxNameLength},
        {"allowFreeTextName", rules.allowFreeTextName},
        {"allowedNameSuggestions", rules.allowedNameSuggestions},
        {"speciesIds", rules.speciesIds},
        {"originIds", rules.originIds},
        {"backgroundIds", rules.backgroundIds},
        {"minAttributes", rules.minAttributes},
        {"maxAttributes", rules.maxAttributes},
        {"attributePointBudget", rules.attributePointBudget.has_value() ? nlohmann::json(*rules.attributePointBudget)
                                                                         : nlohmann::json(nullptr)},
        {"classRules", classRules},
    };
}

} // namespace urpg::character
