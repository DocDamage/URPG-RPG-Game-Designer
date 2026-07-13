#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::editor {

struct MapAuthoringDocumentWrite {
    std::filesystem::path target;
    std::string contents;
};

struct MapAuthoringPersistenceResult {
    bool success = false;
    std::string code;
    std::string message;
};

// Publishes a related set of map documents as one rollback-capable transaction.
// If any publish step fails, already-replaced files are restored from backups
// and callers must retain their dirty state.
MapAuthoringPersistenceResult publishMapAuthoringDocuments(
    const std::vector<MapAuthoringDocumentWrite>& documents);

} // namespace urpg::editor
