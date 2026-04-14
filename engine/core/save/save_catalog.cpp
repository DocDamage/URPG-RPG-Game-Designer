#include "engine/core/save/save_catalog.h"

#include "engine/core/save/save_journal.h"

#include <algorithm>
#include <fstream>
#include <system_error>
#include <utility>

#include <nlohmann/json.hpp>

namespace urpg {

namespace {

bool IsValidSlotId(int32_t slot_id) {
    return slot_id >= 0;
}

bool IsAutosaveSlot(int32_t slot_id, int32_t autosave_slot) {
    return slot_id == autosave_slot;
}

SaveSlotCategory ParseSlotCategory(const SaveSlotMeta& meta, int32_t slot_id, int32_t autosave_slot) {
    if (meta.flags.autosave || IsAutosaveSlot(slot_id, autosave_slot) || meta.category == SaveSlotCategory::Autosave) {
        return SaveSlotCategory::Autosave;
    }
    return meta.category;
}

SaveRetentionClass ParseRetentionClass(const SaveSlotMeta& meta, SaveSlotCategory category) {
    if (category == SaveSlotCategory::Autosave) {
        return SaveRetentionClass::Autosave;
    }
    if (meta.retention_class == SaveRetentionClass::Autosave && category != SaveSlotCategory::Autosave) {
        return RetentionClassForCategory(category);
    }
    return meta.retention_class == SaveRetentionClass::Autosave && category == SaveSlotCategory::Autosave
        ? SaveRetentionClass::Autosave
        : (meta.retention_class == SaveRetentionClass::Quicksave || meta.retention_class == SaveRetentionClass::Manual
            ? meta.retention_class
            : RetentionClassForCategory(category));
}

void NormalizeMeta(SaveSlotMeta& meta, int32_t slot_id, int32_t autosave_slot, const SaveMetadataRegistry* registry = nullptr) {
    meta.slot_id = slot_id;
    meta.category = ParseSlotCategory(meta, slot_id, autosave_slot);
    meta.retention_class = ParseRetentionClass(meta, meta.category);
    meta.flags.autosave = (meta.category == SaveSlotCategory::Autosave);
    if (registry) {
        registry->applyDefaults(meta);
    }
}

size_t RetentionLimitForClass(const SaveRetentionPolicy& policy, SaveRetentionClass retention_class) {
    switch (retention_class) {
    case SaveRetentionClass::Autosave:
        return policy.max_autosave_slots;
    case SaveRetentionClass::Quicksave:
        return policy.max_quicksave_slots;
    case SaveRetentionClass::Manual:
        return policy.max_manual_slots;
    }
    return policy.max_manual_slots;
}

nlohmann::json BuildMetadataJson(const SaveSlotMeta& meta) {
    nlohmann::json root;
    root["_slot_id"] = meta.slot_id;
    root["_slot_category"] = ToString(meta.category);
    root["_retention_class"] = ToString(meta.retention_class);
    root["_save_version"] = meta.save_version;
    root["_timestamp"] = meta.timestamp;
    root["_playtime_seconds"] = meta.playtime_seconds;
    root["_thumbnail_hash"] = meta.thumbnail_hash;
    root["_map_display_name"] = meta.map_display_name;
    root["_data_blob_path"] = meta.data_blob_path;
    root["_flags"] = {
        {"autosave", meta.flags.autosave},
        {"copilot_generated", meta.flags.copilot_generated},
        {"corrupted", meta.flags.corrupted}
    };

    root["_party_snapshot"] = nlohmann::json::array();
    for (const auto& member : meta.party_snapshot) {
        root["_party_snapshot"].push_back({
            {"name", member.name},
            {"level", member.level},
            {"hp", member.hp},
            {"max_hp", member.max_hp}
        });
    }

    if (!meta.custom_metadata.empty()) {
        root["_custom_metadata"] = meta.custom_metadata;
    }

    return root;
}

void RemovePathIfPresent(const std::filesystem::path& path) {
    if (path.empty()) {
        return;
    }

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

} // namespace

bool SaveCatalog::upsert(SaveCatalogEntry entry) {
    if (!IsValidSlotId(entry.meta.slot_id)) {
        return false;
    }

    entries_[entry.meta.slot_id] = std::move(entry);
    return true;
}

bool SaveCatalog::erase(int32_t slot_id) {
    if (!IsValidSlotId(slot_id)) {
        return false;
    }
    return entries_.erase(slot_id) > 0;
}

void SaveCatalog::clear() {
    entries_.clear();
}

const SaveCatalogEntry* SaveCatalog::find(int32_t slot_id) const {
    const auto it = entries_.find(slot_id);
    if (it == entries_.end()) {
        return nullptr;
    }
    return &it->second;
}

SaveCatalogEntry* SaveCatalog::find(int32_t slot_id) {
    const auto it = entries_.find(slot_id);
    if (it == entries_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<SaveCatalogEntry> SaveCatalog::listEntries(bool include_autosave) const {
    std::vector<SaveCatalogEntry> entries;
    entries.reserve(entries_.size());
    for (const auto& [slotId, entry] : entries_) {
        (void)slotId;
        if (!include_autosave && entry.meta.flags.autosave) {
            continue;
        }
        entries.push_back(entry);
    }

    std::sort(entries.begin(), entries.end(), [](const SaveCatalogEntry& lhs, const SaveCatalogEntry& rhs) {
        return lhs.meta.slot_id < rhs.meta.slot_id;
    });
    return entries;
}

SaveSessionCoordinator::SaveSessionCoordinator(SaveCatalog& catalog)
    : catalog_(catalog) {}

const SaveAutosavePolicy& SaveSessionCoordinator::autosavePolicy() const {
    return autosave_policy_;
}

void SaveSessionCoordinator::setAutosavePolicy(const SaveAutosavePolicy& policy) {
    autosave_policy_ = policy;
}

const SaveRetentionPolicy& SaveSessionCoordinator::retentionPolicy() const {
    return retention_policy_;
}

void SaveSessionCoordinator::setRetentionPolicy(const SaveRetentionPolicy& policy) {
    retention_policy_ = policy;
}

bool SaveSessionCoordinator::loadSavePolicies(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        return false;
    }

    try {
        nlohmann::json data = nlohmann::json::parse(f);
        
        // 1. Metadata Fields
        if (data.contains("metadata_fields")) {
            metadata_registry_.loadFromSchema(data);
        }

        // 2. Retention Policy
        if (data.contains("retention")) {
            const auto& r = data["retention"];
            retention_policy_.max_autosave_slots = r.value("max_autosave_slots", retention_policy_.max_autosave_slots);
            retention_policy_.max_quicksave_slots = r.value("max_quicksave_slots", retention_policy_.max_quicksave_slots);
            retention_policy_.max_manual_slots = r.value("max_manual_slots", retention_policy_.max_manual_slots);
            retention_policy_.prune_excess_on_save = r.value("prune_excess_on_save", retention_policy_.prune_excess_on_save);
        }

        // 3. Autosave Policy
        if (data.contains("autosave")) {
            const auto& a = data["autosave"];
            autosave_policy_.enabled = a.value("enabled", autosave_policy_.enabled);
            autosave_policy_.slot_id = a.value("slot_id", autosave_policy_.slot_id);
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool SaveSessionCoordinator::canAutosave() const {
    return autosave_policy_.enabled;
}

int32_t SaveSessionCoordinator::autosaveSlot() const {
    return autosave_policy_.slot_id;
}

SaveSessionLoadResult SaveSessionCoordinator::load(const SaveSessionLoadRequest& request) {
    SaveSessionLoadResult result;
    result.slot_id = request.slot_id;

    if (!IsValidSlotId(request.slot_id)) {
        result.error = "invalid_slot_id";
        return result;
    }

    if (request.slot_id == autosave_policy_.slot_id && !autosave_policy_.enabled) {
        result.error = "autosave_disabled";
        return result;
    }

    const RuntimeSaveLoadResult runtime_result = RuntimeSaveLoader::Load(request.runtime_request);
    if (!runtime_result.ok) {
        result.error = runtime_result.error;
        return result;
    }

    result.ok = true;
    result.boot_safe_mode = runtime_result.boot_safe_mode;
    result.loaded_from_recovery = runtime_result.loaded_from_recovery;
    result.recovery_tier = runtime_result.recovery_tier;
    result.error = runtime_result.error;
    result.active_meta = runtime_result.active_meta;
    if (!IsValidSlotId(result.active_meta.slot_id)) {
        result.active_meta.slot_id = request.slot_id;
    }
    NormalizeMeta(result.active_meta, request.slot_id, autosave_policy_.slot_id, &metadata_registry_);
    result.active_meta.flags.corrupted = (runtime_result.recovery_tier == SaveRecoveryTier::Level2MetadataVariables ||
                                          runtime_result.recovery_tier == SaveRecoveryTier::Level3SafeSkeleton);

    RuntimeSaveLoadResult catalog_result = runtime_result;
    catalog_result.active_meta = result.active_meta;
    updateCatalogEntry(request.slot_id, catalog_result);
    return result;
}

SaveSessionSaveResult SaveSessionCoordinator::save(const SaveSessionSaveRequest& request) {
    SaveSessionSaveResult result;
    result.slot_id = request.slot_id;

    if (!IsValidSlotId(request.slot_id)) {
        result.error = "invalid_slot_id";
        return result;
    }

    if (IsAutosaveSlot(request.slot_id, autosave_policy_.slot_id) && !autosave_policy_.enabled) {
        result.error = "autosave_disabled";
        return result;
    }

    if (request.primary_save_path.empty()) {
        result.error = "missing_primary_save_path";
        return result;
    }

    std::string write_error;
    if (!SaveJournal::WriteAtomically(request.primary_save_path, request.payload, &write_error)) {
        result.error = write_error.empty() ? "failed_to_write_primary_save" : write_error;
        return result;
    }

    SaveSlotMeta meta = request.meta;
    NormalizeMeta(meta, request.slot_id, autosave_policy_.slot_id, &metadata_registry_);
    meta.flags.corrupted = false;
    if (meta.data_blob_path.empty()) {
        meta.data_blob_path = request.primary_save_path.string();
    }

    if (!request.metadata_path.empty()) {
        const std::string metadata_payload = BuildMetadataJson(meta).dump();
        if (!SaveJournal::WriteAtomically(request.metadata_path, metadata_payload, &write_error)) {
            result.error = write_error.empty() ? "failed_to_write_metadata" : write_error;
            return result;
        }
    }

    if (request.write_variables_payload || !request.variables_payload.empty()) {
        if (request.variables_path.empty()) {
            result.error = "missing_variables_path";
            return result;
        }

        if (!SaveJournal::WriteAtomically(request.variables_path, request.variables_payload, &write_error)) {
            result.error = write_error.empty() ? "failed_to_write_variables" : write_error;
            return result;
        }
    }

    SaveCatalogEntry entry;
    if (const SaveCatalogEntry* existing = catalog_.find(request.slot_id)) {
        entry = *existing;
    }

    entry.meta = meta;
    entry.last_recovery_tier = SaveRecoveryTier::None;
    entry.has_primary_payload = true;
    entry.has_variables_payload = request.write_variables_payload || !request.variables_payload.empty();
    entry.diagnostic.clear();
    entry.last_operation = "save";
    entry.save_sequence = next_save_sequence_++;
    entry.primary_save_path = request.primary_save_path;
    entry.metadata_path = request.metadata_path;
    entry.variables_path = request.variables_path;
    catalog_.upsert(entry);

    result.ok = true;
    result.active_meta = meta;
    result.pruned_slot_ids = pruneRetainedSlots(request.slot_id);
    return result;
}

void SaveSessionCoordinator::updateCatalogEntry(int32_t slot_id, const RuntimeSaveLoadResult& runtime_result) {
    SaveCatalogEntry entry;
    if (const SaveCatalogEntry* existing = catalog_.find(slot_id)) {
        entry = *existing;
    }

    entry.meta = runtime_result.active_meta;
    NormalizeMeta(entry.meta, slot_id, autosave_policy_.slot_id, &metadata_registry_);
    entry.meta.flags.corrupted = (runtime_result.recovery_tier == SaveRecoveryTier::Level2MetadataVariables ||
                                  runtime_result.recovery_tier == SaveRecoveryTier::Level3SafeSkeleton);
    entry.last_recovery_tier = runtime_result.recovery_tier;
    entry.has_primary_payload = !runtime_result.payload.empty();
    entry.has_variables_payload = !runtime_result.variables_payload.empty();
    entry.diagnostic = runtime_result.error;
    entry.last_operation = "load";
    catalog_.upsert(std::move(entry));
}

std::vector<int32_t> SaveSessionCoordinator::pruneRetainedSlots(int32_t protected_slot_id) {
    std::vector<int32_t> pruned_slot_ids;
    if (!retention_policy_.prune_excess_on_save) {
        return pruned_slot_ids;
    }

    const std::vector<SaveRetentionClass> retention_classes = {
        SaveRetentionClass::Autosave,
        SaveRetentionClass::Quicksave,
        SaveRetentionClass::Manual,
    };

    for (SaveRetentionClass retentionClass : retention_classes) {
        std::vector<SaveCatalogEntry> entries;
        for (const auto& entry : catalog_.listEntries()) {
            if (entry.meta.retention_class != retentionClass) {
                continue;
            }
            entries.push_back(entry);
        }

        const size_t limit = RetentionLimitForClass(retention_policy_, retentionClass);
        if (entries.size() <= limit) {
            continue;
        }

        std::sort(entries.begin(), entries.end(), [](const SaveCatalogEntry& lhs, const SaveCatalogEntry& rhs) {
            if (lhs.save_sequence != rhs.save_sequence) {
                return lhs.save_sequence < rhs.save_sequence;
            }
            return lhs.meta.slot_id < rhs.meta.slot_id;
        });

        size_t remaining = entries.size();
        for (const auto& entry : entries) {
            if (remaining <= limit) {
                break;
            }
            if (entry.meta.slot_id == protected_slot_id) {
                continue;
            }

            RemovePathIfPresent(entry.primary_save_path);
            RemovePathIfPresent(entry.metadata_path);
            RemovePathIfPresent(entry.variables_path);
            if (catalog_.erase(entry.meta.slot_id)) {
                pruned_slot_ids.push_back(entry.meta.slot_id);
                --remaining;
            }
        }
    }

    return pruned_slot_ids;
}

} // namespace urpg