#include "engine/core/metroidvania/ability_gate_system.h"

#include <algorithm>
#include <deque>
#include <map>

namespace urpg::metroidvania {

namespace {

AbilityGateRegion regionFromJson(const nlohmann::json& json) {
    AbilityGateRegion region;
    region.id = json.value("id", "");
    region.map_id = json.value("map_id", "");
    region.ability_rewards = json.value("ability_rewards", std::vector<std::string>{});
    return region;
}

nlohmann::json regionToJson(const AbilityGateRegion& region) {
    return {{"id", region.id}, {"map_id", region.map_id}, {"ability_rewards", region.ability_rewards}};
}

AbilityGateLink linkFromJson(const nlohmann::json& json) {
    AbilityGateLink link;
    link.id = json.value("id", "");
    link.from_region = json.value("from_region", "");
    link.to_region = json.value("to_region", "");
    link.required_abilities = json.value("required_abilities", std::vector<std::string>{});
    link.one_way = json.value("one_way", false);
    return link;
}

nlohmann::json linkToJson(const AbilityGateLink& link) {
    return {{"id", link.id},
            {"from_region", link.from_region},
            {"to_region", link.to_region},
            {"required_abilities", link.required_abilities},
            {"one_way", link.one_way}};
}

bool hasAllAbilities(const std::vector<std::string>& required, const std::set<std::string>& abilities) {
    return std::all_of(required.begin(), required.end(), [&](const auto& ability) { return abilities.contains(ability); });
}

} // namespace

std::vector<AbilityGateDiagnostic> AbilityGateDocument::validate(const std::set<std::string>& known_abilities) const {
    std::vector<AbilityGateDiagnostic> diagnostics;
    std::set<std::string> region_ids;
    for (const auto& region : regions) {
        if (region.id.empty()) {
            diagnostics.push_back({"missing_region_id", "Ability gate region is missing an id.", region.id});
            continue;
        }
        if (!region_ids.insert(region.id).second) {
            diagnostics.push_back({"duplicate_region_id", "Ability gate region id is duplicated.", region.id});
        }
        if (region.map_id.empty()) {
            diagnostics.push_back({"missing_region_map", "Ability gate region is missing a map id.", region.id});
        }
        for (const auto& ability : region.ability_rewards) {
            if (!known_abilities.empty() && !known_abilities.contains(ability)) {
                diagnostics.push_back({"unknown_reward_ability", "Ability gate region rewards an unknown ability: " + ability, region.id});
            }
        }
    }

    std::set<std::string> link_ids;
    for (const auto& link : links) {
        if (link.id.empty()) {
            diagnostics.push_back({"missing_link_id", "Ability gate link is missing an id.", link.id});
        } else if (!link_ids.insert(link.id).second) {
            diagnostics.push_back({"duplicate_link_id", "Ability gate link id is duplicated.", link.id});
        }
        if (!region_ids.contains(link.from_region)) {
            diagnostics.push_back({"unknown_from_region", "Ability gate link has an unknown source region.", link.id});
        }
        if (!region_ids.contains(link.to_region)) {
            diagnostics.push_back({"unknown_to_region", "Ability gate link has an unknown target region.", link.id});
        }
        for (const auto& ability : link.required_abilities) {
            if (!known_abilities.empty() && !known_abilities.contains(ability)) {
                diagnostics.push_back({"unknown_required_ability", "Ability gate link requires an unknown ability: " + ability, link.id});
            }
        }
    }
    return diagnostics;
}

AbilityGatePreview AbilityGateDocument::preview(const std::string& start_region, const std::set<std::string>& initial_abilities) const {
    AbilityGatePreview preview;
    preview.start_region = start_region;
    preview.unlocked_abilities = initial_abilities;
    preview.diagnostics = validate();

    const auto has_start = std::any_of(regions.begin(), regions.end(), [&](const auto& region) { return region.id == start_region; });
    if (!has_start) {
        preview.diagnostics.push_back({"missing_start_region", "Ability gate preview start region does not exist.", start_region});
        return preview;
    }

    std::map<std::string, AbilityGateRegion> region_by_id;
    for (const auto& region : regions) {
        region_by_id[region.id] = region;
    }

    std::deque<std::string> queue;
    queue.push_back(start_region);
    preview.reachable_regions.insert(start_region);
    bool progressed = true;
    while (progressed) {
        progressed = false;
        queue.clear();
        for (const auto& region_id : preview.reachable_regions) {
            queue.push_back(region_id);
            const auto region_it = region_by_id.find(region_id);
            if (region_it != region_by_id.end()) {
                for (const auto& ability : region_it->second.ability_rewards) {
                    if (preview.unlocked_abilities.insert(ability).second) {
                        progressed = true;
                    }
                }
            }
        }

        while (!queue.empty()) {
            const auto current = queue.front();
            queue.pop_front();
            for (const auto& link : links) {
                std::vector<std::string> destinations;
                if (link.from_region == current) {
                    destinations.push_back(link.to_region);
                }
                if (!link.one_way && link.to_region == current) {
                    destinations.push_back(link.from_region);
                }
                if (destinations.empty()) {
                    continue;
                }
                if (!hasAllAbilities(link.required_abilities, preview.unlocked_abilities)) {
                    if (std::find(preview.blocked_links.begin(), preview.blocked_links.end(), link.id) == preview.blocked_links.end()) {
                        preview.blocked_links.push_back(link.id);
                    }
                    continue;
                }
                for (const auto& destination : destinations) {
                    if (preview.reachable_regions.insert(destination).second) {
                        queue.push_back(destination);
                        progressed = true;
                    }
                }
            }
        }
    }
    return preview;
}

bool AbilityGateDocument::canReach(const std::string& start_region,
                                   const std::string& target_region,
                                   const std::set<std::string>& abilities) const {
    return preview(start_region, abilities).reachable_regions.contains(target_region);
}

nlohmann::json AbilityGateDocument::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["regions"] = nlohmann::json::array();
    for (const auto& region : regions) {
        json["regions"].push_back(regionToJson(region));
    }
    json["links"] = nlohmann::json::array();
    for (const auto& link : links) {
        json["links"].push_back(linkToJson(link));
    }
    return json;
}

AbilityGateDocument AbilityGateDocument::fromJson(const nlohmann::json& json) {
    AbilityGateDocument document;
    document.id = json.value("id", "");
    for (const auto& region_json : json.value("regions", nlohmann::json::array())) {
        document.regions.push_back(regionFromJson(region_json));
    }
    for (const auto& link_json : json.value("links", nlohmann::json::array())) {
        document.links.push_back(linkFromJson(link_json));
    }
    return document;
}

nlohmann::json abilityGatePreviewToJson(const AbilityGatePreview& preview) {
    nlohmann::json json{{"start_region", preview.start_region},
                        {"reachable_regions", preview.reachable_regions},
                        {"unlocked_abilities", preview.unlocked_abilities},
                        {"blocked_links", preview.blocked_links}};
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return json;
}

} // namespace urpg::metroidvania
