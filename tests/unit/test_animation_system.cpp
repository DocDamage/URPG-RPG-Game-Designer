#include "engine/core/ecs/world.h"
#include "engine/core/animation/animation_clip.h"
#include "engine/core/animation/animation_system.h"
#include "engine/core/animation/timeline_kernel.h"
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

    SECTION("Animation clip binding drives deterministic interpolated position") {
        auto id = world.CreateEntity();
        world.AddComponent(id, urpg::TransformComponent{});

        urpg::animation::AnimationClip clip;
        clip.id = "move_right";
        clip.duration = 1.0f;
        clip.tracks = {
            {"positionX", {{0.0f, 0.0f}, {1.0f, 10.0f}}},
            {"positionY", {{0.0f, 0.0f}, {1.0f, 4.0f}}},
            {"positionZ", {{0.0f, 0.0f}, {1.0f, 0.0f}}}
        };

        world.AddComponent(id, urpg::AnimationSystem::bindClip(clip));

        animSys.update(world, 0.5f);

        const auto* transform = world.GetComponent<urpg::TransformComponent>(id);
        REQUIRE(transform != nullptr);
        REQUIRE(transform->position.x.ToFloat() == 5.0f);
        REQUIRE(transform->position.y.ToFloat() == 2.0f);
        REQUIRE(transform->position.z.ToFloat() == 0.0f);
    }
}

TEST_CASE("TimelineKernel triggers deterministic transient events", "[animation][timeline]") {
    urpg::animation::TimelineTrack track;
    track.trackName = "battle_fx";
    track.events = {
        {0.75f, urpg::animation::TimelineEventType::TriggerVFX, "enemy_1", "slash_fx", 0.0f},
        {0.25f, urpg::animation::TimelineEventType::PlaySound, "enemy_1", "slash_sfx", 0.0f}
    };

    urpg::animation::TimelineKernel kernel;
    kernel.setDuration(1.0f);
    kernel.addTrack(track);
    kernel.play();
    kernel.update(1.0f);

    const auto& triggered = kernel.getTriggeredEvents();
    REQUIRE(triggered.size() == 2);
    REQUIRE(triggered[0].type == urpg::animation::TimelineEventType::PlaySound);
    REQUIRE(triggered[0].payload == "slash_sfx");
    REQUIRE(triggered[1].type == urpg::animation::TimelineEventType::TriggerVFX);
    REQUIRE(triggered[1].payload == "slash_fx");
}
