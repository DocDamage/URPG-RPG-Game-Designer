#include "editor/save/save_inspector_model.h"

#include "engine/core/save/save_serialization_hub.h"

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

std::string BuildSummary(const urpg::SaveCatalogEntry& entry, bool reserved_slot) {
    std::string summary = entry.last_operation.empty() ? "untracked" : entry.last_operation;
    summary += " / ";
    summary += ToString(entry.meta.category);
    summary += " / " + RecoveryLabel(entry.last_recovery_tier);
    summary += " / retained:";
    summary += ToString(entry.meta.retention_class);
    if (reserved_slot) {
        summary += " / reserved";
    }
    if (entry.meta.flags.corrupted) {
        summary += " / corrupted";
    }
    return summary;
}

bool IsProblemRow(const urpg::SaveCatalogEntry& entry) {
    return entry.meta.flags.corrupted || entry.last_recovery_tier != urpg::SaveRecoveryTier::None || !entry.diagnostic.empty();
}

void AddPolicyIssue(std::vector<SavePolicyIssue>& issues,
                    SavePolicyValidationSummary& summary,
                    SavePolicyIssueSeverity severity,
                    std::string code,
                    std::string message) {
    issues.push_back(SavePolicyIssue{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
    });
    ++summary.issue_count;
    if (severity == SavePolicyIssueSeverity::Error) {
        ++summary.error_count;
        summary.can_apply = false;
    } else {
        ++summary.warning_count;
    }
}

} // namespace

void SaveInspectorModel::LoadFromCatalog(const urpg::SaveCatalog& catalog,
                                         const urpg::SaveSessionCoordinator& coordinator) {
    const auto previously_selected_slot_id = SelectedSlotId();
    all_entries_ = catalog.listEntries(true);
    slot_descriptors_by_slot_id_.clear();
    slot_descriptors_ = coordinator.slotDescriptors();
    for (const auto& descriptor : slot_descriptors_) {
        slot_descriptors_by_slot_id_[descriptor.slot_id] = descriptor;
    }
    metadata_fields_.clear();
    for (const auto& [key, field] : coordinator.metadataRegistry().getFields()) {
        metadata_fields_.push_back({field.key, field.display_label, field.required, field.default_value});
    }

    summary_ = {};
    recovery_diagnostics_ = {};
    serialization_schema_ = {
        .format_magic = "URSV",
        .version_major = 1,
        .version_minor = 0,
        .differential_supported = true,
        .compression_modes = {"none", "fast", "optimal"},
    };
    policy_draft_ = {
        .autosave_enabled = coordinator.autosavePolicy().enabled,
        .autosave_slot_id = coordinator.autosaveSlot(),
        .max_autosave_slots = coordinator.retentionPolicy().max_autosave_slots,
        .max_quicksave_slots = coordinator.retentionPolicy().max_quicksave_slots,
        .max_manual_slots = coordinator.retentionPolicy().max_manual_slots,
        .prune_excess_on_save = coordinator.retentionPolicy().prune_excess_on_save,
    };
    summary_.autosave_enabled = coordinator.autosavePolicy().enabled;
    summary_.autosave_slot_id = coordinator.autosaveSlot();
    summary_.autosave_retention_limit = coordinator.retentionPolicy().max_autosave_slots;
    summary_.quicksave_retention_limit = coordinator.retentionPolicy().max_quicksave_slots;
    summary_.manual_retention_limit = coordinator.retentionPolicy().max_manual_slots;
    summary_.prune_excess_on_save = coordinator.retentionPolicy().prune_excess_on_save;
    summary_.reserved_slots = 0;
    for (const auto& descriptor : coordinator.slotDescriptors()) {
        if (descriptor.reserved) {
            ++summary_.reserved_slots;
        }
    }

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
            ++recovery_diagnostics_.corrupted_slots;
        }
        if (entry.last_recovery_tier != urpg::SaveRecoveryTier::None) {
            ++summary_.recovery_slots;
            ++recovery_diagnostics_.total_recovery_slots;
        }
        if (entry.last_recovery_tier == urpg::SaveRecoveryTier::Level3SafeSkeleton) {
            ++summary_.safe_mode_slots;
        }
        switch (entry.last_recovery_tier) {
        case urpg::SaveRecoveryTier::None:
            break;
        case urpg::SaveRecoveryTier::Level1Autosave:
            ++recovery_diagnostics_.autosave_recovery_slots;
            break;
        case urpg::SaveRecoveryTier::Level2MetadataVariables:
            ++recovery_diagnostics_.metadata_variables_recovery_slots;
            break;
        case urpg::SaveRecoveryTier::Level3SafeSkeleton:
            ++recovery_diagnostics_.safe_mode_recovery_slots;
            break;
        }
        if (!entry.diagnostic.empty()) {
            ++recovery_diagnostics_.diagnostic_rows;
        }
    }
    RebuildPolicyValidation();

    RebuildVisibleRows();
    if (previously_selected_slot_id.has_value() && !SelectedSlotId().has_value()) {
        for (size_t index = 0; index < visible_rows_.size(); ++index) {
            if (visible_rows_[index].slot_id == *previously_selected_slot_id) {
                selected_row_index_ = index;
                break;
            }
        }
    }
}

void SaveInspectorModel::SetShowProblemSlotsOnly(bool show_problem_slots_only) {
    show_problem_slots_only_ = show_problem_slots_only;
    RebuildVisibleRows();
}

void SaveInspectorModel::SetIncludeAutosave(bool include_autosave) {
    include_autosave_ = include_autosave;
    RebuildVisibleRows();
}

const SaveInspectorSummary& SaveInspectorModel::Summary() const {
    return summary_;
}

const std::vector<SaveInspectorRow>& SaveInspectorModel::VisibleRows() const {
    return visible_rows_;
}

const std::vector<SaveInspectorMetadataFieldRow>& SaveInspectorModel::MetadataFields() const {
    return metadata_fields_;
}

const std::vector<urpg::SaveSlotDescriptor>& SaveInspectorModel::SlotDescriptors() const {
    return slot_descriptors_;
}

const SaveRecoveryDiagnosticsSummary& SaveInspectorModel::RecoveryDiagnostics() const {
    return recovery_diagnostics_;
}

const SaveSerializationSchemaSummary& SaveInspectorModel::SerializationSchema() const {
    return serialization_schema_;
}

const SavePolicyDraft& SaveInspectorModel::PolicyDraft() const {
    return policy_draft_;
}

const std::vector<SavePolicyIssue>& SaveInspectorModel::PolicyIssues() const {
    return policy_issues_;
}

const SavePolicyValidationSummary& SaveInspectorModel::PolicyValidation() const {
    return policy_validation_;
}

bool SaveInspectorModel::SetPolicyAutosaveEnabled(bool autosave_enabled) {
    policy_draft_.autosave_enabled = autosave_enabled;
    RebuildPolicyValidation();
    return true;
}

bool SaveInspectorModel::SetPolicyAutosaveSlotId(int32_t autosave_slot_id) {
    policy_draft_.autosave_slot_id = autosave_slot_id;
    RebuildPolicyValidation();
    return true;
}

bool SaveInspectorModel::SetPolicyRetentionLimits(size_t max_autosave_slots,
                                                  size_t max_quicksave_slots,
                                                  size_t max_manual_slots,
                                                  bool prune_excess_on_save) {
    policy_draft_.max_autosave_slots = max_autosave_slots;
    policy_draft_.max_quicksave_slots = max_quicksave_slots;
    policy_draft_.max_manual_slots = max_manual_slots;
    policy_draft_.prune_excess_on_save = prune_excess_on_save;
    RebuildPolicyValidation();
    return true;
}

bool SaveInspectorModel::ApplyPolicyToRuntime(urpg::SaveSessionCoordinator& coordinator) {
    if (!policy_validation_.can_apply) {
        return false;
    }

    coordinator.setAutosavePolicy({
        .enabled = policy_draft_.autosave_enabled,
        .slot_id = policy_draft_.autosave_slot_id,
    });
    coordinator.setRetentionPolicy({
        .max_autosave_slots = policy_draft_.max_autosave_slots,
        .max_quicksave_slots = policy_draft_.max_quicksave_slots,
        .max_manual_slots = policy_draft_.max_manual_slots,
        .prune_excess_on_save = policy_draft_.prune_excess_on_save,
    });
    return true;
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
    const auto selected_slot_id = SelectedSlotId();
    visible_rows_.clear();
    selected_row_index_.reset();

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
        if (const auto descriptorIt = slot_descriptors_by_slot_id_.find(entry.meta.slot_id);
            descriptorIt != slot_descriptors_by_slot_id_.end()) {
            row.reserved_slot = descriptorIt->second.reserved;
            row.slot_label = descriptorIt->second.label;
            if (entry.meta.map_display_name.empty()) {
                row.map_display_name = descriptorIt->second.label;
            } else {
                row.map_display_name = entry.meta.map_display_name;
            }
        } else {
            row.map_display_name = entry.meta.map_display_name;
        }
        row.operation = entry.last_operation;
        row.category_label = ToString(entry.meta.category);
        row.retention_label = ToString(entry.meta.retention_class);
        row.recovery_label = RecoveryLabel(entry.last_recovery_tier);
        row.summary = BuildSummary(entry, row.reserved_slot);
        row.diagnostic = entry.diagnostic;
        visible_rows_.push_back(std::move(row));
        if (selected_slot_id.has_value() && visible_rows_.back().slot_id == *selected_slot_id) {
            selected_row_index_ = visible_rows_.size() - 1;
        }
    }
}

void SaveInspectorModel::RebuildPolicyValidation() {
    policy_issues_.clear();
    policy_validation_ = {};

    if (policy_draft_.autosave_slot_id < 0) {
        AddPolicyIssue(policy_issues_, policy_validation_, SavePolicyIssueSeverity::Error,
                       "invalid_autosave_slot_id", "Autosave slot id must be zero or greater.");
    }
    if (policy_draft_.autosave_enabled && policy_draft_.max_autosave_slots == 0) {
        AddPolicyIssue(policy_issues_, policy_validation_, SavePolicyIssueSeverity::Error,
                       "autosave_retention_required", "Autosave retention must be at least 1 when autosave is enabled.");
    }
    if (policy_draft_.max_quicksave_slots == 0) {
        AddPolicyIssue(policy_issues_, policy_validation_, SavePolicyIssueSeverity::Warning,
                       "quicksave_retention_zero", "Quicksave retention is 0; quicksave history will be pruned immediately.");
    }
    if (policy_draft_.max_manual_slots == 0) {
        AddPolicyIssue(policy_issues_, policy_validation_, SavePolicyIssueSeverity::Warning,
                       "manual_retention_zero", "Manual retention is 0; manual saves will not be retained.");
    }
}

} // namespace urpg::editor
