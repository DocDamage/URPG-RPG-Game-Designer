#include "engine/core/level/level_assembly.h"
#include <algorithm>
#include <cctype>

namespace urpg::level {

namespace {

char sideGlyph(ConnectorSide side) {
    switch (side) {
    case ConnectorSide::North:
        return 'N';
    case ConnectorSide::South:
        return 'S';
    case ConnectorSide::East:
        return 'E';
    case ConnectorSide::West:
        return 'W';
    case ConnectorSide::Up:
        return 'U';
    case ConnectorSide::Down:
        return 'D';
    }

    return '?';
}

void placeGlyph(std::vector<std::string>& rows, size_t row, size_t column, char glyph) {
    if (row >= rows.size() || column >= rows[row].size()) {
        return;
    }

    char& target = rows[row][column];
    if (target != '.' && target != glyph) {
        target = '*';
        return;
    }

    target = glyph;
}

} // namespace

std::vector<LevelBlockThumbnail> LevelBlockLibrary::buildThumbnails() const {
    std::vector<LevelBlockThumbnail> thumbnails;
    thumbnails.reserve(m_blocks.size());

    for (const auto& block : m_blocks) {
        LevelBlockThumbnail thumbnail;
        thumbnail.blockId = block.getId();
        thumbnail.prefabPath = block.getPrefabPath();
        thumbnail.connectorCount = block.getConnectors().size();
        thumbnail.rows = {
            ".....",
            ".....",
            ".....",
            ".....",
            "....."
        };

        const char centerGlyph = block.getId().empty()
            ? '#'
            : static_cast<char>(std::toupper(static_cast<unsigned char>(block.getId().front())));
        thumbnail.rows[2][2] = centerGlyph;

        for (const auto& connector : block.getConnectors()) {
            switch (connector.side) {
            case ConnectorSide::North:
                placeGlyph(thumbnail.rows, 0, 2, sideGlyph(connector.side));
                break;
            case ConnectorSide::South:
                placeGlyph(thumbnail.rows, 4, 2, sideGlyph(connector.side));
                break;
            case ConnectorSide::West:
                placeGlyph(thumbnail.rows, 2, 0, sideGlyph(connector.side));
                break;
            case ConnectorSide::East:
                placeGlyph(thumbnail.rows, 2, 4, sideGlyph(connector.side));
                break;
            case ConnectorSide::Up:
                placeGlyph(thumbnail.rows, 1, 2, sideGlyph(connector.side));
                break;
            case ConnectorSide::Down:
                placeGlyph(thumbnail.rows, 3, 2, sideGlyph(connector.side));
                break;
            }
        }

        thumbnails.push_back(std::move(thumbnail));
    }

    return thumbnails;
}

void LevelAssemblyWorkspace::registerLibrary(const LevelBlockLibrary& library) {
    for (const auto& block : library.getBlocks()) {
        registerBlockDefinition(block);
    }
}

const LevelBlock* LevelAssemblyWorkspace::findBlockDefinition(const std::string& blockId) const {
    auto it = std::find_if(m_blockDefinitions.begin(), m_blockDefinitions.end(), [&](const LevelBlock& block) {
        return block.getId() == blockId;
    });
    return it == m_blockDefinitions.end() ? nullptr : &(*it);
}

bool LevelAssemblyWorkspace::placeBlock(const std::string& blockId, int32_t x, int32_t y, int32_t z) {
    const auto validation = validatePlacement(blockId, x, y, z);
    if (!validation.allowed) {
        return false;
    }

    m_placedBlocks.push_back({blockId, x, y, z});
    return true;
}

LevelAssemblyWorkspace::PlacementValidation LevelAssemblyWorkspace::validatePlacement(const std::string& blockId,
                                                                                     int32_t x,
                                                                                     int32_t y,
                                                                                     int32_t z) const {
    PlacementValidation result;

    if (hasBlockAt(x, y, z)) {
        result.reason = "occupied_cell";
        return result;
    }

    const LevelBlock* block = findBlockDefinition(blockId);
    if (block == nullptr) {
        result.allowed = true;
        result.reason = "unregistered_block";
        return result;
    }

    if (m_placedBlocks.empty()) {
        result.allowed = true;
        result.reason = "seed_block";
        return result;
    }

    size_t matching_connections = 0;
    bool touched_neighbor = false;
    bool found_conflict = false;

    for (const auto& connector : block->getConnectors()) {
        int32_t dx = 0;
        int32_t dy = 0;
        int32_t dz = 0;
        offsetForSide(connector.side, dx, dy, dz);

        const PlacedBlock* neighbor = getBlockAt(x + dx, y + dy, z + dz);
        if (neighbor == nullptr) {
            continue;
        }

        touched_neighbor = true;
        const LevelBlock* neighborBlock = findBlockDefinition(neighbor->blockId);
        if (neighborBlock == nullptr) {
            found_conflict = true;
            break;
        }

        const bool hasMatchingNeighborConnector = std::any_of(
            neighborBlock->getConnectors().begin(),
            neighborBlock->getConnectors().end(),
            [&](const SnapConnector& neighborConnector) {
                return SnapLogic::canSnap(connector, neighborConnector);
            });

        if (!hasMatchingNeighborConnector) {
            found_conflict = true;
            break;
        }

        ++matching_connections;
    }

    result.matching_connections = matching_connections;

    if (found_conflict) {
        result.reason = "connector_mismatch";
        return result;
    }

    if (!touched_neighbor) {
        result.reason = "detached_registered_block";
        return result;
    }

    if (touched_neighbor && matching_connections == 0) {
        result.reason = "missing_matching_connector";
        return result;
    }

    result.allowed = true;
    result.reason = matching_connections > 0 ? "matched_connector" : "detached_valid";
    return result;
}

bool LevelAssemblyWorkspace::hasBlockAt(int32_t x, int32_t y, int32_t z) const {
    return std::any_of(m_placedBlocks.begin(), m_placedBlocks.end(), 
        [=](const auto& b) {
            return b.x == x && b.y == y && b.z == z;
        });
}

const PlacedBlock* LevelAssemblyWorkspace::getBlockAt(int32_t x, int32_t y, int32_t z) const {
    auto it = std::find_if(m_placedBlocks.begin(), m_placedBlocks.end(), [=](const PlacedBlock& block) {
        return block.x == x && block.y == y && block.z == z;
    });
    return it == m_placedBlocks.end() ? nullptr : &(*it);
}

void LevelAssemblyWorkspace::offsetForSide(ConnectorSide side, int32_t& dx, int32_t& dy, int32_t& dz) {
    dx = 0;
    dy = 0;
    dz = 0;
    switch (side) {
    case ConnectorSide::North:
        dy = -1;
        break;
    case ConnectorSide::South:
        dy = 1;
        break;
    case ConnectorSide::East:
        dx = 1;
        break;
    case ConnectorSide::West:
        dx = -1;
        break;
    case ConnectorSide::Up:
        dz = 1;
        break;
    case ConnectorSide::Down:
        dz = -1;
        break;
    }
}

} // namespace urpg::level
