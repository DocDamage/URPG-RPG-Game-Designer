#pragma once

#include <string>
#include <vector>
#include <cstdint>

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
 * @brief Logic for validating if two blocks can snap together.
 */
class SnapLogic {
public:
    static bool canSnap(const SnapConnector& a, const SnapConnector& b) {
        // Types must match and sides must be opposites
        if (a.type != b.type) return false;
        
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
