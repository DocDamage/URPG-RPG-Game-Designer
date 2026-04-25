#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace urpg::narrative {

struct EndingCondition {
    std::string key;
    int minimum = 0;
};

struct EndingRoute {
    std::string id;
    int priority = 0;
    std::vector<EndingCondition> conditions;
};

class EndingRouteManager {
public:
    void addRoute(EndingRoute route);
    std::optional<EndingRoute> evaluate(const std::map<std::string, int>& state) const;

private:
    std::vector<EndingRoute> routes_;
};

} // namespace urpg::narrative
