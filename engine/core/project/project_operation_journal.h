#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace urpg::project {

struct ProjectOperationOwnerSnapshot {
    std::string owner_id;
    uint64_t revision = 0;
    std::string snapshot;
    std::filesystem::path document_path;

    ProjectOperationOwnerSnapshot() = default;
    ProjectOperationOwnerSnapshot(std::string owner, uint64_t source_revision, std::string state,
                                  std::filesystem::path path = {})
        : owner_id(std::move(owner)), revision(source_revision), snapshot(std::move(state)),
          document_path(std::move(path)) {}
};

struct ProjectOperationJournalEntry {
    std::string operation_id;
    std::string label;
    std::string state;
    std::vector<ProjectOperationOwnerSnapshot> owners;
};

struct ProjectOperationRecoveryState {
    bool success = false;
    std::string code;
    bool has_acknowledged_operation = false;
    ProjectOperationJournalEntry last_acknowledged;
    std::vector<std::string> interrupted_operation_ids;
    std::vector<ProjectOperationJournalEntry> interrupted_operations;
};

struct ProjectOperationRecoveryOwner {
    std::string owner_id;
    std::function<bool(const ProjectOperationOwnerSnapshot&, std::string&)> prepare;
    std::function<bool(const ProjectOperationOwnerSnapshot&, std::string&)> restore;
    std::function<void()> rollback;
};

struct ProjectOperationRestoreResult {
    bool success = false;
    std::string code;
    std::vector<std::string> restored_operation_ids;
    std::string failed_operation_id;
    std::string failed_owner_id;
    std::string diagnostic;
};

class ProjectOperationJournal {
public:
    explicit ProjectOperationJournal(std::filesystem::path path,
                                     std::function<bool()> before_atomic_replace = {});

    bool recordPrepared(std::string operation_id, std::string label,
                        std::vector<ProjectOperationOwnerSnapshot> owners, std::string* diagnostic = nullptr);
    bool acknowledgeCommitted(std::string operation_id, std::string label,
                              std::vector<ProjectOperationOwnerSnapshot> owners, std::string* diagnostic = nullptr);
    bool recordAborted(std::string operation_id, std::string label,
                       std::vector<ProjectOperationOwnerSnapshot> owners, std::string* diagnostic = nullptr);
    ProjectOperationRecoveryState recover() const;
    ProjectOperationRestoreResult restoreInterrupted(const std::vector<ProjectOperationRecoveryOwner>& owners);

private:
    bool append(ProjectOperationJournalEntry entry, std::string* diagnostic);

    std::filesystem::path path_;
    std::function<bool()> before_atomic_replace_;
};

} // namespace urpg::project
