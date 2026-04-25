#include "engine/core/balance/encounter_table.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Encounter table rejects zero-weight pools and previews deterministically", "[balance][encounter][ffs12]") {
    urpg::balance::EncounterTable invalid;
    invalid.addEncounter({"slime", "forest", 0, 1});
    invalid.addEncounter({"bat", "forest", 0, 2});

    REQUIRE(invalid.validate().front().code == "zero_weight_pool");

    urpg::balance::EncounterTable table;
    table.addEncounter({"slime", "forest", 3, 1});
    table.addEncounter({"orc", "forest", 1, 4});

    const auto first = table.preview("forest", 42, 4);
    const auto second = table.preview("forest", 42, 4);

    REQUIRE(first == second);
    REQUIRE(first.size() == 4);
}
