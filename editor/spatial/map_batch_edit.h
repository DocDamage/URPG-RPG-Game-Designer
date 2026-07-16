#pragma once

#include "editor/spatial/spatial_authoring_workspace.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::editor {

struct MapTileCell {
    std::string layer_id;
    std::string tileset_id;
    std::string tile_id;
    int32_t x = 0;
    int32_t y = 0;
};

struct MapCellPoint { int32_t x = 0; int32_t y = 0; };
enum class MapSelectionShape { Point, Box, Lasso };

struct MapBatchEditPreview {
    bool valid = false;
    bool destructive = false;
    std::string code;
    std::vector<SpatialAuthoringWorkspace::Perspective2DNativeTileEdit> edits;
};

class MapBatchEdit {
  public:
    static std::vector<MapTileCell> select(const std::vector<MapTileCell>& cells, MapSelectionShape shape,
                                           const std::vector<MapCellPoint>& points);
    static MapBatchEditPreview cut(const std::vector<MapTileCell>& cells);
    static MapBatchEditPreview paste(const std::vector<MapTileCell>& cells, int32_t anchor_x, int32_t anchor_y);
    static MapBatchEditPreview duplicate(const std::vector<MapTileCell>& cells, int32_t dx, int32_t dy);
    static MapBatchEditPreview move(const std::vector<MapTileCell>& cells, int32_t dx, int32_t dy);
    static MapBatchEditPreview fill(std::string layer, std::string tileset, std::string tile,
                                    MapCellPoint first, MapCellPoint last);
    static MapBatchEditPreview line(std::string layer, std::string tileset, std::string tile,
                                    MapCellPoint first, MapCellPoint last);
    static MapBatchEditPreview rectangle(std::string layer, std::string tileset, std::string tile,
                                         MapCellPoint first, MapCellPoint last);
    static MapBatchEditPreview replace(const std::vector<MapTileCell>& cells, std::string tileset, std::string tile);
    static MapBatchEditPreview stamp(const std::vector<MapTileCell>& cells, int32_t anchor_x, int32_t anchor_y);
};

} // namespace urpg::editor
