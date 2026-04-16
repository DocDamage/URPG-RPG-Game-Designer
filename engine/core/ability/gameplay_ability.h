#pragma once

#include "gameplay_tags.h"
#include "ability_task.h"
#include <string>
#include <vector>
#include <memory>

namespace urpg::ability {

class AbilitySystemComponent;

/**
 * @brief Base class for all executable actions in the Gameplay Ability Framework.
 * 
 * Abilities use Tags to determine if they can be activated and what they block.
 */
class GameplayAbility {
public:
    struct ActivationInfo {
        GameplayTagContainer requiredTags;    // Must have these to activate
        GameplayTagContainer blockingTags;    // Cannot have these to activate
        float cooldownSeconds = 0.0f;
        int32_t mpCost = 0;
    };

    virtual ~GameplayAbility() = default;

    virtual const std::string& getId() const = 0;
    virtual const ActivationInfo& getActivationInfo() const = 0;

    /**
     * @brief High-level check for activation feasibility.
     */
    virtual bool canActivate(const AbilitySystemComponent& source) const;

    /**
     * @brief Execution logic for the ability.
     */
    virtual void activate(AbilitySystemComponent& source) = 0;

    /**
     * @brief Update logic for any active async tasks.
     */
    virtual void update(float deltaTime) {
        for (auto it = m_activeTasks.begin(); it != m_activeTasks.end(); ) {
            (*it)->tick(deltaTime);
            if ((*it)->isFinished()) {
                it = m_activeTasks.erase(it);
            } else {
                ++it;
            }
        }
    }

    /**
     * @brief Register an async task for this ability instance.
     */
    void addTask(std::shared_ptr<AbilityTask> task, AbilitySystemComponent& asc) {
        task->activate(asc);
        m_activeTasks.push_back(task);
    }

    /**
     * @brief Tags applied to the owner while this ability is active.
     */
    virtual const GameplayTagContainer& getActiveTags() const { return m_activeTags; }

protected:
    GameplayTagContainer m_activeTags;
    std::vector<std::shared_ptr<AbilityTask>> m_activeTasks;
};

} // namespace urpg::ability
