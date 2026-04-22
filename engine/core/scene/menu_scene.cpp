#include "engine/core/scene/menu_scene.h"
#include "engine/core/sprite_batcher.h"

namespace urpg::scene {

MenuScene::MenuScene(std::string scene_id) 
    : scene_id_(std::move(scene_id)) {
}

void MenuScene::onCreate() {
    // Initialization logic for the menu graph (e.g., loading schema/styles).
}

void MenuScene::onStart() {
    // Hook into global session state if needed.
}

void MenuScene::onUpdate(float deltaTime) {
    (void)deltaTime;
    if (m_isPaused) return;
    // Handle menu animations, timers, and state transitions.
}

void MenuScene::handleInput(const urpg::input::InputCore& input) {
    (void)input;
    if (m_isPaused) return;
    // Process navigation and command routing.
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
