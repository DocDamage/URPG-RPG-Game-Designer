#pragma once

#include "engine/core/dialogue/dialogue_graph.h"

#include <set>
#include <string>
#include <vector>

namespace urpg::narrative {

class NarrativeContinuityChecker {
public:
    std::vector<std::string> check(const urpg::dialogue::DialogueGraph& graph,
                                   const std::set<std::string>& known_condition_keys) const;
};

} // namespace urpg::narrative
