#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

namespace urpg::level {

/**
 * @brief Directional flags for snap connectors.
 */
enum class ConnectorSide : uint8_t {
    North = 0x01,
    South = 0x02,
    East  = 0x04,
    West  = 0x08,
    Up    = 0x10,
    Down  = 0x20
};

/**
 * @brief A point on a block that can snap to other blocks.
 */
struct SnapConnector {
    std::string type; // e.g. "Hallway", "Wall", "Door"
    ConnectorSide side;
    float localX, localY, localZ;
};

/**
 * @brief Asset definition for a reusable modular level block.
 */
class LevelBlock {
public:
    LevelBlock(const std::string& id) : m_id(id) {}

    const std::string& getId() const { return m_id; }
    void addConnector(const SnapConnector& connector) { m_connectors.push_back(connector); }
    const std::vector<SnapConnector>& getConnectors() const { return m_connectors; }

private:
    std::string m_id;
    std::vector<SnapConnector> m_connectors;
};

/**
 * @brief An instance of a block placed in the world.
 */
struct PlacedBlock {
    std::string blockId;
    int32_t x, y, z; // Grid coordinates
};

/**
 * @brief Authoritative workspace for modular level assembly.
 * Manages placed blocks and enforces deterministic placement rules.
 */
class LevelAssemblyWorkspace {
public:
    struct PlacementValidation {
        bool allowed = false;
        std::string reason;
        size_t matching_connections = 0;
    };

    void registerBlockDefinition(const LevelBlock& block) {
        m_blockDefinitions.push_back(block);
    }

    const LevelBlock* findBlockDefinition(const std::string& blockId) const;

    /**
     * @brief Attempt to place a block at specific grid coordinates.
     * @return true if placement is valid (no collisions, valid connectors if required).
     */
    bool placeBlock(const std::string& blockId, int32_t x, int32_t y, int32_t z = 0);
    PlacementValidation validatePlacement(const std::string& blockId, int32_t x, int32_t y, int32_t z = 0) const;

    /**
     * @brief Check for a block at specific coordinates.
     */
    bool hasBlockAt(int32_t x, int32_t y, int32_t z = 0) const;
    const PlacedBlock* getBlockAt(int32_t x, int32_t y, int32_t z = 0) const;

    const std::vector<PlacedBlock>& getPlacedBlocks() const { return m_placedBlocks; }

    void clear() { m_placedBlocks.clear(); }

private:
    static void offsetForSide(ConnectorSide side, int32_t& dx, int32_t& dy, int32_t& dz);

    std::vector<LevelBlock> m_blockDefinitions;
    std::vector<PlacedBlock> m_placedBlocks;
};

/**
 * @brief Logic for validating if two blocks can snap together.
 */
class SnapLogic {
public:
    static bool metadataMatches(const SnapConnector& a, const SnapConnector& b) {
        constexpr float kTolerance = 0.001f;
        return std::fabs(a.localX - b.localX) <= kTolerance &&
               std::fabs(a.localY - b.localY) <= kTolerance &&
               std::fabs(a.localZ - b.localZ) <= kTolerance;
    }

    static bool canSnap(const SnapConnector& a, const SnapConnector& b) {
        // Types, directional pairing, and authored connector metadata must all match.
        if (a.type != b.type) return false;
        if (!metadataMatches(a, b)) return false;
        
        switch (a.side) {
            case ConnectorSide::North: return b.side == ConnectorSide::South;
            case ConnectorSide::South: return b.side == ConnectorSide::North;
            case ConnectorSide::East:  return b.side == ConnectorSide::West;
            case ConnectorSide::West:  return b.side == ConnectorSide::East;
            case ConnectorSide::Up:    return b.side == ConnectorSide::Down;
            case ConnectorSide::Down:  return b.side == ConnectorSide::Up;
        }
        return false;
    }
};

} // namespace urpg::level
