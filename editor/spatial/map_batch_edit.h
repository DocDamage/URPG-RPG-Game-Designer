#pragma once

#include "editor/spatial/spatial_authoring_workspace.h"

#include <cstdint>
#include <string>
#include <optional>
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
struct MapPropCell { std::string instance_id; int32_t x = 0; int32_t y = 0; };
struct MapEventCell { std::string event_id; int32_t x = 0; int32_t y = 0; };
enum class MapSelectionShape { Point, Box, Lasso };

struct MapBatchEditPreview {
    bool valid = false;
    bool destructive = false;
    std::string code;
    std::vector<SpatialAuthoringWorkspace::Perspective2DNativeTileEdit> edits;
};

struct MapObjectBatchEditPreview {
    bool valid = false;
    bool destructive = true;
    std::string code;
    std::vector<SpatialAuthoringWorkspace::Perspective2DNativePropPropertyEdit> prop_edits;
    std::vector<SpatialAuthoringWorkspace::Perspective2DNativeEventPropertyEdit> event_edits;
};

class MapBatchEdit {
  public:
    static std::vector<MapTileCell> select(const std::vector<MapTileCell>& cells, MapSelectionShape shape,
                                           const std::vector<MapCellPoint>& points);
    static std::vector<MapPropCell> selectProps(const std::vector<MapPropCell>& props, MapSelectionShape shape,
                                                const std::vector<MapCellPoint>& points);
    static std::vector<MapEventCell> selectEvents(const std::vector<MapEventCell>& events, MapSelectionShape shape,
                                                  const std::vector<MapCellPoint>& points);
    static MapObjectBatchEditPreview objectProperties(
        const std::vector<MapPropCell>& props, const std::vector<MapEventCell>& events,
        int32_t delta_x, int32_t delta_y, std::optional<float> prop_rotation_y = {},
        std::optional<float> prop_scale = {}, std::optional<bool> event_blocks_movement = {},
        std::optional<bool> event_sprite_visible = {});
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
