#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace urpg::project {

struct ProjectOperationOwnerSnapshot {
    std::string owner_id;
    uint64_t revision = 0;
    std::string snapshot;
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
};

class ProjectOperationJournal {
public:
    explicit ProjectOperationJournal(std::filesystem::path path,
                                     std::function<bool()> before_atomic_replace = {});

    bool recordPrepared(std::string operation_id, std::string label,
                        std::vector<ProjectOperationOwnerSnapshot> owners, std::string* diagnostic = nullptr);
    bool acknowledgeCommitted(std::string operation_id, std::string label,
                              std::vector<ProjectOperationOwnerSnapshot> owners, std::string* diagnostic = nullptr);
    ProjectOperationRecoveryState recover() const;

private:
    bool append(ProjectOperationJournalEntry entry, std::string* diagnostic);

    std::filesystem::path path_;
    std::function<bool()> before_atomic_replace_;
};

} // namespace urpg::project
