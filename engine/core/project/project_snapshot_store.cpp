#include "engine/core/project/project_snapshot_store.h"

#include <fstream>

namespace urpg::project {

namespace {

void copyTree(const std::filesystem::path& source,
              const std::filesystem::path& destination,
              nlohmann::json& files) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(source)) {
        const auto relative = std::filesystem::relative(entry.path(), source);
        const auto target = destination / relative;
        if (entry.is_directory()) {
            std::filesystem::create_directories(target);
            continue;
        }
        std::filesystem::create_directories(target.parent_path());
        std::filesystem::copy_file(entry.path(), target, std::filesystem::copy_options::overwrite_existing);
        files.push_back(relative.generic_string());
    }
}

void writeManifest(const std::filesystem::path& snapshot_path, const nlohmann::json& manifest) {
    std::ofstream out(snapshot_path / "snapshot_manifest.json", std::ios::binary);
    out << manifest.dump(2);
}

} // namespace

ProjectSnapshotResult ProjectSnapshotStore::createSnapshot(const std::filesystem::path& project_root,
                                                           const std::filesystem::path& snapshot_root,
                                                           const std::string& snapshot_id) const {
    ProjectSnapshotResult result;
    if (!std::filesystem::exists(project_root) || !std::filesystem::is_directory(project_root)) {
        result.errors.push_back("project_root_missing");
        return result;
    }
    if (snapshot_id.empty()) {
        result.errors.push_back("missing_snapshot_id");
        return result;
    }

    result.snapshot_path = snapshot_root / snapshot_id;
    if (std::filesystem::exists(result.snapshot_path)) {
        result.errors.push_back("snapshot_already_exists");
        return result;
    }

    std::filesystem::create_directories(result.snapshot_path);
    nlohmann::json files = nlohmann::json::array();
    copyTree(project_root, result.snapshot_path / "project", files);
    result.manifest = {
        {"schema_version", "urpg.project_snapshot.v1"},
        {"snapshot_id", snapshot_id},
        {"files", files},
    };
    writeManifest(result.snapshot_path, result.manifest);
    result.success = true;
    return result;
}

ProjectSnapshotResult ProjectSnapshotStore::restoreSnapshot(const std::filesystem::path& snapshot_path,
                                                            const std::filesystem::path& restore_target) const {
    ProjectSnapshotResult result;
    result.snapshot_path = snapshot_path;
    const auto project_payload = snapshot_path / "project";
    if (!std::filesystem::exists(project_payload) || !std::filesystem::is_directory(project_payload)) {
        result.errors.push_back("snapshot_payload_missing");
        return result;
    }
    if (std::filesystem::exists(restore_target)) {
        result.errors.push_back("restore_target_exists");
        return result;
    }

    nlohmann::json files = nlohmann::json::array();
    copyTree(project_payload, restore_target, files);
    result.manifest = {
        {"schema_version", "urpg.project_snapshot_restore.v1"},
        {"files", files},
    };
    result.success = true;
    return result;
}

} // namespace urpg::project
