#include "engine/core/project/project_operation_journal.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

using urpg::project::ProjectOperationJournal;
using urpg::project::ProjectOperationOwnerSnapshot;

TEST_CASE("Project operation journal recovers last acknowledged snapshots past interrupted prepare",
          "[project][operation_journal]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_operation_journal_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    ProjectOperationJournal journal(root / "journal.json");
    REQUIRE(journal.recordPrepared("op.one", "First", {ProjectOperationOwnerSnapshot{"map", 1, "before"}}));
    REQUIRE(journal.acknowledgeCommitted("op.one", "First", {ProjectOperationOwnerSnapshot{"map", 2, "after"}}));
    REQUIRE(journal.recordPrepared("op.two", "Interrupted", {ProjectOperationOwnerSnapshot{"dialogue", 4, "draft"}}));
    const auto recovered = journal.recover();
    REQUIRE(recovered.success);
    REQUIRE(recovered.code == "project_operation_recovery_interrupted");
    REQUIRE(recovered.has_acknowledged_operation);
    REQUIRE(recovered.last_acknowledged.operation_id == "op.one");
    REQUIRE(recovered.last_acknowledged.owners[0].revision == 2);
    REQUIRE(recovered.last_acknowledged.owners[0].snapshot == "after");
    REQUIRE(recovered.interrupted_operation_ids == std::vector<std::string>{"op.two"});
    std::filesystem::remove_all(root, error);
}

TEST_CASE("Project operation journal atomic replacement fault preserves acknowledged state",
          "[project][operation_journal]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_operation_journal_fault_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto path = root / "journal.json";
    ProjectOperationJournal journal(path);
    REQUIRE(journal.recordPrepared("op.safe", "Safe", {ProjectOperationOwnerSnapshot{"map", 1, "before"}}));
    REQUIRE(journal.acknowledgeCommitted("op.safe", "Safe", {ProjectOperationOwnerSnapshot{"map", 2, "safe"}}));
    ProjectOperationJournal faulted(path, [] { return false; });
    std::string diagnostic;
    REQUIRE_FALSE(faulted.recordPrepared("op.lost", "Lost", {ProjectOperationOwnerSnapshot{"map", 2, "unsafe"}},
                                          &diagnostic));
    REQUIRE(diagnostic == "project_operation_journal_fault_before_replace");
    const auto recovered = journal.recover();
    REQUIRE(recovered.success);
    REQUIRE(recovered.code == "project_operation_recovery_clean");
    REQUIRE(recovered.last_acknowledged.operation_id == "op.safe");
    REQUIRE(recovered.last_acknowledged.owners[0].snapshot == "safe");
    REQUIRE(recovered.interrupted_operation_ids.empty());
    std::filesystem::remove_all(root, error);
}

TEST_CASE("Project operation journal enforces prepare then commit sequence", "[project][operation_journal]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_operation_journal_sequence_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    ProjectOperationJournal journal(root / "journal.json");
    std::string diagnostic;
    REQUIRE_FALSE(journal.acknowledgeCommitted(
        "op.sequence", "Sequence", {ProjectOperationOwnerSnapshot{"map", 2, "after"}}, &diagnostic));
    REQUIRE(diagnostic == "project_operation_journal_commit_without_prepare");
    REQUIRE(journal.recordPrepared("op.sequence", "Sequence",
                                   {ProjectOperationOwnerSnapshot{"map", 1, "before"}}));
    REQUIRE_FALSE(journal.recordPrepared("op.sequence", "Sequence",
                                         {ProjectOperationOwnerSnapshot{"map", 1, "before"}}, &diagnostic));
    REQUIRE(diagnostic == "project_operation_journal_prepare_duplicate");
    REQUIRE(journal.acknowledgeCommitted("op.sequence", "Sequence",
                                        {ProjectOperationOwnerSnapshot{"map", 2, "after"}}));
    REQUIRE_FALSE(journal.recordPrepared("op.sequence", "Sequence",
                                         {ProjectOperationOwnerSnapshot{"map", 2, "after"}}, &diagnostic));
    REQUIRE(diagnostic == "project_operation_journal_operation_finalized");
    REQUIRE(journal.recover().code == "project_operation_recovery_clean");
    std::filesystem::remove_all(root, error);
}
