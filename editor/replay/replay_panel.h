#pragma once

#include "engine/core/replay/replay_gallery.h"

#include <string>
#include <vector>

namespace urpg::editor {

struct ReplayPanelSnapshot {
    std::size_t artifact_count = 0;
    std::vector<std::string> artifact_ids;
};

class ReplayPanel {
public:
    replay::ReplayGallery& gallery() { return gallery_; }
    const replay::ReplayGallery& gallery() const { return gallery_; }

    ReplayPanelSnapshot snapshot() const;
    void render();
    const ReplayPanelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }

private:
    replay::ReplayGallery gallery_;
    ReplayPanelSnapshot last_render_snapshot_{};
    bool has_rendered_frame_ = false;
};

} // namespace urpg::editor
