#include "engine/core/database/database_vertical_slice_integration.h"
#include "engine/core/save/save_migration.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("Database vertical slice integrates fresh and migrated runtime package closure",
          "[database][integration][vertical_slice][pcq455]") {
    urpg::database::DatabaseVerticalSliceDependencies dependencies;
    dependencies.database.upsertActor({"actor.ari", "Ari", "class.warden", 120, 24});
    dependencies.database.upsertItem({"item.herb", "Moon Herb", 5, {"material"}});
    dependencies.database.upsertItem({"item.potion", "Moon Potion", 40, {"consumable"}});
    dependencies.database.upsertItem({"item.lantern_blade", "Lantern Blade", 120, {"equipment", "weapon"}});

    dependencies.encounters.id = "encounters.willow";
    dependencies.encounters.regions.push_back({"willow.grove", "map.grove", "forest", 1, 10, {}});
    dependencies.encounters.encounters.push_back(
        {"battle.wisp", "enemy.wisp", "willow.grove", 100, 2, 1, 10, {}, {"item.potion"}});

    dependencies.vendors.setKnownItems({"item.herb", "item.potion", "item.lantern_blade"});
    dependencies.vendors.addVendor({"vendor.rowan", {{"item.potion", 3, 40, 20, {}}}});
    dependencies.crafting.addRecipe({"recipe.potion", {{"item.herb", 2}}, {{"item.potion", 1}}, ""});
    REQUIRE(dependencies.quests.registerQuest(
        {"quest.wisp", {{"objective.defeat", urpg::quest::ObjectiveState::Active,
                          {{"battle", "battle.wisp", 0}, {"item", "item.potion", 0}}, ""}}}));
    dependencies.locale.loadFromJson({{"locale", "en-US"},
                                      {"keys", {{"actor.ari.name", "Ari"},
                                                {"item.potion.name", "Moon Potion"},
                                                {"quest.wisp.title", "The Willow Wisp"}}}});

    const auto report = urpg::database::DatabaseVerticalSliceIntegration::run(
        {"lantern.willow", "actor.ari", "item.lantern_blade", "item.potion", "item.herb",
         "enemy.wisp", "willow.grove", "battle.wisp", "quest.wisp", "objective.defeat",
         "vendor.rowan", "recipe.potion", {"actor.ari.name", "item.potion.name", "quest.wisp.title"}, 7},
        std::move(dependencies));

    REQUIRE(report.success);
    REQUIRE(report.diagnostics.empty());
    REQUIRE(report.checkpoints.size() == 13);
    for (const auto& [_, passed] : report.checkpoints) REQUIRE(passed);
    REQUIRE(report.fresh_save_passed);
    REQUIRE(report.fresh_save["database_state"]["inventory"]["item.potion"] == 1);
    REQUIRE(report.fresh_save["database_state"]["equipped_item_id"] == "item.lantern_blade");
    REQUIRE(report.migrated_save_passed);
    REQUIRE(report.migrated_save["database_state"] == report.fresh_save["database_state"]);
    REQUIRE(report.package_closure["actor_ids"] == nlohmann::json::array({"actor.ari"}));
    REQUIRE(report.package_closure["item_ids"].size() == 3);
    REQUIRE(report.package_closure["localization_keys"].size() == 3);
}

TEST_CASE("Database vertical slice reports broken cross-domain references before packaging",
          "[database][integration][vertical_slice][pcq455]") {
    urpg::database::DatabaseVerticalSliceDependencies dependencies;
    dependencies.database.upsertActor({"actor.ari", "Ari", "class.warden", 120, 24});
    dependencies.database.upsertItem({"item.potion", "Moon Potion", 40, {"consumable"}});
    const auto report = urpg::database::DatabaseVerticalSliceIntegration::run(
        {"broken", "actor.ari", "item.missing_equipment", "item.potion", "item.missing_ingredient",
         "enemy.missing", "region.missing", "battle.missing", "quest.missing", "objective.missing",
         "vendor.missing", "recipe.missing", {"missing.key"}, 1}, std::move(dependencies));
    REQUIRE_FALSE(report.success);
    REQUIRE_FALSE(report.diagnostics.empty());
    REQUIRE_FALSE(report.checkpoints.at("equipment_record"));
    REQUIRE_FALSE(report.checkpoints.at("battle"));
    REQUIRE_FALSE(report.checkpoints.at("vendor"));
    REQUIRE_FALSE(report.checkpoints.at("quest"));
    REQUIRE_FALSE(report.checkpoints.at("localization"));
    REQUIRE_FALSE(report.migrated_save_passed);
}

TEST_CASE("Save migration rejects malformed database-bound state losslessly",
          "[save][migration][database_state][pcq455]") {
    const nlohmann::json malformed = {{"databaseState", {{"actor_id", "actor.ari"},
                                                          {"inventory", {{"item.potion", -1}}}}}};
    const auto migrated = urpg::save::ImportCompatSaveDocument(malformed);
    REQUIRE(migrated.used_safe_fallback);
    REQUIRE_FALSE(migrated.native_payload.contains("database_state"));
    REQUIRE(migrated.native_payload["_compat_payload_retained"]["/databaseState"] == malformed["databaseState"]);
    REQUIRE(std::any_of(migrated.diagnostics.begin(), migrated.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "invalid_database_state" && diagnostic.field_path == "/databaseState";
    }));
}
