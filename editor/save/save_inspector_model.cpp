#include "editor/save/save_inspector_model.h"

#include <utility>

namespace urpg::editor {

namespace {

std::string RecoveryLabel(urpg::SaveRecoveryTier tier) {
    switch (tier) {
    case urpg::SaveRecoveryTier::None:
        return "primary";
    case urpg::SaveRecoveryTier::Level1Autosave:
        return "autosave_recovery";
    case urpg::SaveRecoveryTier::Level2MetadataVariables:
        return "metadata_variables_recovery";
    case urpg::SaveRecoveryTier::Level3SafeSkeleton:
        return "safe_mode_recovery";
    }
    return "unknown";
}

std::string BuildSummary(const urpg::SaveCatalogEntry& entry) {
    std::string summary = entry.last_operation.empty() ? "untracked" : entry.last_operation;
    summary += " / ";
    summary += ToString(entry.meta.category);
    summary += " / " + RecoveryLabel(entry.last_recovery_tier);
    summary += " / retained:";
    summary += ToString(entry.meta.retention_class);
    if (entry.meta.flags.corrupted) {
        summary += " / corrupted";
    }
    return summary;
}

bool IsProblemRow(const urpg::SaveCatalogEntry& entry) {
    return entry.meta.flags.corrupted || entry.last_recovery_tier != urpg::SaveRecoveryTier::None || !entry.diagnostic.empty();
}

} // namespace

void SaveInspectorModel::LoadFromCatalog(const urpg::SaveCatalog& catalog,
                                         const urpg::SaveSessionCoordinator& coordinator) {
    all_entries_ = catalog.listEntries(true);
    selected_row_index_.reset();

    summary_ = {};
    summary_.autosave_enabled = coordinator.autosavePolicy().enabled;
    summary_.autosave_slot_id = coordinator.autosaveSlot();
    summary_.autosave_retention_limit = coordinator.retentionPolicy().max_autosave_slots;
    summary_.quicksave_retention_limit = coordinator.retentionPolicy().max_quicksave_slots;
    summary_.manual_retention_limit = coordinator.retentionPolicy().max_manual_slots;

    for (const auto& entry : all_entries_) {
        ++summary_.total_slots;
        if (entry.meta.category == urpg::SaveSlotCategory::Autosave) {
            ++summary_.autosave_slots;
        } else if (entry.meta.category == urpg::SaveSlotCategory::Quicksave) {
            ++summary_.quicksave_slots;
        } else {
            ++summary_.manual_slots;
        }
        if (entry.meta.flags.corrupted) {
            ++summary_.corrupted_slots;
        }
        if (entry.last_recovery_tier != urpg::SaveRecoveryTier::None) {
            ++summary_.recovery_slots;
        }
        if (entry.last_recovery_tier == urpg::SaveRecoveryTier::Level3SafeSkeleton) {
            ++summary_.safe_mode_slots;
        }
    }

    RebuildVisibleRows();
}

void SaveInspectorModel::SetShowProblemSlotsOnly(bool show_problem_slots_only) {
    show_problem_slots_only_ = show_problem_slots_only;
    selected_row_index_.reset();
    RebuildVisibleRows();
}

void SaveInspectorModel::SetIncludeAutosave(bool include_autosave) {
    include_autosave_ = include_autosave;
    selected_row_index_.reset();
    RebuildVisibleRows();
}

const SaveInspectorSummary& SaveInspectorModel::Summary() const {
    return summary_;
}

const std::vector<SaveInspectorRow>& SaveInspectorModel::VisibleRows() const {
    return visible_rows_;
}

bool SaveInspectorModel::SelectRow(size_t row_index) {
    if (row_index >= visible_rows_.size()) {
        selected_row_index_.reset();
        return false;
    }

    selected_row_index_ = row_index;
    return true;
}

std::optional<int32_t> SaveInspectorModel::SelectedSlotId() const {
    if (!selected_row_index_.has_value()) {
        return std::nullopt;
    }
    return visible_rows_[selected_row_index_.value()].slot_id;
}

void SaveInspectorModel::RebuildVisibleRows() {
    visible_rows_.clear();

    for (const auto& entry : all_entries_) {
        if (!include_autosave_ && entry.meta.flags.autosave) {
            continue;
        }
        if (show_problem_slots_only_ && !IsProblemRow(entry)) {
            continue;
        }

        SaveInspectorRow row;
        row.slot_id = entry.meta.slot_id;
        row.autosave = entry.meta.category == urpg::SaveSlotCategory::Autosave;
        row.corrupted = entry.meta.flags.corrupted;
        row.loaded_from_recovery = entry.last_recovery_tier != urpg::SaveRecoveryTier::None;
        row.boot_safe_mode = entry.last_recovery_tier == urpg::SaveRecoveryTier::Level3SafeSkeleton;
        row.category = entry.meta.category;
        row.retention_class = entry.meta.retention_class;
        row.map_display_name = entry.meta.map_display_name;
        row.operation = entry.last_operation;
        row.category_label = ToString(entry.meta.category);
        row.retention_label = ToString(entry.meta.retention_class);
        row.recovery_label = RecoveryLabel(entry.last_recovery_tier);
        row.summary = BuildSummary(entry);
        row.diagnostic = entry.diagnostic;
        visible_rows_.push_back(std::move(row));
    }
}

} // namespace urpg::editor