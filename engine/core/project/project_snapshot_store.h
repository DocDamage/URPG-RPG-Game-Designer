#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::project {

struct ProjectSnapshotResult {
    bool success = false;
    std::filesystem::path snapshot_path;
    nlohmann::json manifest = nlohmann::json::object();
    std::vector<std::string> errors;
};

class ProjectSnapshotStore {
public:
    ProjectSnapshotResult createSnapshot(const std::filesystem::path& project_root,
                                         const std::filesystem::path& snapshot_root,
                                         const std::string& snapshot_id) const;
    ProjectSnapshotResult restoreSnapshot(const std::filesystem::path& snapshot_path,
                                          const std::filesystem::path& restore_target) const;
};

} // namespace urpg::project
