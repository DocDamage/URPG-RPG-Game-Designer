#include "editor/plugin/plugin_inspector_panel.h"

namespace urpg::editor {

void PluginInspectorPanel::render() {
    if (!visible_) {
        return;
    }
    last_render_snapshot_ = model_.snapshot();
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
