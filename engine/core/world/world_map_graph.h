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

struct FastTravelDestination {
    std::string id;
    std::string node_id;
    int map_id{0};
    int x{0};
    int y{0};
    std::string preview_asset;
    std::string description;
    std::string unlock_flag;
    std::string locked_common_event;
    bool hidden_when_current{true};
};

struct FastTravelPreview {
    std::string destination_id;
    bool available{false};
    bool hidden_current_location{false};
    std::string command;
    std::string preview_asset;
    std::string locked_common_event;
    std::vector<WorldDiagnostic> diagnostics;
};

struct VehicleProfile {
    std::string id;
    std::string name;
    std::vector<int> allowed_terrain_tags;
    std::string bgm_asset;
    std::string bgs_asset;
    double speed_multiplier{1.0};
    double encounter_rate_multiplier{1.0};
    int interior_map_id{0};
    int interior_x{0};
    int interior_y{0};
    std::string transition_color;
    int transition_frames{0};
};

struct VehiclePreview {
    std::string vehicle_id;
    bool available{false};
    std::string command;
    std::string audio_command;
    std::string interior_transfer_command;
    std::vector<WorldDiagnostic> diagnostics;
};

class WorldMapGraph {
public:
    void addNode(WorldNode node);
    void addRoute(WorldRoute route);
    void addFastTravelDestination(FastTravelDestination destination);
    void addVehicleProfile(VehicleProfile profile);
    [[nodiscard]] bool isRouteAvailable(const std::string& from,
                                         const std::string& to,
                                         const std::set<std::string>& flags,
                                         const std::set<std::string>& vehicles) const;
    [[nodiscard]] FastTravelPreview previewFastTravel(const std::string& current_node_id,
                                                      const std::string& destination_id,
                                                      const std::set<std::string>& flags) const;
    [[nodiscard]] VehiclePreview previewVehicle(const std::string& vehicle_id, int terrain_tag) const;
    [[nodiscard]] std::vector<WorldDiagnostic> validate() const;

private:
    std::vector<WorldNode> nodes_;
    std::vector<WorldRoute> routes_;
    std::vector<FastTravelDestination> fast_travel_destinations_;
    std::vector<VehicleProfile> vehicle_profiles_;
};

} // namespace urpg::world
