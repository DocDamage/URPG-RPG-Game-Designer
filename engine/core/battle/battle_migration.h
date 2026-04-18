#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::battle {

/**
 * @brief Logic for migrating RPG Maker MV/MZ battle data (Troops, Skills, Items).
 */
class BattleMigration {
public:
    struct Progress {
        size_t total_enemies = 0;
        size_t total_troops = 0;
        size_t total_actions = 0;
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
    };

    /**
     * @brief Migrates an RPG Maker Troop to URPG Native format.
     */
    static nlohmann::json migrateTroop(const nlohmann::json& rm_troop, Progress& progress) {
        nlohmann::json native;
        native["id"] = "TRP_" + std::to_string(rm_troop.value("id", 0));
        native["name"] = rm_troop.value("name", "Unknown Troop");
        
        nlohmann::json members = nlohmann::json::array();
        for (const auto& mem : rm_troop.value("members", nlohmann::json::array())) {
            nlohmann::json native_mem;
            native_mem["enemy_id"] = "ENM_" + std::to_string(mem.value("enemyId", 0));
            native_mem["x"] = mem.value("x", 0);
            native_mem["y"] = mem.value("y", 0);
            native_mem["hidden"] = mem.value("hidden", false);
            members.push_back(native_mem);
        }
        native["members"] = members;

        // Note: phases/pages mapping is complex due to event commands, 
        // we map the structure but not the full command script here.
        native["phases"] = nlohmann::json::array(); 
        
        progress.total_troops++;
        return native;
    }

    /**
     * @brief Migrates an RPG Maker Skill or Item to URPG Native battle action.
     */
    static nlohmann::json migrateAction(const nlohmann::json& rm_action, bool is_item, Progress& progress) {
        nlohmann::json native;
        std::string prefix = is_item ? "ITM_" : "SKL_";
        native["id"] = prefix + std::to_string(rm_action.value("id", 0));
        native["name"] = rm_action.value("name", "");
        native["description"] = rm_action.value("description", "");

        // Map Scope (simplified)
        int scope = rm_action.value("scope", 1);
        switch (scope) {
            case 1: native["scope"] = "single_enemy"; break;
            case 2: native["scope"] = "all_enemies"; break;
            case 7: native["scope"] = "single_ally"; break;
            case 8: native["scope"] = "all_allies"; break;
            case 11: native["scope"] = "user"; break;
            default: native["scope"] = "none"; break;
        }

        // Map Cost
        nlohmann::json cost;
        cost["hp"] = 0;
        cost["mp"] = rm_action.value("mpCost", 0);
        cost["tp"] = rm_action.value("tpCost", 0);
        native["cost"] = cost;

        // Map Effects
        nlohmann::json effects = nlohmann::json::array();
        // damage mapping
        auto damage = rm_action.value("damage", nlohmann::json::object());
        if (damage.contains("formula") && !damage["formula"].get<std::string>().empty()) {
            nlohmann::json dmg_effect;
            dmg_effect["type"] = "damage";
            dmg_effect["formula"] = damage["formula"];
            dmg_effect["variance"] = damage.value("variance", 20);
            dmg_effect["critical"] = damage.value("critical", false);
            effects.push_back(dmg_effect);
        }
        native["effects"] = effects;
        native["animation_id"] = "ANI_" + std::to_string(rm_action.value("animationId", 0));

        progress.total_actions++;
        return native;
    }
};

} // namespace urpg::battle
