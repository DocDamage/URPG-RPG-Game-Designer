#include "editor/project/project_recovery_coordinator.h"

#include <algorithm>
#include <set>

namespace urpg::editor {

std::string_view projectRecoveryClassName(const ProjectRecoveryClass recovery_class) {
    switch (recovery_class) {
    case ProjectRecoveryClass::PrimaryDocument: return "primary_document";
    case ProjectRecoveryClass::AssetJob: return "asset_job";
    case ProjectRecoveryClass::ExternalChangeConflict: return "external_change_conflict";
    case ProjectRecoveryClass::LayoutOrSettings: return "layout_or_settings";
    case ProjectRecoveryClass::Migration: return "migration";
    case ProjectRecoveryClass::Package: return "package";
    }
    return "unknown";
}

std::string_view projectRecoveryDispositionName(const ProjectRecoveryDisposition disposition) {
    switch (disposition) {
    case ProjectRecoveryDisposition::RestorePrivateCopy: return "restore_private_copy";
    case ProjectRecoveryDisposition::ResumeOrRetry: return "resume_or_retry";
    case ProjectRecoveryDisposition::RequireUserResolution: return "require_user_resolution";
    case ProjectRecoveryDisposition::ResetSafeDefaults: return "reset_safe_defaults";
    case ProjectRecoveryDisposition::RestoreMigrationBackup: return "restore_migration_backup";
    case ProjectRecoveryDisposition::PreserveSourcesAndRetry: return "preserve_sources_and_retry";
    case ProjectRecoveryDisposition::PreserveFailure: return "preserve_failure";
    }
    return "preserve_failure";
}

const std::vector<std::string>& primaryRecoveryDocumentTypes() {
    static const std::vector<std::string> types = {
        "project", "map", "event", "dialogue", "quest", "database", "localization",
        "audio", "script", "plugin", "ui", "input", "save"};
    return types;
}

ProjectRecoveryCoverage ProjectRecoveryCoordinator::evaluate(const std::vector<ProjectRecoveryFault>& faults) const {
    ProjectRecoveryCoverage coverage;
    std::set<std::string> fault_ids;
    std::set<std::string> covered_documents;
    std::set<ProjectRecoveryClass> covered_classes;
    for (const auto& fault : faults) {
        if (fault.id.empty() || fault.owner.empty() || !fault_ids.insert(fault.id).second) {
            coverage.diagnostics.push_back("recovery_fault_invalid_or_duplicate:" + fault.id);
            continue;
        }
        if (!fault.failed) continue;
        ProjectRecoveryAction action;
        action.fault_id = fault.id;
        action.recovery_class = fault.recovery_class;
        action.owner = fault.owner;
        action.project_data_preserved = true;
        covered_classes.insert(fault.recovery_class);
        if (!fault.diagnostic.empty()) action.diagnostics.push_back(fault.diagnostic);

        switch (fault.recovery_class) {
        case ProjectRecoveryClass::PrimaryDocument:
            covered_documents.insert(fault.owner);
            action.disposition = fault.recovery_artifact_available
                ? ProjectRecoveryDisposition::RestorePrivateCopy : ProjectRecoveryDisposition::PreserveFailure;
            action.action = fault.recovery_artifact_available
                ? "Offer the private recovery copy beside the authoritative document."
                : "Preserve the authoritative document and report that no recovery copy exists.";
            action.requires_user_action = true;
            break;
        case ProjectRecoveryClass::AssetJob:
            action.disposition = fault.recovery_artifact_available
                ? ProjectRecoveryDisposition::ResumeOrRetry : ProjectRecoveryDisposition::PreserveFailure;
            action.action = fault.recovery_artifact_available
                ? "Resume or retry from the validated asset-job checkpoint."
                : "Preserve staged inputs and report that the job has no valid checkpoint.";
            action.requires_user_action = true;
            break;
        case ProjectRecoveryClass::ExternalChangeConflict:
            action.disposition = ProjectRecoveryDisposition::RequireUserResolution;
            action.action = fault.local_changes_present
                ? "Require compare, reload, or keep-local resolution before continuing."
                : "Offer reload after showing the external change.";
            action.requires_user_action = true;
            break;
        case ProjectRecoveryClass::LayoutOrSettings:
            action.disposition = ProjectRecoveryDisposition::ResetSafeDefaults;
            action.action = "Quarantine the corrupted preferences and restore safe defaults.";
            action.automatic = true;
            break;
        case ProjectRecoveryClass::Migration:
            action.disposition = fault.recovery_artifact_available
                ? ProjectRecoveryDisposition::RestoreMigrationBackup : ProjectRecoveryDisposition::PreserveFailure;
            action.action = fault.recovery_artifact_available
                ? "Restore the pre-migration backup and retain migration diagnostics."
                : "Stop migration without publishing the partial result.";
            action.automatic = fault.recovery_artifact_available;
            action.requires_user_action = !fault.recovery_artifact_available;
            break;
        case ProjectRecoveryClass::Package:
            action.disposition = ProjectRecoveryDisposition::PreserveSourcesAndRetry;
            action.action = "Discard partial package output, preserve project sources, and offer retry.";
            action.requires_user_action = true;
            break;
        }
        coverage.actions.push_back(std::move(action));
    }

    for (const auto& document : primaryRecoveryDocumentTypes()) {
        if (!covered_documents.contains(document)) coverage.missing_primary_documents.push_back(document);
    }
    for (const auto required_class : {ProjectRecoveryClass::AssetJob, ProjectRecoveryClass::ExternalChangeConflict,
                                      ProjectRecoveryClass::LayoutOrSettings, ProjectRecoveryClass::Migration,
                                      ProjectRecoveryClass::Package}) {
        if (!covered_classes.contains(required_class)) {
            coverage.diagnostics.push_back("recovery_class_missing:" + std::string(projectRecoveryClassName(required_class)));
        }
    }
    std::stable_sort(coverage.actions.begin(), coverage.actions.end(), [](const auto& left, const auto& right) {
        return left.fault_id < right.fault_id;
    });
    coverage.complete = coverage.missing_primary_documents.empty() && coverage.diagnostics.empty();
    coverage.code = coverage.complete ? "project_recovery_coverage_complete" : "project_recovery_coverage_incomplete";
    return coverage;
}

} // namespace urpg::editor
