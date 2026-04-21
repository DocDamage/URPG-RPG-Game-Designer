#include <catch2/catch_test_macros.hpp>

#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/pattern_field.h"
#include <memory>

using namespace urpg;
using namespace urpg::ability;

class PatternAbilityProxy : public GameplayAbility {
public:
    PatternAbilityProxy(const std::string& abilityId, std::shared_ptr<PatternField> pattern)
        : m_pattern(pattern) {
        id = abilityId;
    }

    const std::string& getId() const override { return id; }

    const ActivationInfo& getActivationInfo() const override {
        static ActivationInfo info;
        info.pattern = m_pattern;
        return info;
    }

    void activate(AbilitySystemComponent& source) override {
        commitAbility(source);
    }

    std::vector<PatternField::Point> getAffectedCells(int32_t originX, int32_t originY) const {
        if (!m_pattern) return {};
        std::vector<PatternField::Point> worldPoints;
        for (const auto& point : m_pattern->getPoints()) {
            worldPoints.push_back({originX + point.x, originY + point.y});
        }
        return worldPoints;
    }

protected:
    bool canActivate(const AbilitySystemComponent& owner) const override {
        return GameplayAbility::canActivate(owner) && m_pattern != nullptr;
    }

private:
    std::shared_ptr<PatternField> m_pattern;
};

TEST_CASE("PatternAbility integrates with PatternField", "[ability][pattern]") {
    auto pattern = std::make_shared<PatternField>();
    pattern->setName("Cross");
    pattern->setPoints({{0,0}, {1,0}, {-1,0}, {0,1}, {0,-1}});

    PatternAbilityProxy ability("CrossStrike", pattern);
    AbilitySystemComponent owner;

    SECTION("Can calculate global affected cells") {
        auto affected = ability.getAffectedCells(10, 10);

        REQUIRE(affected.size() == 5);

        bool foundCenter = false;
        bool foundNorth = false;
        for (const auto& p : affected) {
            if (p.x == 10 && p.y == 10) foundCenter = true;
            if (p.x == 10 && p.y == 11) foundNorth = true;
        }

        REQUIRE(foundCenter);
        REQUIRE(foundNorth);
    }

    SECTION("Fails activation without pattern") {
        PatternAbilityProxy nullAbility("Broken", nullptr);
        REQUIRE_FALSE(owner.canActivateAbility(nullAbility));
    }

    SECTION("Successfully activates with pattern") {
        REQUIRE(owner.tryActivateAbility(ability));
    }
}
