#include "engine/core/scene/options_scene.h"

#include "engine/core/render/render_layer.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace urpg::scene {

namespace {

template <typename T>
T clampValue(T value, T minimum, T maximum) {
    return std::clamp(value, minimum, maximum);
}

std::string percentText(float value) {
    std::ostringstream out;
    out << static_cast<int>(std::round(clampValue(value, 0.0f, 1.0f) * 100.0f)) << "%";
    return out.str();
}

std::string scaleText(float value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << value << "x";
    return out.str();
}

std::string boolText(bool value) {
    return value ? "On" : "Off";
}

std::string pathText(const std::filesystem::path& path) {
    return path.empty() ? "(not configured)" : path.generic_string();
}

} // namespace

RuntimeOptionsScene::RuntimeOptionsScene(urpg::settings::RuntimeSettings settings, std::filesystem::path settings_path,
                                         Callbacks callbacks)
    : settings_(std::move(settings)), settings_path_(std::move(settings_path)), callbacks_(std::move(callbacks)) {
    rebuildRows();
}

void RuntimeOptionsScene::onUpdate(float deltaTime) {
    (void)deltaTime;

    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    urpg::RectCommand background;
    background.x = 0.0f;
    background.y = 0.0f;
    background.w = 640.0f;
    background.h = 360.0f;
    background.r = settings_.accessibility.high_contrast ? 0.0f : 0.04f;
    background.g = settings_.accessibility.high_contrast ? 0.0f : 0.05f;
    background.b = settings_.accessibility.high_contrast ? 0.0f : 0.07f;
    background.a = 1.0f;
    background.zOrder = 0;
    layer.submit(urpg::toFrameRenderCommand(background));

    urpg::TextCommand title;
    title.text = "Options";
    title.x = 56.0f;
    title.y = 34.0f;
    title.fontSize = 30;
    title.zOrder = 1;
    layer.submit(urpg::toFrameRenderCommand(title));

    float y = 82.0f;
    for (size_t index = 0; index < rows_.size(); ++index) {
        const auto& row = rows_[index];
        const bool selected = index == selected_row_index_;
        if (selected) {
            urpg::RectCommand focus;
            focus.x = 48.0f;
            focus.y = y - 4.0f;
            focus.w = 544.0f;
            focus.h = 25.0f;
            focus.r = settings_.accessibility.high_contrast ? 0.9f : 0.18f;
            focus.g = settings_.accessibility.high_contrast ? 0.9f : 0.34f;
            focus.b = settings_.accessibility.high_contrast ? 0.1f : 0.52f;
            focus.a = 1.0f;
            focus.zOrder = 1;
            layer.submit(urpg::toFrameRenderCommand(focus));
        }

        urpg::TextCommand text;
        text.text = row.value.empty() ? row.label : row.label + ": " + row.value;
        text.x = 64.0f;
        text.y = y;
        text.fontSize = 18;
        text.maxWidth = 520;
        if (selected) {
            text.r = settings_.accessibility.high_contrast ? 0 : 255;
            text.g = settings_.accessibility.high_contrast ? 0 : 244;
            text.b = settings_.accessibility.high_contrast ? 0 : 168;
        }
        text.zOrder = 2;
        layer.submit(urpg::toFrameRenderCommand(text));
        y += 26.0f;
    }

    if (!last_command_result_.message.empty()) {
        urpg::TextCommand status;
        status.text = last_command_result_.message;
        status.x = 56.0f;
        status.y = 326.0f;
        status.fontSize = 16;
        status.maxWidth = 520;
        status.r = last_command_result_.success ? 180 : 255;
        status.g = last_command_result_.success ? 255 : 160;
        status.b = last_command_result_.success ? 180 : 160;
        status.zOrder = 2;
        layer.submit(urpg::toFrameRenderCommand(status));
    }
}

void RuntimeOptionsScene::handleInput(const urpg::input::InputCore& input) {
    if (m_isPaused || rows_.empty()) {
        return;
    }

    if (input.isActionJustPressed(urpg::input::InputAction::MoveUp)) {
        moveSelection(-1);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveDown)) {
        moveSelection(1);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveLeft)) {
        adjustSelected(-1);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveRight)) {
        adjustSelected(1);
    }
    if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) {
        last_command_result_ = activateSelected();
    }
    if (input.isActionJustPressed(urpg::input::InputAction::Cancel)) {
        last_command_result_ = back();
    }
}

const RuntimeOptionsRow* RuntimeOptionsScene::selectedRow() const {
    if (selected_row_index_ >= rows_.size()) {
        return nullptr;
    }
    return &rows_[selected_row_index_];
}

RuntimeOptionsCommandResult RuntimeOptionsScene::activateSelected() {
    const auto* row = selectedRow();
    if (row == nullptr) {
        return {false, false, "options_row_missing", "No options row is selected."};
    }

    switch (row->id) {
    case RuntimeOptionsRowId::HighContrast:
        settings_.accessibility.high_contrast = !settings_.accessibility.high_contrast;
        rebuildRows();
        return {true, true, "high_contrast_toggled", "High contrast updated."};
    case RuntimeOptionsRowId::ReduceMotion:
        settings_.accessibility.reduce_motion = !settings_.accessibility.reduce_motion;
        rebuildRows();
        return {true, true, "reduce_motion_toggled", "Reduce motion updated."};
    case RuntimeOptionsRowId::InputMapping:
        settings_.input_mapping_path = urpg::settings::defaultRuntimeSettings().input_mapping_path;
        rebuildRows();
        return {true, true, "input_mapping_reset", "Input mapping path reset to the runtime default."};
    case RuntimeOptionsRowId::Save:
        return save();
    case RuntimeOptionsRowId::Back:
        return back();
    case RuntimeOptionsRowId::WindowWidth:
    case RuntimeOptionsRowId::WindowHeight:
    case RuntimeOptionsRowId::MasterVolume:
    case RuntimeOptionsRowId::BgmVolume:
    case RuntimeOptionsRowId::UiScale:
        adjustSelected(1);
        return {true, true, "option_adjusted", "Option value updated."};
    }

    return {false, false, "options_row_unhandled", "Selected options row is not handled."};
}

RuntimeOptionsCommandResult RuntimeOptionsScene::save() {
    if (settings_path_.empty()) {
        return {true, false, "settings_path_missing", "Runtime settings path is not configured."};
    }

    std::string error;
    if (!urpg::settings::saveRuntimeSettings(settings_path_, settings_, &error)) {
        return {true, false, "settings_save_failed", error.empty() ? "Runtime settings save failed." : error};
    }

    if (callbacks_.settings_saved) {
        callbacks_.settings_saved(settings_);
    }
    return {true, true, "settings_saved", "Runtime options saved."};
}

RuntimeOptionsCommandResult RuntimeOptionsScene::back() {
    if (callbacks_.request_back) {
        callbacks_.request_back();
    }
    return {true, true, "options_back", "Returning to title."};
}

void RuntimeOptionsScene::adjustSelected(int direction) {
    if (direction == 0) {
        return;
    }

    const auto* row = selectedRow();
    if (row == nullptr) {
        return;
    }

    switch (row->id) {
    case RuntimeOptionsRowId::WindowWidth:
        settings_.window.width = clampValue<std::uint32_t>(
            static_cast<std::uint32_t>(static_cast<int>(settings_.window.width) + direction * 80), 320, 16384);
        break;
    case RuntimeOptionsRowId::WindowHeight:
        settings_.window.height = clampValue<std::uint32_t>(
            static_cast<std::uint32_t>(static_cast<int>(settings_.window.height) + direction * 45), 320, 16384);
        break;
    case RuntimeOptionsRowId::MasterVolume:
        settings_.audio.master_volume = clampValue(settings_.audio.master_volume + direction * 0.1f, 0.0f, 1.0f);
        break;
    case RuntimeOptionsRowId::BgmVolume:
        settings_.audio.bgm_volume = clampValue(settings_.audio.bgm_volume + direction * 0.1f, 0.0f, 1.0f);
        break;
    case RuntimeOptionsRowId::UiScale:
        settings_.accessibility.ui_scale =
            clampValue(settings_.accessibility.ui_scale + direction * 0.1f, 0.5f, 3.0f);
        break;
    case RuntimeOptionsRowId::InputMapping:
    case RuntimeOptionsRowId::HighContrast:
    case RuntimeOptionsRowId::ReduceMotion:
    case RuntimeOptionsRowId::Save:
    case RuntimeOptionsRowId::Back:
        break;
    }

    rebuildRows();
}

void RuntimeOptionsScene::moveSelection(int direction) {
    if (rows_.empty()) {
        selected_row_index_ = 0;
        return;
    }

    const auto count = rows_.size();
    selected_row_index_ = std::min(selected_row_index_, count - 1);
    if (direction < 0) {
        selected_row_index_ = (selected_row_index_ + count - 1) % count;
    } else if (direction > 0) {
        selected_row_index_ = (selected_row_index_ + 1) % count;
    }
}

void RuntimeOptionsScene::rebuildRows() {
    rows_ = {
        {RuntimeOptionsRowId::WindowWidth, "Display Width", rowValue(RuntimeOptionsRowId::WindowWidth)},
        {RuntimeOptionsRowId::WindowHeight, "Display Height", rowValue(RuntimeOptionsRowId::WindowHeight)},
        {RuntimeOptionsRowId::MasterVolume, "Master Volume", rowValue(RuntimeOptionsRowId::MasterVolume)},
        {RuntimeOptionsRowId::BgmVolume, "BGM Volume", rowValue(RuntimeOptionsRowId::BgmVolume)},
        {RuntimeOptionsRowId::InputMapping, "Input Mapping", rowValue(RuntimeOptionsRowId::InputMapping)},
        {RuntimeOptionsRowId::HighContrast, "High Contrast", rowValue(RuntimeOptionsRowId::HighContrast)},
        {RuntimeOptionsRowId::ReduceMotion, "Reduce Motion", rowValue(RuntimeOptionsRowId::ReduceMotion)},
        {RuntimeOptionsRowId::UiScale, "UI Scale", rowValue(RuntimeOptionsRowId::UiScale)},
        {RuntimeOptionsRowId::Save, "Save Settings", ""},
        {RuntimeOptionsRowId::Back, "Back", ""},
    };
    if (!rows_.empty()) {
        selected_row_index_ = std::min(selected_row_index_, rows_.size() - 1);
    }
}

std::string RuntimeOptionsScene::rowValue(RuntimeOptionsRowId id) const {
    switch (id) {
    case RuntimeOptionsRowId::WindowWidth:
        return std::to_string(settings_.window.width);
    case RuntimeOptionsRowId::WindowHeight:
        return std::to_string(settings_.window.height);
    case RuntimeOptionsRowId::MasterVolume:
        return percentText(settings_.audio.master_volume);
    case RuntimeOptionsRowId::BgmVolume:
        return percentText(settings_.audio.bgm_volume);
    case RuntimeOptionsRowId::InputMapping:
        return pathText(settings_.input_mapping_path);
    case RuntimeOptionsRowId::HighContrast:
        return boolText(settings_.accessibility.high_contrast);
    case RuntimeOptionsRowId::ReduceMotion:
        return boolText(settings_.accessibility.reduce_motion);
    case RuntimeOptionsRowId::UiScale:
        return scaleText(settings_.accessibility.ui_scale);
    case RuntimeOptionsRowId::Save:
    case RuntimeOptionsRowId::Back:
        return {};
    }
    return {};
}

std::shared_ptr<RuntimeOptionsScene> makeRuntimeOptionsScene(urpg::settings::RuntimeSettings settings,
                                                             std::filesystem::path settings_path,
                                                             RuntimeOptionsScene::Callbacks callbacks) {
    return std::make_shared<RuntimeOptionsScene>(std::move(settings), std::move(settings_path), std::move(callbacks));
}

} // namespace urpg::scene
