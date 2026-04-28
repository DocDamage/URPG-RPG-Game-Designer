#pragma once

#include "engine/core/battle/battle_vfx_timeline.h"

#include <cstddef>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::editor {

struct BattleVfxTimelinePanelSnapshot {
    bool has_document = false;
    bool is_playing = false;
    int32_t current_frame = 0;
    int32_t duration_frames = 0;
    std::size_t event_count = 0;
    std::size_t track_count = 0;
    std::size_t visible_track_count = 0;
    std::size_t visible_event_count = 0;
    std::size_t runtime_preview_cue_count = 0;
    std::size_t runtime_preview_command_count = 0;
    std::size_t diagnostic_count = 0;
    std::vector<std::string> track_ids;
    std::vector<std::string> visible_event_ids;
    std::vector<std::string> runtime_preview_commands;
    std::vector<std::string> diagnostics;
};

class BattleVfxTimelinePanel {
public:
    void loadDocument(urpg::battle::BattleVfxTimelineDocument document);
    void clearDocument();
    bool hasDocument() const { return has_document_; }

    void scrubToFrame(int32_t frame);
    bool setTrackVisible(std::string track_id, bool visible);
    void setPlaying(bool playing);
    void update(float delta_seconds);
    void render();

    nlohmann::json saveProjectData() const;
    const urpg::battle::BattleVfxTimelineDocument& document() const { return document_; }
    const BattleVfxTimelinePanelSnapshot& snapshot() const { return snapshot_; }
    const std::vector<urpg::presentation::effects::EffectCue>& runtimePreviewCues() const {
        return runtime_preview_cues_;
    }

private:
    void refreshSnapshot();
    void rebuildRuntimePreview();

    urpg::battle::BattleVfxTimelineDocument document_;
    bool has_document_ = false;
    bool is_playing_ = false;
    int32_t current_frame_ = 0;
    BattleVfxTimelinePanelSnapshot snapshot_{};
    std::vector<urpg::presentation::effects::EffectCue> runtime_preview_cues_;
    std::vector<std::string> runtime_preview_commands_;
};

} // namespace urpg::editor
