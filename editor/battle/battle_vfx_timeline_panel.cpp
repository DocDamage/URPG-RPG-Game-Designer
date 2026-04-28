#include "editor/battle/battle_vfx_timeline_panel.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace urpg::editor {

void BattleVfxTimelinePanel::loadDocument(urpg::battle::BattleVfxTimelineDocument document) {
    document_ = std::move(document);
    has_document_ = true;
    current_frame_ = std::clamp(current_frame_, 0, std::max(0, document_.duration_frames));
    rebuildRuntimePreview();
    refreshSnapshot();
}

void BattleVfxTimelinePanel::clearDocument() {
    document_ = {};
    has_document_ = false;
    is_playing_ = false;
    current_frame_ = 0;
    runtime_preview_cues_.clear();
    refreshSnapshot();
}

void BattleVfxTimelinePanel::scrubToFrame(int32_t frame) {
    current_frame_ = has_document_
        ? std::clamp(frame, 0, std::max(0, document_.duration_frames))
        : 0;
    rebuildRuntimePreview();
    refreshSnapshot();
}

void BattleVfxTimelinePanel::setPlaying(bool playing) {
    is_playing_ = playing && has_document_;
    refreshSnapshot();
}

void BattleVfxTimelinePanel::update(float delta_seconds) {
    if (!has_document_ || !is_playing_) {
        refreshSnapshot();
        return;
    }

    const int32_t advance = std::max(1, static_cast<int32_t>(std::round(delta_seconds * document_.fps)));
    current_frame_ += advance;
    if (current_frame_ > document_.duration_frames) {
        current_frame_ = document_.duration_frames;
        is_playing_ = false;
    }
    rebuildRuntimePreview();
    refreshSnapshot();
}

void BattleVfxTimelinePanel::render() {
    refreshSnapshot();
}

nlohmann::json BattleVfxTimelinePanel::saveProjectData() const {
    return has_document_ ? document_.toJson() : nlohmann::json::object();
}

void BattleVfxTimelinePanel::refreshSnapshot() {
    snapshot_ = {};
    snapshot_.has_document = has_document_;
    snapshot_.is_playing = is_playing_;
    snapshot_.current_frame = current_frame_;
    if (!has_document_) {
        return;
    }

    snapshot_.duration_frames = document_.duration_frames;
    snapshot_.event_count = document_.events().size();
    snapshot_.runtime_preview_cue_count = runtime_preview_cues_.size();

    for (const auto& event : document_.eventsAtFrame(current_frame_)) {
        snapshot_.visible_event_ids.push_back(event.id);
    }
    snapshot_.visible_event_count = snapshot_.visible_event_ids.size();

    for (const auto& diagnostic : document_.validate()) {
        snapshot_.diagnostics.push_back(diagnostic.code + ":" + diagnostic.event_id);
    }
    snapshot_.diagnostic_count = snapshot_.diagnostics.size();
}

void BattleVfxTimelinePanel::rebuildRuntimePreview() {
    runtime_preview_cues_.clear();
    if (!has_document_) {
        return;
    }
    for (const auto& cue : document_.toEffectCues()) {
        if (cue.frameTick <= static_cast<std::uint64_t>(std::max(0, current_frame_))) {
            runtime_preview_cues_.push_back(cue);
        }
    }
}

} // namespace urpg::editor
