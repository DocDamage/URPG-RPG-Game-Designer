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
}

std::vector<MapTileCell> MapBatchEdit::select(const std::vector<MapTileCell>& cells, const MapSelectionShape shape,
                                               const std::vector<MapCellPoint>& points) {
    if (points.empty() || (shape != MapSelectionShape::Point && points.size() < 2) ||
        (shape == MapSelectionShape::Lasso && points.size() < 3)) return {};
    const auto minmaxX = std::minmax_element(points.begin(), points.end(), [](auto a, auto b) { return a.x < b.x; });
    const auto minmaxY = std::minmax_element(points.begin(), points.end(), [](auto a, auto b) { return a.y < b.y; });
    std::vector<MapTileCell> selected;
    for (const auto& cell : cells) {
        const MapCellPoint point{cell.x, cell.y};
        const bool match = shape == MapSelectionShape::Point ? (cell.x == points[0].x && cell.y == points[0].y) :
            (shape == MapSelectionShape::Box ? cell.x >= minmaxX.first->x && cell.x <= minmaxX.second->x &&
                                               cell.y >= minmaxY.first->y && cell.y <= minmaxY.second->y
                                             : insidePolygon(point, points));
        if (match) selected.push_back(cell);
    }
    return selected;
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
