#pragma once

#include "gameplay_ability.h"
#include <string>

namespace urpg::ability {

struct AbilityConditionEvaluation {
    bool parsed = true;
    bool value = true;
    std::string reason;
    std::string detail;
};

class AbilityConditionEvaluator {
  public:
    AbilityConditionEvaluation evaluate(const std::string& expression, const AbilitySystemComponent& source,
                                        const GameplayAbility::AbilityExecutionContext* context = nullptr) const;
};

} // namespace urpg::ability
