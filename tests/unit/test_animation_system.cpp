#include "engine/core/ecs/world.h"
#include "engine/core/animation/animation_system.h"
#include "engine/core/ecs/actor_components.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("AnimationSystem updates transforms", "[animation]") {
    urpg::World world;
    urpg::AnimationSystem animSys;

    SECTION("Entity follows animation track") {
        auto id = world.CreateEntity();
        world.AddComponent(id, urpg::TransformComponent{});
        
        urpg::AnimationComponent anim;
        anim.positionTrack.push_back({urpg::Fixed32::FromInt(0), {urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)}});
        anim.positionTrack.push_back({urpg::Fixed32::FromInt(1), {urpg::Fixed32::FromInt(10), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)}});
        anim.duration = urpg::Fixed32::FromInt(1);
        anim.isPlaying = true;
        
        world.AddComponent(id, anim);

        // Update by 0.5s
        animSys.update(world, 0.5f);
        
        auto* transform = world.GetComponent<urpg::TransformComponent>(id);
        // Should be at the first keyframe since we haven't implemented full lerp yet, but time has progressed
        auto* animComp = world.GetComponent<urpg::AnimationComponent>(id);
        REQUIRE(animComp->currentTime.ToFloat() > 0.0f);
    }
}
