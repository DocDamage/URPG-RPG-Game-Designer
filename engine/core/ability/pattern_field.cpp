#include "engine/core/ability/pattern_field.h"
#include <nlohmann/json.hpp>

namespace urpg {

void PatternField::addPoint(int32_t x, int32_t y) {
    if (!hasPoint(x, y)) {
        m_points.push_back({x, y});
    }
}

void PatternField::removePoint(int32_t x, int32_t y) {
    for (auto it = m_points.begin(); it != m_points.end(); ++it) {
        if (it->x == x && it->y == y) {
            m_points.erase(it);
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

PatternValidator::ValidationResult PatternValidator::Validate(const PatternField& pattern, int32_t maxRadius) {
    ValidationResult result;
    const auto& points = pattern.getPoints();

    if (points.empty()) {
        result.isValid = false;
        result.issues.push_back("Pattern contains no points.");
        return result;
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
    auto points = j.at("points").get<std::vector<PatternField::Point>>();
    for (const auto& pt : points) {
        p.addPoint(pt.x, pt.y);
    }
}

} // namespace urpg
