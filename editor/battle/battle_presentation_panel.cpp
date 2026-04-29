#include "editor/battle/battle_presentation_panel.h"

#include <utility>

namespace urpg::editor {

void BattlePresentationPanel::loadProfile(urpg::battle::BattlePresentationProfile profile,
                                          std::set<std::string> available_assets) {
    profile_ = std::move(profile);
    available_assets_ = std::move(available_assets);
    validation_ = urpg::battle::ValidateBattlePresentationProfile(profile_, available_assets_);
    has_profile_ = true;
    snapshot_ = {
        true,
        profile_.hud_elements.size(),
        profile_.cue_timeline.size(),
        profile_.media_layers.size(),
        profile_.light_cues.size(),
        validation_.diagnostics.size(),
        validation_.replay_cues.size(),
    };
}

void BattlePresentationPanel::render() {
    if (!has_profile_) {
        snapshot_ = {};
    }
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
