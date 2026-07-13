#include "editor/project/editor_recovery_service.h"
#include "engine/core/save/save_journal.h"
#include "engine/core/version.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iterator>
#include <limits>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace urpg::editor {

namespace {

uint64_t getFolderSize(const std::filesystem::path& path) {
    uint64_t total = 0;
    std::error_code ec;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path, ec)) {
        if (entry.is_regular_file(ec)) {
            total += entry.file_size(ec);
        }
    }
    return total;
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {};
    }
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

bool validateSnapshotPayload(const std::filesystem::path& snapshotPath, const nlohmann::json& manifest) {
    const auto checksums = manifest.find("file_checksums");
    if (checksums == manifest.end() || !checksums->is_object()) {
        return false;
    }
    const auto payloadRoot = snapshotPath / "project";
    for (const auto& [relative, checksumValue] : checksums->items()) {
        if (!checksumValue.is_number_unsigned()) {
            return false;
        }
        const auto relativePath = std::filesystem::path(relative);
        if (relativePath.empty() || relativePath.is_absolute()) return false;
        for (const auto& part : relativePath.lexically_normal()) {
            if (part == "..") return false;
        }
        const auto payloadPath = (payloadRoot / relativePath).lexically_normal();
        const auto recorded = checksumValue.get<uint64_t>();
        if (recorded > std::numeric_limits<uint32_t>::max()) return false;
        if (!std::filesystem::is_regular_file(payloadPath) ||
            EditorRecoveryService::calculateChecksum(readFile(payloadPath)) != static_cast<uint32_t>(recorded)) {
            return false;
        }
    }
    return true;
}

} // namespace

uint32_t EditorRecoveryService::calculateChecksum(std::string_view data) {
    uint32_t hash = 0x811c9dc5;
    for (char c : data) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 0x01000193;
    }
    return hash;
}

bool EditorRecoveryService::createRecoverySnapshot(const std::filesystem::path& project_root,
                                                    const std::string& project_id,
                                                    const std::vector<std::string>& dirty_document_ids) {
    if (project_root.empty() || !std::filesystem::exists(project_root)) {
        return false;
    }

    const auto recovery_root = project_root / ".urpg" / "recovery";
    std::error_code ec;
    std::filesystem::create_directories(recovery_root, ec);

    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    const auto uniqueStamp = now.time_since_epoch().count();
    const std::string snapshot_id = "recovery_" + std::to_string(uniqueStamp);

    urpg::project::ProjectSnapshotStore store;
    auto snap_res = store.createSnapshot(project_root, recovery_root, snapshot_id);
    if (!snap_res.success) {
        return false;
    }

    // Update the snapshot manifest with recovery details and calculate checksum
    nlohmann::json manifest = snap_res.manifest;
    manifest["project_id"] = project_id;
    manifest["dirty_document_ids"] = dirty_document_ids;
    manifest["timestamp"] = seconds;
    manifest["app_version"] = urpg::versionString();

    nlohmann::json fileChecksums = nlohmann::json::object();
    const auto payloadRoot = snap_res.snapshot_path / "project";
    if (const auto files = manifest.find("files"); files != manifest.end() && files->is_array()) {
        for (const auto& relative : *files) {
            if (!relative.is_string()) {
                std::filesystem::remove_all(snap_res.snapshot_path, ec);
                return false;
            }
            const auto payloadPath = payloadRoot / std::filesystem::path(relative.get<std::string>());
            if (!std::filesystem::is_regular_file(payloadPath)) {
                std::filesystem::remove_all(snap_res.snapshot_path, ec);
                return false;
            }
            fileChecksums[relative.get<std::string>()] = calculateChecksum(readFile(payloadPath));
        }
    }
    manifest["file_checksums"] = std::move(fileChecksums);

    std::string serialized = manifest.dump();
    const uint32_t checksum = calculateChecksum(serialized);
    manifest["checksum"] = checksum;

    std::string writeError;
    if (!urpg::SaveJournal::WriteAtomically(snap_res.snapshot_path / "snapshot_manifest.json",
                                            manifest.dump(2) + "\n", &writeError)) {
        std::filesystem::remove_all(snap_res.snapshot_path, ec);
        return false;
    }
    return true;
}

std::vector<RecoverySnapshotMeta> EditorRecoveryService::listSnapshots(const std::filesystem::path& project_root) const {
    std::vector<RecoverySnapshotMeta> result;
    const auto recovery_root = project_root / ".urpg" / "recovery";
    if (!std::filesystem::exists(recovery_root)) {
        return result;
    }

    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(recovery_root, ec)) {
        if (!entry.is_directory()) {
            continue;
        }

        const auto manifest_path = entry.path() / "snapshot_manifest.json";
        if (!std::filesystem::exists(manifest_path)) {
            continue;
        }

        std::ifstream in(manifest_path, std::ios::binary);
        if (!in) {
            continue;
        }

        nlohmann::json manifest = nlohmann::json::parse(in, nullptr, false);
        if (manifest.is_discarded() || !manifest.is_object() || !manifest.contains("checksum")) {
            continue;
        }

        if (!manifest["checksum"].is_number_unsigned()) {
            continue;
        }
        const auto recordedValue = manifest["checksum"].get<uint64_t>();
        if (recordedValue > std::numeric_limits<uint32_t>::max()) {
            continue;
        }
        const uint32_t recorded_checksum = static_cast<uint32_t>(recordedValue);
        manifest.erase("checksum");

        std::string serialized = manifest.dump();
        uint32_t computed_checksum = calculateChecksum(serialized);
        if (computed_checksum != recorded_checksum || !validateSnapshotPayload(entry.path(), manifest)) {
            continue;
        }

        RecoverySnapshotMeta meta;
        try {
            meta.snapshot_id = manifest.value("snapshot_id", "");
            meta.path = entry.path();
            meta.timestamp = manifest.value("timestamp", 0LL);
            meta.project_id = manifest.value("project_id", "");
            meta.app_version = manifest.value("app_version", "");
            meta.checksum = recorded_checksum;
        } catch (const nlohmann::json::exception&) {
            continue;
        }

        if (manifest.contains("dirty_document_ids") && manifest["dirty_document_ids"].is_array()) {
            for (const auto& doc_id : manifest["dirty_document_ids"]) {
                if (doc_id.is_string()) {
                    meta.dirty_document_ids.push_back(doc_id.get<std::string>());
                }
            }
        }

        result.push_back(std::move(meta));
    }

    // Sort by timestamp, newest first
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.timestamp > b.timestamp;
    });

    return result;
}

void EditorRecoveryService::pruneSnapshots(const std::filesystem::path& project_root, size_t max_count, uint64_t max_bytes) {
    auto snapshots = listSnapshots(project_root);
    if (snapshots.empty()) {
        return;
    }

    // Remove by count
    while (snapshots.size() > max_count) {
        const auto& oldest = snapshots.back();
        std::error_code ec;
        std::filesystem::remove_all(oldest.path, ec);
        snapshots.pop_back();
    }

    // Remove by size
    uint64_t total_size = 0;
    for (const auto& snap : snapshots) {
        total_size += getFolderSize(snap.path);
    }

    while (total_size > max_bytes && !snapshots.empty()) {
        const auto& oldest = snapshots.back();
        uint64_t snap_size = getFolderSize(oldest.path);
        std::error_code ec;
        std::filesystem::remove_all(oldest.path, ec);
        total_size -= snap_size;
        snapshots.pop_back();
    }
}

bool EditorRecoveryService::writeSessionMarker(const std::filesystem::path& project_root) {
    if (project_root.empty()) {
        return false;
    }

    const auto marker_dir = project_root / ".urpg";
    std::error_code ec;
    std::filesystem::create_directories(marker_dir, ec);

    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    const auto pid =
#ifdef _WIN32
        static_cast<uint64_t>(GetCurrentProcessId());
#else
        static_cast<uint64_t>(getpid());
#endif
    nlohmann::json marker = {
        {"schema_version", "urpg.session_marker.v1"},
        {"timestamp", seconds},
        {"pid", pid}
    };

    std::string writeError;
    return urpg::SaveJournal::WriteAtomically(marker_dir / "session_marker.json", marker.dump(2) + "\n",
                                               &writeError);
}

bool EditorRecoveryService::clearSessionMarker(const std::filesystem::path& project_root) {
    if (project_root.empty()) {
        return false;
    }
    std::error_code ec;
    return std::filesystem::remove(project_root / ".urpg" / "session_marker.json", ec);
}

bool EditorRecoveryService::hasUncleanSessionMarker(const std::filesystem::path& project_root) const {
    if (project_root.empty()) {
        return false;
    }
    return std::filesystem::exists(project_root / ".urpg" / "session_marker.json");
}

bool EditorRecoveryService::restoreRecoverySnapshot(const std::filesystem::path& snapshot_path,
                                                     const std::filesystem::path& restore_dest,
                                                     bool replace_existing) const {
    if (snapshot_path.empty() || !std::filesystem::exists(snapshot_path)) {
        return false;
    }

    const auto snapshots = listSnapshots(snapshot_path.parent_path().parent_path().parent_path());
    const bool snapshotValid = std::any_of(snapshots.begin(), snapshots.end(), [&](const auto& snapshot) {
        return snapshot.path == snapshot_path;
    });
    if (!snapshotValid) {
        return false;
    }

    urpg::project::ProjectSnapshotStore store;
    if (!std::filesystem::exists(restore_dest)) {
        return store.restoreSnapshot(snapshot_path, restore_dest).success;
    }
    if (!replace_existing || !std::filesystem::is_directory(restore_dest)) {
        return false;
    }

    const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
    const auto temporary = restore_dest.parent_path() /
        ("." + restore_dest.filename().string() + ".recovery-" + std::to_string(stamp) + ".tmp");
    const auto backup = restore_dest.parent_path() /
        (restore_dest.filename().string() + ".pre-recovery-" + std::to_string(stamp));

    if (!store.restoreSnapshot(snapshot_path, temporary).success) {
        return false;
    }

    std::error_code ec;
    std::filesystem::rename(restore_dest, backup, ec);
    if (ec) {
        std::filesystem::remove_all(temporary, ec);
        return false;
    }
    std::filesystem::rename(temporary, restore_dest, ec);
    if (!ec) {
        return true;
    }

    std::error_code rollbackError;
    std::filesystem::rename(backup, restore_dest, rollbackError);
    std::filesystem::remove_all(temporary, rollbackError);
    return false;
}

} // namespace urpg::editor
