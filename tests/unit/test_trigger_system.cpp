#include "engine/core/ecs/world.h"
#include "engine/core/events/trigger_system.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/collision_components.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("TriggerSystem detects entry and exit", "[events][trigger]") {
    urpg::World world;
    urpg::TriggerSystem triggerSys;

    auto triggerId = world.CreateEntity();
    world.AddComponent(triggerId, urpg::TransformComponent{});
    world.AddComponent(triggerId, urpg::CollisionBoxComponent{urpg::Vector3::Zero(), urpg::Fixed32::FromInt(2)});
    world.AddComponent(triggerId, urpg::TriggerVolumeComponent{"OnEnter", "OnExit"});

    auto actorId = world.CreateEntity();
    world.AddComponent(actorId, urpg::TransformComponent{{urpg::Fixed32::FromInt(10), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)}});
    world.AddComponent(actorId, urpg::CollisionBoxComponent{urpg::Vector3::Zero(), urpg::Fixed32::FromInt(1)});

    SECTION("Entry event fires when entity overlaps") {
        // Move entity into trigger
        auto* transform = world.GetComponent<urpg::TransformComponent>(actorId);
        transform->position = {urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)};

        triggerSys.update(world);

        const auto& events = triggerSys.getPendingEvents();
        REQUIRE(events.size() == 1);
        REQUIRE(events[0].eventId == "OnEnter");
    }

    SECTION("Exit event fires when entity leaves") {
        // First frame: Occupied
        auto* transform = world.GetComponent<urpg::TransformComponent>(actorId);
        transform->position = {urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)};
        triggerSys.update(world);

        // Second frame: Move away
        transform->position = {urpg::Fixed32::FromInt(10), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)};
        triggerSys.update(world);

        const auto& events = triggerSys.getPendingEvents();
        REQUIRE(events.size() == 1);
        REQUIRE(events[0].eventId == "OnExit");
    }
}
