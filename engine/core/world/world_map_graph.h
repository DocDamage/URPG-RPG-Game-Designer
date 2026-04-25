#pragma once

#include <set>
#include <string>
#include <vector>

namespace urpg::world {

struct WorldDiagnostic {
    std::string code;
    std::string message;
};

struct WorldNode {
    std::string id;
    std::string name;
};

struct WorldRoute {
    std::string from;
    std::string to;
    std::string required_flag;
    std::string required_vehicle;
    bool fast_travel{false};
};

class WorldMapGraph {
public:
    void addNode(WorldNode node);
    void addRoute(WorldRoute route);
    [[nodiscard]] bool isRouteAvailable(const std::string& from,
                                         const std::string& to,
                                         const std::set<std::string>& flags,
                                         const std::set<std::string>& vehicles) const;
    [[nodiscard]] std::vector<WorldDiagnostic> validate() const;

private:
    std::vector<WorldNode> nodes_;
    std::vector<WorldRoute> routes_;
};

} // namespace urpg::world
