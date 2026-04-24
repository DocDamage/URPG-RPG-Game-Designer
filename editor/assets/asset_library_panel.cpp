#include "editor/assets/asset_library_panel.h"

namespace urpg::editor {

void AssetLibraryPanel::render() {
    if (!visible_) {
        return;
    }
    last_render_snapshot_ = model_.snapshot();
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
