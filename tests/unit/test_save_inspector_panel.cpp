#include "editor/save/save_inspector_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

TEST_CASE("SaveInspectorPanel - Refresh ingests runtime save catalog state", "[save][panel][integration]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_inspector_panel";
    std::filesystem::create_directories(base);

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);
    coordinator.setAutosavePolicy({true, 0});
    coordinator.setRetentionPolicy({1, 1, 4, true});

    urpg::editor::SaveInspectorPanel panel;
    panel.bindRuntime(catalog, coordinator);

    urpg::SaveSessionSaveRequest quicksaveRequest;
    quicksaveRequest.slot_id = 8;
    quicksaveRequest.meta.category = urpg::SaveSlotCategory::Quicksave;
    quicksaveRequest.meta.retention_class = urpg::SaveRetentionClass::Quicksave;
    quicksaveRequest.meta.map_display_name = "Bridge";
    quicksaveRequest.payload = "{\"slot\":8}";
    quicksaveRequest.primary_save_path = base / "slot_8.json";
    REQUIRE(coordinator.save(quicksaveRequest).ok);

    panel.refresh();

    const auto& firstSummary = panel.getModel().Summary();
    REQUIRE(firstSummary.total_slots == 1);
    REQUIRE(firstSummary.quicksave_slots == 1);
    REQUIRE(firstSummary.manual_slots == 0);
    REQUIRE(panel.getModel().VisibleRows().size() == 1);
    REQUIRE(panel.getModel().VisibleRows()[0].category_label == "quicksave");

    urpg::SaveCatalogEntry recovered;
    recovered.meta.slot_id = 0;
    recovered.meta.flags.autosave = true;
    recovered.meta.category = urpg::SaveSlotCategory::Autosave;
    recovered.meta.retention_class = urpg::SaveRetentionClass::Autosave;
    recovered.meta.flags.corrupted = true;
    recovered.meta.map_display_name = "Fallback Camp";
    recovered.last_operation = "load";
    recovered.last_recovery_tier = urpg::SaveRecoveryTier::Level3SafeSkeleton;
    recovered.diagnostic = "safe_mode_triggered";
    REQUIRE(catalog.upsert(recovered));

    panel.setShowProblemSlotsOnly(true);
    panel.update();

    const auto& rows = panel.getModel().VisibleRows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0].slot_id == 0);
    REQUIRE(rows[0].autosave);
    REQUIRE(rows[0].boot_safe_mode);

    std::filesystem::remove_all(base);
}