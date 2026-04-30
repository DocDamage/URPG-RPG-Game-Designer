#include "engine/core/map/grid_part_document.h"

#include <algorithm>
#include <set>
#include <utility>

namespace urpg::map {

namespace {

bool chunkLess(const GridPartChunkCoord& left, const GridPartChunkCoord& right) {
    if (left.chunk_y != right.chunk_y) {
        return left.chunk_y < right.chunk_y;
    }
    return left.chunk_x < right.chunk_x;
}

} // namespace

GridPartDocument::GridPartDocument(std::string map_id, int32_t width, int32_t height, int32_t chunk_size)
    : map_id_(std::move(map_id)), width_(width), height_(height), chunk_size_(std::max(1, chunk_size)) {}

const std::string& GridPartDocument::mapId() const {
    return map_id_;
}

int32_t GridPartDocument::width() const {
    return width_;
}

int32_t GridPartDocument::height() const {
    return height_;
}

int32_t GridPartDocument::chunkSize() const {
    return chunk_size_;
}

bool GridPartDocument::placePart(const PlacedPartInstance& instance) {
    if (!validForStorage(instance) || hasInstanceId(instance.instance_id)) {
        return false;
    }

    parts_.push_back(instance);
    rebuildChunks();
    markChunksDirtyForFootprint(instance);
    return true;
}

bool GridPartDocument::replacePart(const PlacedPartInstance& instance) {
    if (!validForStorage(instance)) {
        return false;
    }

    auto* existing = findPartMutable(instance.instance_id);
    if (existing == nullptr || existing->locked) {
        return false;
    }

    const auto previous = *existing;
    *existing = instance;
    rebuildChunks();
    markChunksDirtyForFootprint(previous);
    markChunksDirtyForFootprint(instance);
    return true;
}

bool GridPartDocument::removePart(const std::string& instance_id) {
    const auto* existing = findPart(instance_id);
    if (existing == nullptr || existing->locked) {
        return false;
    }

    const auto previous = *existing;
    const auto originalSize = parts_.size();
    parts_.erase(std::remove_if(parts_.begin(), parts_.end(),
                                [&](const PlacedPartInstance& part) { return part.instance_id == instance_id; }),
                 parts_.end());
    const bool removed = parts_.size() != originalSize;
    if (removed) {
        rebuildChunks();
        markChunksDirtyForFootprint(previous);
    }
    return removed;
}

bool GridPartDocument::movePart(const std::string& instance_id, int32_t next_x, int32_t next_y, int32_t next_z) {
    auto* part = findPartMutable(instance_id);
    if (part == nullptr || part->locked) {
        return false;
    }

    auto moved = *part;
    moved.grid_x = next_x;
    moved.grid_y = next_y;
    moved.grid_z = next_z;
    if (!footprintInBounds(moved)) {
        return false;
    }

    const auto previous = *part;
    part->grid_x = next_x;
    part->grid_y = next_y;
    part->grid_z = next_z;
    rebuildChunks();
    markChunksDirtyForFootprint(previous);
    markChunksDirtyForFootprint(*part);
    return true;
}

bool GridPartDocument::resizePart(const std::string& instance_id, int32_t next_width, int32_t next_height) {
    auto* part = findPartMutable(instance_id);
    if (part == nullptr || part->locked) {
        return false;
    }

    auto resized = *part;
    resized.width = next_width;
    resized.height = next_height;
    if (!footprintInBounds(resized)) {
        return false;
    }

    const auto previous = *part;
    part->width = next_width;
    part->height = next_height;
    rebuildChunks();
    markChunksDirtyForFootprint(previous);
    markChunksDirtyForFootprint(*part);
    return true;
}

PlacedPartInstance* GridPartDocument::findPartMutable(const std::string& instance_id) {
    auto found = std::find_if(parts_.begin(), parts_.end(),
                              [&](const PlacedPartInstance& part) { return part.instance_id == instance_id; });
    return found == parts_.end() ? nullptr : &(*found);
}

const PlacedPartInstance* GridPartDocument::findPart(const std::string& instance_id) const {
    auto found = std::find_if(parts_.begin(), parts_.end(),
                              [&](const PlacedPartInstance& part) { return part.instance_id == instance_id; });
    return found == parts_.end() ? nullptr : &(*found);
}

std::vector<const PlacedPartInstance*> GridPartDocument::partsAt(int32_t x, int32_t y) const {
    std::vector<const PlacedPartInstance*> matches;
    for (const auto& part : parts_) {
        if (x >= part.grid_x && x < part.grid_x + part.width && y >= part.grid_y && y < part.grid_y + part.height) {
            matches.push_back(&part);
        }
    }
    return matches;
}

std::vector<const PlacedPartInstance*> GridPartDocument::partsAtLayer(int32_t x, int32_t y, GridPartLayer layer) const {
    std::vector<const PlacedPartInstance*> matches;
    for (const auto* part : partsAt(x, y)) {
        if (part->layer == layer) {
            matches.push_back(part);
        }
    }
    return matches;
}

bool GridPartDocument::inBounds(int32_t x, int32_t y) const {
    return x >= 0 && y >= 0 && x < width_ && y < height_;
}

bool GridPartDocument::footprintInBounds(const PlacedPartInstance& instance) const {
    if (instance.width <= 0 || instance.height <= 0) {
        return false;
    }
    return inBounds(instance.grid_x, instance.grid_y) &&
           inBounds(instance.grid_x + instance.width - 1, instance.grid_y + instance.height - 1);
}

bool GridPartDocument::hasInstanceId(const std::string& instance_id) const {
    return findPart(instance_id) != nullptr;
}

const std::vector<PlacedPartInstance>& GridPartDocument::parts() const {
    return parts_;
}

const std::vector<GridPartChunk>& GridPartDocument::chunks() const {
    return chunks_;
}

std::vector<GridPartChunk> GridPartDocument::dirtyChunks() const {
    std::vector<GridPartChunk> dirty;
    for (const auto& chunk : chunks_) {
        if (chunk.dirty) {
            dirty.push_back(chunk);
        }
    }
    return dirty;
}

void GridPartDocument::clearDirtyChunks() {
    for (auto& chunk : chunks_) {
        chunk.dirty = false;
    }
    chunks_.erase(std::remove_if(chunks_.begin(), chunks_.end(),
                                 [](const GridPartChunk& chunk) { return chunk.instance_ids.empty(); }),
                  chunks_.end());
}

bool GridPartDocument::markPartDirty(const std::string& instance_id) {
    const auto* part = findPart(instance_id);
    if (part == nullptr) {
        return false;
    }

    markChunksDirtyForFootprint(*part);
    return true;
}

GridPartChunkCoord GridPartDocument::chunkCoordForCell(int32_t x, int32_t y) const {
    return {x / chunk_size_, y / chunk_size_};
}

std::vector<GridPartChunkCoord> GridPartDocument::chunkCoordsForFootprint(const PlacedPartInstance& instance) const {
    if (instance.width <= 0 || instance.height <= 0) {
        return {};
    }

    std::vector<GridPartChunkCoord> coords;
    const auto min = chunkCoordForCell(std::max(0, instance.grid_x), std::max(0, instance.grid_y));
    const auto max = chunkCoordForCell(std::max(0, instance.grid_x + instance.width - 1),
                                       std::max(0, instance.grid_y + instance.height - 1));
    for (int32_t y = min.chunk_y; y <= max.chunk_y; ++y) {
        for (int32_t x = min.chunk_x; x <= max.chunk_x; ++x) {
            coords.push_back({x, y});
        }
    }
    return coords;
}

bool GridPartDocument::validForStorage(const PlacedPartInstance& instance) const {
    return !instance.instance_id.empty() && !instance.part_id.empty() && footprintInBounds(instance);
}

void GridPartDocument::rebuildChunks() {
    std::set<GridPartChunkCoord, decltype(&chunkLess)> dirtyCoords(chunkLess);
    for (const auto& chunk : chunks_) {
        if (chunk.dirty) {
            dirtyCoords.insert(chunk.coord);
        }
    }

    std::vector<GridPartChunk> rebuilt;
    for (const auto& part : parts_) {
        for (const auto& coord : chunkCoordsForFootprint(part)) {
            auto found = std::find_if(rebuilt.begin(), rebuilt.end(), [&](const GridPartChunk& chunk) {
                return chunk.coord == coord;
            });
            if (found == rebuilt.end()) {
                GridPartChunk chunk;
                chunk.coord = coord;
                chunk.dirty = dirtyCoords.find(coord) != dirtyCoords.end();
                found = rebuilt.insert(rebuilt.end(), std::move(chunk));
            }
            found->instance_ids.push_back(part.instance_id);
        }
    }
    for (const auto& coord : dirtyCoords) {
        const auto found = std::find_if(rebuilt.begin(), rebuilt.end(), [&](const GridPartChunk& chunk) {
            return chunk.coord == coord;
        });
        if (found == rebuilt.end()) {
            GridPartChunk chunk;
            chunk.coord = coord;
            chunk.dirty = true;
            rebuilt.push_back(std::move(chunk));
        }
    }

    for (auto& chunk : rebuilt) {
        std::sort(chunk.instance_ids.begin(), chunk.instance_ids.end());
        chunk.instance_ids.erase(std::unique(chunk.instance_ids.begin(), chunk.instance_ids.end()),
                                 chunk.instance_ids.end());
    }
    std::sort(rebuilt.begin(), rebuilt.end(),
              [](const GridPartChunk& left, const GridPartChunk& right) { return chunkLess(left.coord, right.coord); });
    chunks_ = std::move(rebuilt);
}

void GridPartDocument::markChunksDirtyForFootprint(const PlacedPartInstance& instance) {
    for (const auto& coord : chunkCoordsForFootprint(instance)) {
        markChunkDirty(coord);
    }
}

void GridPartDocument::markChunkDirty(GridPartChunkCoord coord) {
    auto found = std::find_if(chunks_.begin(), chunks_.end(), [&](const GridPartChunk& chunk) {
        return chunk.coord == coord;
    });
    if (found != chunks_.end()) {
        found->dirty = true;
        return;
    }

    GridPartChunk chunk;
    chunk.coord = coord;
    chunk.dirty = true;
    chunks_.push_back(std::move(chunk));
    std::sort(chunks_.begin(), chunks_.end(),
              [](const GridPartChunk& left, const GridPartChunk& right) { return chunkLess(left.coord, right.coord); });
}

} // namespace urpg::map
