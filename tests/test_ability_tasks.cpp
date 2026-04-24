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

class WaitInputAbility : public GameplayAbility {
public:
    WaitInputAbility() {
        task = std::make_shared<AbilityTask_WaitInput>("confirm", 0.5f);
    }

    const std::string& getId() const override { static std::string id = "WaitInputAbility"; return id; }
    const ActivationInfo& getActivationInfo() const override { static ActivationInfo info; return info; }

    void activate(AbilitySystemComponent& source) override {
        task->onFinished = [this]() { ++finished_count; };
        addTask(task, source);
    }

    std::shared_ptr<AbilityTask_WaitInput> task;
    int finished_count = 0;
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

TEST_CASE("AbilityTask_WaitInput completes on matching input and ignores mismatches", "[ability][task]") {
    AbilitySystemComponent asc;
    AbilityTask_WaitInput task("confirm", 1.0f);
    int finishedCount = 0;
    task.onFinished = [&finishedCount]() { ++finishedCount; };

    task.activate(asc);

    REQUIRE_FALSE(task.receiveInput("cancel"));
    REQUIRE_FALSE(task.isFinished());

    task.tick(0.25f);
    REQUIRE(task.receiveInput("confirm"));
    REQUIRE(task.isFinished());
    REQUIRE(task.matchedInput() == "confirm");
    REQUIRE(task.completionReason() == "input");
    REQUIRE(finishedCount == 1);

    task.tick(1.0f);
    REQUIRE(finishedCount == 1);
}

TEST_CASE("AbilityTask_WaitInput times out deterministically", "[ability][task]") {
    AbilitySystemComponent asc;
    AbilityTask_WaitInput task("confirm", 0.5f);

    task.activate(asc);

    task.tick(0.25f);
    REQUIRE_FALSE(task.isFinished());

    task.tick(0.25f);
    REQUIRE(task.isFinished());
    REQUIRE(task.matchedInput().empty());
    REQUIRE(task.completionReason() == "timeout");
    REQUIRE_FALSE(task.receiveInput("confirm"));
}

TEST_CASE("AbilityTask_WaitEvent completes with matching event payload", "[ability][task]") {
    AbilitySystemComponent asc;
    AbilityTask_WaitEvent task("enemy_defeated", 2.0f);

    task.activate(asc);

    REQUIRE_FALSE(task.dispatchEvent("turn_started", "round=1"));
    REQUIRE_FALSE(task.isFinished());

    REQUIRE(task.dispatchEvent("enemy_defeated", "enemy=slime"));
    REQUIRE(task.isFinished());
    REQUIRE(task.matchedEvent() == "enemy_defeated");
    REQUIRE(task.payload() == "enemy=slime");
    REQUIRE(task.completionReason() == "event");
}

TEST_CASE("AbilityTask_WaitEvent times out without matching event", "[ability][task]") {
    AbilitySystemComponent asc;
    AbilityTask_WaitEvent task("cast_window_open", 0.25f);

    task.activate(asc);

    task.tick(0.1f);
    REQUIRE_FALSE(task.isFinished());

    task.tick(0.2f);
    REQUIRE(task.isFinished());
    REQUIRE(task.completionReason() == "timeout");
    REQUIRE_FALSE(task.dispatchEvent("cast_window_open"));
}

TEST_CASE("AbilityTask_WaitProjectileCollision records matching collision only", "[ability][task]") {
    AbilitySystemComponent asc;
    AbilityTask_WaitProjectileCollision task("bolt_01", "slime_a", 1.0f);

    task.activate(asc);

    REQUIRE_FALSE(task.recordCollision({"bolt_02", "slime_a", 3.0f, 4.0f}));
    REQUIRE_FALSE(task.recordCollision({"bolt_01", "slime_b", 3.0f, 4.0f}));
    REQUIRE_FALSE(task.isFinished());

    REQUIRE(task.recordCollision({"bolt_01", "slime_a", 6.0f, 7.5f}));
    REQUIRE(task.isFinished());
    REQUIRE(task.hasCollision());
    REQUIRE(task.collision().projectileId == "bolt_01");
    REQUIRE(task.collision().targetId == "slime_a");
    REQUIRE(task.collision().x == 6.0f);
    REQUIRE(task.collision().y == 7.5f);
    REQUIRE(task.completionReason() == "projectile_collision");
}

TEST_CASE("AbilitySystemComponent update ticks granted ability tasks", "[ability][task][runtime]") {
    AbilitySystemComponent asc;
    auto ability = std::make_shared<WaitInputAbility>();
    asc.grantAbility(ability);

    ability->activate(asc);

    asc.update(0.25f);
    REQUIRE_FALSE(ability->task->isFinished());
    REQUIRE(ability->finished_count == 0);

    asc.update(0.3f);
    REQUIRE(ability->task->isFinished());
    REQUIRE(ability->task->completionReason() == "timeout");
    REQUIRE(ability->finished_count == 1);
}
