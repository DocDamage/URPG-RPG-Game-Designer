#pragma once

#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

namespace urpg::metroidvania {

struct AbilityGateRegion {
    std::string id;
    std::string map_id;
    std::vector<std::string> ability_rewards;
};

struct AbilityGateLink {
    std::string id;
    std::string from_region;
    std::string to_region;
    std::vector<std::string> required_abilities;
    bool one_way = false;
};

struct AbilityGateDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct AbilityGatePreview {
    std::string start_region;
    std::set<std::string> reachable_regions;
    std::set<std::string> unlocked_abilities;
    std::vector<std::string> blocked_links;
    std::vector<AbilityGateDiagnostic> diagnostics;
};

class AbilityGateDocument {
public:
    std::string id;
    std::vector<AbilityGateRegion> regions;
    std::vector<AbilityGateLink> links;

    [[nodiscard]] std::vector<AbilityGateDiagnostic> validate(const std::set<std::string>& known_abilities = {}) const;
    [[nodiscard]] AbilityGatePreview preview(const std::string& start_region, const std::set<std::string>& initial_abilities) const;
    [[nodiscard]] bool canReach(const std::string& start_region,
                                const std::string& target_region,
                                const std::set<std::string>& abilities) const;
    [[nodiscard]] nlohmann::json toJson() const;

    static AbilityGateDocument fromJson(const nlohmann::json& json);
};

nlohmann::json abilityGatePreviewToJson(const AbilityGatePreview& preview);

} // namespace urpg::metroidvania
