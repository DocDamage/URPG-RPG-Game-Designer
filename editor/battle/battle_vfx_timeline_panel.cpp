#include "editor/battle/battle_vfx_timeline_panel.h"

#include <algorithm>
#include <cmath>
#include <limits>
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
    runtime_preview_commands_.clear();
    refreshSnapshot();
}

void BattleVfxTimelinePanel::scrubToFrame(int32_t frame) {
    current_frame_ = has_document_
        ? std::clamp(frame, 0, std::max(0, document_.duration_frames))
        : 0;
    rebuildRuntimePreview();
    refreshSnapshot();
}

bool BattleVfxTimelinePanel::setTrackVisible(std::string track_id, bool visible) {
    const bool changed = has_document_ && document_.setTrackVisible(std::move(track_id), visible);
    if (changed) {
        rebuildRuntimePreview();
    }
    refreshSnapshot();
    return changed;
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
    snapshot_.track_count = document_.tracks().size();
    snapshot_.event_count = document_.events().size();
    snapshot_.timeline_progress = document_.duration_frames <= 0
        ? 0.0f
        : static_cast<float>(current_frame_) / static_cast<float>(document_.duration_frames);
    snapshot_.runtime_preview_cue_count = runtime_preview_cues_.size();
    snapshot_.runtime_preview_commands = runtime_preview_commands_;
    snapshot_.runtime_preview_command_count = snapshot_.runtime_preview_commands.size();

    std::vector<std::string> visible_track_ids;
    for (const auto& track : document_.tracks()) {
        snapshot_.track_ids.push_back(track.id);
        if (track.visible) {
            visible_track_ids.push_back(track.id);
        }
    }
    snapshot_.visible_track_count = visible_track_ids.size();
    auto event_is_visible = [&](const auto& event) {
        return event.track_id.empty() ||
               std::find(visible_track_ids.begin(), visible_track_ids.end(), event.track_id) != visible_track_ids.end();
    };

    for (const auto& event : document_.eventsAtFrame(current_frame_)) {
        if (event_is_visible(event)) {
            snapshot_.visible_event_ids.push_back(event.id);
            if (snapshot_.active_track_id.empty()) {
                snapshot_.active_track_id = event.track_id;
            }
        }
    }
    snapshot_.visible_event_count = snapshot_.visible_event_ids.size();
    snapshot_.visible_event_ratio = snapshot_.event_count == 0
        ? 0.0f
        : static_cast<float>(snapshot_.visible_event_count) / static_cast<float>(snapshot_.event_count);

    int32_t nearest_next_frame = std::numeric_limits<int32_t>::max();
    for (const auto& event : document_.events()) {
        if (event.frame > current_frame_ && event.frame < nearest_next_frame && event_is_visible(event)) {
            nearest_next_frame = event.frame;
            snapshot_.next_event_id = event.id;
        }
    }

    for (const auto& diagnostic : document_.validate()) {
        snapshot_.diagnostics.push_back(diagnostic.code + ":" + diagnostic.event_id);
    }
    snapshot_.diagnostic_count = snapshot_.diagnostics.size();
    if (snapshot_.diagnostic_count > 0) {
        snapshot_.ux_focus_lane = "diagnostics";
        snapshot_.primary_action = "Fix VFX timeline diagnostics before preview approval.";
    } else if (!snapshot_.visible_event_ids.empty()) {
        snapshot_.ux_focus_lane = "live_cue";
        snapshot_.primary_action = "Tune the active cue intensity, anchor, and asset payload.";
    } else if (!snapshot_.next_event_id.empty()) {
        snapshot_.ux_focus_lane = "scrub";
        snapshot_.primary_action = "Scrub to the next authored VFX event.";
    } else {
        snapshot_.ux_focus_lane = "timeline";
        snapshot_.primary_action = "Add or reveal a VFX event track for this frame.";
    }
}

void BattleVfxTimelinePanel::rebuildRuntimePreview() {
    runtime_preview_cues_.clear();
    runtime_preview_commands_.clear();
    if (!has_document_) {
        return;
    }
    runtime_preview_commands_ = document_.runtimeCommandsAtFrame(current_frame_);

    std::vector<std::string> visible_track_ids;
    for (const auto& track : document_.tracks()) {
        if (track.visible) {
            visible_track_ids.push_back(track.id);
        }
    }
    auto event_is_visible = [&](const auto& event) {
        return event.track_id.empty() ||
               std::find(visible_track_ids.begin(), visible_track_ids.end(), event.track_id) != visible_track_ids.end();
    };
    for (const auto& cue : document_.toEffectCues()) {
        if (cue.frameTick <= static_cast<std::uint64_t>(std::max(0, current_frame_))) {
            const auto events = document_.eventsAtFrame(static_cast<int32_t>(cue.frameTick));
            if (std::any_of(events.begin(), events.end(), event_is_visible)) {
                runtime_preview_cues_.push_back(cue);
            }
        }
    }
}

} // namespace urpg::editor
