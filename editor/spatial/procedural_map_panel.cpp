#include "editor/spatial/procedural_map_panel.h"

namespace urpg::editor {

void ProceduralMapPanel::generate(const urpg::map::ProceduralMapProfile& profile) {
    result_ = urpg::map::GenerateProceduralMap(profile);
    snapshot_ = {
        true,
        snapshot_.rendered,
        false,
        true,
        result_.document.width(),
        result_.document.height(),
        result_.document.layers().size(),
        result_.diagnostics.size(),
        result_.diagnostics.empty() ? "Procedural map preview is ready." : "Procedural map preview has diagnostics.",
    };
}

void ProceduralMapPanel::render() {
    snapshot_.visible = true;
    snapshot_.rendered = true;
    if (!snapshot_.has_result) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Generate a procedural map profile before previewing this panel.";
        return;
    }

    snapshot_.disabled = false;
    snapshot_.width = result_.document.width();
    snapshot_.height = result_.document.height();
    snapshot_.layer_count = result_.document.layers().size();
    snapshot_.diagnostic_count = result_.diagnostics.size();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "Procedural map preview is ready." : "Procedural map preview has diagnostics.";
}

} // namespace urpg::editor
