#pragma once

#include "engine/core/scene/scene_manager.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"

#include <memory>
#include <string>

namespace urpg::scene {

/**
 * @brief Native scene for rendering and coordinating UI/Menu interactions.
 *
 * This fulfills the Wave 1 requirement for "UI/Menu" native runtime ownership.
 * It encapsulates the MenuSceneGraph and MenuCommandRegistry, bridging them
 * to the engine's SceneManager and SpriteBatcher.
 */
class MenuScene : public GameScene {
  public:
    explicit MenuScene(std::string scene_id);

    SceneType getType() const override { return SceneType::MENU; }
    std::string getName() const override { return "MenuScene: " + scene_id_; }

    void onCreate() override;
    void onStart() override;
    void onUpdate(float deltaTime) override;
    void handleInput(const urpg::input::InputCore& input) override;
    void draw(urpg::SpriteBatcher& batcher) override;
    void onStop() override;
    void onDestroy() override;

    // Component accessors for the inspector
    const ui::MenuSceneGraph& getSceneGraph() const { return scene_graph_; }
    ui::MenuSceneGraph& getSceneGraphMutable() { return scene_graph_; }
    const ui::MenuCommandRegistry& getRegistry() const { return registry_; }

  protected:
    std::string scene_id_;
    ui::MenuSceneGraph scene_graph_;
    ui::MenuCommandRegistry registry_;
};

} // namespace urpg::scene
