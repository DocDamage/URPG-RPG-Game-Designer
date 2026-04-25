#pragma once

#include "engine/core/battle/battle_presentation_profile.h"

#include <set>

namespace urpg::editor {

struct BattlePresentationPanelSnapshot {
    bool has_profile = false;
    size_t hud_element_count = 0;
    size_t cue_count = 0;
    size_t diagnostic_count = 0;
    size_t replay_cue_count = 0;
};

class BattlePresentationPanel {
public:
    void loadProfile(urpg::battle::BattlePresentationProfile profile, std::set<std::string> available_assets);
    void render();

    const BattlePresentationPanelSnapshot& snapshot() const { return snapshot_; }
    const urpg::battle::BattlePresentationValidationResult& validation() const { return validation_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }

private:
    urpg::battle::BattlePresentationProfile profile_;
    std::set<std::string> available_assets_;
    urpg::battle::BattlePresentationValidationResult validation_;
    BattlePresentationPanelSnapshot snapshot_{};
    bool has_profile_ = false;
    bool has_rendered_frame_ = false;
};

} // namespace urpg::editor
