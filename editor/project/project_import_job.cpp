#include "editor/project/project_import_job.h"

#include "editor/project/editor_project_session.h"
#include "engine/core/save/save_journal.h"

#include <algorithm>
#include <fstream>
#include <system_error>
#include <utility>

#include <nlohmann/json.hpp>

namespace urpg::editor {
namespace {

bool isWithin(const std::filesystem::path& candidate, const std::filesystem::path& root) {
    const auto relative = candidate.lexically_relative(root);
    if (relative.empty() || relative.is_absolute()) return candidate == root;
    return std::none_of(relative.begin(), relative.end(), [](const auto& component) { return component == ".."; });
}

bool excluded(const std::filesystem::path& relative) {
    if (relative.empty()) return false;
    const auto first = *relative.begin();
    return first == ".git" || first == ".urpg";
}

} // namespace

const char* projectImportJobStateName(const ProjectImportJobState state) {
    switch (state) {
    case ProjectImportJobState::Pending: return "pending";
    case ProjectImportJobState::Discovering: return "discovering";
    case ProjectImportJobState::Copying: return "copying";
    case ProjectImportJobState::Finalizing: return "finalizing";
    case ProjectImportJobState::Completed: return "completed";
    case ProjectImportJobState::Failed: return "failed";
    case ProjectImportJobState::Cancelled: return "cancelled";
    }
    return "failed";
}

ProjectImportJob::ProjectImportJob(ProjectImportRequest request) : request_(std::move(request)) {}

ProjectImportJob::~ProjectImportJob() {
    if (state_ != ProjectImportJobState::Completed) discardStage();
}

bool ProjectImportJob::preflight() {
    if (request_.source_root.empty() || request_.destination_root.empty() || request_.project_id.empty() ||
        request_.project_name.empty()) {
        fail("project_import_request_incomplete",
             "Project import requires source, destination, new project ID, and new project name.");
        return false;
    }
    const auto inspection = EditorProjectSession{}.inspectProject(request_.source_root);
    if (!inspection.result.success) {
        fail("project_import_source_invalid", inspection.result.message);
        return false;
    }
    std::error_code error;
    normalized_source_ = std::filesystem::weakly_canonical(request_.source_root, error);
    if (error) {
        fail("project_import_source_unresolvable", "The source project path could not be normalized.");
        return false;
    }
    const auto destinationParent = std::filesystem::weakly_canonical(request_.destination_root.parent_path(), error);
    if (error || destinationParent.empty()) {
        fail("project_import_destination_parent_missing", "The destination parent folder must already exist.");
        return false;
    }
    normalized_destination_ = (destinationParent / request_.destination_root.filename()).lexically_normal();
    if (std::filesystem::exists(normalized_destination_, error) || error) {
        fail("project_import_destination_exists", "The import destination must not already exist.");
        return false;
    }
    if (isWithin(normalized_destination_, normalized_source_)) {
        fail("project_import_destination_inside_source",
             "The import destination cannot be inside the source project.");
        return false;
    }
    stage_root_ = destinationParent / (request_.destination_root.filename().string() + ".urpg-import-stage");
    if (std::filesystem::exists(stage_root_, error) || error) {
        fail("project_import_stage_exists", "A prior import staging folder already exists; review it before retrying.");
        return false;
    }
    std::filesystem::create_directory(stage_root_, error);
    if (error) {
        fail("project_import_stage_create_failed", "The import staging folder could not be created: " + error.message());
        return false;
    }
    owns_stage_ = true;
    discovery_ = std::filesystem::recursive_directory_iterator(
        normalized_source_, std::filesystem::directory_options::none, error);
    if (error) {
        fail("project_import_discovery_failed", "The source project could not be enumerated: " + error.message());
        return false;
    }
    state_ = ProjectImportJobState::Discovering;
    code_ = "project_import_discovering";
    message_ = "Discovering project files in bounded slices.";
    return true;
}

bool ProjectImportJob::advanceDiscovery(const std::size_t maximum_items) {
    last_slice_items_ = 0;
    std::error_code error;
    while (discovery_ != discovery_end_ && last_slice_items_ < maximum_items) {
        const auto entry = *discovery_;
        const auto relative = entry.path().lexically_relative(normalized_source_);
        const auto status = entry.symlink_status(error);
        if (error || std::filesystem::is_symlink(status)) {
            fail("project_import_unsafe_entry", "Project import refuses unreadable entries and symbolic links: " +
                                                  relative.generic_string());
            return false;
        }
        if (excluded(relative)) {
            if (entry.is_directory(error)) discovery_.disable_recursion_pending();
        } else if (entry.is_directory(error)) {
            items_.push_back({relative, true});
        } else if (entry.is_regular_file(error)) {
            items_.push_back({relative, false});
        } else {
            fail("project_import_unsupported_entry", "Project import found an unsupported filesystem entry: " +
                                                       relative.generic_string());
            return false;
        }
        if (error) {
            fail("project_import_discovery_failed", "Project import could not inspect " + relative.generic_string());
            return false;
        }
        discovery_.increment(error);
        if (error) {
            fail("project_import_discovery_failed", "Project import enumeration failed: " + error.message());
            return false;
        }
        ++last_slice_items_;
    }
    if (discovery_ == discovery_end_) {
        std::sort(items_.begin(), items_.end(), [](const auto& left, const auto& right) {
            if (left.directory != right.directory) return left.directory > right.directory;
            return left.relative_path < right.relative_path;
        });
        state_ = ProjectImportJobState::Copying;
        code_ = "project_import_copying";
        message_ = "Copying validated project files into private staging.";
    }
    return true;
}

bool ProjectImportJob::advanceCopy(const std::size_t maximum_items) {
    last_slice_items_ = 0;
    std::error_code error;
    while (copy_cursor_ < items_.size() && last_slice_items_ < maximum_items) {
        const auto& item = items_[copy_cursor_];
        const auto source = normalized_source_ / item.relative_path;
        const auto destination = stage_root_ / item.relative_path;
        if (item.directory) {
            std::filesystem::create_directories(destination, error);
        } else {
            std::filesystem::create_directories(destination.parent_path(), error);
            if (!error) std::filesystem::copy_file(source, destination, std::filesystem::copy_options::none, error);
        }
        if (error) {
            fail("project_import_copy_failed", "Project import could not stage " + item.relative_path.generic_string() +
                                                   ": " + error.message());
            return false;
        }
        ++copy_cursor_;
        ++last_slice_items_;
    }
    if (copy_cursor_ == items_.size()) {
        state_ = ProjectImportJobState::Finalizing;
        code_ = "project_import_finalizing";
        message_ = "Validating and publishing the imported project.";
    }
    return true;
}

bool ProjectImportJob::finalize() {
    std::ifstream input(stage_root_ / "project.json", std::ios::binary);
    auto manifest = nlohmann::json::parse(input, nullptr, false);
    if (!manifest.is_object()) {
        fail("project_import_staged_manifest_invalid", "The staged project manifest is invalid.");
        return false;
    }
    // Windows prevents the atomic manifest replacement and final directory
    // publication while the discovery read handle remains open.
    input.close();
    manifest["project_id"] = request_.project_id;
    manifest["project_name"] = request_.project_name;
    if (manifest.contains("id")) manifest["id"] = request_.project_id;
    if (manifest.contains("name")) manifest["name"] = request_.project_name;
    std::string writeError;
    if (!urpg::SaveJournal::WriteAtomically(stage_root_ / "project.json", manifest.dump(2) + "\n", &writeError)) {
        fail("project_import_manifest_write_failed", "The imported project identity could not be written: " + writeError);
        return false;
    }
    const auto inspection = EditorProjectSession{}.inspectProject(stage_root_);
    if (!inspection.result.success || inspection.identity.project_id != request_.project_id) {
        fail("project_import_staged_project_invalid", inspection.result.message);
        return false;
    }
    std::error_code error;
    std::filesystem::rename(stage_root_, normalized_destination_, error);
    if (error) {
        fail("project_import_publish_failed", "The staged project could not be published: " + error.message());
        return false;
    }
    owns_stage_ = false;
    state_ = ProjectImportJobState::Completed;
    code_ = "project_import_completed";
    message_ = "Imported project published successfully.";
    return true;
}

bool ProjectImportJob::advance(const std::size_t maximum_items) {
    if (maximum_items == 0 || state_ == ProjectImportJobState::Completed ||
        state_ == ProjectImportJobState::Failed || state_ == ProjectImportJobState::Cancelled) return false;
    if (state_ == ProjectImportJobState::Pending && !preflight()) return false;
    if (state_ == ProjectImportJobState::Discovering) return advanceDiscovery(maximum_items);
    if (state_ == ProjectImportJobState::Copying) return advanceCopy(maximum_items);
    if (state_ == ProjectImportJobState::Finalizing) return finalize();
    return false;
}

void ProjectImportJob::cancel() {
    if (state_ == ProjectImportJobState::Completed || state_ == ProjectImportJobState::Failed) return;
    discardStage();
    state_ = ProjectImportJobState::Cancelled;
    code_ = "project_import_cancelled";
    message_ = "Project import was cancelled before publication.";
}

ProjectImportJobSnapshot ProjectImportJob::snapshot() const {
    return {state_, code_, message_, items_.size(), copy_cursor_, last_slice_items_, normalized_destination_,
            state_ == ProjectImportJobState::Completed || state_ == ProjectImportJobState::Failed ||
                state_ == ProjectImportJobState::Cancelled,
            state_ == ProjectImportJobState::Completed};
}

void ProjectImportJob::fail(std::string code, std::string message) {
    code_ = std::move(code);
    message_ = std::move(message);
    state_ = ProjectImportJobState::Failed;
    discardStage();
}

void ProjectImportJob::discardStage() {
    if (!owns_stage_ || stage_root_.empty()) return;
    std::error_code error;
    const auto expectedParent = normalized_destination_.parent_path();
    if (stage_root_.parent_path() == expectedParent && stage_root_.filename().string().ends_with(".urpg-import-stage")) {
        std::filesystem::remove_all(stage_root_, error);
    }
    owns_stage_ = false;
}

} // namespace urpg::editor
