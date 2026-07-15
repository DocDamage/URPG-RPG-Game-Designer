#include "engine/core/database/rpg_database.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("RPG database CSV round trip preserves IDs and values", "[database][balance][ffs12]") {
    urpg::database::RpgDatabase database;
    database.upsertActor({"actor.hero", "Hero", "class.warrior", 120, 18});
    database.upsertItem({"item.potion", "Potion", 50, {"healing"}});

    const auto csv = database.exportItemsCsv();
    const auto imported = urpg::database::RpgDatabase::fromItemsCsv(csv);

    REQUIRE(imported.items().size() == 1);
    REQUIRE(imported.items().at("item.potion").name == "Potion");
    REQUIRE(imported.items().at("item.potion").price == 50);
}

TEST_CASE("RPG database rejects duplicate IDs and malformed CSV rows", "[database][balance][ffs12]") {
    urpg::database::RpgDatabase database;
    database.upsertItem({"item.potion", "Potion", 50, {}});
    database.upsertItem({"item.potion", "Potion 2", 60, {}});

    const auto diagnostics = database.validate();
    REQUIRE_FALSE(diagnostics.empty());
    REQUIRE(diagnostics[0].code == "duplicate_item_id");

    const auto malformed = urpg::database::RpgDatabase::fromItemsCsv("id,name\nitem.bad,Bad");
    REQUIRE(malformed.validate().front().code == "csv_missing_required_column");
}

TEST_CASE("RPG database autofill is deterministic and respects caps", "[database][balance][ffs12]") {
    const urpg::database::AutofillProfile profile{5, 999, 80, 100};

    const auto first = urpg::database::RpgDatabase::autofill(profile, 1234);
    const auto second = urpg::database::RpgDatabase::autofill(profile, 1234);

    REQUIRE(first.toJson() == second.toJson());
    REQUIRE(first.actors().at("actor.generated").max_hp <= 100);
    REQUIRE(first.validate().empty());
}

TEST_CASE("RPG database round-trips its durable project JSON", "[database][json]") {
    urpg::database::RpgDatabase database;
    database.upsertActor({"willow_hero", "Willow Hero", "class_ranger", 90, 12});
    database.upsertItem({"moonwell_lantern", "Moonwell Lantern", 75, {"quest"}});

    const auto restored = urpg::database::RpgDatabase::fromJson(database.toJson());
    REQUIRE(restored.actors().at("willow_hero").name == "Willow Hero");
    REQUIRE(restored.items().at("moonwell_lantern").tags.contains("quest"));
}
