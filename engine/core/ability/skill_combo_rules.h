#pragma once

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace urpg::ability {

struct SkillComboDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct SkillComboRule {
    std::string id;
    std::set<std::string> required_tags;
    std::set<std::string> required_skills;
    std::string result_effect;
};

class SkillComboRules {
public:
    void setKnownSkills(std::set<std::string> skill_ids);
    void addRule(SkillComboRule rule);
    std::optional<SkillComboRule> match(const std::set<std::string>& tags, const std::set<std::string>& skills) const;
    std::vector<SkillComboDiagnostic> validate() const;

private:
    std::set<std::string> known_skills_;
    std::vector<SkillComboRule> rules_;
};

} // namespace urpg::ability
