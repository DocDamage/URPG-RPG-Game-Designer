#include "engine/core/database/database_vertical_slice_integration.h"

#include "engine/core/gameplay/inventory_components.h"
#include "engine/core/save/save_migration.h"

#include <algorithm>
#include <set>

namespace urpg::database {

namespace {

void fail(DatabaseVerticalSliceReport& report, std::string diagnostic) {
    report.diagnostics.push_back(std::move(diagnostic));
}

std::map<std::string, int> inventoryMap(const InventoryComponent& inventory) {
    std::map<std::string, int> result;
    for (const auto& slot : inventory.slots) result[slot.itemId] += slot.count;
    return result;
}

bool inventoryReferencesKnownItems(const nlohmann::json& inventory, const RpgDatabase& database) {
    if (!inventory.is_object()) return false;
    for (const auto& [id, count] : inventory.items()) {
        if (!count.is_number_integer() || count.get<int>() < 0 || !database.items().contains(id)) return false;
    }
    return true;
}

} // namespace

DatabaseVerticalSliceReport DatabaseVerticalSliceIntegration::run(
    const DatabaseVerticalSliceManifest& manifest, DatabaseVerticalSliceDependencies dependencies) {
    DatabaseVerticalSliceReport report;
    const auto checkpoint = [&](std::string id, bool passed) {
        report.checkpoints[std::move(id)] = passed;
        return passed;
    };
    if (manifest.id.empty()) {
        fail(report, "manifest_id_missing");
        return report;
    }
    const auto actor = dependencies.database.actors().find(manifest.actor_id);
    checkpoint("actor", actor != dependencies.database.actors().end());
    if (actor == dependencies.database.actors().end()) fail(report, "actor_missing:" + manifest.actor_id);

    const auto equipment = dependencies.database.items().find(manifest.equipment_item_id);
    const auto consumable = dependencies.database.items().find(manifest.consumable_item_id);
    const auto ingredient = dependencies.database.items().find(manifest.ingredient_item_id);
    checkpoint("inventory_records", consumable != dependencies.database.items().end() &&
                                    ingredient != dependencies.database.items().end());
    checkpoint("equipment_record", equipment != dependencies.database.items().end() &&
                                    equipment->second.tags.contains("equipment"));
    if (!report.checkpoints["inventory_records"]) fail(report, "inventory_item_missing");
    if (!report.checkpoints["equipment_record"]) fail(report, "equipment_item_invalid");

    const auto encounterDiagnostics = dependencies.encounters.validate({manifest.enemy_id});
    const auto encounterPreview = dependencies.encounters.preview(manifest.region_id, 1, {}, manifest.seed, 1);
    const bool battlePassed = encounterDiagnostics.empty() && encounterPreview.diagnostics.empty() &&
        encounterPreview.encounters.size() == 1 &&
        encounterPreview.encounters.front().encounter_id == manifest.encounter_id &&
        encounterPreview.encounters.front().enemy_id == manifest.enemy_id &&
        std::find(encounterPreview.encounters.front().rewards.begin(),
                  encounterPreview.encounters.front().rewards.end(), manifest.consumable_item_id) !=
            encounterPreview.encounters.front().rewards.end();
    checkpoint("battle", battlePassed);
    if (!battlePassed) fail(report, "battle_integration_failed");

    InventoryComponent inventory;
    const bool inventoryPassed = inventory.addItem(manifest.ingredient_item_id, 2) &&
                                 inventory.addItem(manifest.equipment_item_id, 1) &&
                                 inventory.addItem(manifest.consumable_item_id, 1);
    checkpoint("inventory", inventoryPassed);
    checkpoint("equipment", inventoryPassed && equipment != dependencies.database.items().end());

    const auto craftingPreview = dependencies.crafting.preview(manifest.recipe_id, inventoryMap(inventory), {});
    const bool craftingPassed = craftingPreview.canCraft &&
                                craftingPreview.results.contains(manifest.consumable_item_id);
    checkpoint("crafting", craftingPassed);
    if (!craftingPassed) fail(report, "crafting_integration_failed");

    const auto* vendor = dependencies.vendors.findVendor(manifest.vendor_id);
    const auto stock = dependencies.vendors.refreshStock(manifest.vendor_id, {});
    const bool vendorPassed = vendor != nullptr && dependencies.vendors.validate().empty() &&
        std::any_of(stock.begin(), stock.end(), [&](const auto& row) {
            return row.item_id == manifest.consumable_item_id;
        });
    checkpoint("vendor", vendorPassed);
    if (!vendorPassed) fail(report, "vendor_integration_failed");

    quest::QuestWorldState world;
    world.items = {manifest.consumable_item_id};
    world.battles = {manifest.encounter_id};
    const bool questPassed = dependencies.quests.evaluateObjective(manifest.quest_id, manifest.objective_id,
                                                                   world, "vertical-slice");
    checkpoint("quest", questPassed);
    if (!questPassed) fail(report, "quest_integration_failed");

    bool localizationPassed = !manifest.localization_keys.empty();
    for (const auto& key : manifest.localization_keys) localizationPassed &= dependencies.locale.hasKey(key);
    checkpoint("localization", localizationPassed);
    if (!localizationPassed) fail(report, "localization_integration_failed");

    const auto inventoryState = inventoryMap(inventory);
    const nlohmann::json databaseState = {
        {"actor_id", manifest.actor_id}, {"inventory", inventoryState},
        {"equipped_item_id", manifest.equipment_item_id},
        {"completed_battles", nlohmann::json::array({manifest.encounter_id})},
        {"quest_state", dependencies.quests.serialize()},
    };
    report.fresh_save = {{"_urpg_format_version", "1.0"}, {"database_state", databaseState}};
    const auto restoredQuests = quest::QuestRegistry::deserialize(databaseState["quest_state"]);
    const auto* restoredQuest = restoredQuests.findQuest(manifest.quest_id);
    report.fresh_save_passed = checkpoint("fresh_save",
        databaseState["actor_id"] == manifest.actor_id &&
        inventoryReferencesKnownItems(databaseState["inventory"], dependencies.database) &&
        databaseState["equipped_item_id"] == manifest.equipment_item_id && restoredQuest != nullptr &&
        !restoredQuest->objectives.empty() && restoredQuest->objectives.front().state == quest::ObjectiveState::Completed);

    const auto migrated = save::ImportCompatSaveDocument({{"_urpg_format_version", "mz_compat_1"},
                                                           {"databaseState", databaseState}});
    report.migrated_save = migrated.native_payload;
    report.migrated_save_passed = checkpoint("migrated_save",
        migrated.native_payload.contains("database_state") &&
        migrated.native_payload["database_state"] == databaseState && !migrated.used_safe_fallback &&
        actor != dependencies.database.actors().end() &&
        inventoryReferencesKnownItems(databaseState["inventory"], dependencies.database) &&
        equipment != dependencies.database.items().end());
    if (!report.migrated_save_passed) fail(report, "migrated_save_integration_failed");

    report.package_closure = {
        {"schema", "urpg.database_vertical_slice_closure.v1"}, {"manifest_id", manifest.id},
        {"actor_ids", nlohmann::json::array({manifest.actor_id})},
        {"item_ids", nlohmann::json::array({manifest.consumable_item_id, manifest.equipment_item_id,
                                             manifest.ingredient_item_id})},
        {"enemy_ids", nlohmann::json::array({manifest.enemy_id})},
        {"encounter_ids", nlohmann::json::array({manifest.encounter_id})},
        {"quest_ids", nlohmann::json::array({manifest.quest_id})},
        {"vendor_ids", nlohmann::json::array({manifest.vendor_id})},
        {"recipe_ids", nlohmann::json::array({manifest.recipe_id})},
        {"localization_keys", manifest.localization_keys},
    };
    checkpoint("package_closure", report.package_closure["item_ids"].size() == 3 && localizationPassed);
    report.success = report.diagnostics.empty() &&
        std::all_of(report.checkpoints.begin(), report.checkpoints.end(), [](const auto& value) { return value.second; });
    return report;
}

} // namespace urpg::database
