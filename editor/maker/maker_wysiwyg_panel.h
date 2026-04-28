#pragma once

#include "engine/core/maker/maker_wysiwyg_feature.h"

#include <nlohmann/json.hpp>

namespace urpg::editor::maker {

struct MakerWysiwygPanelSnapshot {
    std::string feature_type;
    std::string trigger;
    std::size_t visual_layer_count = 0;
    std::size_t active_action_count = 0;
    std::size_t emitted_operation_count = 0;
    std::size_t diagnostic_count = 0;
};

class MakerWysiwygPanel {
public:
    void loadDocument(urpg::maker::MakerWysiwygFeatureDocument document);
    void setPreviewContext(urpg::maker::MakerFeatureRuntimeState state, std::string trigger);
    void render();
    urpg::maker::MakerFeaturePreview executePreview();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const urpg::maker::MakerFeaturePreview& preview() const { return preview_; }
    [[nodiscard]] const MakerWysiwygPanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    urpg::maker::MakerWysiwygFeatureDocument document_;
    urpg::maker::MakerFeatureRuntimeState state_;
    std::string trigger_ = "default";
    urpg::maker::MakerFeaturePreview preview_{};
    MakerWysiwygPanelSnapshot snapshot_{};
};

} // namespace urpg::editor::maker
