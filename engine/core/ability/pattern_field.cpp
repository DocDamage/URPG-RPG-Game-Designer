#include "engine/core/ability/pattern_field.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>

namespace urpg {

PatternField PatternField::MakeCross(const std::string& name, int32_t radius) {
    PatternField pattern(name);
    const int32_t clamped_radius = std::max(0, radius);
    for (int32_t step = -clamped_radius; step <= clamped_radius; ++step) {
        pattern.addPoint(step, 0);
        pattern.addPoint(0, step);
    }
    return pattern;
}

PatternField PatternField::MakeLine(const std::string& name, int32_t length, bool horizontal) {
    PatternField pattern(name);
    const int32_t clamped_length = std::max(1, length);
    pattern.addPoint(0, 0);
    for (int32_t step = 1; step < clamped_length; ++step) {
        pattern.addPoint(horizontal ? step : 0, horizontal ? 0 : step);
        pattern.addPoint(horizontal ? -step : 0, horizontal ? 0 : -step);
    }
    return pattern;
}

void PatternField::addPoint(int32_t x, int32_t y) {
    if (!hasPoint(x, y)) {
        m_points.push_back({x, y});
        normalize();
    }
}

void PatternField::removePoint(int32_t x, int32_t y) {
    for (auto it = m_points.begin(); it != m_points.end(); ++it) {
        if (it->x == x && it->y == y) {
            m_points.erase(it);
            normalize();
            return;
        }
    }
}

bool PatternField::hasPoint(int32_t x, int32_t y) const {
    for (const auto& p : m_points) {
        if (p.x == x && p.y == y) return true;
    }
    return false;
}

void PatternField::setPoints(const std::vector<Point>& points) {
    m_points = points;
    normalize();
}

void PatternField::getBounds(int32_t& minX, int32_t& minY, int32_t& maxX, int32_t& maxY) const {
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

void PatternField::normalize() {
    std::sort(m_points.begin(), m_points.end(), [](const Point& lhs, const Point& rhs) {
        if (lhs.y != rhs.y) {
            return lhs.y < rhs.y;
        }
        return lhs.x < rhs.x;
    });

    m_points.erase(
        std::unique(m_points.begin(), m_points.end(), [](const Point& lhs, const Point& rhs) {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }),
        m_points.end());
}

PatternValidator::ValidationResult PatternValidator::Validate(const PatternField& pattern, int32_t maxRadius) {
    ValidationResult result;
    const auto& points = pattern.getPoints();

    if (points.empty()) {
        result.isValid = false;
        result.issues.push_back("Pattern contains no points.");
        return result;
    }

    if (!pattern.hasPoint(0, 0)) {
        result.isValid = false;
        result.issues.push_back("Pattern must include the origin point (0,0).");
    }

    int32_t minX, minY, maxX, maxY;
    pattern.getBounds(minX, minY, maxX, maxY);

    if (std::abs(minX) > maxRadius || std::abs(maxX) > maxRadius ||
        std::abs(minY) > maxRadius || std::abs(maxY) > maxRadius) {
        result.isValid = false;
        result.issues.push_back("Pattern exceeds maximum radius of " + std::to_string(maxRadius) + ".");
    }

    return result;
}

void to_json(nlohmann::json& j, const PatternField::Point& p) {
    j = nlohmann::json{{"x", p.x}, {"y", p.y}};
}

void from_json(const nlohmann::json& j, PatternField::Point& p) {
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
}

void to_json(nlohmann::json& j, const PatternField& p) {
    j = nlohmann::json{
        {"name", p.getName()},
        {"points", p.getPoints()}
    };
}

void from_json(const nlohmann::json& j, PatternField& p) {
    p.setName(j.at("name").get<std::string>());
    p.setPoints(j.at("points").get<std::vector<PatternField::Point>>());
}

} // namespace urpg
