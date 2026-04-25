#pragma once

#include "engine/core/battle/boss_profile.h"

namespace urpg::editor {

struct BossDesignerPanelSnapshot {
    bool has_profile = false;
    size_t phase_count = 0;
    size_t diagnostic_count = 0;
};

class BossDesignerPanel {
public:
    void loadProfile(urpg::battle::BossProfile profile);
    void render();

    const BossDesignerPanelSnapshot& snapshot() const { return snapshot_; }
    const urpg::battle::BossProfileValidationResult& validation() const { return validation_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }

private:
    urpg::battle::BossProfile profile_;
    urpg::battle::BossProfileValidationResult validation_;
    BossDesignerPanelSnapshot snapshot_{};
    bool has_profile_ = false;
    bool has_rendered_frame_ = false;
};

} // namespace urpg::editor
