#pragma once

#include "editor/plugin/plugin_inspector_model.h"

namespace urpg::editor {

class PluginInspectorPanel {
public:
    PluginInspectorModel& model() { return model_; }
    const PluginInspectorModel& model() const { return model_; }

    void render();
    const PluginInspectorSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }

    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }

private:
    PluginInspectorModel model_;
    PluginInspectorSnapshot last_render_snapshot_{};
    bool has_rendered_frame_ = false;
    bool visible_ = true;
};

} // namespace urpg::editor
