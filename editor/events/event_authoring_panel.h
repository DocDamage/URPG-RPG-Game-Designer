#pragma once

#include "editor/events/event_authoring_model.h"

namespace urpg::editor {

class EventAuthoringPanel {
public:
    EventAuthoringModel& model() { return model_; }
    const EventAuthoringModel& model() const { return model_; }

    void render();
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }
    const EventAuthoringModelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    EventAuthoringModel model_;
    EventAuthoringModelSnapshot last_render_snapshot_{};
    bool visible_ = true;
    bool has_rendered_frame_ = false;
};

} // namespace urpg::editor
