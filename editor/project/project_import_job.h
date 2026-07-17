#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace urpg::editor {

enum class ProjectImportJobState { Pending, Discovering, Copying, Finalizing, Completed, Failed, Cancelled };

struct ProjectImportRequest {
    std::filesystem::path source_root;
    std::filesystem::path destination_root;
    std::string project_id;
    std::string project_name;
};

struct ProjectImportJobSnapshot {
    ProjectImportJobState state = ProjectImportJobState::Pending;
    std::string code;
    std::string message;
    std::size_t discovered_items = 0;
    std::size_t copied_items = 0;
    std::size_t last_slice_items = 0;
    std::filesystem::path destination_root;
    bool complete = false;
    bool success = false;
};

// Stages a validated project copy beside the requested destination and
// publishes it only after every discovered item and the rewritten manifest
// succeed. Discovery and copying advance in caller-bounded slices.
class ProjectImportJob {
public:
    explicit ProjectImportJob(ProjectImportRequest request);
    ~ProjectImportJob();

    ProjectImportJob(const ProjectImportJob&) = delete;
    ProjectImportJob& operator=(const ProjectImportJob&) = delete;
    ProjectImportJob(ProjectImportJob&&) = delete;
    ProjectImportJob& operator=(ProjectImportJob&&) = delete;

    bool advance(std::size_t maximum_items = 64);
    void cancel();
    ProjectImportJobSnapshot snapshot() const;

private:
    struct Item {
        std::filesystem::path relative_path;
        bool directory = false;
    };

    bool preflight();
    bool advanceDiscovery(std::size_t maximum_items);
    bool advanceCopy(std::size_t maximum_items);
    bool finalize();
    void fail(std::string code, std::string message);
    void discardStage();

    ProjectImportRequest request_;
    ProjectImportJobState state_ = ProjectImportJobState::Pending;
    std::string code_;
    std::string message_;
    std::filesystem::path normalized_source_;
    std::filesystem::path normalized_destination_;
    std::filesystem::path stage_root_;
    std::filesystem::recursive_directory_iterator discovery_;
    std::filesystem::recursive_directory_iterator discovery_end_;
    std::vector<Item> items_;
    std::size_t copy_cursor_ = 0;
    std::size_t last_slice_items_ = 0;
    bool owns_stage_ = false;
};

const char* projectImportJobStateName(ProjectImportJobState state);

} // namespace urpg::editor
