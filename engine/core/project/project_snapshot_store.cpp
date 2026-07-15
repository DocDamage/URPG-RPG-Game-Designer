#include "engine/core/project/project_snapshot_store.h"

#include <fstream>

namespace urpg::project {

namespace {

bool copyTree(const std::filesystem::path& source,
              const std::filesystem::path& destination,
              nlohmann::json& files,
              std::vector<std::string>& errors) {
    std::error_code ec;
    std::filesystem::recursive_directory_iterator iterator(
        source, std::filesystem::directory_options::skip_permission_denied, ec);
    const std::filesystem::recursive_directory_iterator end;
    if (ec) {
        errors.push_back("snapshot_enumeration_failed:" + ec.message());
        return false;
    }

    while (iterator != end) {
        const auto& entry = *iterator;
        const auto relative = std::filesystem::relative(entry.path(), source, ec);
        if (ec) {
            errors.push_back("snapshot_relative_path_failed:" + ec.message());
            return false;
        }
        if (!relative.empty() && relative.begin()->string() == ".urpg") {
            if (entry.is_directory(ec) && !ec) {
                iterator.disable_recursion_pending();
            }
            ec.clear();
            iterator.increment(ec);
            if (ec) {
                errors.push_back("snapshot_enumeration_failed:" + ec.message());
                return false;
            }
            continue;
        }
        const auto target = destination / relative;
        if (entry.is_symlink(ec)) {
            ec.clear();
            iterator.increment(ec);
            if (ec) {
                errors.push_back("snapshot_enumeration_failed:" + ec.message());
                return false;
            }
            continue;
        }
        if (entry.is_directory(ec)) {
            std::filesystem::create_directories(target, ec);
            if (ec) {
                errors.push_back("snapshot_directory_create_failed:" + ec.message());
                return false;
            }
            iterator.increment(ec);
            if (ec) {
                errors.push_back("snapshot_enumeration_failed:" + ec.message());
                return false;
            }
            continue;
        }
        if (ec || !entry.is_regular_file(ec) || ec) {
            ec.clear();
            iterator.increment(ec);
            if (ec) {
                errors.push_back("snapshot_enumeration_failed:" + ec.message());
                return false;
            }
            continue;
        }
        std::filesystem::create_directories(target.parent_path(), ec);
        if (ec) {
            errors.push_back("snapshot_directory_create_failed:" + ec.message());
            return false;
        }
        std::filesystem::copy_file(entry.path(), target, std::filesystem::copy_options::overwrite_existing, ec);
        if (ec) {
            errors.push_back("snapshot_file_copy_failed:" + relative.generic_string() + ":" + ec.message());
            return false;
        }
        files.push_back(relative.generic_string());
        iterator.increment(ec);
        if (ec) {
            errors.push_back("snapshot_enumeration_failed:" + ec.message());
            return false;
        }
    }
    return true;
}

bool writeManifest(const std::filesystem::path& snapshot_path, const nlohmann::json& manifest) {
    std::ofstream out(snapshot_path / "snapshot_manifest.json", std::ios::binary);
    out << manifest.dump(2) << '\n';
    return static_cast<bool>(out);
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
    if (snapshot_id.empty() || std::filesystem::path(snapshot_id).filename().string() != snapshot_id ||
        snapshot_id == "." || snapshot_id == "..") {
        result.errors.push_back("missing_snapshot_id");
        return result;
    }

    result.snapshot_path = snapshot_root / snapshot_id;
    if (std::filesystem::exists(result.snapshot_path)) {
        result.errors.push_back("snapshot_already_exists");
        return result;
    }

    std::error_code ec;
    std::filesystem::create_directories(result.snapshot_path, ec);
    if (ec) {
        result.errors.push_back("snapshot_directory_create_failed:" + ec.message());
        return result;
    }
    nlohmann::json files = nlohmann::json::array();
    if (!copyTree(project_root, result.snapshot_path / "project", files, result.errors)) {
        std::filesystem::remove_all(result.snapshot_path, ec);
        return result;
    }
    result.manifest = {
        {"schema_version", "urpg.project_snapshot.v1"},
        {"snapshot_id", snapshot_id},
        {"files", files},
    };
    if (!writeManifest(result.snapshot_path, result.manifest)) {
        result.errors.push_back("snapshot_manifest_write_failed");
        std::filesystem::remove_all(result.snapshot_path, ec);
        return result;
    }
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
    if (!copyTree(project_payload, restore_target, files, result.errors)) {
        std::error_code ec;
        std::filesystem::remove_all(restore_target, ec);
        return result;
    }
    result.manifest = {
        {"schema_version", "urpg.project_snapshot_restore.v1"},
        {"files", files},
    };
    result.success = true;
    return result;
}

} // namespace urpg::project
