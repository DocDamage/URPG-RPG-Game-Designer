#pragma once

#include "engine/core/save/save_metadata_registry.h"
#include "engine/core/save/save_recovery.h"
#include "engine/core/save/save_runtime.h"
#include "engine/core/save/save_types.h"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg {

struct SaveCatalogEntry {
    SaveSlotMeta meta;
    SaveRecoveryTier last_recovery_tier = SaveRecoveryTier::None;
    bool has_primary_payload = false;
    bool has_variables_payload = false;
    std::string diagnostic;
    std::string last_operation;
    uint64_t save_sequence = 0;
    std::filesystem::path primary_save_path;
    std::filesystem::path metadata_path;
    std::filesystem::path variables_path;
};

struct SaveAutosavePolicy {
    bool enabled = true;
    int32_t slot_id = 0;
};

struct SaveRetentionPolicy {
    size_t max_autosave_slots = 1;
    size_t max_quicksave_slots = 3;
    size_t max_manual_slots = 20;
    bool prune_excess_on_save = true;
};

struct SaveSlotDescriptor {
    int32_t slot_id = -1;
    SaveSlotCategory category = SaveSlotCategory::Manual;
    std::string label;
    bool reserved = false;
};

class SaveCatalog {
public:
    bool upsert(SaveCatalogEntry entry);
    bool erase(int32_t slot_id);
    void clear();

    const SaveCatalogEntry* find(int32_t slot_id) const;
    SaveCatalogEntry* find(int32_t slot_id);
    std::vector<SaveCatalogEntry> listEntries(bool include_autosave = true) const;

private:
    std::unordered_map<int32_t, SaveCatalogEntry> entries_;
};

struct SaveSessionLoadRequest {
    int32_t slot_id = 0;
    RuntimeSaveLoadRequest runtime_request;
};

struct SaveSessionLoadResult {
    bool ok = false;
    int32_t slot_id = -1;
    bool boot_safe_mode = false;
    bool loaded_from_recovery = false;
    SaveRecoveryTier recovery_tier = SaveRecoveryTier::None;
    std::string error;
    SaveSlotMeta active_meta;
    std::vector<std::string> diagnostics;
};

struct SaveSessionSaveRequest {
    int32_t slot_id = 0;
    SaveSlotMeta meta;
    std::string payload;
    std::string variables_payload;
    std::filesystem::path primary_save_path;
    std::filesystem::path metadata_path;
    std::filesystem::path variables_path;
    bool write_variables_payload = false;
};

struct SaveSessionSaveResult {
    bool ok = false;
    int32_t slot_id = -1;
    std::string error;
    SaveSlotMeta active_meta;
    std::vector<int32_t> pruned_slot_ids;
};

class SaveSessionCoordinator {
public:
    explicit SaveSessionCoordinator(SaveCatalog& catalog);

    const SaveAutosavePolicy& autosavePolicy() const;
    void setAutosavePolicy(const SaveAutosavePolicy& policy);
    const SaveRetentionPolicy& retentionPolicy() const;
    void setRetentionPolicy(const SaveRetentionPolicy& policy);

    bool loadSavePolicies(const std::filesystem::path& path);
    bool loadSaveSlots(const std::filesystem::path& path);

    const SaveMetadataRegistry& metadataRegistry() const { return metadata_registry_; }
    SaveMetadataRegistry& metadataRegistry() { return metadata_registry_; }
    const std::vector<SaveSlotDescriptor>& slotDescriptors() const { return slot_descriptors_; }
    std::optional<SaveSlotDescriptor> slotDescriptor(int32_t slot_id) const;

    bool canAutosave() const;
    int32_t autosaveSlot() const;

    SaveSessionLoadResult load(const SaveSessionLoadRequest& request);
    SaveSessionSaveResult save(const SaveSessionSaveRequest& request);

private:
    void updateCatalogEntry(int32_t slot_id, const RuntimeSaveLoadResult& runtime_result);
    std::vector<int32_t> pruneRetainedSlots(int32_t protected_slot_id);

    SaveCatalog& catalog_;
    SaveMetadataRegistry metadata_registry_;
    std::vector<SaveSlotDescriptor> slot_descriptors_;
    SaveAutosavePolicy autosave_policy_;
    SaveRetentionPolicy retention_policy_;
    uint64_t next_save_sequence_ = 1;
};

} // namespace urpg
