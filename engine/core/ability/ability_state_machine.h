#pragma once

#include "engine/core/ability/gameplay_tags.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace urpg {

namespace ability {
class AbilitySystemComponent;
class GameplayTagContainer;
} // namespace ability

using namespace urpg::ability;

enum class AbilityStateStatus { Ready, Running, Finished, Failed };

/**
 * Represents a single phase of a multi-wave ability (e.g., "Windup", "Impact", "Recovery").
 */
struct AbilityState {
    std::string name;

    // Condition to enter this state
    std::function<bool(const AbilitySystemComponent&)> canEnter;

    // Logic executed when entering the state
    std::function<void(AbilitySystemComponent&)> onEnter;

    // Logic executed every tick while in this state
    // Returns true if the state logic is "done" and ready to transition
    std::function<bool(AbilitySystemComponent&, float)> onTick;

    // Tags applied to the actor while in this specific state
    GameplayTagContainer inherentTags;
};

/**
 * Orchestrates transitions between multiple AbilityStates.
 */
class AbilityStateMachine {
  public:
    AbilityStateMachine(const std::string& abilityName);

    void addState(const AbilityState& state);

    void start(AbilitySystemComponent& asc);
    void update(AbilitySystemComponent& asc, float deltaTime);

    bool isRunning() const { return m_status == AbilityStateStatus::Running; }
    AbilityStateStatus getStatus() const { return m_status; }
    const std::string& getCurrentStateName() const;
    const std::string& getAbilityName() const { return m_abilityName; }

  private:
    std::string m_abilityName;
    std::vector<AbilityState> m_states;
    size_t m_currentStateIndex = 0;
    AbilityStateStatus m_status = AbilityStateStatus::Ready;

    void transitionTo(size_t index, AbilitySystemComponent& asc);
};

} // namespace urpg
