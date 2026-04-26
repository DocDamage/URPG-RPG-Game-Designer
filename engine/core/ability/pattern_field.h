#pragma once

#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

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

    static PatternField MakeCross(const std::string& name, int32_t radius);
    static PatternField MakeLine(const std::string& name, int32_t length, bool horizontal);

    void addPoint(int32_t x, int32_t y);
    void removePoint(int32_t x, int32_t y);
    bool hasPoint(int32_t x, int32_t y) const;

    const std::vector<Point>& getPoints() const { return m_points; }
    void setPoints(const std::vector<Point>& points);

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    void getBounds(int32_t& minX, int32_t& minY, int32_t& maxX, int32_t& maxY) const;

  private:
    void normalize();

    std::string m_name;
    std::vector<Point> m_points;
};

/**
 * @brief Utility for validating PatternField resources.
 * Part of Wave 2 Pattern Field Editor completion.
 */
class PatternValidator {
  public:
    struct ValidationResult {
        bool isValid = true;
        std::vector<std::string> issues;
    };

    /**
     * @brief Check for common issues (empty, too large, out of bounds).
     */
    static ValidationResult Validate(const PatternField& pattern, int32_t maxRadius = 10);
};

// JSON conversion declarations
void to_json(nlohmann::json& j, const PatternField::Point& p);
void from_json(const nlohmann::json& j, PatternField::Point& p);
void to_json(nlohmann::json& j, const PatternField& p);
void from_json(const nlohmann::json& j, PatternField& p);

} // namespace urpg
