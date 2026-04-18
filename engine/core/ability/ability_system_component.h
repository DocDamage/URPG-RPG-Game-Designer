#pragma once

#include "gameplay_tags.h"
#include "gameplay_effect.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace urpg::ability {

class GameplayAbility;

/**
 * @brief Manages Abilities, Tags, and Effects for a source actor/enemy.
 */
class AbilitySystemComponent {
public:
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
     * @brief Checks if effect has required tags and no blocking tags on the source.
     */
    bool canApplyEffect(const GameplayEffect& effect) const;

    /**
     * @brief Check if the owner can activate the given ability.
     */
    bool canActivateAbility(const GameplayAbility& ability) const;

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
        float totalAdd = 0.0f;
        float totalMult = 1.0f;
        float overrideValue = baseValue;
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
        return (baseValue + totalAdd) * totalMult;
    }

    /**
     * @brief Manages granting and revoking abilities.
     */
    void grantAbility(std::shared_ptr<GameplayAbility> ability) {
        if (ability) m_abilities.push_back(ability);
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
        // Placeholder: Needs integration with whichever component stores base stats.
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
    }

private:
    GameplayTagContainer m_tags;
    std::vector<std::shared_ptr<GameplayAbility>> m_abilities;
    std::unordered_map<std::string, float> m_cooldowns;
    std::vector<GameplayEffect> m_activeEffects;
};

} // namespace urpg::ability
