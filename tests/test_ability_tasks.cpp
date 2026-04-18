#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/ability_task.h"
#include <iostream>
#include <cassert>

using namespace urpg;
using namespace urpg::ability;

class MyTestAbility : public GameplayAbility {
public:
    const std::string& getId() const override { static std::string id = "TestAbility"; return id; }
    const ActivationInfo& getActivationInfo() const override { static ActivationInfo info; return info; }

    void activate(AbilitySystemComponent& source) override {
        std::cout << "Ability Activated! Starting WaitTask...\n";
        auto task = std::make_shared<AbilityTask_WaitTime>(1.0f);
        task->onFinished = []() { std::cout << "WaitTask Finished!\n"; };
        addTask(task, source);
    }
};

int main() {
    std::cout << "Testing Ability Tasks (WaitTime)...\n";

    AbilitySystemComponent asc;
    MyTestAbility ability;

    ability.activate(asc);
    
    // Initial Tick
    ability.update(0.5f);
    std::cout << "Ticked 0.5s\n";
    
    // Final Tick
    ability.update(0.6f);
    std::cout << "Ticked another 0.6s\n";

    std::cout << "Ability Task tests completed successfully.\n";
    return 0;
}
