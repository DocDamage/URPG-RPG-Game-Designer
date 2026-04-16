#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace urpg {

/**
 * @brief Represents a 2D grid pattern for AoE, range, or layout requirements.
 * Patterns are defined relative to a (0,0) center point.
 */
class PatternField {
public:
    struct Point {
        int32_t x;
        int32_t y;
    };

    PatternField() = default;
    PatternField(const std::string& name) : m_name(name) {}

    void addPoint(int32_t x, int32_t y) {
        if (!hasPoint(x, y)) {
            m_points.push_back({x, y});
        }
    }

    void removePoint(int32_t x, int32_t y) {
        for (auto it = m_points.begin(); it != m_points.end(); ++it) {
            if (it->x == x && it->y == y) {
                m_points.erase(it);
                return;
            }
        }
    }

    bool hasPoint(int32_t x, int32_t y) const {
        for (const auto& p : m_points) {
            if (p.x == x && p.y == y) return true;
        }
        return false;
    }

    const std::vector<Point>& getPoints() const { return m_points; }
    
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    void getBounds(int32_t& minX, int32_t& minY, int32_t& maxX, int32_t& maxY) const {
        if (m_points.empty()) {
            minX = minY = maxX = maxY = 0;
            return;
        }
        minX = maxX = m_points[0].x;
        minY = maxY = m_points[0].y;
        for (const auto& p : m_points) {
            if (p.x < minX) minX = p.x;
            if (p.y < minY) minY = p.y;
            if (p.x > maxX) maxX = p.x;
            if (p.y > maxY) maxY = p.y;
        }
    }

private:
    std::string m_name;
    std::vector<Point> m_points;
};

} // namespace urpg
