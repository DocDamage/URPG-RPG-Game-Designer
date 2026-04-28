#include "editor/spatial/dungeon3d_world_panel.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

void Dungeon3DWorldPanel::loadDocument(urpg::render::Dungeon3DWorldDocument document) {
    document_ = std::move(document);
    loaded_ = true;
    refresh();
}

void Dungeon3DWorldPanel::setMode(std::string mode) {
    document_.mode = std::move(mode);
    if (loaded_) {
        refresh();
    }
}

void Dungeon3DWorldPanel::render() {
    snapshot_.visible = true;
    if (!loaded_) {
        snapshot_.disabled = true;
        snapshot_.status_message = "Load a 3D dungeon world document before rendering this panel.";
        return;
    }
    refresh();
}

nlohmann::json Dungeon3DWorldPanel::saveProjectData() const {
    return document_.toJson();
}

void Dungeon3DWorldPanel::refresh() {
    preview_ = document_.preview();
    snapshot_.disabled = false;
    snapshot_.map_id = document_.map_id;
    snapshot_.mode = document_.mode;
    snapshot_.raycast_column_count = preview_.columns.size();
    snapshot_.minimap_tile_count = preview_.minimap_tiles.size();
    snapshot_.discovered_tile_count = static_cast<size_t>(
        std::count_if(preview_.minimap_tiles.begin(), preview_.minimap_tiles.end(), [](const auto& tile) {
            return tile.discovered;
        }));
    snapshot_.runtime_command_count = preview_.runtime_commands.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
    snapshot_.status_message =
        snapshot_.diagnostic_count == 0 ? "3D dungeon world preview is ready." : "3D dungeon world has diagnostics.";
}

} // namespace urpg::editor
