#pragma once

#include "engine/core/quest/quest_registry.h"

#include <set>
#include <string>
#include <vector>

namespace urpg::quest {

class QuestValidator {
public:
    std::vector<std::string> validate(const QuestDefinition& quest,
                                      const std::set<std::string>& known_items) const;
};

} // namespace urpg::quest
