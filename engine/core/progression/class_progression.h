#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace urpg::progression {

struct ProgressionDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct SkillUnlock {
    std::string skill_id;
    int32_t level = 1;
};

struct StatPoint {
    int32_t level = 1;
    int32_t hp = 1;
};

struct ClassDefinition {
    std::string id;
    std::vector<std::string> prerequisites;
    std::vector<SkillUnlock> skills;
    std::vector<StatPoint> stat_curve;
};

class ClassProgression {
public:
    void addClass(ClassDefinition definition);
    std::vector<ProgressionDiagnostic> validate() const;
    std::vector<std::string> learnedSkills(const std::string& class_id, int32_t level) const;

private:
    bool hasCycleFrom(const std::string& id, std::vector<std::string>& stack) const;

    std::map<std::string, ClassDefinition> classes_;
};

} // namespace urpg::progression
