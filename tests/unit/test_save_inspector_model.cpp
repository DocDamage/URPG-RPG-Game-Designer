#include "editor/save/save_inspector_model.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

namespace {

void WriteText(const std::filesystem::path& path, const std::string& value) {
    std::ofstream out(path, std::ios::binary);
    out << value;
}

} // namespace

TEST_CASE("Save inspector model builds rows and summary from catalog", "[save][editor]") {
    urpg::SaveCatalog catalog;

    urpg::SaveCatalogEntry autosave;
    autosave.meta.slot_id = 0;
    autosave.meta.flags.autosave = true;
    autosave.meta.category = urpg::SaveSlotCategory::Autosave;
    autosave.meta.retention_class = urpg::SaveRetentionClass::Autosave;
    autosave.meta.map_display_name = "Autosave Dock";
    autosave.last_operation = "save";
    catalog.upsert(autosave);

    urpg::SaveCatalogEntry recovered;
    recovered.meta.slot_id = 4;
    recovered.meta.category = urpg::SaveSlotCategory::Quicksave;
    recovered.meta.retention_class = urpg::SaveRetentionClass::Quicksave;
    recovered.meta.flags.corrupted = true;
    recovered.meta.map_display_name = "Cinder Vault";
    recovered.last_recovery_tier = urpg::SaveRecoveryTier::Level3SafeSkeleton;
    recovered.last_operation = "load";
    recovered.diagnostic = "safe_mode_triggered";
    catalog.upsert(recovered);

    urpg::SaveSessionCoordinator coordinator(catalog);
    coordinator.setAutosavePolicy({true, 0});
    coordinator.setRetentionPolicy({1, 2, 6, true});
    const auto saveSlotsPath = std::filesystem::temp_directory_path() / "save_slots_inspector_model.json";
    WriteText(
        saveSlotsPath,
        R"({
            "slots": [
                { "slot_id": 0, "category": "autosave", "label": "Autosave Dock Label", "reserved": true },
                { "slot_id": 4, "category": "quicksave", "label": "Recovery Queue", "reserved": false }
            ]
        })");
    REQUIRE(coordinator.loadSaveSlots(saveSlotsPath));

    urpg::editor::SaveInspectorModel model;
    model.LoadFromCatalog(catalog, coordinator);

    const auto& summary = model.Summary();
    REQUIRE(summary.total_slots == 2);
    REQUIRE(summary.autosave_slots == 1);
    REQUIRE(summary.quicksave_slots == 1);
    REQUIRE(summary.manual_slots == 0);
    REQUIRE(summary.corrupted_slots == 1);
    REQUIRE(summary.recovery_slots == 1);
    REQUIRE(summary.safe_mode_slots == 1);
    REQUIRE(summary.autosave_enabled);
    REQUIRE(summary.autosave_slot_id == 0);
    REQUIRE(summary.autosave_retention_limit == 1);
    REQUIRE(summary.quicksave_retention_limit == 2);
    REQUIRE(summary.manual_retention_limit == 6);
    REQUIRE(summary.reserved_slots == 1);

    const auto& rows = model.VisibleRows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].slot_id == 0);
    REQUIRE(rows[0].reserved_slot);
    REQUIRE(rows[0].slot_label == "Autosave Dock Label");
    REQUIRE(rows[0].autosave);
    REQUIRE(rows[0].category_label == "autosave");
    REQUIRE(rows[1].slot_id == 4);
    REQUIRE_FALSE(rows[1].reserved_slot);
    REQUIRE(rows[1].slot_label == "Recovery Queue");
    REQUIRE(rows[1].corrupted);
    REQUIRE(rows[1].boot_safe_mode);
    REQUIRE(rows[1].category_label == "quicksave");
    REQUIRE(rows[1].retention_label == "quicksave");
    REQUIRE(rows[1].summary.find("load / quicksave / safe_mode_recovery / retained:quicksave / corrupted") != std::string::npos);

    std::filesystem::remove(saveSlotsPath);
}

TEST_CASE("Save inspector model filters problem rows and autosave rows", "[save][editor]") {
    urpg::SaveCatalog catalog;

    urpg::SaveCatalogEntry autosave;
    autosave.meta.slot_id = 0;
    autosave.meta.flags.autosave = true;
    autosave.meta.category = urpg::SaveSlotCategory::Autosave;
    autosave.meta.retention_class = urpg::SaveRetentionClass::Autosave;
    autosave.last_operation = "save";
    catalog.upsert(autosave);

    urpg::SaveCatalogEntry clean;
    clean.meta.slot_id = 2;
    clean.meta.category = urpg::SaveSlotCategory::Manual;
    clean.meta.retention_class = urpg::SaveRetentionClass::Manual;
    clean.meta.map_display_name = "Harbor";
    clean.last_operation = "save";
    catalog.upsert(clean);

    urpg::SaveCatalogEntry problem;
    problem.meta.slot_id = 7;
    problem.meta.category = urpg::SaveSlotCategory::Quicksave;
    problem.meta.retention_class = urpg::SaveRetentionClass::Quicksave;
    problem.meta.flags.corrupted = true;
    problem.last_recovery_tier = urpg::SaveRecoveryTier::Level2MetadataVariables;
    problem.last_operation = "load";
    catalog.upsert(problem);

    urpg::SaveSessionCoordinator coordinator(catalog);
    urpg::editor::SaveInspectorModel model;
    model.LoadFromCatalog(catalog, coordinator);
    model.SetShowProblemSlotsOnly(true);

    auto rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].slot_id == 7);

    model.SetIncludeAutosave(false);
    rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].slot_id == 7);
}

TEST_CASE("Save inspector model selection returns slot id", "[save][editor]") {
    urpg::SaveCatalog catalog;

    urpg::SaveCatalogEntry entry;
    entry.meta.slot_id = 5;
    entry.meta.category = urpg::SaveSlotCategory::Manual;
    entry.meta.retention_class = urpg::SaveRetentionClass::Manual;
    entry.last_operation = "save";
    catalog.upsert(entry);

    urpg::SaveSessionCoordinator coordinator(catalog);
    urpg::editor::SaveInspectorModel model;
    model.LoadFromCatalog(catalog, coordinator);

    REQUIRE(model.SelectRow(0));
    const auto selected_slot = model.SelectedSlotId();
    REQUIRE(selected_slot.has_value());
    REQUIRE(selected_slot.value() == 5);

    REQUIRE_FALSE(model.SelectRow(9));
    REQUIRE_FALSE(model.SelectedSlotId().has_value());
}

TEST_CASE("Save inspector model uses slot descriptor label when map name is empty", "[save][editor]") {
    urpg::SaveCatalog catalog;

    urpg::SaveCatalogEntry entry;
    entry.meta.slot_id = 8;
    entry.meta.category = urpg::SaveSlotCategory::Manual;
    entry.meta.retention_class = urpg::SaveRetentionClass::Manual;
    entry.last_operation = "save";
    catalog.upsert(entry);

    urpg::SaveSessionCoordinator coordinator(catalog);
    const auto saveSlotsPath = std::filesystem::temp_directory_path() / "save_slots_inspector_label_fallback.json";
    WriteText(
        saveSlotsPath,
        R"({
            "slots": [
                { "slot_id": 8, "category": "manual", "label": "Manual Slot 8", "reserved": true }
            ]
        })");
    REQUIRE(coordinator.loadSaveSlots(saveSlotsPath));

    urpg::editor::SaveInspectorModel model;
    model.LoadFromCatalog(catalog, coordinator);
    const auto& rows = model.VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].slot_id == 8);
    REQUIRE(rows[0].slot_label == "Manual Slot 8");
    REQUIRE(rows[0].reserved_slot);
    REQUIRE(rows[0].map_display_name == "Manual Slot 8");
    REQUIRE(rows[0].summary.find("/ reserved") != std::string::npos);

    std::filesystem::remove(saveSlotsPath);
}
