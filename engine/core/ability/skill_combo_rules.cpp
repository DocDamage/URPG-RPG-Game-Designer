#include "engine/core/ability/skill_combo_rules.h"

#include <algorithm>
#include <utility>

namespace urpg::ability {

void SkillComboRules::setKnownSkills(std::set<std::string> skill_ids) {
    known_skills_ = std::move(skill_ids);
}

void SkillComboRules::addRule(SkillComboRule rule) {
    rules_.push_back(std::move(rule));
    std::stable_sort(rules_.begin(), rules_.end(), [](const auto& lhs, const auto& rhs) { return lhs.id < rhs.id; });
}

std::optional<SkillComboRule> SkillComboRules::match(const std::set<std::string>& tags, const std::set<std::string>& skills) const {
    for (const auto& rule : rules_) {
        if (std::includes(tags.begin(), tags.end(), rule.required_tags.begin(), rule.required_tags.end()) &&
            std::includes(skills.begin(), skills.end(), rule.required_skills.begin(), rule.required_skills.end())) {
            return rule;
        }
    }
    return std::nullopt;
}

std::vector<SkillComboDiagnostic> SkillComboRules::validate() const {
    std::vector<SkillComboDiagnostic> diagnostics;
    for (const auto& rule : rules_) {
        for (const auto& skill : rule.required_skills) {
            if (!known_skills_.empty() && known_skills_.count(skill) == 0) {
                diagnostics.push_back({"missing_combo_skill", "Combo rule references an unknown skill.", rule.id + ":" + skill});
            }
        }
    }
    return diagnostics;
}

} // namespace urpg::ability
