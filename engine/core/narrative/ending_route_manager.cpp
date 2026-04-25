#include "engine/core/narrative/ending_route_manager.h"

#include <algorithm>
#include <utility>

namespace urpg::narrative {

void EndingRouteManager::addRoute(EndingRoute route) {
    routes_.push_back(std::move(route));
}

std::optional<EndingRoute> EndingRouteManager::evaluate(const std::map<std::string, int>& state) const {
    std::vector<EndingRoute> eligible;
    for (const auto& route : routes_) {
        const bool matches = std::all_of(route.conditions.begin(), route.conditions.end(), [&](const auto& condition) {
            const auto it = state.find(condition.key);
            return it != state.end() && it->second >= condition.minimum;
        });
        if (matches) {
            eligible.push_back(route);
        }
    }
    if (eligible.empty()) {
        return std::nullopt;
    }
    std::sort(eligible.begin(), eligible.end(), [](const auto& a, const auto& b) {
        if (a.priority != b.priority) {
            return a.priority > b.priority;
        }
        return a.id < b.id;
    });
    return eligible.front();
}

} // namespace urpg::narrative
