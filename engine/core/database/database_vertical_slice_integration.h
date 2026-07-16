#pragma once

#include "engine/core/balance/encounter_table.h"
#include "engine/core/crafting/crafting_registry.h"
#include "engine/core/database/rpg_database.h"
#include "engine/core/localization/locale_catalog.h"
#include "engine/core/quest/quest_registry.h"
#include "engine/core/shop/vendor_catalog.h"

#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <vector>

namespace urpg::database {

struct DatabaseVerticalSliceManifest {
    std::string id;
    std::string actor_id;
    std::string equipment_item_id;
    std::string consumable_item_id;
    std::string ingredient_item_id;
    std::string enemy_id;
    std::string region_id;
    std::string encounter_id;
    std::string quest_id;
    std::string objective_id;
    std::string vendor_id;
    std::string recipe_id;
    std::vector<std::string> localization_keys;
    uint64_t seed = 0;
};

struct DatabaseVerticalSliceDependencies {
    RpgDatabase database;
    balance::EncounterDesignerDocument encounters;
    shop::VendorCatalog vendors;
    crafting::CraftingRegistry crafting;
    quest::QuestRegistry quests;
    localization::LocaleCatalog locale;
};

struct DatabaseVerticalSliceReport {
    bool success = false;
    bool fresh_save_passed = false;
    bool migrated_save_passed = false;
    std::map<std::string, bool> checkpoints;
    std::vector<std::string> diagnostics;
    nlohmann::json fresh_save;
    nlohmann::json migrated_save;
    nlohmann::json package_closure;
};

class DatabaseVerticalSliceIntegration {
public:
    static DatabaseVerticalSliceReport run(const DatabaseVerticalSliceManifest& manifest,
                                           DatabaseVerticalSliceDependencies dependencies);
};

} // namespace urpg::database
