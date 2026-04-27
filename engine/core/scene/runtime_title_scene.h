#pragma once

#include "engine/core/input/input_core.h"
#include "engine/core/scene/scene_manager.h"

#include <functional>
#include <cstddef>
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
        std::function<RuntimeTitleCommandResult()> continue_game;
        std::function<void()> open_options;
    };

    explicit RuntimeTitleScene(Callbacks callbacks = {});

    SceneType getType() const override { return SceneType::TITLE; }
    std::string getName() const override { return "RuntimeTitle"; }

    void onUpdate(float deltaTime) override;
    void handleInput(const urpg::input::InputCore& input) override;

    const std::vector<RuntimeTitleCommand>& commands() const { return commands_; }
    const RuntimeTitleCommand* findCommand(RuntimeTitleCommandId id) const;
    size_t selectedCommandIndex() const { return selected_command_index_; }
    const RuntimeTitleCommand* selectedCommand() const;
    void setContinueAvailability(bool enabled, std::string disabled_reason = {});
    RuntimeTitleCommandResult activateCommand(RuntimeTitleCommandId id);
    const RuntimeTitleCommandResult& lastCommandResult() const { return last_command_result_; }

  private:
    void moveSelection(int direction);

    Callbacks callbacks_;
    std::vector<RuntimeTitleCommand> commands_;
    size_t selected_command_index_ = 0;
    RuntimeTitleCommandResult last_command_result_;
};

std::shared_ptr<RuntimeTitleScene> makeDefaultRuntimeTitleScene(RuntimeTitleScene::Callbacks callbacks = {});

} // namespace urpg::scene
