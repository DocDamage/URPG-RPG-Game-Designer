#pragma once

#include "engine/core/map/procedural_map_generator.h"

#include <cstddef>
#include <string>

namespace urpg::editor {

struct ProceduralMapPanelSnapshot {
    bool visible = true;
    bool rendered = false;
    bool disabled = true;
    bool has_result = false;
    int32_t width = 0;
    int32_t height = 0;
    size_t layer_count = 0;
    size_t diagnostic_count = 0;
    std::string status_message = "Generate a procedural map profile before previewing this panel.";
};

class ProceduralMapPanel {
public:
    void generate(const urpg::map::ProceduralMapProfile& profile);
    void render();

    const ProceduralMapPanelSnapshot& snapshot() const { return snapshot_; }
    const urpg::map::ProceduralMapResult& result() const { return result_; }
    bool hasRenderedFrame() const { return snapshot_.rendered; }

private:
    urpg::map::ProceduralMapResult result_;
    ProceduralMapPanelSnapshot snapshot_{};
};

} // namespace urpg::editor
