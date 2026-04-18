#pragma once

#include "gameplay_ability.h"
#include "pattern_field.h"
#include <memory>

namespace urpg::ability {

/**
 * @brief An ability that uses a PatternField for area-of-effect calculation.
 * Demonstrates integration between 3.1 (Abilities) and 3.2 (Patterns).
 */
class PatternAbility : public GameplayAbility {
public:
    PatternAbility(const std::string& id, std::shared_ptr<PatternField> pattern) 
        : GameplayAbility(id), m_pattern(pattern) {}

    void setPattern(std::shared_ptr<PatternField> pattern) { m_pattern = pattern; }
    std::shared_ptr<PatternField> getPattern() const { return m_pattern; }

    /**
     * @brief Gets all affected coordinates relative to an origin.
     */
    std::vector<PatternPoint> getAffectedCells(int32_t originX, int32_t originY) const {
        if (!m_pattern) return {};
        
        std::vector<PatternPoint> worldPoints;
        for (const auto& point : m_pattern->points) {
            worldPoints.push_back({originX + point.x, originY + point.y});
        }
        return worldPoints;
    }

protected:
    virtual bool canActivate(const AbilitySystemComponent& owner) const override {
        return GameplayAbility::canActivate(owner) && m_pattern != nullptr;
    }

private:
    std::shared_ptr<PatternField> m_pattern;
};

} // namespace urpg::ability
