#include <catch2/catch_test_macros.hpp>

#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/ability_task.h"

using namespace urpg;
using namespace urpg::ability;

class MyTestAbility : public GameplayAbility {
public:
    const std::string& getId() const override { static std::string id = "TestAbility"; return id; }
    const ActivationInfo& getActivationInfo() const override { static ActivationInfo info; return info; }

    void activate(AbilitySystemComponent& source) override {
        auto task = std::make_shared<AbilityTask_WaitTime>(1.0f);
        task->onFinished = []() {};
        addTask(task, source);
    }
};

TEST_CASE("Ability Task async execution", "[ability]") {
    AbilitySystemComponent asc;
    MyTestAbility ability;

    ability.activate(asc);

    // Initial Tick
    ability.update(0.5f);

    // Final Tick
    ability.update(0.6f);
}
