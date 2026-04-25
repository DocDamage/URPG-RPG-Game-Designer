#include "engine/core/map/tile_layer_document.h"

#include <algorithm>

namespace urpg::map {

TileLayerDocument::TileLayerDocument(int32_t width, int32_t height)
    : width_(std::max(0, width)), height_(std::max(0, height)) {}

void TileLayerDocument::addLayer(TileLayer layer) {
    const size_t expected_size = static_cast<size_t>(width_ * height_);
    layer.tiles.resize(expected_size, 0);
    layers_.push_back(std::move(layer));
    std::stable_sort(layers_.begin(), layers_.end(), [](const auto& a, const auto& b) {
        if (a.draw_order != b.draw_order) {
            return a.draw_order < b.draw_order;
        }
        return a.id < b.id;
    });
}

bool TileLayerDocument::setTile(const std::string& layer_id, int32_t x, int32_t y, int32_t tile_id) {
    auto* layer = findLayer(layer_id);
    if (layer == nullptr || layer->locked || !inBounds(x, y)) {
        return false;
    }
    layer->tiles[index(x, y)] = tile_id;
    return true;
}

std::optional<int32_t> TileLayerDocument::tileAt(const std::string& layer_id, int32_t x, int32_t y) const {
    const auto* layer = findLayer(layer_id);
    if (layer == nullptr || !inBounds(x, y)) {
        return std::nullopt;
    }
    return layer->tiles[index(x, y)];
}

std::vector<MapDiagnostic> TileLayerDocument::validateNavigation() const {
    std::vector<MapDiagnostic> diagnostics;
    for (int32_t y = 0; y < height_; ++y) {
        for (int32_t x = 0; x < width_; ++x) {
            bool blocked = false;
            bool navigable = false;
            for (const auto& layer : layers_) {
                if (layer.collision && layer.tiles[index(x, y)] != 0) {
                    blocked = true;
                }
                if (layer.navigation && layer.tiles[index(x, y)] != 0) {
                    navigable = true;
                }
            }
            if (blocked && navigable) {
                diagnostics.push_back({"navigation_on_blocked_cell", "Navigation generated for impassable cell.", x, y, ""});
            }
        }
    }
    return diagnostics;
}

TileLayer* TileLayerDocument::findLayer(const std::string& layer_id) {
    auto it = std::find_if(layers_.begin(), layers_.end(), [&](const auto& layer) { return layer.id == layer_id; });
    return it == layers_.end() ? nullptr : &*it;
}

const TileLayer* TileLayerDocument::findLayer(const std::string& layer_id) const {
    auto it = std::find_if(layers_.begin(), layers_.end(), [&](const auto& layer) { return layer.id == layer_id; });
    return it == layers_.end() ? nullptr : &*it;
}

bool TileLayerDocument::inBounds(int32_t x, int32_t y) const {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
}

size_t TileLayerDocument::index(int32_t x, int32_t y) const {
    return static_cast<size_t>(y * width_ + x);
}

} // namespace urpg::map
