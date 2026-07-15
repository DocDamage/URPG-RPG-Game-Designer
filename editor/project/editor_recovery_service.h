#pragma once

#include "engine/core/project/project_snapshot_store.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::editor {

struct RecoverySnapshotMeta {
    std::string snapshot_id;
    std::filesystem::path path;
    int64_t timestamp = 0;
    std::vector<std::string> dirty_document_ids;
    std::string project_id;
    std::string app_version;
    uint32_t checksum = 0;
};

class EditorRecoveryService {
public:
    bool createRecoverySnapshot(const std::filesystem::path& project_root,
                                const std::string& project_id,
                                const std::vector<std::string>& dirty_document_ids);

    std::vector<RecoverySnapshotMeta> listSnapshots(const std::filesystem::path& project_root) const;

    void pruneSnapshots(const std::filesystem::path& project_root, size_t max_count, uint64_t max_bytes);

    bool writeSessionMarker(const std::filesystem::path& project_root);
    bool clearSessionMarker(const std::filesystem::path& project_root);
    bool hasUncleanSessionMarker(const std::filesystem::path& project_root) const;

    bool restoreRecoverySnapshot(const std::filesystem::path& snapshot_path,
                                 const std::filesystem::path& restore_dest,
                                 bool replace_existing = false) const;

    static uint32_t calculateChecksum(std::string_view data);
};

} // namespace urpg::editor
