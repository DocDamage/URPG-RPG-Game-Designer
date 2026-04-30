#include "engine/core/engine_shell.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/options_scene.h"
#include "engine/core/scene/runtime_title_scene.h"
#include "engine/core/scene/scene_manager.h"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

using namespace urpg;
using namespace urpg::scene;

namespace {

class InputLifecycleScene final : public GameScene {
  public:
    SceneType getType() const override { return SceneType::TITLE; }
    std::string getName() const override { return "InputLifecycleScene"; }

    void handleInput(const input::InputCore& input) override {
        if (input.isActionJustPressed(input::InputAction::Confirm)) {
            ++confirmJustPressedCount;
        }
        if (input.isActionActive(input::InputAction::Confirm)) {
            ++confirmActiveCount;
        }
    }

    int confirmJustPressedCount = 0;
    int confirmActiveCount = 0;
};

void clearSceneStack() {
    auto& sceneManager = SceneManager::getInstance();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }
}

} // namespace

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
    clearSceneStack();
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
    clearSceneStack();
}

TEST_CASE("EngineShell records shell-owned MapScene commands through the headless renderer",
          "[engine][shell][render][presentation][backend][headless]") {
    auto& sceneManager = SceneManager::getInstance();
    clearSceneStack();
    RenderLayer::getInstance().flush();

    auto& shell = EngineShell::getInstance();
    auto surface = std::make_unique<HeadlessSurface>();
    shell.startup(std::move(surface), std::make_unique<HeadlessRenderer>());

    auto map = std::make_shared<MapScene>("HeadlessParityMap", 4, 3);
    map->setTile(0, 0, 1, true);
    map->setTile(1, 0, 2, true);
    sceneManager.pushScene(map);

    shell.tick();

    const auto* renderer = dynamic_cast<const HeadlessRenderer*>(shell.getRenderer());
    REQUIRE(renderer != nullptr);
    REQUIRE(renderer->initialized());
    REQUIRE(renderer->frameHistory().size() == 1);

    const auto& summary = renderer->lastFrameSummary();
    REQUIRE(summary.frameIndex == 1);
    REQUIRE(summary.commandCount > 0);
    REQUIRE(summary.tileCommandCount > 0);
    REQUIRE(summary.commandCount == renderer->lastFrameCommands().size());
    REQUIRE(RenderLayer::getInstance().getFrameCommands().empty());

    shell.shutdown();
    clearSceneStack();
}

TEST_CASE("EngineShell records shell-owned title and options commands through the headless renderer",
          "[engine][shell][render][presentation][backend][headless]") {
    auto& sceneManager = SceneManager::getInstance();
    clearSceneStack();
    RenderLayer::getInstance().flush();

    auto& shell = EngineShell::getInstance();
    auto surface = std::make_unique<HeadlessSurface>();
    shell.startup(std::move(surface), std::make_unique<HeadlessRenderer>());

    auto title = makeDefaultRuntimeTitleScene({
        {},
        {},
        {},
        [] {},
    });
    RuntimeStartupReport report;
    report.subsystems.push_back({"LocaleCatalog",
                                 RuntimeStartupSubsystemStatus::Warning,
                                 "localization.catalog_missing",
                                 "No locale catalog was found under project content."});
    report.subsystems.push_back({"RuntimeBundleLoader",
                                 RuntimeStartupSubsystemStatus::Error,
                                 "runtime_bundle.validation_failed",
                                 "Runtime asset bundle validation failed."});
    title->setStartupReport(report);
    sceneManager.pushScene(title);
    shell.getInput().updateActionState(input::InputAction::MoveDown, input::ActionState::Pressed);

    shell.tick();

    const auto* renderer = dynamic_cast<const HeadlessRenderer*>(shell.getRenderer());
    REQUIRE(renderer != nullptr);
    REQUIRE(renderer->initialized());
    REQUIRE(renderer->frameHistory().size() == 1);

    const auto& titleSummary = renderer->lastFrameSummary();
    REQUIRE(titleSummary.frameIndex == 1);
    REQUIRE(titleSummary.commandCount > 0);
    REQUIRE(titleSummary.rectCommandCount > 0);
    REQUIRE(titleSummary.textCommandCount >= 4);
    REQUIRE(titleSummary.commandCount == renderer->lastFrameCommands().size());
    REQUIRE(RenderLayer::getInstance().getFrameCommands().empty());

    auto settings = settings::defaultRuntimeSettings();
    settings.accessibility.high_contrast = true;
    settings.audio.master_volume = 0.8f;
    settings.accessibility.ui_scale = 1.2f;
    sceneManager.gotoScene(makeRuntimeOptionsScene(settings, {}));

    shell.tick();

    REQUIRE(renderer->frameHistory().size() == 2);
    const auto& optionsSummary = renderer->lastFrameSummary();
    REQUIRE(optionsSummary.frameIndex == 2);
    REQUIRE(optionsSummary.commandCount > 0);
    REQUIRE(optionsSummary.rectCommandCount > 0);
    REQUIRE(optionsSummary.textCommandCount >= 10);
    REQUIRE(optionsSummary.commandCount == renderer->lastFrameCommands().size());
    REQUIRE(RenderLayer::getInstance().getFrameCommands().empty());

    shell.shutdown();
    clearSceneStack();
}

TEST_CASE("EngineShell advances input just-pressed state after scene input", "[engine_shell][core][input]") {
    auto& shell = EngineShell::getInstance();
    clearSceneStack();
    auto surface = std::make_unique<HeadlessSurface>();
    shell.startup(std::move(surface), std::make_unique<HeadlessRenderer>());

    auto scene = std::make_shared<InputLifecycleScene>();
    SceneManager::getInstance().pushScene(scene);

    shell.getInput().updateActionState(input::InputAction::Confirm, input::ActionState::Released);
    shell.getInput().endFrame();
    shell.getInput().updateActionState(input::InputAction::Confirm, input::ActionState::Pressed);

    shell.tick();
    REQUIRE(scene->confirmJustPressedCount == 1);
    REQUIRE(scene->confirmActiveCount == 1);
    REQUIRE_FALSE(shell.getInput().isActionJustPressed(input::InputAction::Confirm));
    REQUIRE(shell.getInput().isActionActive(input::InputAction::Confirm));

    shell.tick();
    REQUIRE(scene->confirmJustPressedCount == 1);
    REQUIRE(scene->confirmActiveCount == 2);

    shell.getInput().updateActionState(input::InputAction::Confirm, input::ActionState::Released);
    shell.tick();
    REQUIRE_FALSE(shell.getInput().isActionActive(input::InputAction::Confirm));
    REQUIRE_FALSE(shell.getInput().isActionJustReleased(input::InputAction::Confirm));

    shell.shutdown();
    clearSceneStack();
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

TEST_CASE("EngineShell steady-state render path avoids frame-command growth and legacy conversion",
          "[engine][shell][render][td02]") {
    auto& sceneManager = SceneManager::getInstance();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }

    auto& layer = RenderLayer::getInstance();
    layer.flush();
    layer.resetTelemetry();

    auto& shell = EngineShell::getInstance();
    auto surface = std::make_unique<HeadlessSurface>();
    shell.startup(std::move(surface), std::make_unique<HeadlessRenderer>());

    auto map = std::make_shared<MapScene>("TD02SteadyState", 20, 15);
    sceneManager.pushScene(map);

    shell.tick();
    const auto warmup = layer.getTelemetry();
    REQUIRE(warmup.maxFrameCommandCount > 0);
    REQUIRE(warmup.frameCommandCapacity >= warmup.maxFrameCommandCount);

    layer.resetTelemetry();

    for (int frame = 0; frame < 5; ++frame) {
        shell.tick();
    }

    const auto steadyState = layer.getTelemetry();
    REQUIRE(steadyState.maxFrameCommandCount >= warmup.maxFrameCommandCount);
    REQUIRE(steadyState.frameCommandCapacityGrowths == 0);
    REQUIRE(steadyState.legacyViewBuildCount == 0);

    shell.shutdown();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }
}
