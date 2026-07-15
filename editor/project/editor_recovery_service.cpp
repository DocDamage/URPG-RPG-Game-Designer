#include "editor/project/editor_recovery_service.h"

#include "engine/core/project/project_snapshot_store.h"
#include "engine/core/save/save_journal.h"
#include "engine/core/version.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iterator>
#include <limits>
#include <nlohmann/json.hpp>

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

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return input ? std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()) : std::string{};
}

uint64_t directorySize(const std::filesystem::path& path) {
    uint64_t size = 0;
    std::error_code error;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path, error)) {
        if (entry.is_regular_file(error) && !error) size += entry.file_size(error);
        error.clear();
    }
    return size;
}

bool isSafeRelativePath(const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute()) return false;
    const auto normalized = path.lexically_normal();
    return std::none_of(normalized.begin(), normalized.end(),
                       [](const auto& part) { return part == ".."; });
}

bool validatePayload(const std::filesystem::path& snapshot_path, const nlohmann::json& manifest) {
    if (!manifest.contains("file_checksums") || !manifest["file_checksums"].is_object()) return false;
    for (const auto& [relative, checksum] : manifest["file_checksums"].items()) {
        const std::filesystem::path path(relative);
        if (!checksum.is_number_unsigned() || !isSafeRelativePath(path)) return false;
        const auto payload = snapshot_path / "project" / path;
        const auto value = checksum.get<uint64_t>();
        if (value > std::numeric_limits<uint32_t>::max() || !std::filesystem::is_regular_file(payload) ||
            EditorRecoveryService::calculateChecksum(readFile(payload)) != static_cast<uint32_t>(value)) {
            return false;
        }
    }
    return true;
}

} // namespace

uint32_t EditorRecoveryService::calculateChecksum(const std::string_view data) {
    uint32_t checksum = 0x811c9dc5;
    for (const auto character : data) {
        checksum ^= static_cast<uint8_t>(character);
        checksum *= 0x01000193;
    }
    return checksum;
}

bool EditorRecoveryService::createRecoverySnapshot(const std::filesystem::path& project_root,
                                                    const std::string& project_id,
                                                    const std::vector<std::string>& dirty_document_ids) {
    if (project_root.empty() || !std::filesystem::is_directory(project_root) || dirty_document_ids.empty()) return false;
    const auto now = std::chrono::system_clock::now();
    const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    const auto snapshot_id = "recovery_" + std::to_string(now.time_since_epoch().count());
    project::ProjectSnapshotStore store;
    const auto result = store.createSnapshot(project_root, project_root / ".urpg" / "recovery", snapshot_id);
    if (!result.success) return false;

    nlohmann::json manifest = result.manifest;
    manifest["project_id"] = project_id;
    manifest["dirty_document_ids"] = dirty_document_ids;
    manifest["timestamp"] = timestamp;
    manifest["app_version"] = versionString();
    manifest["file_checksums"] = nlohmann::json::object();
    for (const auto& file : manifest["files"]) {
        if (!file.is_string()) return false;
        const auto relative = std::filesystem::path(file.get<std::string>());
        if (!isSafeRelativePath(relative)) return false;
        const auto payload = result.snapshot_path / "project" / relative;
        if (!std::filesystem::is_regular_file(payload)) return false;
        manifest["file_checksums"][relative.generic_string()] = calculateChecksum(readFile(payload));
    }
    manifest["checksum"] = calculateChecksum(manifest.dump());
    std::string error;
    return SaveJournal::WriteAtomically(result.snapshot_path / "snapshot_manifest.json", manifest.dump(2) + "\n", &error);
}

std::vector<RecoverySnapshotMeta> EditorRecoveryService::listSnapshots(const std::filesystem::path& project_root) const {
    std::vector<RecoverySnapshotMeta> snapshots;
    std::error_code error;
    const auto root = project_root / ".urpg" / "recovery";
    for (const auto& entry : std::filesystem::directory_iterator(root, error)) {
        if (error) break;
        if (!entry.is_directory(error) || error || entry.is_symlink(error)) {
            error.clear();
            continue;
        }
        const auto manifestPath = entry.path() / "snapshot_manifest.json";
        std::ifstream input(manifestPath, std::ios::binary);
        const auto manifest = input ? nlohmann::json::parse(input, nullptr, false) : nlohmann::json{};
        if (!manifest.is_object() || !manifest.contains("checksum") || !manifest["checksum"].is_number_unsigned()) continue;
        const auto stored = manifest["checksum"].get<uint64_t>();
        if (stored > std::numeric_limits<uint32_t>::max()) continue;
        auto unsignedManifest = manifest;
        unsignedManifest.erase("checksum");
        if (calculateChecksum(unsignedManifest.dump()) != static_cast<uint32_t>(stored) ||
            !validatePayload(entry.path(), unsignedManifest)) continue;
        RecoverySnapshotMeta snapshot;
        snapshot.snapshot_id = unsignedManifest.value("snapshot_id", "");
        snapshot.path = entry.path();
        snapshot.timestamp = unsignedManifest.value("timestamp", int64_t{0});
        snapshot.project_id = unsignedManifest.value("project_id", "");
        snapshot.app_version = unsignedManifest.value("app_version", "");
        snapshot.checksum = static_cast<uint32_t>(stored);
        for (const auto& id : unsignedManifest.value("dirty_document_ids", nlohmann::json::array())) {
            if (id.is_string()) snapshot.dirty_document_ids.push_back(id.get<std::string>());
        }
        snapshots.push_back(std::move(snapshot));
    }
    std::sort(snapshots.begin(), snapshots.end(), [](const auto& left, const auto& right) {
        return left.timestamp > right.timestamp;
    });
    return snapshots;
}

void EditorRecoveryService::pruneSnapshots(const std::filesystem::path& project_root, const size_t max_count,
                                           const uint64_t max_bytes) const {
    auto snapshots = listSnapshots(project_root);
    std::error_code error;
    while (snapshots.size() > max_count) {
        std::filesystem::remove_all(snapshots.back().path, error);
        error.clear();
        snapshots.pop_back();
    }
    uint64_t total = 0;
    for (const auto& snapshot : snapshots) total += directorySize(snapshot.path);
    while (total > max_bytes && !snapshots.empty()) {
        total -= directorySize(snapshots.back().path);
        std::filesystem::remove_all(snapshots.back().path, error);
        error.clear();
        snapshots.pop_back();
    }
}

bool EditorRecoveryService::writeSessionMarker(const std::filesystem::path& project_root) const {
    if (project_root.empty()) return false;
    std::error_code error;
    std::filesystem::create_directories(project_root / ".urpg", error);
    if (error) return false;
#ifdef _WIN32
    const auto processId = static_cast<uint64_t>(GetCurrentProcessId());
#else
    const auto processId = static_cast<uint64_t>(getpid());
#endif
    const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
    std::string writeError;
    return SaveJournal::WriteAtomically(project_root / ".urpg" / "session_marker.json",
                                        nlohmann::json{{"schema_version", "urpg.session_marker.v1"},
                                                       {"timestamp", timestamp}, {"pid", processId}}
                                                .dump(2) + "\n",
                                        &writeError);
}

bool EditorRecoveryService::clearSessionMarker(const std::filesystem::path& project_root) const {
    std::error_code error;
    return !project_root.empty() && std::filesystem::remove(project_root / ".urpg" / "session_marker.json", error) && !error;
}

bool EditorRecoveryService::hasUncleanSessionMarker(const std::filesystem::path& project_root) const {
    std::error_code error;
    return !project_root.empty() && std::filesystem::is_regular_file(project_root / ".urpg" / "session_marker.json", error) && !error;
}

bool EditorRecoveryService::restoreRecoverySnapshot(const std::filesystem::path& snapshot_path,
                                                     const std::filesystem::path& restore_dest,
                                                     const bool replace_existing) const {
    if (snapshot_path.empty() || !std::filesystem::is_directory(snapshot_path)) return false;
    const auto projectRoot = snapshot_path.parent_path().parent_path().parent_path();
    const auto snapshots = listSnapshots(projectRoot);
    if (std::none_of(snapshots.begin(), snapshots.end(), [&](const auto& snapshot) { return snapshot.path == snapshot_path; })) return false;
    project::ProjectSnapshotStore store;
    if (!std::filesystem::exists(restore_dest)) return store.restoreSnapshot(snapshot_path, restore_dest).success;
    if (!replace_existing || !std::filesystem::is_directory(restore_dest)) return false;
    const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
    const auto temporary = restore_dest.parent_path() / ("." + restore_dest.filename().string() + ".recovery-" + std::to_string(stamp));
    const auto backup = restore_dest.parent_path() / (restore_dest.filename().string() + ".pre-recovery-" + std::to_string(stamp));
    if (!store.restoreSnapshot(snapshot_path, temporary).success) return false;
    std::error_code error;
    std::filesystem::rename(restore_dest, backup, error);
    if (error) return false;
    std::filesystem::rename(temporary, restore_dest, error);
    if (!error) return true;
    std::error_code rollbackError;
    std::filesystem::rename(backup, restore_dest, rollbackError);
    std::filesystem::remove_all(temporary, rollbackError);
    return false;
}

} // namespace urpg::editor
