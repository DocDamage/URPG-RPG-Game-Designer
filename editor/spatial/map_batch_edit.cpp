#include "editor/spatial/map_batch_edit.h"

#include <algorithm>
#include <cmath>

namespace urpg::editor {
namespace {
using Edit = SpatialAuthoringWorkspace::Perspective2DNativeTileEdit;
MapBatchEditPreview placed(const std::vector<MapTileCell>& cells, int32_t dx, int32_t dy, std::string code) {
    MapBatchEditPreview result{!cells.empty(), false, std::move(code), {}};
    for (const auto& cell : cells) result.edits.push_back({cell.layer_id, cell.tileset_id, cell.tile_id, cell.x + dx, cell.y + dy});
    return result;
}
bool insidePolygon(const MapCellPoint point, const std::vector<MapCellPoint>& polygon) {
    bool inside = false;
    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const auto& a = polygon[i]; const auto& b = polygon[j];
        if (((a.y > point.y) != (b.y > point.y)) &&
            point.x < (b.x - a.x) * static_cast<double>(point.y - a.y) / static_cast<double>(b.y - a.y) + a.x) inside = !inside;
    }
    return inside;
}
bool selectedPoint(const MapCellPoint point, const MapSelectionShape shape,
                   const std::vector<MapCellPoint>& points) {
    if (points.empty() || (shape != MapSelectionShape::Point && points.size() < 2) ||
        (shape == MapSelectionShape::Lasso && points.size() < 3)) return false;
    if (shape == MapSelectionShape::Point) return point.x == points[0].x && point.y == points[0].y;
    if (shape == MapSelectionShape::Lasso) return insidePolygon(point, points);
    const auto minmaxX = std::minmax_element(points.begin(), points.end(), [](auto a, auto b) { return a.x < b.x; });
    const auto minmaxY = std::minmax_element(points.begin(), points.end(), [](auto a, auto b) { return a.y < b.y; });
    return point.x >= minmaxX.first->x && point.x <= minmaxX.second->x &&
           point.y >= minmaxY.first->y && point.y <= minmaxY.second->y;
}
}

std::vector<MapTileCell> MapBatchEdit::select(const std::vector<MapTileCell>& cells, const MapSelectionShape shape,
                                               const std::vector<MapCellPoint>& points) {
    std::vector<MapTileCell> selected;
    for (const auto& cell : cells) {
        if (selectedPoint({cell.x, cell.y}, shape, points)) selected.push_back(cell);
    }
    return selected;
}

std::vector<MapPropCell> MapBatchEdit::selectProps(const std::vector<MapPropCell>& props,
                                                    const MapSelectionShape shape,
                                                    const std::vector<MapCellPoint>& points) {
    std::vector<MapPropCell> selected;
    for (const auto& prop : props) if (selectedPoint({prop.x, prop.y}, shape, points)) selected.push_back(prop);
    return selected;
}

std::vector<MapEventCell> MapBatchEdit::selectEvents(const std::vector<MapEventCell>& events,
                                                      const MapSelectionShape shape,
                                                      const std::vector<MapCellPoint>& points) {
    std::vector<MapEventCell> selected;
    for (const auto& event : events) if (selectedPoint({event.x, event.y}, shape, points)) selected.push_back(event);
    return selected;
}

MapObjectBatchEditPreview MapBatchEdit::objectProperties(
    const std::vector<MapPropCell>& props, const std::vector<MapEventCell>& events,
    const int32_t delta_x, const int32_t delta_y, const std::optional<float> prop_rotation_y,
    const std::optional<float> prop_scale, const std::optional<bool> event_blocks_movement,
    const std::optional<bool> event_sprite_visible) {
    MapObjectBatchEditPreview result;
    const bool changesPosition = delta_x != 0 || delta_y != 0;
    result.valid = (!props.empty() || !events.empty()) &&
                   (changesPosition || prop_rotation_y || prop_scale || event_blocks_movement || event_sprite_visible);
    result.code = result.valid ? "map_object_batch_preview" : "map_object_batch_empty";
    if (!result.valid) return result;
    for (const auto& prop : props) {
        SpatialAuthoringWorkspace::Perspective2DNativePropPropertyEdit edit;
        edit.instance_id = prop.instance_id;
        if (changesPosition) { edit.tile_x = prop.x + delta_x; edit.tile_y = prop.y + delta_y; }
        edit.rotation_y = prop_rotation_y;
        edit.scale = prop_scale;
        result.prop_edits.push_back(std::move(edit));
    }
    for (const auto& event : events) {
        SpatialAuthoringWorkspace::Perspective2DNativeEventPropertyEdit edit;
        edit.event_id = event.event_id;
        if (changesPosition) { edit.tile_x = event.x + delta_x; edit.tile_y = event.y + delta_y; }
        edit.blocks_movement = event_blocks_movement;
        edit.sprite_visible = event_sprite_visible;
        result.event_edits.push_back(std::move(edit));
    }
    return result;
}

MapBatchEditPreview MapBatchEdit::cut(const std::vector<MapTileCell>& cells) {
    MapBatchEditPreview result{!cells.empty(), true, "map_cut_preview", {}};
    for (const auto& cell : cells) result.edits.push_back({cell.layer_id, "", "", cell.x, cell.y, true});
    return result;
}
MapBatchEditPreview MapBatchEdit::paste(const std::vector<MapTileCell>& cells, int32_t x, int32_t y) {
    if (cells.empty()) return {};
    const auto minX = std::min_element(cells.begin(), cells.end(), [](auto& a, auto& b){ return a.x < b.x; })->x;
    const auto minY = std::min_element(cells.begin(), cells.end(), [](auto& a, auto& b){ return a.y < b.y; })->y;
    return placed(cells, x - minX, y - minY, "map_paste_preview");
}
MapBatchEditPreview MapBatchEdit::duplicate(const std::vector<MapTileCell>& cells, int32_t dx, int32_t dy) { return placed(cells, dx, dy, "map_duplicate_preview"); }
MapBatchEditPreview MapBatchEdit::move(const std::vector<MapTileCell>& cells, int32_t dx, int32_t dy) {
    auto result = cut(cells); result.code = "map_move_preview";
    auto destination = placed(cells, dx, dy, ""); result.edits.insert(result.edits.end(), destination.edits.begin(), destination.edits.end());
    return result;
}
MapBatchEditPreview MapBatchEdit::fill(std::string layer, std::string tileset, std::string tile, MapCellPoint a, MapCellPoint b) {
    MapBatchEditPreview result{true, false, "map_fill_preview", {}};
    for (int y = std::min(a.y,b.y); y <= std::max(a.y,b.y); ++y) for (int x = std::min(a.x,b.x); x <= std::max(a.x,b.x); ++x)
        result.edits.push_back({layer, tileset, tile, x, y});
    return result;
}
MapBatchEditPreview MapBatchEdit::line(std::string layer, std::string tileset, std::string tile, MapCellPoint a, MapCellPoint b) {
    MapBatchEditPreview result{true, false, "map_line_preview", {}};
    int dx=std::abs(b.x-a.x), sx=a.x<b.x?1:-1, dy=-std::abs(b.y-a.y), sy=a.y<b.y?1:-1, error=dx+dy;
    for (;;) { result.edits.push_back({layer,tileset,tile,a.x,a.y}); if(a.x==b.x&&a.y==b.y) break; const int twice=2*error; if(twice>=dy){error+=dy;a.x+=sx;} if(twice<=dx){error+=dx;a.y+=sy;} }
    return result;
}
MapBatchEditPreview MapBatchEdit::rectangle(std::string layer, std::string tileset, std::string tile, MapCellPoint a, MapCellPoint b) {
    auto result = fill(std::move(layer), std::move(tileset), std::move(tile), a, b); result.code="map_rectangle_preview";
    result.edits.erase(std::remove_if(result.edits.begin(), result.edits.end(), [&](const Edit& e){ return e.tile_x!=std::min(a.x,b.x)&&e.tile_x!=std::max(a.x,b.x)&&e.tile_y!=std::min(a.y,b.y)&&e.tile_y!=std::max(a.y,b.y); }), result.edits.end()); return result;
}
MapBatchEditPreview MapBatchEdit::replace(const std::vector<MapTileCell>& cells, std::string tileset, std::string tile) {
    MapBatchEditPreview result{!cells.empty(), true, "map_replace_preview", {}};
    for (const auto& cell : cells) {
        result.edits.push_back({cell.layer_id, tileset, tile, cell.x, cell.y});
    }
    return result;
}
MapBatchEditPreview MapBatchEdit::stamp(const std::vector<MapTileCell>& cells, int32_t x, int32_t y) { auto result=paste(cells,x,y); result.code="map_stamp_preview"; return result; }

} // namespace urpg::editor
