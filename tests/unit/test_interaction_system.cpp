#include "engine/core/ecs/world.h"
#include "engine/core/ecs/interaction_system.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/player_control_system.h"
#include "engine/core/input/input_core.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("InteractionSystem detects nearby interactables", "[ecs][interaction]") {
    urpg::World world;
    urpg::input::InputCore input;
    urpg::InteractionSystem interactSys;

    // Setup Player
    auto playerId = world.CreateEntity();
    world.AddComponent(playerId, urpg::TransformComponent{});
    world.AddComponent(playerId, urpg::PlayerControlComponent{});

    // Setup NPC
    auto npcId = world.CreateEntity();
    world.AddComponent(npcId, urpg::TransformComponent{{urpg::Fixed32::FromInt(1), urpg::Fixed32::FromInt(0), urpg::Fixed32::FromInt(0)}});
    world.AddComponent(npcId, urpg::InteractionComponent{"TalkToNPC", urpg::Fixed32::FromInt(2)});

    SECTION("Interaction triggers on Confirm press") {
        input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
        
        interactSys.update(world, input);

        const auto& results = interactSys.getPendingInteractions();
        REQUIRE(results.size() == 1);
        REQUIRE(results[0].target == npcId);
        REQUIRE(results[0].eventId == "TalkToNPC");
    }

    SECTION("Interaction ignores targets out of range") {
        auto* trans = world.GetComponent<urpg::TransformComponent>(npcId);
        trans->position.x = urpg::Fixed32::FromInt(10); // Move far away

        input.updateActionState(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
        interactSys.update(world, input);

        REQUIRE(interactSys.getPendingInteractions().empty());
    }
}
