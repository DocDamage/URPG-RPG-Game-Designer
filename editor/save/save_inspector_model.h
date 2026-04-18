#pragma once

#include "engine/core/save/save_catalog.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

struct SaveInspectorSummary {
    size_t total_slots = 0;
    size_t autosave_slots = 0;
    size_t quicksave_slots = 0;
    size_t manual_slots = 0;
    size_t corrupted_slots = 0;
    size_t recovery_slots = 0;
    size_t safe_mode_slots = 0;
    bool autosave_enabled = true;
    int32_t autosave_slot_id = 0;
    size_t autosave_retention_limit = 0;
    size_t quicksave_retention_limit = 0;
    size_t manual_retention_limit = 0;
    size_t reserved_slots = 0;
};

struct SaveInspectorRow {
    int32_t slot_id = -1;
    bool reserved_slot = false;
    bool autosave = false;
    bool corrupted = false;
    bool loaded_from_recovery = false;
    bool boot_safe_mode = false;
    urpg::SaveSlotCategory category = urpg::SaveSlotCategory::Manual;
    urpg::SaveRetentionClass retention_class = urpg::SaveRetentionClass::Manual;
    std::string slot_label;
    std::string map_display_name;
    std::string operation;
    std::string category_label;
    std::string retention_label;
    std::string recovery_label;
    std::string summary;
    std::string diagnostic;
};

class SaveInspectorModel {
public:
    void LoadFromCatalog(const urpg::SaveCatalog& catalog,
                         const urpg::SaveSessionCoordinator& coordinator);
    void SetShowProblemSlotsOnly(bool show_problem_slots_only);
    void SetIncludeAutosave(bool include_autosave);

    const SaveInspectorSummary& Summary() const;
    const std::vector<SaveInspectorRow>& VisibleRows() const;

    bool SelectRow(size_t row_index);
    std::optional<int32_t> SelectedSlotId() const;

private:
    void RebuildVisibleRows();

    std::vector<urpg::SaveCatalogEntry> all_entries_;
    std::vector<SaveInspectorRow> visible_rows_;
    SaveInspectorSummary summary_;
    std::map<int32_t, urpg::SaveSlotDescriptor> slot_descriptors_by_slot_id_;
    std::optional<size_t> selected_row_index_;
    bool show_problem_slots_only_ = false;
    bool include_autosave_ = true;
};

} // namespace urpg::editor
