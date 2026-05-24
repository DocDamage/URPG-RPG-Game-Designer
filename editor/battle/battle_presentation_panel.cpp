#include "editor/battle/battle_presentation_panel.h"

#include <string>
#include <utility>

namespace urpg::editor {

namespace {

std::vector<std::string>
buildFeedbackCoverageRows(const urpg::battle::BattleFeedbackPolicyImportResult& import_result) {
    std::vector<std::string> rows;
    rows.reserve(import_result.coverage_rows.size());
    for (const auto& row : import_result.coverage_rows) {
        rows.push_back(row.id + ":" + (row.covered ? "covered" : "missing") + ":" + row.source_key);
    }
    return rows;
}

} // namespace

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
        profile_.feedback_policy_import.coverage_rows.size(),
        profile_.feedback_policy_import.diagnostics.size(),
        buildFeedbackCoverageRows(profile_.feedback_policy_import),
    };
}

void BattlePresentationPanel::render() {
    if (!has_profile_) {
        snapshot_ = {};
    }
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
