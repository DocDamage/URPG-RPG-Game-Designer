#pragma once

#include "engine/core/timeline/cutscene_timeline.h"

#include <nlohmann/json.hpp>
#include <set>

namespace urpg::editor {

struct CutsceneTimelinePanelSnapshot {
    std::string cutscene_id;
    int64_t cursor_tick = 0;
    std::size_t active_cue_count = 0;
    std::size_t played_command_count = 0;
    std::size_t variable_write_count = 0;
    std::size_t diagnostic_count = 0;
};

class CutsceneTimelinePanel {
public:
    void loadDocument(timeline::CutsceneTimelineDocument document);
    void setPreviewContext(int64_t cursor_tick, uint64_t seed, std::set<std::string> localization_keys);
    void render();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const timeline::CutsceneTimelinePreview& preview() const { return preview_; }
    [[nodiscard]] const CutsceneTimelinePanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    timeline::CutsceneTimelineDocument document_;
    int64_t cursor_tick_ = 0;
    uint64_t seed_ = 0;
    std::set<std::string> localization_keys_;
    timeline::CutsceneTimelinePreview preview_{};
    CutsceneTimelinePanelSnapshot snapshot_{};
};

} // namespace urpg::editor
