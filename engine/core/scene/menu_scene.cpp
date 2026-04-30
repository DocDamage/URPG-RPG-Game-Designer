#include "engine/core/scene/menu_scene.h"
#include "engine/core/sprite_batcher.h"

namespace urpg::scene {

MenuScene::MenuScene(std::string scene_id) : scene_id_(std::move(scene_id)) {}

void MenuScene::onCreate() {
    // Initialization logic for the menu graph (e.g., loading schema/styles).
}

void MenuScene::onStart() {
    // Hook into global session state if needed.
}

void MenuScene::onUpdate(float deltaTime) {
    (void)deltaTime;
    if (m_isPaused)
        return;
    // Handle menu animations, timers, and state transitions.
}

void MenuScene::handleInput(const urpg::input::InputCore& input) {
    if (m_isPaused)
        return;

    if (input.isActionJustPressed(urpg::input::InputAction::MoveUp)) {
        scene_graph_.handleInput(urpg::input::InputAction::MoveUp, urpg::input::ActionState::Pressed);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveDown)) {
        scene_graph_.handleInput(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Pressed);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveLeft)) {
        scene_graph_.handleInput(urpg::input::InputAction::MoveLeft, urpg::input::ActionState::Pressed);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveRight)) {
        scene_graph_.handleInput(urpg::input::InputAction::MoveRight, urpg::input::ActionState::Pressed);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) {
        scene_graph_.handleInput(urpg::input::InputAction::Confirm, urpg::input::ActionState::Pressed);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::Cancel)) {
        scene_graph_.handleInput(urpg::input::InputAction::Cancel, urpg::input::ActionState::Pressed);
    }
}

void MenuScene::draw(urpg::SpriteBatcher& batcher) {
    (void)batcher;
    // Render the scene graph panes and commands.
}

void MenuScene::onStop() {
    // Finalize state changes if necessary.
}

void MenuScene::onDestroy() {
    // Resource cleanup.
}

} // namespace urpg::scene
