#pragma once

#include "ability_task.h"
#include "gameplay_tags.h"
#include "pattern_field.h"
#include <memory>
#include <string>
#include <vector>

namespace urpg::ability {

class AbilitySystemComponent;

/**
 * @brief Base class for all executable actions in the Gameplay Ability Framework.
 *
 * Abilities use Tags to determine if they can be activated and what they block.
 */
class GameplayAbility {
  public:
    struct AbilityExecutionTarget {
        AbilitySystemComponent* abilitySystem = nullptr;
        const void* runtimeHandle = nullptr;
        std::string runtimeId;
    };

    struct AbilityExecutionContext {
        const void* sourceRuntimeHandle = nullptr;
        std::string sourceRuntimeId;
        std::vector<AbilityExecutionTarget> targets;
    };

    struct ActivationCheckResult {
        bool allowed = true;
        std::string reason;
        std::string detail;
        float cooldown_remaining = 0.0f;
        float current_mp = 0.0f;
    };

    struct ActivationInfo {
        GameplayTagContainer requiredTags; // Must have these to activate
        GameplayTagContainer blockingTags; // Cannot have these to activate

        // Active conditions use the bounded in-tree evaluator documented by
        // gameplay_ability.schema.json. Unsupported grammar fails closed with
        // "condition_parse_error". passiveCondition remains out of scope for
        // runtime cancellation unless a future evaluator is added.
        std::string activeCondition;
        std::string passiveCondition;

        float cooldownSeconds = 0.0f;
        int32_t mpCost = 0;
        std::shared_ptr<PatternField> pattern; // Optional pattern for AoE/Range
    };

    virtual ~GameplayAbility() = default;

    virtual const std::string& getId() const = 0;
    virtual const ActivationInfo& getActivationInfo() const = 0;

    /**
     * @brief High-level check for activation feasibility.
     */
    virtual bool canActivate(const AbilitySystemComponent& source) const;
    virtual bool canActivate(const AbilitySystemComponent& source, const AbilityExecutionContext& context) const;
    virtual ActivationCheckResult evaluateActivation(const AbilitySystemComponent& source) const;
    virtual ActivationCheckResult evaluateActivation(const AbilitySystemComponent& source,
                                                     const AbilityExecutionContext& context) const;

    /**
     * @brief Execution logic for the ability.
     */
    virtual void activate(AbilitySystemComponent& source) = 0;
    virtual void activate(AbilitySystemComponent& source, const AbilityExecutionContext& context);

    /**
     * @brief Commit the ability (consume costs, start cooldowns).
     * Usually called at the start of activate().
     */
    virtual void commitAbility(AbilitySystemComponent& source);

    /**
     * @brief Update logic for any active async tasks.
     */
    virtual void update(float deltaTime);

    /**
     * @brief Unique identifier for the ability (for cooldowns/lookup).
     */
    std::string id;

    /**
     * @brief Base cooldown in seconds.
     */
    float cooldownTime = 0.0f;

    /**
     * @brief Legacy/fallback authored activation condition string.
     * Evaluated by the bounded in-tree condition evaluator.
     */
    std::string activeCondition;

    /**
     * @brief Resource cost (simple float for now, could be map of attr -> value).
     */
    float mpCost = 0.0f;

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
    float resolveCooldownSeconds() const;
    float resolveMpCost() const;
    const std::string& resolveActiveCondition() const;

    GameplayTagContainer m_activeTags;
    std::vector<std::shared_ptr<AbilityTask>> m_activeTasks;
};

} // namespace urpg::ability
