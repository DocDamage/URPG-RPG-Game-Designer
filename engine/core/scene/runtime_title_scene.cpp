#include "engine/core/scene/runtime_title_scene.h"

#include "engine/core/render/render_layer.h"

#include <algorithm>
#include <utility>

namespace urpg::scene {

namespace {

std::string commandLabel(const RuntimeTitleCommand& command) {
    if (command.enabled || command.disabled_reason.empty()) {
        return command.label;
    }
    return command.label + " - " + command.disabled_reason;
}

std::string noticeLabel(const RuntimeTitleStartupNotice& notice) {
    return std::string("Startup ") + toString(notice.status) + " [" + notice.subsystem + ":" + notice.code +
           "]: " + notice.message;
}

} // namespace

RuntimeTitleScene::RuntimeTitleScene(Callbacks callbacks) : callbacks_(std::move(callbacks)) {
    commands_ = {
        {RuntimeTitleCommandId::NewGame, "New Game", true, ""},
        {RuntimeTitleCommandId::Continue, "Continue", false, "No save data found"},
        {RuntimeTitleCommandId::Options, "Options", static_cast<bool>(callbacks_.open_options),
         callbacks_.open_options ? "" : "Options flow is not wired yet"},
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
    for (size_t index = 0; index < commands_.size(); ++index) {
        const auto& command = commands_[index];
        const bool selected = index == selected_command_index_;

        if (selected) {
            urpg::RectCommand focus;
            focus.x = 56.0f;
            focus.y = y - 4.0f;
            focus.w = 528.0f;
            focus.h = 30.0f;
            focus.r = command.enabled ? 0.18f : 0.12f;
            focus.g = command.enabled ? 0.34f : 0.12f;
            focus.b = command.enabled ? 0.52f : 0.14f;
            focus.a = 1.0f;
            focus.zOrder = 1;
            layer.submit(urpg::toFrameRenderCommand(focus));
        }

        urpg::TextCommand text;
        text.text = commandLabel(command);
        text.x = 72.0f;
        text.y = y;
        text.fontSize = 22;
        text.maxWidth = 520;
        if (selected && command.enabled) {
            text.r = 255;
            text.g = 244;
            text.b = 168;
        } else if (selected) {
            text.r = 190;
            text.g = 190;
            text.b = 190;
        } else {
            text.r = command.enabled ? 255 : 150;
            text.g = command.enabled ? 255 : 150;
            text.b = command.enabled ? 255 : 150;
        }
        text.zOrder = 2;
        layer.submit(urpg::toFrameRenderCommand(text));
        y += 34.0f;
    }

    if (!startup_notices_.empty()) {
        urpg::TextCommand heading;
        heading.text = "Startup Diagnostics";
        heading.x = 56.0f;
        heading.y = 270.0f;
        heading.fontSize = 16;
        heading.r = 255;
        heading.g = 220;
        heading.b = 120;
        heading.maxWidth = 528;
        heading.zOrder = 2;
        layer.submit(urpg::toFrameRenderCommand(heading));

        float noticeY = 294.0f;
        for (const auto& notice : startup_notices_) {
            urpg::TextCommand text;
            text.text = noticeLabel(notice);
            text.x = 72.0f;
            text.y = noticeY;
            text.fontSize = 13;
            text.r = notice.status == RuntimeStartupSubsystemStatus::Error ? 255 : 230;
            text.g = notice.status == RuntimeStartupSubsystemStatus::Error ? 150 : 210;
            text.b = notice.status == RuntimeStartupSubsystemStatus::Error ? 150 : 170;
            text.maxWidth = 500;
            text.zOrder = 2;
            layer.submit(urpg::toFrameRenderCommand(text));
            noticeY += 18.0f;
            if (noticeY > 346.0f) {
                break;
            }
        }
    }
}

void RuntimeTitleScene::handleInput(const urpg::input::InputCore& input) {
    if (m_isPaused || commands_.empty()) {
        return;
    }

    if (input.isActionJustPressed(urpg::input::InputAction::MoveUp)) {
        moveSelection(-1);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveDown)) {
        moveSelection(1);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) {
        if (const auto* command = selectedCommand()) {
            last_command_result_ = activateCommand(command->id);
        }
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

const RuntimeTitleCommand* RuntimeTitleScene::selectedCommand() const {
    if (selected_command_index_ >= commands_.size()) {
        return nullptr;
    }
    return &commands_[selected_command_index_];
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

void RuntimeTitleScene::setStartupReport(const RuntimeStartupReport& report) {
    startup_notices_.clear();
    for (const auto& subsystem : report.subsystems) {
        if (subsystem.status != RuntimeStartupSubsystemStatus::Warning &&
            subsystem.status != RuntimeStartupSubsystemStatus::Error) {
            continue;
        }

        startup_notices_.push_back({
            subsystem.status,
            subsystem.subsystem,
            subsystem.code,
            subsystem.message,
        });
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
        if (callbacks_.open_options) {
            callbacks_.open_options();
        }
        return {true, true, "options_opened", "Options selected."};
    }

    return {true, false, "command_disabled", command->disabled_reason};
}

void RuntimeTitleScene::moveSelection(int direction) {
    if (commands_.empty()) {
        selected_command_index_ = 0;
        return;
    }

    const auto count = commands_.size();
    selected_command_index_ = std::min(selected_command_index_, count - 1);
    if (direction < 0) {
        selected_command_index_ = (selected_command_index_ + count - 1) % count;
    } else if (direction > 0) {
        selected_command_index_ = (selected_command_index_ + 1) % count;
    }
}

std::shared_ptr<RuntimeTitleScene> makeDefaultRuntimeTitleScene(RuntimeTitleScene::Callbacks callbacks) {
    return std::make_shared<RuntimeTitleScene>(std::move(callbacks));
}

} // namespace urpg::scene
