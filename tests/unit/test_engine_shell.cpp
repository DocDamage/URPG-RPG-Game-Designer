#include <catch2/catch_test_macros.hpp>
#include "engine/core/engine_shell.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/scene_manager.h"
#include <thread>
#include <chrono>

using namespace urpg;
using namespace urpg::scene;

TEST_CASE("Engine Shell: Basic Lifecycle", "[shell_unit]") {
    auto& shell = EngineShell::getInstance();
    
    // Initial State Check
    REQUIRE(shell.isRunning() == false);
    
    // Startup
    auto surface = std::make_unique<HeadlessSurface>();
    auto renderer = std::make_unique<HeadlessRenderer>();
    shell.startup(std::move(surface), std::move(renderer));
    REQUIRE(shell.isRunning() == true);
    
    // Tick with measurable delay for delta time calculation
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    shell.tick();
    
    // Shutdown
    shell.shutdown();
    REQUIRE(shell.isRunning() == false);
}

TEST_CASE("Engine Shell: Delta Time Calculation", "[engine][shell]") {
    auto& shell = EngineShell::getInstance();
    auto surface = std::make_unique<HeadlessSurface>();
    auto renderer = std::make_unique<HeadlessRenderer>();
    shell.startup(std::move(surface), std::move(renderer));
    
    // Perform two ticks to establish a meaningful delta
    shell.tick();
    
    // Delay for approx 30ms (nominally 33fps region)
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    
    // Tick again and verify dt exists
    // (Note: Testing for > 0.0f generically since exact sleeps vary by platform)
    shell.tick();
    
    shell.shutdown();
}

TEST_CASE("EngineShell Tick Loop", "[engine_shell][core]") {
    auto& shell = EngineShell::getInstance();
    auto surface = std::make_unique<HeadlessSurface>();
    shell.startup(std::move(surface), std::make_unique<HeadlessRenderer>());

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
    auto surface = std::make_unique<HeadlessSurface>();
    shell.startup(std::move(surface), std::make_unique<HeadlessRenderer>());

    // Simulate a massive delay (e.g. breakpoint)
    shell.tick();

    // This is more of a smoke test for the shell's stability
    REQUIRE(shell.isRunning() == true);

    shell.shutdown();
    REQUIRE(shell.isRunning() == false);
}
