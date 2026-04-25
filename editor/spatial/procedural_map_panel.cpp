#include "editor/spatial/procedural_map_panel.h"

namespace urpg::editor {

void ProceduralMapPanel::generate(const urpg::map::ProceduralMapProfile& profile) {
    result_ = urpg::map::GenerateProceduralMap(profile);
    snapshot_ = {
        true,
        result_.document.width(),
        result_.document.height(),
        result_.document.layers().size(),
        result_.diagnostics.size(),
    };
}

void ProceduralMapPanel::render() {}

} // namespace urpg::editor
