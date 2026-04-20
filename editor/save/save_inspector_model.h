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
    bool prune_excess_on_save = true;
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

struct SaveInspectorMetadataFieldRow {
    std::string key;
    std::string display_label;
    bool required = false;
    std::string default_value;
};

struct SaveRecoveryDiagnosticsSummary {
    size_t total_recovery_slots = 0;
    size_t autosave_recovery_slots = 0;
    size_t metadata_variables_recovery_slots = 0;
    size_t safe_mode_recovery_slots = 0;
    size_t corrupted_slots = 0;
    size_t diagnostic_rows = 0;
};

struct SaveSerializationSchemaSummary {
    std::string format_magic = "URSV";
    int32_t version_major = 1;
    int32_t version_minor = 0;
    bool differential_supported = true;
    std::vector<std::string> compression_modes;
};

enum class SavePolicyIssueSeverity : uint8_t {
    Warning,
    Error,
};

struct SavePolicyDraft {
    bool autosave_enabled = true;
    int32_t autosave_slot_id = 0;
    size_t max_autosave_slots = 1;
    size_t max_quicksave_slots = 3;
    size_t max_manual_slots = 20;
    bool prune_excess_on_save = true;
};

struct SavePolicyIssue {
    SavePolicyIssueSeverity severity = SavePolicyIssueSeverity::Warning;
    std::string code;
    std::string message;
};

struct SavePolicyValidationSummary {
    size_t issue_count = 0;
    size_t warning_count = 0;
    size_t error_count = 0;
    bool can_apply = true;
};

class SaveInspectorModel {
public:
    void LoadFromCatalog(const urpg::SaveCatalog& catalog,
                         const urpg::SaveSessionCoordinator& coordinator);
    void SetShowProblemSlotsOnly(bool show_problem_slots_only);
    void SetIncludeAutosave(bool include_autosave);

    const SaveInspectorSummary& Summary() const;
    const std::vector<SaveInspectorRow>& VisibleRows() const;
    const std::vector<SaveInspectorMetadataFieldRow>& MetadataFields() const;
    const std::vector<urpg::SaveSlotDescriptor>& SlotDescriptors() const;
    const SaveRecoveryDiagnosticsSummary& RecoveryDiagnostics() const;
    const SaveSerializationSchemaSummary& SerializationSchema() const;
    const SavePolicyDraft& PolicyDraft() const;
    const std::vector<SavePolicyIssue>& PolicyIssues() const;
    const SavePolicyValidationSummary& PolicyValidation() const;

    bool SetPolicyAutosaveEnabled(bool autosave_enabled);
    bool SetPolicyAutosaveSlotId(int32_t autosave_slot_id);
    bool SetPolicyRetentionLimits(size_t max_autosave_slots,
                                  size_t max_quicksave_slots,
                                  size_t max_manual_slots,
                                  bool prune_excess_on_save);
    bool ApplyPolicyToRuntime(urpg::SaveSessionCoordinator& coordinator);

    bool SelectRow(size_t row_index);
    std::optional<int32_t> SelectedSlotId() const;

private:
    void RebuildVisibleRows();
    void RebuildPolicyValidation();

    std::vector<urpg::SaveCatalogEntry> all_entries_;
    std::vector<SaveInspectorRow> visible_rows_;
    std::vector<SaveInspectorMetadataFieldRow> metadata_fields_;
    std::vector<urpg::SaveSlotDescriptor> slot_descriptors_;
    SaveInspectorSummary summary_;
    SaveRecoveryDiagnosticsSummary recovery_diagnostics_;
    SaveSerializationSchemaSummary serialization_schema_;
    SavePolicyDraft policy_draft_;
    std::vector<SavePolicyIssue> policy_issues_;
    SavePolicyValidationSummary policy_validation_;
    std::map<int32_t, urpg::SaveSlotDescriptor> slot_descriptors_by_slot_id_;
    std::optional<size_t> selected_row_index_;
    bool show_problem_slots_only_ = false;
    bool include_autosave_ = true;
};

} // namespace urpg::editor
