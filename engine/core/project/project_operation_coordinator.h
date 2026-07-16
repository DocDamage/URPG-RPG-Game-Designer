#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace urpg::project {

struct ProjectOperationParticipant {
    std::string owner_id;
    uint64_t expected_source_revision = 0;
    std::function<uint64_t()> current_revision;
    std::function<bool(std::string&)> prepare;
    std::function<bool(std::string&)> commit;
    std::function<void()> rollback;
    std::function<bool(std::string&)> inverse;
};

struct ProjectOperationRequest {
    std::string operation_id;
    std::string label;
    std::vector<ProjectOperationParticipant> participants;
};

struct ProjectOperationResult {
    bool success = false;
    bool replayed = false;
    std::string code;
    std::string message;
    std::vector<std::string> committed_owners;
};

// Coordinates typed domain owners without becoming a mutable document store.
// Participants validate and mutate their own documents and supply rollback and
// inverse behavior for the exact revisions admitted by the request.
class ProjectOperationCoordinator {
public:
    ProjectOperationResult execute(const ProjectOperationRequest& request);
    ProjectOperationResult undoLast();
    ProjectOperationResult redoLast();

    size_t historySize() const { return history_.size(); }
    size_t redoSize() const { return redo_.size(); }
    std::string undoLabel() const;
    std::string redoLabel() const;

private:
    struct HistoryEntry {
        std::string operation_id;
        std::string label;
        std::vector<ProjectOperationParticipant> participants;
        std::vector<uint64_t> post_revisions;
        ProjectOperationResult result;
    };

    std::vector<HistoryEntry> history_;
    std::vector<HistoryEntry> redo_;
};

} // namespace urpg::project
