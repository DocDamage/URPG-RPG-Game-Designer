#include <catch2/catch_test_macros.hpp>
#include "engine/core/engine_shell.h"
#include "engine/core/scene/map_scene.h"

using namespace urpg;
using namespace urpg::scene;

TEST_CASE("EngineShell Tick Loop", "[engine_shell][core]") {
    auto& shell = EngineShell::getInstance();
    shell.startup();
    
    // Create a mock scene
    auto map = std::make_shared<MapScene>("ShellTest", 10, 10);
    SceneManager::getInstance().pushScene(map);
    
    // Simulate a tick with input
    shell.getInput().mapKey(101, input::InputAction::MoveRight);
    shell.getInput().processKeyEvent(101, input::ActionState::Pressed);
    
    shell.tick();
    
    // Verify that the scene processed the input and began moving
    auto& movement = map->getPlayerMovement();
    REQUIRE(movement.isMoving == true);
    
    shell.shutdown();
}

TEST_CASE("EngineShell Delta Time Clamp", "[engine][core]") {
    auto& shell = EngineShell::getInstance();
    shell.startup();
    
    // Simulate a massive delay (e.g. breakpoint)
    // We can't easily fake the internal clock, but we can verify the tick logic 
    // by calling tick twice and checking if it handles state.
    shell.tick();
    
    // This is more of a smoke test for the shell's stability
    REQUIRE(shell.isRunning() == true);
    
    shell.shutdown();
    REQUIRE(shell.isRunning() == false);
}
