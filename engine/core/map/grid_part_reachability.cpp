#include "engine/core/map/grid_part_reachability.h"

#include <algorithm>
#include <queue>
#include <string>
#include <unordered_set>
#include <utility>

namespace urpg::map {

namespace {

GridPartDiagnostic makeReachabilityDiagnostic(GridPartSeverity severity, std::string code, std::string message,
                                              const PlacedPartInstance* instance = nullptr, std::string target = {}) {
    GridPartDiagnostic diagnostic;
    diagnostic.severity = severity;
    diagnostic.code = std::move(code);
    diagnostic.message = std::move(message);
    diagnostic.target = std::move(target);
    if (instance != nullptr) {
        diagnostic.instance_id = instance->instance_id;
        diagnostic.part_id = instance->part_id;
        diagnostic.x = instance->grid_x;
        diagnostic.y = instance->grid_y;
    }
    return diagnostic;
}

bool hasPropertyValue(const PlacedPartInstance& instance, const std::string& key, const std::string& value) {
    const auto found = instance.properties.find(key);
    return found != instance.properties.end() && found->second == value;
}

std::string propertyValue(const PlacedPartInstance& instance, const std::string& key) {
    const auto found = instance.properties.find(key);
    return found == instance.properties.end() ? std::string{} : found->second;
}

bool isSpawnPart(const PlacedPartInstance& instance) {
    return hasPropertyValue(instance, "role", "player_spawn") || hasPropertyValue(instance, "spawn", "player") ||
           instance.part_id.find("player_spawn") != std::string::npos ||
           instance.part_id.find("spawn.player") != std::string::npos;
}

bool isExitPart(const PlacedPartInstance& instance) {
    return hasPropertyValue(instance, "role", "exit") || hasPropertyValue(instance, "objective", "exit") ||
           instance.part_id.find(".exit") != std::string::npos || instance.part_id.find("exit.") != std::string::npos;
}

bool isLockedDoor(const PlacedPartInstance& instance) {
    return instance.category == GridPartCategory::Door && hasPropertyValue(instance, "locked", "true");
}

bool definitionBlocksNavigation(const GridPartDefinition& definition) {
    return definition.footprint.blocks_navigation || definition.collision_policy == GridPartCollisionPolicy::Solid;
}

bool cellCoveredByPart(const PlacedPartInstance& instance, int32_t x, int32_t y) {
    return x >= instance.grid_x && x < instance.grid_x + instance.width && y >= instance.grid_y &&
           y < instance.grid_y + instance.height;
}

bool cellBlocked(const GridPartDocument& document, const GridPartCatalog& catalog, int32_t x, int32_t y) {
    for (const auto& part : document.parts()) {
        if (!cellCoveredByPart(part, x, y) || isSpawnPart(part) || isExitPart(part)) {
            continue;
        }

        if (isLockedDoor(part)) {
            return true;
        }

        const auto* definition = catalog.find(part.part_id);
        if (definition != nullptr && definitionBlocksNavigation(*definition)) {
            return true;
        }
    }
    return false;
}

std::vector<std::pair<int32_t, int32_t>> runBfs(const GridPartDocument& document, const GridPartCatalog& catalog,
                                                int32_t startX, int32_t startY) {
    if (!document.inBounds(startX, startY)) {
        return {};
    }

    std::vector<uint8_t> visited(static_cast<size_t>(document.width() * document.height()), 0);
    std::queue<std::pair<int32_t, int32_t>> queue;
    const auto indexOf = [&](int32_t x, int32_t y) { return static_cast<size_t>(y * document.width() + x); };

    visited[indexOf(startX, startY)] = 1;
    queue.push({startX, startY});

    constexpr std::pair<int32_t, int32_t> kDirections[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    while (!queue.empty()) {
        const auto [x, y] = queue.front();
        queue.pop();

        for (const auto& [dx, dy] : kDirections) {
            const int32_t nextX = x + dx;
            const int32_t nextY = y + dy;
            if (!document.inBounds(nextX, nextY) || visited[indexOf(nextX, nextY)] != 0 ||
                cellBlocked(document, catalog, nextX, nextY)) {
                continue;
            }

            visited[indexOf(nextX, nextY)] = 1;
            queue.push({nextX, nextY});
        }
    }

    std::vector<std::pair<int32_t, int32_t>> cells;
    for (int32_t y = 0; y < document.height(); ++y) {
        for (int32_t x = 0; x < document.width(); ++x) {
            if (visited[indexOf(x, y)] != 0) {
                cells.push_back({x, y});
            }
        }
    }
    return cells;
}

bool containsCell(const std::vector<std::pair<int32_t, int32_t>>& cells, int32_t x, int32_t y) {
    return std::find(cells.begin(), cells.end(), std::pair<int32_t, int32_t>{x, y}) != cells.end();
}

bool partReachable(const PlacedPartInstance& instance, const std::vector<std::pair<int32_t, int32_t>>& cells) {
    for (int32_t y = instance.grid_y; y < instance.grid_y + instance.height; ++y) {
        for (int32_t x = instance.grid_x; x < instance.grid_x + instance.width; ++x) {
            if (containsCell(cells, x, y)) {
                return true;
            }
        }
    }
    return false;
}

const PlacedPartInstance* findSpawn(const GridPartDocument& document) {
    const auto& parts = document.parts();
    const auto found = std::find_if(parts.begin(), parts.end(), [](const auto& part) { return isSpawnPart(part); });
    return found == parts.end() ? nullptr : &(*found);
}

std::vector<const PlacedPartInstance*> findExits(const GridPartDocument& document) {
    std::vector<const PlacedPartInstance*> exits;
    for (const auto& part : document.parts()) {
        if (isExitPart(part)) {
            exits.push_back(&part);
        }
    }
    return exits;
}

void sortDiagnostics(std::vector<GridPartDiagnostic>& diagnostics) {
    std::sort(diagnostics.begin(), diagnostics.end(),
              [](const GridPartDiagnostic& left, const GridPartDiagnostic& right) {
                  if (left.severity != right.severity) {
                      return static_cast<uint8_t>(left.severity) > static_cast<uint8_t>(right.severity);
                  }
                  if (left.code != right.code) {
                      return left.code < right.code;
                  }
                  if (left.instance_id != right.instance_id) {
                      return left.instance_id < right.instance_id;
                  }
                  return left.target < right.target;
              });
}

} // namespace

ReachabilityReport ValidateReachability(const GridPartDocument& document, const GridPartCatalog& catalog,
                                        const GridRulesetProfile& ruleset, const MapObjective& objective) {
    ReachabilityReport report;

    const auto* spawn = findSpawn(document);
    if (ruleset.requires_player_spawn && spawn == nullptr) {
        report.diagnostics.push_back(makeReachabilityDiagnostic(GridPartSeverity::Error, "ruleset_requires_spawn",
                                                                "Reachability validation requires a player spawn."));
    }

    if (objective.type == MapObjectiveType::None || objective.objective_id.empty()) {
        report.diagnostics.push_back(makeReachabilityDiagnostic(GridPartSeverity::Error, "objective_missing",
                                                                "Reachability validation requires an objective."));
    }

    if (spawn != nullptr) {
        report.reachable_cells = runBfs(document, catalog, spawn->grid_x, spawn->grid_y);
    }

    const auto exits = findExits(document);
    if (ruleset.requires_exit && exits.empty()) {
        report.diagnostics.push_back(makeReachabilityDiagnostic(GridPartSeverity::Error, "ruleset_requires_exit",
                                                                "Reachability validation requires an exit."));
    }

    if (!objective.target_instance_id.empty()) {
        const auto* target = document.findPart(objective.target_instance_id);
        if (target != nullptr && !partReachable(*target, report.reachable_cells)) {
            report.diagnostics.push_back(makeReachabilityDiagnostic(GridPartSeverity::Error, "objective_unreachable",
                                                                    "Objective target is not reachable from spawn.",
                                                                    target, objective.target_instance_id));
        }
    }

    for (const auto* exit : exits) {
        if (!partReachable(*exit, report.reachable_cells)) {
            report.diagnostics.push_back(makeReachabilityDiagnostic(GridPartSeverity::Error, "exit_unreachable",
                                                                    "Exit is not reachable from spawn.", exit,
                                                                    exit->instance_id));
        }
    }

    for (const auto& part : document.parts()) {
        if (!isLockedDoor(part)) {
            continue;
        }

        const std::string keyInstanceId = propertyValue(part, "keyInstanceId");
        if (keyInstanceId.empty()) {
            continue;
        }
        if (keyInstanceId == part.instance_id) {
            report.diagnostics.push_back(makeReachabilityDiagnostic(GridPartSeverity::Error, "door_key_cycle",
                                                                    "Locked door depends on itself as a key.", &part,
                                                                    keyInstanceId));
            continue;
        }

        const auto* key = document.findPart(keyInstanceId);
        if (key == nullptr || !partReachable(*key, report.reachable_cells)) {
            report.diagnostics.push_back(makeReachabilityDiagnostic(
                GridPartSeverity::Error, "required_key_unreachable",
                "Required locked-door key is not reachable before the door.", &part, keyInstanceId));
        }
    }

    sortDiagnostics(report.diagnostics);
    report.ok = report.diagnostics.empty();
    return report;
}

} // namespace urpg::map
