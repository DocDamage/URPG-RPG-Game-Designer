#pragma once

#include "engine/core/map/grid_part_types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::map {

class GridPartDocument {
  public:
    GridPartDocument() = default;
    GridPartDocument(std::string map_id, int32_t width, int32_t height, int32_t chunk_size = 16);

    const std::string& mapId() const;
    int32_t width() const;
    int32_t height() const;
    int32_t chunkSize() const;

    bool placePart(const PlacedPartInstance& instance);
    bool replacePart(const PlacedPartInstance& instance);
    bool removePart(const std::string& instance_id);
    bool movePart(const std::string& instance_id, int32_t next_x, int32_t next_y, int32_t next_z = 0);
    bool resizePart(const std::string& instance_id, int32_t next_width, int32_t next_height);

    PlacedPartInstance* findPartMutable(const std::string& instance_id);
    const PlacedPartInstance* findPart(const std::string& instance_id) const;

    std::vector<const PlacedPartInstance*> partsAt(int32_t x, int32_t y) const;
    std::vector<const PlacedPartInstance*> partsAtLayer(int32_t x, int32_t y, GridPartLayer layer) const;

    bool inBounds(int32_t x, int32_t y) const;
    bool footprintInBounds(const PlacedPartInstance& instance) const;
    bool hasInstanceId(const std::string& instance_id) const;

    const std::vector<PlacedPartInstance>& parts() const;
    const std::vector<GridPartChunk>& chunks() const;
    std::vector<GridPartChunk> dirtyChunks() const;
    void clearDirtyChunks();
    bool markPartDirty(const std::string& instance_id);
    GridPartChunkCoord chunkCoordForCell(int32_t x, int32_t y) const;
    std::vector<GridPartChunkCoord> chunkCoordsForFootprint(const PlacedPartInstance& instance) const;

  private:
    bool validForStorage(const PlacedPartInstance& instance) const;
    void rebuildChunks();
    void markChunksDirtyForFootprint(const PlacedPartInstance& instance);
    void markChunkDirty(GridPartChunkCoord coord);

    std::string map_id_;
    int32_t width_ = 0;
    int32_t height_ = 0;
    int32_t chunk_size_ = 16;
    std::vector<PlacedPartInstance> parts_;
    std::vector<GridPartChunk> chunks_;
};

} // namespace urpg::map
