#include "editor/replay/replay_panel.h"

#include <algorithm>

namespace urpg::editor {

ReplayPanelSnapshot ReplayPanel::snapshot() const {
    ReplayPanelSnapshot snapshot;
    snapshot.artifact_count = gallery_.artifacts().size();
    for (const auto& artifact : gallery_.artifacts()) {
        snapshot.artifact_ids.push_back(artifact.id);
    }
    std::stable_sort(snapshot.artifact_ids.begin(), snapshot.artifact_ids.end());
    return snapshot;
}

void ReplayPanel::render() {
    last_render_snapshot_ = snapshot();
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
