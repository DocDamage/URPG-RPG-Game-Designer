#include "engine/core/narrative/choice_consequence_tracker.h"

#include <utility>

namespace urpg::narrative {

void ChoiceConsequenceTracker::registerConsequence(ChoiceConsequence consequence) {
    consequences_.push_back(std::move(consequence));
}

void ChoiceConsequenceTracker::applyChoice(const std::string& choice_id, std::map<std::string, int>& state) const {
    for (const auto& consequence : consequences_) {
        if (consequence.choice_id == choice_id) {
            state[consequence.state_key] += consequence.delta;
        }
    }
}

std::vector<ChoiceConsequence> ChoiceConsequenceTracker::consequencesFor(const std::string& choice_id) const {
    std::vector<ChoiceConsequence> result;
    for (const auto& consequence : consequences_) {
        if (consequence.choice_id == choice_id) {
            result.push_back(consequence);
        }
    }
    return result;
}

} // namespace urpg::narrative
