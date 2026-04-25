#include "engine/core/quest/quest_validator.h"

namespace urpg::quest {

std::vector<std::string> QuestValidator::validate(const QuestDefinition& quest,
                                                  const std::set<std::string>& known_items) const {
    std::vector<std::string> diagnostics;
    if (quest.id.empty()) {
        diagnostics.push_back("missing_quest_id");
    }
    for (const auto& objective : quest.objectives) {
        for (const auto& dependency : objective.dependencies) {
            if (dependency.type == "item" && !known_items.contains(dependency.id)) {
                diagnostics.push_back("missing_item_reference:" + dependency.id);
            }
        }
    }
    return diagnostics;
}

} // namespace urpg::quest
