#pragma once

#include "engine/core/scene/scene_manager.h"
#include "engine/core/settings/app_settings_store.h"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace urpg::scene {

enum class RuntimeOptionsRowId {
    WindowWidth,
    WindowHeight,
    MasterVolume,
    BgmVolume,
    InputMapping,
    HighContrast,
    ReduceMotion,
    UiScale,
    Save,
    Back,
};

struct RuntimeOptionsRow {
    RuntimeOptionsRowId id = RuntimeOptionsRowId::WindowWidth;
    std::string label;
    std::string value;
};

struct RuntimeOptionsCommandResult {
    bool handled = false;
    bool success = false;
    std::string code;
    std::string message;
};

class RuntimeOptionsScene final : public GameScene {
  public:
    struct Callbacks {
        std::function<void()> request_back;
        std::function<void(const urpg::settings::RuntimeSettings&)> settings_saved;
    };

    RuntimeOptionsScene(urpg::settings::RuntimeSettings settings, std::filesystem::path settings_path,
                        Callbacks callbacks = {});

    SceneType getType() const override { return SceneType::OPTIONS; }
    std::string getName() const override { return "RuntimeOptions"; }

    void onUpdate(float deltaTime) override;
    void handleInput(const urpg::input::InputCore& input) override;

    const urpg::settings::RuntimeSettings& settings() const { return settings_; }
    const std::vector<RuntimeOptionsRow>& rows() const { return rows_; }
    size_t selectedRowIndex() const { return selected_row_index_; }
    const RuntimeOptionsRow* selectedRow() const;
    const RuntimeOptionsCommandResult& lastCommandResult() const { return last_command_result_; }

    RuntimeOptionsCommandResult activateSelected();
    RuntimeOptionsCommandResult save();
    RuntimeOptionsCommandResult back();
    void adjustSelected(int direction);

  private:
    void moveSelection(int direction);
    void rebuildRows();
    std::string rowValue(RuntimeOptionsRowId id) const;

    urpg::settings::RuntimeSettings settings_;
    std::filesystem::path settings_path_;
    Callbacks callbacks_;
    std::vector<RuntimeOptionsRow> rows_;
    size_t selected_row_index_ = 0;
    RuntimeOptionsCommandResult last_command_result_;
};

std::shared_ptr<RuntimeOptionsScene> makeRuntimeOptionsScene(urpg::settings::RuntimeSettings settings,
                                                             std::filesystem::path settings_path,
                                                             RuntimeOptionsScene::Callbacks callbacks = {});

} // namespace urpg::scene
