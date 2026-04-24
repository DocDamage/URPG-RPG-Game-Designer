#pragma once

#include "gameplay_ability.h"
#include "gameplay_tags.h"
#include "gameplay_effect.h"
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace urpg::ability {

/**
 * @brief Manages Abilities, Tags, and Effects for a source actor/enemy.
 */
class AbilitySystemComponent {
public:
    struct AbilityExecutionRecord {
        size_t sequence_id = 0;
        std::string ability_id;
        std::string stage;
        std::string outcome;
        std::string reason;
        std::string detail;
        std::string state_name;
        float mp_before = 0.0f;
        float mp_after = 0.0f;
        float cooldown_after = 0.0f;
        size_t active_effect_count = 0;
    };

    /**
     * @brief Adds a tag to the owner.
     */
    void addTag(const GameplayTag& tag) { m_tags.addTag(tag); }

    /**
     * @brief Removes a tag from the owner.
     */
    void removeTag(const GameplayTag& tag) { m_tags.removeTag(tag); }

    const GameplayTagContainer& getTags() const { return m_tags; }

    /**
     * @brief Checks whether an effect may be admitted to the active-effect list.
     *
     * Current TD-09 contract: this gate is intentionally always true. Effect-level tag
     * requirements are not modeled yet; modifier-level `requiredTag` checks are enforced
     * later during attribute resolution in `getAttribute()`.
     */
    bool canApplyEffect(const GameplayEffect& effect) const;

    /**
     * @brief Check if the owner can activate the given ability.
     */
    bool canActivateAbility(const GameplayAbility& ability) const;
    bool canActivateAbility(const GameplayAbility& ability,
                            const GameplayAbility::AbilityExecutionContext& context) const;
    bool tryActivateAbility(GameplayAbility& ability);
    bool tryActivateAbility(GameplayAbility& ability,
                            const GameplayAbility::AbilityExecutionContext& context);

    /**
     * @brief Validates if the target is within the ability's pattern from specified source.
     */
    bool isTargetInPattern(const GameplayAbility& ability, int32_t sourceX, int32_t sourceY, int32_t targetX, int32_t targetY) const;

    /**
     * @brief Track cooldowns (in seconds).
     */
    void setCooldown(const std::string& abilityId, float seconds) {
        m_cooldowns[abilityId] = seconds;
    }

    float getCooldownRemaining(const std::string& abilityId) const {
        auto it = m_cooldowns.find(abilityId);
        return it != m_cooldowns.end() ? it->second : 0.0f;
    }

    /**
     * @brief Apply a temporary or permanent effect. 
     * Handles stacking and refresh logic based on policy.
     */
    void applyEffect(const GameplayEffect& effect) {
        if (!canApplyEffect(effect)) return;

        // Check for existing instance with same ID
        if (!effect.id.empty()) {
            for (auto& active : m_activeEffects) {
                if (active.id == effect.id && !active.isExpired) {
                    switch (effect.stackingPolicy) {
                        case GameplayEffectStackingPolicy::Refresh:
                            active.elapsed = 0.0f;
                            return;
                        case GameplayEffectStackingPolicy::Stack:
                            if (active.stackCount < active.maxStacks) {
                                active.stackCount++;
                            }
                            active.elapsed = 0.0f;
                            return;
                        default:
                            break;
                    }
                }
            }
        }

        m_activeEffects.push_back(effect);
    }

    /**
     * @brief Get modified attribute value.
     */
    float getAttribute(const std::string& attr, float baseValue) const {
        float resolvedBase = baseValue;
        auto baseIt = m_baseAttributes.find(attr);
        if (baseIt != m_baseAttributes.end()) {
            resolvedBase = baseIt->second;
        }

        float totalAdd = 0.0f;
        float totalMult = 1.0f;
        float overrideValue = resolvedBase;
        bool hasOverride = false;

        for (const auto& effect : m_activeEffects) {
            if (effect.isExpired) continue;
            for (const auto& mod : effect.modifiers) {
                if (mod.attributeName == attr) {
                    // Check if modifier requirements are met
                    if (!mod.requiredTag.empty() && !m_tags.hasTag(GameplayTag(mod.requiredTag))) {
                        continue;
                    }

                    float stackedValue = mod.value * effect.stackCount;

                    switch (mod.operation) {
                        case ModifierOp::Add: totalAdd += stackedValue; break;
                        case ModifierOp::Multiply: 
                            // Compounding vs Additive multiplication is an architectual choice.
                            // Here we assume modifiers within a stack compound if it's a multiplier.
                            // But usually, multipliers add to a sum (1.0 + 0.1 + 0.1).
                            totalMult += (mod.value - 1.0f) * effect.stackCount; 
                            break;
                        case ModifierOp::Override: 
                            overrideValue = mod.value; // Override doesn't stack usually
                            hasOverride = true; 
                            break;
                    }
                }
            }
        }
        
        if (hasOverride) return overrideValue;
        return (resolvedBase + totalAdd) * totalMult;
    }

    void setAttribute(const std::string& attr, float value) {
        m_baseAttributes[attr] = value;
    }

    /**
     * @brief Manages granting and revoking abilities.
     */
    void grantAbility(std::shared_ptr<GameplayAbility> ability) {
        if (ability) m_abilities.push_back(ability);
    }

    bool removeAbilityById(const std::string& abilityId) {
        const auto original_size = m_abilities.size();
        m_abilities.erase(
            std::remove_if(
                m_abilities.begin(),
                m_abilities.end(),
                [&](const std::shared_ptr<GameplayAbility>& ability) {
                    return ability && ability->getId() == abilityId;
                }),
            m_abilities.end());
        return m_abilities.size() != original_size;
    }

    void grantOrReplaceAbility(std::shared_ptr<GameplayAbility> ability) {
        if (!ability) {
            return;
        }

        removeAbilityById(ability->getId());
        m_abilities.push_back(std::move(ability));
    }

    const std::vector<std::shared_ptr<GameplayAbility>>& getAbilities() const {
        return m_abilities;
    }

    /**
     * @brief Returns a reference to the cooldown map.
     */
    const std::unordered_map<std::string, float>& getActiveCooldowns() const {
        return m_cooldowns;
    }

    /**
     * @brief Manual attribute setter for cost deduction (mock implementation).
     * In a real ECS system, this would write to the Actor's Stat component.
     */
    void modifyAttribute(const std::string& attr, float delta) {
        const float current = getAttribute(attr, defaultBaseAttribute(attr));
        m_baseAttributes[attr] = current + delta;
    }

    /**
     * @brief Update per-frame logic (e.g. decrementing cooldowns and effects).
     */
    void update(float deltaTime) {
        // Update Cooldowns
        for (auto it = m_cooldowns.begin(); it != m_cooldowns.end(); ) {
            it->second -= deltaTime;
            if (it->second <= 0.0f) {
                it = m_cooldowns.erase(it);
            } else {
                ++it;
            }
        }

        // Update Effects
        for (auto it = m_activeEffects.begin(); it != m_activeEffects.end(); ) {
            if (it->duration > 0.0f) {
                it->elapsed += deltaTime;
                if (it->elapsed >= it->duration) {
                    it->isExpired = true;
                }
            }

            if (it->isExpired) {
                it = m_activeEffects.erase(it);
            } else {
                ++it;
            }
        }

        // Scene loops tick AbilitySystemComponent, so granted abilities own their
        // async task progression through the same deterministic update path.
        for (const auto& ability : m_abilities) {
            if (ability) {
                ability->update(deltaTime);
            }
        }
    }

    const std::vector<AbilityExecutionRecord>& getAbilityExecutionHistory() const {
        return m_executionHistory;
    }

    size_t getActiveEffectCount() const {
        return m_activeEffects.size();
    }

    void recordAbilityExecution(const std::string& abilityId,
                                const std::string& stage,
                                const std::string& outcome,
                                const std::string& reason,
                                const std::string& stateName,
                                float mpBefore,
                                float mpAfter,
                                float cooldownAfter,
                                size_t activeEffectCount,
                                const std::string& detail = "") {
        AbilityExecutionRecord record;
        record.sequence_id = ++m_nextExecutionSequence;
        record.ability_id = abilityId;
        record.stage = stage;
        record.outcome = outcome;
        record.reason = reason;
        record.detail = detail;
        record.state_name = stateName;
        record.mp_before = mpBefore;
        record.mp_after = mpAfter;
        record.cooldown_after = cooldownAfter;
        record.active_effect_count = activeEffectCount;
        m_executionHistory.push_back(std::move(record));
        constexpr size_t max_records = 128;
        if (m_executionHistory.size() > max_records) {
            m_executionHistory.erase(m_executionHistory.begin(), m_executionHistory.begin() + (m_executionHistory.size() - max_records));
        }
    }

private:
    static float defaultBaseAttribute(const std::string& attr) {
        return attr == "MP" ? 9999.0f : 0.0f;
    }

    GameplayTagContainer m_tags;
    std::vector<std::shared_ptr<GameplayAbility>> m_abilities;
    std::unordered_map<std::string, float> m_baseAttributes;
    std::unordered_map<std::string, float> m_cooldowns;
    std::vector<GameplayEffect> m_activeEffects;
    std::vector<AbilityExecutionRecord> m_executionHistory;
    size_t m_nextExecutionSequence = 0;
};

} // namespace urpg::ability
