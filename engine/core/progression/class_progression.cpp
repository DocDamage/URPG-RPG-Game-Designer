#include "engine/core/progression/class_progression.h"

#include <algorithm>
#include <utility>

namespace urpg::progression {

void ClassProgression::addClass(ClassDefinition definition) {
    classes_[definition.id] = std::move(definition);
}

std::vector<ProgressionDiagnostic> ClassProgression::validate() const {
    std::vector<ProgressionDiagnostic> diagnostics;
    for (const auto& [id, definition] : classes_) {
        std::vector<std::string> stack;
        if (hasCycleFrom(id, stack)) {
            diagnostics.push_back({"class_graph_cycle", "Class prerequisite graph contains a cycle.", id});
            break;
        }
        for (const auto& point : definition.stat_curve) {
            if (point.level <= 0 || point.hp <= 0) {
                diagnostics.push_back({"invalid_stat_curve", "Class stat curve contains non-positive values.", id});
            }
        }
    }
    return diagnostics;
}

std::vector<std::string> ClassProgression::learnedSkills(const std::string& class_id, int32_t level) const {
    std::vector<std::string> skills;
    const auto it = classes_.find(class_id);
    if (it == classes_.end()) {
        return skills;
    }
    for (const auto& skill : it->second.skills) {
        if (skill.level <= level) {
            skills.push_back(skill.skill_id);
        }
    }
    std::stable_sort(skills.begin(), skills.end());
    return skills;
}

bool ClassProgression::hasCycleFrom(const std::string& id, std::vector<std::string>& stack) const {
    if (std::find(stack.begin(), stack.end(), id) != stack.end()) {
        return true;
    }
    const auto it = classes_.find(id);
    if (it == classes_.end()) {
        return false;
    }
    stack.push_back(id);
    for (const auto& prerequisite : it->second.prerequisites) {
        if (hasCycleFrom(prerequisite, stack)) {
            return true;
        }
    }
    stack.pop_back();
    return false;
}

} // namespace urpg::progression
