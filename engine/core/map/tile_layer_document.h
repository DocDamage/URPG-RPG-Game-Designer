#pragma once

#include <cstdint>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace urpg::map {

struct MapDiagnostic {
    std::string code;
    std::string message;
    int32_t x = -1;
    int32_t y = -1;
    std::string target;
};

struct TileLayer {
    std::string id;
    bool visible = true;
    bool locked = false;
    bool collision = false;
    bool navigation = true;
    int32_t draw_order = 0;
    std::vector<int32_t> tiles;
};

class TileLayerDocument {
public:
    TileLayerDocument() = default;
    TileLayerDocument(int32_t width, int32_t height);

    void addLayer(TileLayer layer);
    bool setTile(const std::string& layer_id, int32_t x, int32_t y, int32_t tile_id);
    std::optional<int32_t> tileAt(const std::string& layer_id, int32_t x, int32_t y) const;
    std::vector<MapDiagnostic> validateNavigation() const;

    int32_t width() const { return width_; }
    int32_t height() const { return height_; }
    const std::vector<TileLayer>& layers() const { return layers_; }

private:
    TileLayer* findLayer(const std::string& layer_id);
    const TileLayer* findLayer(const std::string& layer_id) const;
    bool inBounds(int32_t x, int32_t y) const;
    size_t index(int32_t x, int32_t y) const;

    int32_t width_ = 0;
    int32_t height_ = 0;
    std::vector<TileLayer> layers_;
};

} // namespace urpg::map
