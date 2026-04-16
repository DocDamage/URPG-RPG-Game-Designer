#include "engine/core/ecs/world.h"
#include "engine/core/ecs/ai_system.h"
#include "engine/core/ecs/player_control_system.h"
#include "engine/core/ecs/actor_manager.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("AISystem drives actor behavior", "[ecs][ai]") {
    urpg::World world;
    urpg::AISystem aiSys;
    urpg::ActorManager actorMgr(world);

    auto playerId = actorMgr.CreateActor("Hero");
    world.AddComponent(playerId, urpg::PlayerControlComponent{});
    actorMgr.SetActorPosition(playerId, urpg::Fixed32::FromInt(100), urpg::Fixed32::FromInt(0));

    auto npcId = actorMgr.CreateActor("Guard");
    auto& ai = world.AddComponent(npcId, urpg::AIControlComponent{});
    ai.state = urpg::AIState::Idle;
    ai.detectionRadius = urpg::Fixed32::FromInt(5);

    SECTION("Idle NPC switches to Chase when player is nearby") {
        actorMgr.SetActorPosition(playerId, urpg::Fixed32::FromInt(2), urpg::Fixed32::FromInt(0));
        
        aiSys.update(world, 0.1f);
        
        REQUIRE(ai.state == urpg::AIState::Chase);
    }

    SECTION("Patrolling NPC moves towards points") {
        ai.state = urpg::AIState::Patrol;
        ai.patrolPoints.push_back({urpg::Fixed32::FromInt(10), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)});
        ai.moveSpeed = urpg::Fixed32::FromInt(5);

        aiSys.update(world, 0.1f);

        auto* velocity = world.GetComponent<urpg::VelocityComponent>(npcId);
        REQUIRE(velocity->linear.x.ToFloat() > 0.0f);
    }
}
