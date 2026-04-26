#include "engine/core/scene/runtime_title_scene.h"

#include "engine/core/render/render_layer.h"

#include <utility>

namespace urpg::scene {

namespace {

std::string commandLabel(const RuntimeTitleCommand& command) {
    if (command.enabled || command.disabled_reason.empty()) {
        return command.label;
    }
    return command.label + " - " + command.disabled_reason;
}

} // namespace

RuntimeTitleScene::RuntimeTitleScene(Callbacks callbacks) : callbacks_(std::move(callbacks)) {
    commands_ = {
        {RuntimeTitleCommandId::NewGame, "New Game", true, ""},
        {RuntimeTitleCommandId::Continue, "Continue", false, "No save data found"},
        {RuntimeTitleCommandId::Options, "Options", false, "Options flow is not wired yet"},
        {RuntimeTitleCommandId::Exit, "Exit", true, ""},
    };
}

void RuntimeTitleScene::onUpdate(float deltaTime) {
    (void)deltaTime;

    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    urpg::RectCommand background;
    background.x = 0.0f;
    background.y = 0.0f;
    background.w = 640.0f;
    background.h = 360.0f;
    background.r = 0.04f;
    background.g = 0.05f;
    background.b = 0.07f;
    background.a = 1.0f;
    background.zOrder = 0;
    layer.submit(urpg::toFrameRenderCommand(background));

    urpg::TextCommand title;
    title.text = "URPG";
    title.x = 56.0f;
    title.y = 48.0f;
    title.fontSize = 36;
    title.zOrder = 1;
    layer.submit(urpg::toFrameRenderCommand(title));

    float y = 116.0f;
    for (const auto& command : commands_) {
        urpg::TextCommand text;
        text.text = commandLabel(command);
        text.x = 72.0f;
        text.y = y;
        text.fontSize = 22;
        text.maxWidth = 520;
        text.r = command.enabled ? 255 : 150;
        text.g = command.enabled ? 255 : 150;
        text.b = command.enabled ? 255 : 150;
        text.zOrder = 2;
        layer.submit(urpg::toFrameRenderCommand(text));
        y += 34.0f;
    }
}

const RuntimeTitleCommand* RuntimeTitleScene::findCommand(RuntimeTitleCommandId id) const {
    for (const auto& command : commands_) {
        if (command.id == id) {
            return &command;
        }
    }
    return nullptr;
}

void RuntimeTitleScene::setContinueAvailability(bool enabled, std::string disabled_reason) {
    for (auto& command : commands_) {
        if (command.id == RuntimeTitleCommandId::Continue) {
            command.enabled = enabled;
            command.disabled_reason = enabled ? std::string{} : std::move(disabled_reason);
            if (!enabled && command.disabled_reason.empty()) {
                command.disabled_reason = "No save data found";
            }
            return;
        }
    }
}

RuntimeTitleCommandResult RuntimeTitleScene::activateCommand(RuntimeTitleCommandId id) {
    const RuntimeTitleCommand* command = findCommand(id);
    if (command == nullptr) {
        return {false, false, "unknown_command", "Runtime title command is not registered."};
    }

    if (!command->enabled) {
        return {true, false, "command_disabled", command->disabled_reason};
    }

    switch (id) {
    case RuntimeTitleCommandId::NewGame:
        if (callbacks_.start_new_game) {
            callbacks_.start_new_game();
        }
        return {true, true, "new_game_started", "New Game selected."};
    case RuntimeTitleCommandId::Exit:
        if (callbacks_.request_exit) {
            callbacks_.request_exit();
        }
        return {true, true, "exit_requested", "Exit selected."};
    case RuntimeTitleCommandId::Continue:
        if (callbacks_.continue_game) {
            return callbacks_.continue_game();
        }
        return {true, false, "continue_handler_missing", "Continue is enabled but no save loader is registered."};
    case RuntimeTitleCommandId::Options:
        break;
    }

    return {true, false, "command_disabled", command->disabled_reason};
}

std::shared_ptr<RuntimeTitleScene> makeDefaultRuntimeTitleScene(RuntimeTitleScene::Callbacks callbacks) {
    return std::make_shared<RuntimeTitleScene>(std::move(callbacks));
}

} // namespace urpg::scene
