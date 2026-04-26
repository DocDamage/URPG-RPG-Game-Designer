#include "engine/core/ability/ability_state_machine.h"
#include "engine/core/ability/ability_system_component.h"

namespace urpg {

AbilityStateMachine::AbilityStateMachine(const std::string& abilityName) : m_abilityName(abilityName) {}

void AbilityStateMachine::addState(const AbilityState& state) {
    m_states.push_back(state);
}

void AbilityStateMachine::start(AbilitySystemComponent& asc) {
    if (m_states.empty()) {
        m_status = AbilityStateStatus::Finished;
        asc.recordAbilityExecution(m_abilityName, "state_machine", "finished", "no_states", "", 0.0f, 0.0f, 0.0f, 0);
        return;
    }

    m_currentStateIndex = 0;
    m_status = AbilityStateStatus::Running;
    transitionTo(0, asc);
}

void AbilityStateMachine::update(AbilitySystemComponent& asc, float deltaTime) {
    if (m_status != AbilityStateStatus::Running)
        return;

    if (m_currentStateIndex >= m_states.size()) {
        m_status = AbilityStateStatus::Finished;
        asc.recordAbilityExecution(m_abilityName, "state_machine", "finished", "state_index_out_of_range", "", 0.0f,
                                   0.0f, 0.0f, 0);
        return;
    }

    auto& activeState = m_states[m_currentStateIndex];

    // Process current state tick
    bool stateFinished = true;
    if (activeState.onTick) {
        stateFinished = activeState.onTick(asc, deltaTime);
    }

    // If state reached its natural end, move to next
    if (stateFinished) {
        size_t nextIndex = m_currentStateIndex + 1;
        if (nextIndex < m_states.size()) {
            transitionTo(nextIndex, asc);
        } else {
            // Clean up last state tags
            for (const auto& tag : activeState.inherentTags.getTags()) {
                asc.removeTag(tag);
            }
            m_status = AbilityStateStatus::Finished;
            asc.recordAbilityExecution(m_abilityName, "state_machine", "finished", "", activeState.name, 0.0f, 0.0f,
                                       0.0f, 0);
        }
    }
}

void AbilityStateMachine::transitionTo(size_t index, AbilitySystemComponent& asc) {
    // 1. Cleanup old state tags if applicable
    if (m_currentStateIndex < m_states.size()) {
        for (const auto& tag : m_states[m_currentStateIndex].inherentTags.getTags()) {
            asc.removeTag(tag);
        }
    }

    m_currentStateIndex = index;
    auto& newState = m_states[m_currentStateIndex];

    // 2. Validate entry (for dynamic skipping if needed, though usually sequential)
    if (newState.canEnter && !newState.canEnter(asc)) {
        m_status = AbilityStateStatus::Failed;
        asc.recordAbilityExecution(m_abilityName, "state_machine", "failed", "state_gate_blocked", newState.name, 0.0f,
                                   0.0f, 0.0f, 0);
        return;
    }

    // 3. Apply inherent tags (e.g., "State.Ability.Windup")
    for (const auto& tag : newState.inherentTags.getTags()) {
        asc.addTag(tag);
    }

    // 4. Fire onEnter logic (e.g., spawn particles, sound)
    if (newState.onEnter) {
        newState.onEnter(asc);
    }

    asc.recordAbilityExecution(m_abilityName, "state_machine", "entered", "", newState.name, 0.0f, 0.0f, 0.0f, 0);
}

const std::string& AbilityStateMachine::getCurrentStateName() const {
    static const std::string empty = "None";
    if (m_currentStateIndex < m_states.size()) {
        return m_states[m_currentStateIndex].name;
    }
    return empty;
}

} // namespace urpg
