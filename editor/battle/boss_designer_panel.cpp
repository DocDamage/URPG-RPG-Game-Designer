#include "editor/battle/boss_designer_panel.h"

#include <utility>

namespace urpg::editor {

void BossDesignerPanel::loadProfile(urpg::battle::BossProfile profile) {
    profile_ = std::move(profile);
    validation_ = urpg::battle::ValidateBossProfile(profile_);
    has_profile_ = true;
    snapshot_ = {true, profile_.phases.size(), validation_.diagnostics.size()};
}

void BossDesignerPanel::render() {
    if (!has_profile_) {
        snapshot_ = {};
    }
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
