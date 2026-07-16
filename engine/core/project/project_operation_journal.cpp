#include "engine/core/project/project_operation_journal.h"

#include <algorithm>
#include <fstream>
#include <set>

#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace urpg::project {
namespace {

nlohmann::json entryToJson(const ProjectOperationJournalEntry& entry) {
    auto owners = nlohmann::json::array();
    for (const auto& owner : entry.owners) {
        owners.push_back({{"owner_id", owner.owner_id}, {"revision", owner.revision}, {"snapshot", owner.snapshot}});
    }
    return {{"operation_id", entry.operation_id}, {"label", entry.label}, {"state", entry.state},
            {"owners", std::move(owners)}};
}

bool entryFromJson(const nlohmann::json& json, ProjectOperationJournalEntry& entry) {
    if (!json.is_object() || !json.contains("operation_id") || !json["operation_id"].is_string() ||
        !json.contains("label") || !json["label"].is_string() || !json.contains("state") ||
        !json["state"].is_string() || !json.contains("owners") || !json["owners"].is_array()) return false;
    entry = {json["operation_id"].get<std::string>(), json["label"].get<std::string>(),
             json["state"].get<std::string>(), {}};
    if (entry.operation_id.empty() || entry.label.empty() ||
        (entry.state != "prepared" && entry.state != "committed")) return false;
    std::set<std::string> ownerIds;
    for (const auto& owner : json["owners"]) {
        if (!owner.is_object() || !owner.contains("owner_id") || !owner["owner_id"].is_string() ||
            !owner.contains("revision") || !owner["revision"].is_number_unsigned() ||
            !owner.contains("snapshot") || !owner["snapshot"].is_string()) return false;
        ProjectOperationOwnerSnapshot parsed{owner["owner_id"].get<std::string>(),
                                             owner["revision"].get<uint64_t>(),
                                             owner["snapshot"].get<std::string>()};
        if (parsed.owner_id.empty() || !ownerIds.insert(parsed.owner_id).second) return false;
        entry.owners.push_back(std::move(parsed));
    }
    return !entry.owners.empty();
}

bool readEntries(const std::filesystem::path& path, std::vector<ProjectOperationJournalEntry>& entries) {
    std::ifstream input(path, std::ios::binary);
    if (!input.good()) return !std::filesystem::exists(path);
    const auto json = nlohmann::json::parse(input, nullptr, false);
    if (json.is_discarded() || !json.is_object() ||
        json.value("schema", "") != "urpg.project_operation_journal.v1" || !json.contains("entries") ||
        !json["entries"].is_array()) return false;
    for (const auto& value : json["entries"]) {
        ProjectOperationJournalEntry entry;
        if (!entryFromJson(value, entry)) return false;
        entries.push_back(std::move(entry));
    }
    return true;
}

} // namespace

ProjectOperationJournal::ProjectOperationJournal(std::filesystem::path path,
                                                 std::function<bool()> before_atomic_replace)
    : path_(std::move(path)), before_atomic_replace_(std::move(before_atomic_replace)) {}

bool ProjectOperationJournal::recordPrepared(std::string operation_id, std::string label,
                                             std::vector<ProjectOperationOwnerSnapshot> owners,
                                             std::string* diagnostic) {
    return append({std::move(operation_id), std::move(label), "prepared", std::move(owners)}, diagnostic);
}

bool ProjectOperationJournal::acknowledgeCommitted(std::string operation_id, std::string label,
                                                   std::vector<ProjectOperationOwnerSnapshot> owners,
                                                   std::string* diagnostic) {
    return append({std::move(operation_id), std::move(label), "committed", std::move(owners)}, diagnostic);
}

bool ProjectOperationJournal::append(ProjectOperationJournalEntry entry, std::string* diagnostic) {
    ProjectOperationJournalEntry validated;
    if (!entryFromJson(entryToJson(entry), validated)) {
        if (diagnostic) *diagnostic = "project_operation_journal_entry_invalid";
        return false;
    }
    std::vector<ProjectOperationJournalEntry> entries;
    if (!readEntries(path_, entries)) {
        if (diagnostic) *diagnostic = "project_operation_journal_existing_invalid";
        return false;
    }
    bool prepared = false;
    bool finalized = false;
    for (const auto& existing : entries) {
        if (existing.operation_id != entry.operation_id) continue;
        if (existing.state == "prepared") prepared = true;
        else {
            prepared = false;
            finalized = true;
        }
    }
    if ((entry.state == "prepared" && (prepared || finalized)) ||
        (entry.state == "committed" && (!prepared || finalized))) {
        if (diagnostic) {
            *diagnostic = entry.state == "prepared" ? (finalized ? "project_operation_journal_operation_finalized"
                                                                  : "project_operation_journal_prepare_duplicate")
                                                     : "project_operation_journal_commit_without_prepare";
        }
        return false;
    }
    entries.push_back(std::move(validated));
    nlohmann::json payload{{"schema", "urpg.project_operation_journal.v1"}, {"entries", nlohmann::json::array()}};
    for (const auto& value : entries) payload["entries"].push_back(entryToJson(value));
    try {
        std::filesystem::create_directories(path_.parent_path());
        const auto temporary = std::filesystem::path(path_.string() + ".tmp");
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        output << payload.dump(2) << '\n';
        output.close();
        if (!output) {
            if (diagnostic) *diagnostic = "project_operation_journal_write_failed";
            return false;
        }
        if (before_atomic_replace_ && !before_atomic_replace_()) {
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            if (diagnostic) *diagnostic = "project_operation_journal_fault_before_replace";
            return false;
        }
#ifdef _WIN32
        if (!MoveFileExW(temporary.c_str(), path_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            if (diagnostic) *diagnostic = "project_operation_journal_replace_failed";
            return false;
        }
#else
        std::error_code replaceError;
        std::filesystem::rename(temporary, path_, replaceError);
        if (replaceError) {
            std::filesystem::remove(temporary, replaceError);
            if (diagnostic) *diagnostic = "project_operation_journal_replace_failed";
            return false;
        }
#endif
    } catch (const std::exception&) {
        if (diagnostic) *diagnostic = "project_operation_journal_write_failed";
        return false;
    }
    return true;
}

ProjectOperationRecoveryState ProjectOperationJournal::recover() const {
    ProjectOperationRecoveryState result;
    std::vector<ProjectOperationJournalEntry> entries;
    if (!readEntries(path_, entries)) {
        result.code = "project_operation_journal_invalid";
        return result;
    }
    std::set<std::string> pending;
    for (const auto& entry : entries) {
        if (entry.state == "prepared") pending.insert(entry.operation_id);
        else {
            pending.erase(entry.operation_id);
            result.has_acknowledged_operation = true;
            result.last_acknowledged = entry;
        }
    }
    result.interrupted_operation_ids.assign(pending.begin(), pending.end());
    result.success = true;
    result.code = result.interrupted_operation_ids.empty() ? "project_operation_recovery_clean"
                                                           : "project_operation_recovery_interrupted";
    return result;
}

} // namespace urpg::project
