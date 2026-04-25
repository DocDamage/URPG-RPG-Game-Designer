#include "editor/events/event_authoring_panel.h"

namespace urpg::editor {

void EventAuthoringPanel::render() {
    if (!visible_) {
        return;
    }
    last_render_snapshot_ = model_.snapshot();
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
