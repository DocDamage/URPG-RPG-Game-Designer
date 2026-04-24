#pragma once

#include "editor/assets/asset_library_model.h"

namespace urpg::editor {

class AssetLibraryPanel {
public:
    AssetLibraryModel& model() { return model_; }
    const AssetLibraryModel& model() const { return model_; }

    void render();
    const AssetLibraryModelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }

private:
    AssetLibraryModel model_;
    AssetLibraryModelSnapshot last_render_snapshot_{};
    bool has_rendered_frame_ = false;
    bool visible_ = true;
};

} // namespace urpg::editor
