#pragma once

#include <cstdint>
#include <filesystem>
#include <cstddef>
#include <string>
#include <string_view>
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

// A private recovery overlay for a dirty authoritative document. The path is
// always project-relative; it is written only inside a recovery snapshot and
// never publishes or overwrites the creator's manual-save file.
struct RecoveryDocumentDraft {
    std::string document_id;
    std::filesystem::path project_relative_path;
    std::string serialized_contents;
};

// Stores crash-recovery copies separately from author-owned project files.
// Callers decide when to offer restore; this service never silently replaces a
// project on disk.
class EditorRecoveryService {
  public:
    bool createRecoverySnapshot(const std::filesystem::path& project_root, const std::string& project_id,
                                const std::vector<std::string>& dirty_document_ids,
                                const std::vector<RecoveryDocumentDraft>& drafts = {});
    std::vector<RecoverySnapshotMeta> listSnapshots(const std::filesystem::path& project_root) const;
    void pruneSnapshots(const std::filesystem::path& project_root, size_t max_count, uint64_t max_bytes) const;

    bool writeSessionMarker(const std::filesystem::path& project_root) const;
    bool clearSessionMarker(const std::filesystem::path& project_root) const;
    bool hasUncleanSessionMarker(const std::filesystem::path& project_root) const;

    bool restoreRecoverySnapshot(const std::filesystem::path& snapshot_path, const std::filesystem::path& restore_dest,
                                 bool replace_existing = false) const;

    static uint32_t calculateChecksum(std::string_view data);
};

} // namespace urpg::editor
