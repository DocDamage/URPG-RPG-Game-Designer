#pragma once

#include "engine/core/scene/scene_manager.h"

#include <functional>
#include <string>
#include <vector>

namespace urpg::scene {

enum class RuntimeTitleCommandId {
    NewGame,
    Continue,
    Options,
    Exit,
};

struct RuntimeTitleCommand {
    RuntimeTitleCommandId id = RuntimeTitleCommandId::NewGame;
    std::string label;
    bool enabled = false;
    std::string disabled_reason;
};

struct RuntimeTitleCommandResult {
    bool handled = false;
    bool success = false;
    std::string code;
    std::string message;
};

class RuntimeTitleScene final : public GameScene {
  public:
    struct Callbacks {
        std::function<void()> start_new_game;
        std::function<void()> request_exit;
    };

    explicit RuntimeTitleScene(Callbacks callbacks = {});

    SceneType getType() const override { return SceneType::TITLE; }
    std::string getName() const override { return "RuntimeTitle"; }

    void onUpdate(float deltaTime) override;

    const std::vector<RuntimeTitleCommand>& commands() const { return commands_; }
    const RuntimeTitleCommand* findCommand(RuntimeTitleCommandId id) const;
    RuntimeTitleCommandResult activateCommand(RuntimeTitleCommandId id);

  private:
    Callbacks callbacks_;
    std::vector<RuntimeTitleCommand> commands_;
};

std::shared_ptr<RuntimeTitleScene> makeDefaultRuntimeTitleScene(RuntimeTitleScene::Callbacks callbacks = {});

} // namespace urpg::scene
