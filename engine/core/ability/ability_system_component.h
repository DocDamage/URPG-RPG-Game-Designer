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
     * @brief Check if the owner can activate the given ability.
     */
    bool canActivateAbility(const GameplayAbility& ability) const;

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
     */
    void applyEffect(const GameplayEffect& effect) {
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
                    if (!mod.requiredTag.empty() && !m_tags.hasTag(mod.requiredTag)) {
                        continue;
                    }

                    switch (mod.operation) {
                        case ModifierOp::Add: totalAdd += mod.value; break;
                        case ModifierOp::Multiply: totalMult *= mod.value; break;
                        case ModifierOp::Override: 
                            overrideValue = mod.value; 
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
    std::unordered_map<std::string, float> m_cooldowns;
    std::vector<GameplayEffect> m_activeEffects;
};

} // namespace urpg::ability
