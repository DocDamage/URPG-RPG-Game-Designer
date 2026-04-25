#include "engine/core/map/map_region_rules.h"

namespace urpg::map {

namespace {

bool contains(const MapRegionRule& rule, int32_t x, int32_t y) {
    return x >= rule.x && y >= rule.y && x < rule.x + rule.width && y < rule.y + rule.height;
}

} // namespace

std::vector<MapDiagnostic> ValidateMapRegionRules(const std::vector<MapRegionRule>& rules) {
    std::vector<MapDiagnostic> diagnostics;
    for (size_t i = 0; i < rules.size(); ++i) {
        for (size_t j = i + 1; j < rules.size(); ++j) {
            const auto& a = rules[i];
            const auto& b = rules[j];
            if (a.movement_rule.empty() || b.movement_rule.empty() || a.movement_rule == b.movement_rule) {
                continue;
            }
            for (int32_t y = std::max(a.y, b.y); y < std::min(a.y + a.height, b.y + b.height); ++y) {
                for (int32_t x = std::max(a.x, b.x); x < std::min(a.x + a.width, b.x + b.width); ++x) {
                    if (contains(a, x, y) && contains(b, x, y)) {
                        diagnostics.push_back({
                            "region_movement_conflict",
                            "Overlapping regions define conflicting movement rules.",
                            x,
                            y,
                            a.id + ":" + b.id,
                        });
                        return diagnostics;
                    }
                }
            }
        }
    }
    return diagnostics;
}

} // namespace urpg::map
