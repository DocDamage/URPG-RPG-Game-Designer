#pragma once

#include <vector>
#include <string>
#include <map>
#include <memory>
#include "math/vector2.h"

namespace urpg::templates {

    /**
     * @brief Grid-based movement and combat data for Tactics/SRPG.
     * Part of Wave 3.15/3.18 Template Expansion.
     */
    enum class GridCellType { Normal, Difficult, Lethal, Blocked };

    struct UnitStats {
        int moveRange = 4;
        int attackRange = 1;
        float height = 0.0f;
        int teamId = 0;
        bool hasMoved = false;
        bool hasActed = false;

        // Combat Stats
        int hp = 100;
        int maxHp = 100;
        int attack = 15;
        int defense = 5;
        int speed = 10;
        float hitRate = 0.85f;
    };

    /**
     * @brief Discrete grid coordinate.
     */
    struct GridCoord {
        int x, y;
        bool operator==(const GridCoord& other) const { return x == other.x; } // simplified
        bool operator<(const GridCoord& other) const { 
            return x < other.x || (x == other.x && y < other.y); 
        }
    };

    /**
     * @brief Result of a combat engagement.
     */
    struct CombatResult {
        int damageDealt;
        bool isCritical;
        bool isMiss;
        bool targetDied;
    };

    /**
     * @brief Core Battle Engine for Tactics-style combat.
     * Manages turn orders, movement reachability, and tile-based selection.
     */
    class TacticsEngine {
    public:
        /**
         * @brief Tactics-style deterministic or RNG-based combat resolver.
         */
        CombatResult resolveCombat(const std::string& attackerId, const std::string& defenderId) {
            auto& a = m_units[attackerId];
            auto& d = m_units[defenderId];

            float hitChance = a.hitRate * (1.0f - (d.speed * 0.01f));
            bool isMiss = (static_cast<float>(rand()) / RAND_MAX) > hitChance;

            int damage = 0;
            if (!isMiss) {
                damage = (a.attack - d.defense);
                if (damage < 0) damage = 0;
                d.hp -= damage;
            }

            return { damage, false, isMiss, d.hp <= 0 };
        }

        /**
         * @brief Simple BFS implementation for movement range.
         */
        std::vector<GridCoord> calculateMoveRange(const GridCoord& start, int range) {
            std::vector<GridCoord> reachable;
            // Simple Manhattan distance for range finding
            for (int dx = -range; dx <= range; ++dx) {
                for (int dy = -(range - std::abs(dx)); dy <= (range - std::abs(dx)); ++dy) {
                    GridCoord c = {start.x + dx, start.y + dy};
                    if (isWalkable(c)) reachable.push_back(c);
                }
            }
            return reachable;
        }

        bool isWalkable(const GridCoord& c) const {
            if (m_grid.find(c.x) == m_grid.end() || m_grid.at(c.x).find(c.y) == m_grid.at(c.x).end()) return false;
            return m_grid.at(c.x).at(c.y) != GridCellType::Blocked;
        }

        void setCell(int x, int y, GridCellType type) { m_grid[x][y] = type; }

        /**
         * @brief Unit management and turn sequence.
         */
        void startTurn() {
            m_turnIndex++;
            for (auto& [id, unit] : m_units) {
                unit.hasMoved = false;
                unit.hasActed = false;
            }
        }

    private:
        std::map<int, std::map<int, GridCellType>> m_grid;
        std::map<std::string, UnitStats> m_units;
        int m_turnIndex = 0;
    };

} // namespace urpg::templates
