#pragma once

#include "engine/core/gameplay/gameplay_wysiwyg_system.h"

#include <nlohmann/json.hpp>

namespace urpg::editor::gameplay {

struct GameplayWysiwygPanelSnapshot {
    std::string feature_id;
    std::string feature_type;
    std::string trigger;
    std::size_t visual_layer_count = 0;
    std::size_t active_rule_count = 0;
    std::size_t event_count = 0;
    std::size_t diagnostic_count = 0;
};

class GameplayWysiwygPanel {
public:
    void loadDocument(urpg::gameplay::GameplayWysiwygDocument document);
    void setPreviewContext(urpg::gameplay::GameplayWysiwygState state, std::string trigger);
    void render();
    urpg::gameplay::GameplayWysiwygPreview executePreview();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const urpg::gameplay::GameplayWysiwygPreview& preview() const { return preview_; }
    [[nodiscard]] const GameplayWysiwygPanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    urpg::gameplay::GameplayWysiwygDocument document_;
    urpg::gameplay::GameplayWysiwygState state_;
    std::string trigger_ = "default";
    urpg::gameplay::GameplayWysiwygPreview preview_{};
    GameplayWysiwygPanelSnapshot snapshot_{};
};

} // namespace urpg::editor::gameplay
