#pragma once

#include "engine/core/render/dungeon3d_world.h"

#include <nlohmann/json.hpp>

#include <string>

namespace urpg::editor {

struct Dungeon3DWorldPanelSnapshot {
    bool visible = true;
    bool disabled = true;
    std::string map_id;
    std::string mode;
    size_t raycast_column_count = 0;
    size_t minimap_tile_count = 0;
    size_t discovered_tile_count = 0;
    size_t runtime_command_count = 0;
    size_t diagnostic_count = 0;
    std::string status_message = "Load a 3D dungeon world document before rendering this panel.";
};

class Dungeon3DWorldPanel {
public:
    void loadDocument(urpg::render::Dungeon3DWorldDocument document);
    void setMode(std::string mode);
    void render();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const urpg::render::Dungeon3DPreview& preview() const { return preview_; }
    [[nodiscard]] const Dungeon3DWorldPanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    urpg::render::Dungeon3DWorldDocument document_;
    urpg::render::Dungeon3DPreview preview_;
    Dungeon3DWorldPanelSnapshot snapshot_;
    bool loaded_ = false;
};

} // namespace urpg::editor
