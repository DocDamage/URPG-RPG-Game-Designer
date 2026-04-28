#pragma once

#include "engine/core/community/community_wysiwyg_feature.h"

#include <nlohmann/json.hpp>

namespace urpg::editor::community {

struct CommunityWysiwygPanelSnapshot {
    std::string feature_type;
    std::string trigger;
    std::size_t visual_layer_count = 0;
    std::size_t active_action_count = 0;
    std::size_t emitted_command_count = 0;
    std::size_t diagnostic_count = 0;
};

class CommunityWysiwygPanel {
public:
    void loadDocument(urpg::community::CommunityWysiwygFeatureDocument document);
    void setPreviewContext(urpg::community::CommunityFeatureRuntimeState state, std::string trigger);
    void render();
    urpg::community::CommunityFeaturePreview executePreview();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const urpg::community::CommunityFeaturePreview& preview() const { return preview_; }
    [[nodiscard]] const CommunityWysiwygPanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    urpg::community::CommunityWysiwygFeatureDocument document_;
    urpg::community::CommunityFeatureRuntimeState state_;
    std::string trigger_ = "default";
    urpg::community::CommunityFeaturePreview preview_{};
    CommunityWysiwygPanelSnapshot snapshot_{};
};

} // namespace urpg::editor::community
