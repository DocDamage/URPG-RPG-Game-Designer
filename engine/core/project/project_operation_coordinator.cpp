#include "engine/core/project/project_operation_coordinator.h"

#include <algorithm>
#include <set>

namespace urpg::project {

ProjectOperationResult ProjectOperationCoordinator::execute(const ProjectOperationRequest& request) {
    if (request.operation_id.empty() || request.label.empty() || request.participants.empty()) {
        return {false, false, "project_operation_request_invalid",
                "Project operations require a stable ID, label, and at least one domain owner.", {}};
    }
    const auto replay = std::find_if(history_.begin(), history_.end(), [&](const auto& entry) {
        return entry.operation_id == request.operation_id;
    });
    if (replay != history_.end()) {
        auto result = replay->result;
        result.replayed = true;
        result.message = "Completed project operation returned without reapplying it.";
        return result;
    }

    std::set<std::string> owners;
    for (const auto& participant : request.participants) {
        if (participant.owner_id.empty() || !owners.insert(participant.owner_id).second ||
            !participant.current_revision || !participant.prepare || !participant.commit || !participant.rollback ||
            !participant.inverse) {
            return {false, false, "project_operation_participant_invalid",
                    "Every project operation participant requires one unique owner and complete lifecycle callbacks.", {}};
        }
        if (participant.current_revision() != participant.expected_source_revision) {
            return {false, false, "project_operation_source_revision_mismatch",
                    "A participating document changed after the operation was planned.", {}};
        }
    }

    for (const auto& participant : request.participants) {
        std::string diagnostic;
        if (!participant.prepare(diagnostic)) {
            return {false, false, "project_operation_prepare_failed",
                    diagnostic.empty() ? "A domain owner rejected operation preparation." : diagnostic, {}};
        }
    }

    std::vector<size_t> committed;
    for (size_t index = 0; index < request.participants.size(); ++index) {
        std::string diagnostic;
        if (!request.participants[index].commit(diagnostic)) {
            request.participants[index].rollback();
            for (auto committedIndex = committed.rbegin(); committedIndex != committed.rend(); ++committedIndex) {
                request.participants[*committedIndex].rollback();
            }
            return {false, false, "project_operation_commit_failed",
                    diagnostic.empty() ? "A domain owner failed commit; all admitted owners were rolled back." : diagnostic, {}};
        }
        committed.push_back(index);
    }

    ProjectOperationResult result{true, false, "project_operation_committed",
                                  "Project operation committed across all participating domain owners.", {}};
    std::vector<uint64_t> postRevisions;
    for (const auto& participant : request.participants) {
        result.committed_owners.push_back(participant.owner_id);
        postRevisions.push_back(participant.current_revision());
    }
    history_.push_back({request.operation_id, request.label, request.participants, std::move(postRevisions), result});
    redo_.clear();
    return result;
}

ProjectOperationResult ProjectOperationCoordinator::undoLast() {
    if (history_.empty()) {
        return {false, false, "project_operation_history_empty", "No project operation is available to undo.", {}};
    }
    const auto& entry = history_.back();
    for (size_t index = 0; index < entry.participants.size(); ++index) {
        if (entry.participants[index].current_revision() != entry.post_revisions[index]) {
            return {false, false, "project_operation_inverse_revision_mismatch",
                    "A participating document changed after commit; the composite inverse was not applied.", {}};
        }
    }
    for (size_t offset = entry.participants.size(); offset > 0; --offset) {
        std::string diagnostic;
        if (!entry.participants[offset - 1].inverse(diagnostic)) {
            return {false, false, "project_operation_inverse_failed",
                    diagnostic.empty() ? "A domain owner rejected the composite inverse." : diagnostic, {}};
        }
    }
    ProjectOperationResult result{true, false, "project_operation_undone",
                                  "Project operation inverse applied in reverse owner order.", {}};
    for (const auto& participant : entry.participants) result.committed_owners.push_back(participant.owner_id);
    auto undone = std::move(history_.back());
    history_.pop_back();
    undone.post_revisions.clear();
    for (const auto& participant : undone.participants) undone.post_revisions.push_back(participant.current_revision());
    redo_.push_back(std::move(undone));
    return result;
}

ProjectOperationResult ProjectOperationCoordinator::redoLast() {
    if (redo_.empty()) {
        return {false, false, "project_operation_redo_empty", "No project operation is available to redo.", {}};
    }
    auto entry = std::move(redo_.back());
    for (size_t index = 0; index < entry.participants.size(); ++index) {
        if (entry.participants[index].current_revision() != entry.post_revisions[index]) {
            redo_.back() = std::move(entry);
            return {false, false, "project_operation_redo_revision_mismatch",
                    "A participating document changed after undo; the composite operation was not redone.", {}};
        }
    }
    for (const auto& participant : entry.participants) {
        std::string diagnostic;
        if (!participant.prepare(diagnostic)) {
            redo_.back() = std::move(entry);
            return {false, false, "project_operation_redo_prepare_failed",
                    diagnostic.empty() ? "A domain owner rejected redo preparation." : diagnostic, {}};
        }
    }
    std::vector<size_t> committed;
    for (size_t index = 0; index < entry.participants.size(); ++index) {
        std::string diagnostic;
        if (!entry.participants[index].commit(diagnostic)) {
            entry.participants[index].rollback();
            for (auto committedIndex = committed.rbegin(); committedIndex != committed.rend(); ++committedIndex) {
                entry.participants[*committedIndex].rollback();
            }
            redo_.back() = std::move(entry);
            return {false, false, "project_operation_redo_commit_failed",
                    diagnostic.empty() ? "A domain owner failed redo; all admitted owners were rolled back." : diagnostic,
                    {}};
        }
        committed.push_back(index);
    }
    ProjectOperationResult result{true, false, "project_operation_redone",
                                  "Project operation reapplied across all participating domain owners.", {}};
    entry.post_revisions.clear();
    for (const auto& participant : entry.participants) {
        result.committed_owners.push_back(participant.owner_id);
        entry.post_revisions.push_back(participant.current_revision());
    }
    redo_.pop_back();
    entry.result = result;
    history_.push_back(std::move(entry));
    return result;
}

std::string ProjectOperationCoordinator::undoLabel() const {
    return history_.empty() ? std::string{} : history_.back().label;
}

std::string ProjectOperationCoordinator::redoLabel() const {
    return redo_.empty() ? std::string{} : redo_.back().label;
}

} // namespace urpg::project
