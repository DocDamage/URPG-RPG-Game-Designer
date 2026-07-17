#include "editor/replay/replay_panel.h"

#include <algorithm>

namespace urpg::editor {

ReplayPanelSnapshot ReplayPanel::snapshot() const {
    ReplayPanelSnapshot snapshot;
    snapshot.artifact_count = gallery_.artifacts().size();
    for (const auto& artifact : gallery_.artifacts()) {
        snapshot.artifact_ids.push_back(artifact.id);
    }
    std::stable_sort(snapshot.artifact_ids.begin(), snapshot.artifact_ids.end());
    if (live_snapshot_) {
        snapshot.live_connected = true;
        snapshot.capturing = live_snapshot_->capturing;
        snapshot.live_revision = live_snapshot_->revision;
        snapshot.semantic_input_count = live_snapshot_->artifact.input_log.size();
        snapshot.checkpoint_count = live_snapshot_->artifact.checkpoints.size();
        snapshot.project_revision = live_snapshot_->artifact.project_revision;
        snapshot.runtime_version = live_snapshot_->artifact.runtime_version;
        snapshot.last_control_id = live_snapshot_->last_control_id;
        snapshot.last_result = live_snapshot_->last_result;
    }
    return snapshot;
}

void ReplayPanel::render() {
    (void)refreshLive();
    last_render_snapshot_ = snapshot();
    has_rendered_frame_ = true;
}

void ReplayPanel::bindLiveSession(const std::filesystem::path& session_directory) {
    live_bridge_ = std::make_unique<playtest::PlaytestScenarioReplayBridge>(session_directory);
    live_snapshot_.reset();
    next_control_id_ = 1;
}

bool ReplayPanel::refreshLive(std::string* diagnostic) {
    if (!live_bridge_) return false;
    const auto next = live_bridge_->readAfter(live_snapshot_ ? live_snapshot_->revision : 0, diagnostic);
    if (!next) return diagnostic == nullptr || diagnostic->empty();
    live_snapshot_ = *next;
    gallery_.upsert(live_snapshot_->artifact);
    next_control_id_ = std::max(next_control_id_, live_snapshot_->last_control_id + 1);
    return true;
}

bool ReplayPanel::requestLiveReplay(const replay::ReplayExecutionMode mode, std::string* diagnostic) {
    if (!live_bridge_ || !live_snapshot_) {
        if (diagnostic) *diagnostic = "scenario_replay_not_connected";
        return false;
    }
    const playtest::ScenarioReplayControl control{next_control_id_, live_snapshot_->revision, mode};
    if (!live_bridge_->appendControl(control, diagnostic)) return false;
    ++next_control_id_;
    return true;
}

} // namespace urpg::editor
