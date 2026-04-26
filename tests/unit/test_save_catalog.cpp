#include "engine/core/save/save_catalog.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

void WriteText(const std::filesystem::path& path, const std::string& value) {
    std::ofstream out(path, std::ios::binary);
    out << value;
}

} // namespace

TEST_CASE("Save catalog orders slots deterministically", "[save][catalog]") {
    urpg::SaveCatalog catalog;

    urpg::SaveCatalogEntry autosave;
    autosave.meta.slot_id = 0;
    autosave.meta.flags.autosave = true;
    autosave.meta.category = urpg::SaveSlotCategory::Autosave;
    autosave.meta.retention_class = urpg::SaveRetentionClass::Autosave;
    autosave.meta.map_display_name = "Autosave";
    REQUIRE(catalog.upsert(autosave));

    urpg::SaveCatalogEntry slotFive;
    slotFive.meta.slot_id = 5;
    slotFive.meta.category = urpg::SaveSlotCategory::Manual;
    slotFive.meta.retention_class = urpg::SaveRetentionClass::Manual;
    slotFive.meta.map_display_name = "Archive";
    REQUIRE(catalog.upsert(slotFive));

    urpg::SaveCatalogEntry slotTwo;
    slotTwo.meta.slot_id = 2;
    slotTwo.meta.category = urpg::SaveSlotCategory::Quicksave;
    slotTwo.meta.retention_class = urpg::SaveRetentionClass::Quicksave;
    slotTwo.meta.map_display_name = "Atrium";
    REQUIRE(catalog.upsert(slotTwo));

    const auto allEntries = catalog.listEntries();
    REQUIRE(allEntries.size() == 3);
    REQUIRE(allEntries[0].meta.slot_id == 0);
    REQUIRE(allEntries[1].meta.slot_id == 2);
    REQUIRE(allEntries[2].meta.slot_id == 5);

    const auto nonAutosaveEntries = catalog.listEntries(false);
    REQUIRE(nonAutosaveEntries.size() == 2);
    REQUIRE(nonAutosaveEntries[0].meta.slot_id == 2);
    REQUIRE(nonAutosaveEntries[1].meta.slot_id == 5);

    REQUIRE(catalog.erase(2));
    REQUIRE(catalog.find(2) == nullptr);
}

TEST_CASE("SaveMetadataRegistry applies defaults to SaveSlotMeta", "[save][metadata]") {
    urpg::SaveMetadataRegistry registry;
    registry.registerField({"quest_id", "Quest ID", true, "none"});
    registry.registerField({"difficulty", "Difficulty", false, "Normal"});

    urpg::SaveSlotMeta meta;
    meta.custom_metadata["quest_id"] = "prologue_01";
    // difficulty is missing, should get default

    registry.applyDefaults(meta);

    REQUIRE(meta.custom_metadata["quest_id"] == "prologue_01");
    REQUIRE(meta.custom_metadata["difficulty"] == "Normal");
}

TEST_CASE("SaveSessionCoordinator integrates MetadataRegistry", "[save][catalog][metadata]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_metadata_integration";
    std::filesystem::create_directories(base);

    const auto primary = base / "slot_007.json";
    WriteText(primary, "{\"state\":\"ok\"}");

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);
    coordinator.metadataRegistry().registerField({"engine_mode", "Engine Mode", false, "native"});

    urpg::SaveSessionSaveRequest request;
    request.slot_id = 7;
    request.primary_save_path = primary;
    request.meta.map_display_name = "Registry Test";

    // We don't set engine_mode in request.meta.custom_metadata

    const auto result = coordinator.save(request);
    REQUIRE(result.ok);
    REQUIRE(result.active_meta.custom_metadata.count("engine_mode") == 1);
    REQUIRE(result.active_meta.custom_metadata.at("engine_mode") == "native");

    const auto* entry = catalog.find(7);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->meta.custom_metadata.at("engine_mode") == "native");

    std::filesystem::remove_all(base);
}

TEST_CASE("SaveSessionCoordinator reports primary save write failures with actionable diagnostics",
          "[save][catalog][persistence][error]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_write_failure";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base / "slot_008.json");

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);

    urpg::SaveSessionSaveRequest request;
    request.slot_id = 8;
    request.primary_save_path = base / "slot_008.json";
    request.payload = R"({"state":"cannot-write-over-directory"})";
    request.meta.map_display_name = "Write Failure";

    const auto result = coordinator.save(request);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.slot_id == 8);
    REQUIRE_FALSE(result.error.empty());
    const bool hasActionableWriteError = result.error == "failed_to_open_temp_file" ||
                                         result.error == "failed_to_write_backup" ||
                                         result.error == "failed_to_rename_temp_file";
    REQUIRE(hasActionableWriteError);
    REQUIRE(catalog.find(8) == nullptr);

    std::filesystem::remove_all(base);
}

TEST_CASE("SaveSessionCoordinator loads save_policies.json", "[save][catalog][schema]") {
    const auto path = std::filesystem::temp_directory_path() / "save_policies.json";
    const std::string content = R"({
        "metadata_fields": [
            { "key": "map_id", "display_label": "Map ID", "default_value": "0" },
            { "key": "difficulty", "default_value": "Hard" }
        ],
        "retention": {
            "max_autosave_slots": 5,
            "max_manual_slots": 50
        },
        "autosave": {
            "enabled": true,
            "slot_id": 99
        }
    })";
    WriteText(path, content);

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);

    REQUIRE(coordinator.loadSavePolicies(path));

    // Check Metadata
    const auto& fields = coordinator.metadataRegistry().getFields();
    REQUIRE(fields.count("map_id") == 1);
    REQUIRE(fields.at("map_id").display_label == "Map ID");
    REQUIRE(fields.at("difficulty").default_value == "Hard");

    // Check Retention
    REQUIRE(coordinator.retentionPolicy().max_autosave_slots == 5);
    REQUIRE(coordinator.retentionPolicy().max_manual_slots == 50);
    REQUIRE(coordinator.retentionPolicy().max_quicksave_slots == 3); // Default preserved

    // Check Autosave
    REQUIRE(coordinator.canAutosave());
    REQUIRE(coordinator.autosaveSlot() == 99);

    std::filesystem::remove(path);
}

TEST_CASE("SaveSessionCoordinator loads save_slots.json descriptors", "[save][catalog][schema]") {
    const auto path = std::filesystem::temp_directory_path() / "save_slots.json";
    const std::string content = R"({
        "slots": [
            { "slot_id": 9, "category": "manual", "label": "Archive Slot 9", "reserved": false },
            { "slot_id": 0, "category": "autosave", "label": "Autosave Root", "reserved": true },
            { "slot_id": 3, "category": "quicksave", "label": "Quick Slot 3", "reserved": false }
        ]
    })";
    WriteText(path, content);

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);
    REQUIRE(coordinator.loadSaveSlots(path));

    const auto& descriptors = coordinator.slotDescriptors();
    REQUIRE(descriptors.size() == 3);
    REQUIRE(descriptors[0].slot_id == 0);
    REQUIRE(descriptors[0].category == urpg::SaveSlotCategory::Autosave);
    REQUIRE(descriptors[0].label == "Autosave Root");
    REQUIRE(descriptors[0].reserved);

    REQUIRE(descriptors[1].slot_id == 3);
    REQUIRE(descriptors[1].category == urpg::SaveSlotCategory::Quicksave);
    REQUIRE(descriptors[1].label == "Quick Slot 3");
    REQUIRE_FALSE(descriptors[1].reserved);

    REQUIRE(descriptors[2].slot_id == 9);
    REQUIRE(descriptors[2].category == urpg::SaveSlotCategory::Manual);
    REQUIRE(descriptors[2].label == "Archive Slot 9");

    const auto autosaveDescriptor = coordinator.slotDescriptor(0);
    REQUIRE(autosaveDescriptor.has_value());
    REQUIRE(autosaveDescriptor->reserved);

    REQUIRE_FALSE(coordinator.slotDescriptor(404).has_value());

    std::filesystem::remove(path);
}

TEST_CASE("Save session coordinator records primary slot loads", "[save][catalog]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_catalog_primary";
    std::filesystem::create_directories(base);

    const auto primary = base / "slot_003.json";
    const auto meta = base / "meta.json";
    WriteText(primary, "{\"state\":\"primary\"}");
    WriteText(meta, "{\"_slot_id\":3,\"_map_display_name\":\"Sky Wharf\"}");

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);

    urpg::SaveSessionLoadRequest request;
    request.slot_id = 3;
    request.runtime_request.primary_save_path = primary;
    request.runtime_request.metadata_path = meta;

    const auto result = coordinator.load(request);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.loaded_from_recovery);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::None);
    REQUIRE(result.active_meta.slot_id == 3);
    REQUIRE(result.active_meta.map_display_name == "Sky Wharf");
    REQUIRE_FALSE(result.active_meta.flags.autosave);
    REQUIRE(result.active_meta.category == urpg::SaveSlotCategory::Manual);
    REQUIRE(result.active_meta.retention_class == urpg::SaveRetentionClass::Manual);

    const auto* entry = catalog.find(3);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->has_primary_payload);
    REQUIRE_FALSE(entry->has_variables_payload);
    REQUIRE(entry->last_recovery_tier == urpg::SaveRecoveryTier::None);
    REQUIRE(entry->meta.map_display_name == "Sky Wharf");

    std::filesystem::remove_all(base);
}

TEST_CASE("Save session coordinator tracks autosave recovery and safe mode", "[save][catalog]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_catalog_recovery";
    std::filesystem::create_directories(base);

    const auto autosave = base / "autosave.json";
    const auto safeMeta = base / "safe_meta.json";
    WriteText(autosave, "{\"state\":\"autosave\"}");
    WriteText(safeMeta, "{\"_slot_id\":7,\"_map_display_name\":\"Cinder Vault\"}");

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);

    urpg::SaveSessionLoadRequest autosaveRequest;
    autosaveRequest.slot_id = 0;
    autosaveRequest.runtime_request.primary_save_path = base / "missing_primary.json";
    autosaveRequest.runtime_request.autosave_path = autosave;

    const auto autosaveResult = coordinator.load(autosaveRequest);
    REQUIRE(autosaveResult.ok);
    REQUIRE(autosaveResult.loaded_from_recovery);
    REQUIRE_FALSE(autosaveResult.boot_safe_mode);
    REQUIRE(autosaveResult.recovery_tier == urpg::SaveRecoveryTier::Level1Autosave);
    REQUIRE(autosaveResult.active_meta.flags.autosave);
    REQUIRE(autosaveResult.active_meta.category == urpg::SaveSlotCategory::Autosave);

    const auto* autosaveEntry = catalog.find(0);
    REQUIRE(autosaveEntry != nullptr);
    REQUIRE(autosaveEntry->meta.flags.autosave);
    REQUIRE(autosaveEntry->meta.retention_class == urpg::SaveRetentionClass::Autosave);
    REQUIRE_FALSE(autosaveEntry->meta.flags.corrupted);
    REQUIRE(autosaveEntry->last_recovery_tier == urpg::SaveRecoveryTier::Level1Autosave);

    urpg::SaveSessionLoadRequest safeModeRequest;
    safeModeRequest.slot_id = 7;
    safeModeRequest.runtime_request.primary_save_path = base / "missing_slot_007.json";
    safeModeRequest.runtime_request.metadata_path = safeMeta;
    safeModeRequest.runtime_request.variables_path = base / "missing_vars.json";

    const auto safeModeResult = coordinator.load(safeModeRequest);
    REQUIRE(safeModeResult.ok);
    REQUIRE(safeModeResult.loaded_from_recovery);
    REQUIRE(safeModeResult.boot_safe_mode);
    REQUIRE(safeModeResult.recovery_tier == urpg::SaveRecoveryTier::Level3SafeSkeleton);
    REQUIRE(safeModeResult.active_meta.slot_id == 7);
    REQUIRE(safeModeResult.active_meta.flags.corrupted);
    REQUIRE(safeModeResult.active_meta.category == urpg::SaveSlotCategory::Manual);
    REQUIRE(safeModeResult.active_meta.map_display_name == "Cinder Vault");

    const auto* safeModeEntry = catalog.find(7);
    REQUIRE(safeModeEntry != nullptr);
    REQUIRE(safeModeEntry->meta.flags.corrupted);
    REQUIRE(safeModeEntry->last_recovery_tier == urpg::SaveRecoveryTier::Level3SafeSkeleton);
    REQUIRE_FALSE(safeModeEntry->has_primary_payload);

    coordinator.setAutosavePolicy({false, 0});
    REQUIRE_FALSE(coordinator.canAutosave());
    REQUIRE(coordinator.autosaveSlot() == 0);

    urpg::SaveSessionLoadRequest disabledAutosaveRequest;
    disabledAutosaveRequest.slot_id = 0;
    const auto disabledAutosaveResult = coordinator.load(disabledAutosaveRequest);
    REQUIRE_FALSE(disabledAutosaveResult.ok);
    REQUIRE(disabledAutosaveResult.error == "autosave_disabled");

    std::filesystem::remove_all(base);
}

TEST_CASE("Save session coordinator writes slot files and prunes retained manual saves", "[save][catalog]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_catalog_save_retention";
    std::filesystem::create_directories(base);

    urpg::SaveCatalog catalog;
    urpg::SaveSessionCoordinator coordinator(catalog);
    coordinator.setRetentionPolicy({1, 1, 2, true});

    auto makeRequest = [&](int32_t slotId, const std::string& mapName, urpg::SaveSlotCategory category) {
        urpg::SaveSessionSaveRequest request;
        request.slot_id = slotId;
        request.meta.category = category;
        request.meta.retention_class = urpg::RetentionClassForCategory(category);
        request.meta.map_display_name = mapName;
        request.meta.save_version = "1.0";
        request.payload = std::string("{\"slot\":") + std::to_string(slotId) + "}";
        request.variables_payload = std::string("{\"vars\":") + std::to_string(slotId) + "}";
        request.primary_save_path = base / ("slot_" + std::to_string(slotId) + ".json");
        request.metadata_path = base / ("slot_" + std::to_string(slotId) + "_meta.json");
        request.variables_path = base / ("slot_" + std::to_string(slotId) + "_vars.json");
        request.write_variables_payload = true;
        return request;
    };

    const auto saveOne = coordinator.save(makeRequest(1, "Atrium", urpg::SaveSlotCategory::Manual));
    REQUIRE(saveOne.ok);
    REQUIRE(saveOne.pruned_slot_ids.empty());
    REQUIRE(std::filesystem::exists(base / "slot_1.json"));

    const auto saveTwo = coordinator.save(makeRequest(2, "Harbor", urpg::SaveSlotCategory::Manual));
    REQUIRE(saveTwo.ok);
    REQUIRE(saveTwo.pruned_slot_ids.empty());

    const auto quicksaveOne = coordinator.save(makeRequest(90, "Quick Nest", urpg::SaveSlotCategory::Quicksave));
    REQUIRE(quicksaveOne.ok);
    REQUIRE(quicksaveOne.pruned_slot_ids.empty());

    const auto quicksaveTwo = coordinator.save(makeRequest(91, "Quick Nest II", urpg::SaveSlotCategory::Quicksave));
    REQUIRE(quicksaveTwo.ok);
    REQUIRE(quicksaveTwo.pruned_slot_ids.size() == 1);
    REQUIRE(quicksaveTwo.pruned_slot_ids[0] == 90);
    REQUIRE(catalog.find(90) == nullptr);

    const auto saveThree = coordinator.save(makeRequest(3, "Vault", urpg::SaveSlotCategory::Manual));
    REQUIRE(saveThree.ok);
    REQUIRE(saveThree.pruned_slot_ids.size() == 1);
    REQUIRE(saveThree.pruned_slot_ids[0] == 1);
    REQUIRE_FALSE(std::filesystem::exists(base / "slot_1.json"));
    REQUIRE(catalog.find(1) == nullptr);

    const auto* retainedTwo = catalog.find(2);
    const auto* retainedThree = catalog.find(3);
    REQUIRE(retainedTwo != nullptr);
    REQUIRE(retainedThree != nullptr);
    REQUIRE(retainedTwo->last_operation == "save");
    REQUIRE(retainedTwo->meta.retention_class == urpg::SaveRetentionClass::Manual);
    REQUIRE(retainedThree->has_variables_payload);

    const auto* retainedQuick = catalog.find(91);
    REQUIRE(retainedQuick != nullptr);
    REQUIRE(retainedQuick->meta.category == urpg::SaveSlotCategory::Quicksave);
    REQUIRE(retainedQuick->meta.retention_class == urpg::SaveRetentionClass::Quicksave);

    coordinator.setAutosavePolicy({false, 0});
    urpg::SaveSessionSaveRequest autosaveRequest;
    autosaveRequest.slot_id = 0;
    autosaveRequest.meta.category = urpg::SaveSlotCategory::Autosave;
    autosaveRequest.payload = "{}";
    autosaveRequest.primary_save_path = base / "autosave.json";
    const auto autosaveResult = coordinator.save(autosaveRequest);
    REQUIRE_FALSE(autosaveResult.ok);
    REQUIRE(autosaveResult.error == "autosave_disabled");

    std::filesystem::remove_all(base);
}
