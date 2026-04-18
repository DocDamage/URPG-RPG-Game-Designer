#include <catch2/catch_test_macros.hpp>
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"

using namespace urpg::ability;

class MockAbility : public GameplayAbility {
public:
    MockAbility(std::string id) : m_id(std::move(id)) {}

    const std::string& getId() const override { return m_id; }
    const ActivationInfo& getActivationInfo() const override { return m_info; }
    void activate([[maybe_unused]] AbilitySystemComponent& source) override {}

    ActivationInfo& editInfo() { return m_info; }

private:
    std::string m_id;
    ActivationInfo m_info;
};

TEST_CASE("GameplayAbility: Activation Pipeline", "[ability][activation]") {
    AbilitySystemComponent owner;
    MockAbility fireAbility("skill.fireball");

    SECTION("Ability activation fails if owner is Stunned") {
        fireAbility.editInfo().blockingTags.addTag(GameplayTag("State.Stunned"));
        
        owner.addTag(GameplayTag("State.Stunned"));
        REQUIRE_FALSE(fireAbility.canActivate(owner));

        owner.removeTag(GameplayTag("State.Stunned"));
        REQUIRE(fireAbility.canActivate(owner));
    }

    SECTION("Ability activation fails if cooldown is active") {
        owner.setCooldown("skill.fireball", 5.0f);
        REQUIRE_FALSE(fireAbility.canActivate(owner));

        owner.update(6.0f);
        REQUIRE(fireAbility.canActivate(owner));
    }

    SECTION("Ability activation requires specific tags") {
        fireAbility.editInfo().requiredTags.addTag(GameplayTag("State.Concentrated"));
        
        REQUIRE_FALSE(fireAbility.canActivate(owner));

        owner.addTag(GameplayTag("State.Concentrated"));
        REQUIRE(fireAbility.canActivate(owner));
    }
}
