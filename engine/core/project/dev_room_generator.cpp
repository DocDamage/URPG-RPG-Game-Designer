#include "engine/core/project/dev_room_generator.h"

#include <array>
#include <set>

namespace urpg::project {

namespace {

const std::array<std::string, 9> kStations = {
    "message",
    "menu",
    "battle",
    "save_load",
    "plugin_report",
    "audio",
    "input",
    "asset_warning",
    "export_preflight",
};

nlohmann::json buildStations() {
    nlohmann::json stations = nlohmann::json::array();
    for (std::size_t i = 0; i < kStations.size(); ++i) {
        stations.push_back({
            {"id", kStations[i]},
            {"display_name", kStations[i]},
            {"position", {{"x", static_cast<int>(i * 2)}, {"y", 0}}},
            {"required", true},
        });
    }
    return stations;
}

} // namespace

DevRoomResult DevRoomGenerator::generate(const std::string& project_id) const {
    nlohmann::json route = nlohmann::json::array();
    for (const auto& station : kStations) {
        route.push_back(station);
    }

    DevRoomResult result;
    result.room = {
        {"schema_version", "urpg.dev_room.v1"},
        {"project_id", project_id},
        {"map_id", "dev_room_001"},
        {"stations", buildStations()},
        {"scripted_route", route},
    };
    result.audit_report = {
        {"project_id", project_id},
        {"status", "passed"},
        {"station_count", kStations.size()},
        {"failures", nlohmann::json::array()},
    };
    return result;
}

std::vector<std::string> DevRoomGenerator::validateScriptedRoute(const nlohmann::json& room) const {
    std::vector<std::string> errors;
    std::set<std::string> visited;
    if (room.contains("scripted_route") && room["scripted_route"].is_array()) {
        for (const auto& step : room["scripted_route"]) {
            if (step.is_string()) {
                visited.insert(step.get<std::string>());
            }
        }
    }
    for (const auto& station : kStations) {
        if (!visited.contains(station)) {
            errors.push_back("route_missing_station:" + station);
        }
    }
    return errors;
}

} // namespace urpg::project
