#include "engine/core/save/save_debugger.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("save debugger inspects metadata recovery tier migration notes and subsystem state", "[save][debugger][ffs09]") {
    const nlohmann::json save = {
        {"schema_version", "urpg.save.v1"},
        {"slot_id", 4},
        {"recovery_tier", "level2_metadata_variables"},
        {"metadata", {{"_save_version", "1.0.0"}, {"_map_display_name", "Harbor"}}},
        {"subsystems", {{"battle", {{"turn", 3}}}, {"quests", {{"active", 2}}}}},
        {"migration_notes", {"mapped gold", "retained plugin blob"}},
    };

    const urpg::save::SaveDebugger debugger;
    const auto slot = debugger.inspectSlot(save);
    const auto diagnostics = debugger.exportDiagnostics(slot);

    REQUIRE(slot.slot_id == 4);
    REQUIRE(slot.recovery_tier == "level2_metadata_variables");
    REQUIRE(slot.metadata["_map_display_name"] == "Harbor");
    REQUIRE(slot.subsystem_state["battle"]["turn"] == 3);
    REQUIRE(slot.migration_notes.size() == 2);
    REQUIRE(slot.diagnostics.empty());
    REQUIRE(diagnostics["slot_id"] == 4);
}

TEST_CASE("save debugger diagnoses wrong schema versions", "[save][debugger][ffs09]") {
    const urpg::save::SaveDebugger debugger;

    const auto slot = debugger.inspectSlot({{"schema_version", "urpg.save.legacy"}, {"slot_id", 1}});

    REQUIRE(std::find(slot.diagnostics.begin(),
                      slot.diagnostics.end(),
                      "unexpected_schema_version:urpg.save.legacy") != slot.diagnostics.end());
}
