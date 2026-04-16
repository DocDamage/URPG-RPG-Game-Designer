#pragma once

#include "engine/core/ability/pattern_field.h"
#include <nlohmann/json.hpp>
#include <string>

namespace urpg {

/**
 * @brief Serializer for PatternField objects.
 * Supports JSON-based persistence for easy editing and migration.
 */
class PatternFieldSerializer {
public:
    static nlohmann::json toJson(const PatternField& pattern) {
        nlohmann::json j;
        j["name"] = pattern.getName();
        j["points"] = nlohmann::json::array();
        for (const auto& p : pattern.getPoints()) {
            j["points"].push_back({{"x", p.x}, {"y", p.y}});
        }
        return j;
    }

    static PatternField fromJson(const nlohmann::json& j) {
        PatternField pattern(j.value("name", "Unnamed Pattern"));
        if (j.contains("points") && j["points"].is_array()) {
            for (const auto& pj : j["points"]) {
                pattern.addPoint(pj.value("x", 0), pj.value("y", 0));
            }
        }
        return pattern;
    }
};

} // namespace urpg
