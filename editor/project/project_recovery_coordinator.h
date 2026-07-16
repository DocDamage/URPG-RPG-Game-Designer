#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

enum class ProjectRecoveryClass : uint8_t {
    PrimaryDocument,
    AssetJob,
    ExternalChangeConflict,
    LayoutOrSettings,
    Migration,
    Package
};

enum class ProjectRecoveryDisposition : uint8_t {
    RestorePrivateCopy,
    ResumeOrRetry,
    RequireUserResolution,
    ResetSafeDefaults,
    RestoreMigrationBackup,
    PreserveSourcesAndRetry,
    PreserveFailure
};

std::string_view projectRecoveryClassName(ProjectRecoveryClass recovery_class);
std::string_view projectRecoveryDispositionName(ProjectRecoveryDisposition disposition);
const std::vector<std::string>& primaryRecoveryDocumentTypes();

struct ProjectRecoveryFault {
    std::string id;
    ProjectRecoveryClass recovery_class = ProjectRecoveryClass::PrimaryDocument;
    std::string owner;
    bool failed = true;
    bool recovery_artifact_available = false;
    bool local_changes_present = false;
    std::string diagnostic;
};

struct ProjectRecoveryAction {
    std::string fault_id;
    ProjectRecoveryClass recovery_class = ProjectRecoveryClass::PrimaryDocument;
    ProjectRecoveryDisposition disposition = ProjectRecoveryDisposition::PreserveFailure;
    std::string owner;
    std::string action;
    bool automatic = false;
    bool project_data_preserved = true;
    bool requires_user_action = false;
    std::vector<std::string> diagnostics;
};

struct ProjectRecoveryCoverage {
    bool complete = false;
    std::string code;
    std::vector<ProjectRecoveryAction> actions;
    std::vector<std::string> missing_primary_documents;
    std::vector<std::string> diagnostics;
};

class ProjectRecoveryCoordinator {
public:
    ProjectRecoveryCoverage evaluate(const std::vector<ProjectRecoveryFault>& faults) const;
};

} // namespace urpg::editor
