#pragma once

#include "engine/core/timeline/timeline_document.h"

#include <string>
#include <vector>

namespace urpg::editor {

struct TimelinePanelSnapshot {
    bool has_document = false;
    std::size_t actor_count = 0;
    std::size_t command_count = 0;
    std::vector<std::string> diagnostics;
};

class TimelinePanel {
public:
    void setDocument(timeline::TimelineDocument document);
    void clearDocument();
    TimelinePanelSnapshot snapshot() const;

    void render();
    const TimelinePanelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }

private:
    timeline::TimelineDocument document_;
    bool has_document_ = false;
    TimelinePanelSnapshot last_render_snapshot_{};
    bool has_rendered_frame_ = false;
};

} // namespace urpg::editor
