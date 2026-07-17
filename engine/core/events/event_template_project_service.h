#pragma once

#include "engine/core/events/event_template_library.h"
#include "engine/core/project/project_operation_coordinator.h"

#include <filesystem>
#include <optional>
#include <string>

namespace urpg::events {

struct EventTemplateProjectResult {
    bool success = false;
    std::string code;
    std::string message;
    EventTemplateImpact impact;
};

// Owns the durable aggregate that keeps template definitions, instances, and
// the rendered event document at one atomic project-file boundary. Reviewed
// changes participate in the shared project operation history.
class EventTemplateProjectService {
public:
    explicit EventTemplateProjectService(project::ProjectOperationCoordinator& coordinator)
        : coordinator_(coordinator) {}

    EventTemplateProjectResult initialize(const std::filesystem::path& project_root,
                                          EventTemplateLibrary library,
                                          EventDocument document);
    EventTemplateProjectResult open(const std::filesystem::path& project_root);

    EventTemplateImpact previewUpdate(const EventTemplateDefinition& replacement) const;
    EventTemplateImpact previewDelete(const std::string& template_id,
                                      const std::string& replacement_template_id) const;
    EventTemplateProjectResult applyReviewedUpdate(std::string operation_id,
                                                   EventTemplateDefinition replacement,
                                                   uint64_t expected_revision);
    EventTemplateProjectResult applyReviewedDelete(std::string operation_id,
                                                   std::string template_id,
                                                   std::string replacement_template_id,
                                                   uint64_t expected_revision);
    project::ProjectOperationResult undoLast();
    project::ProjectOperationResult redoLast();

    const EventTemplateLibrary& library() const { return library_; }
    const EventDocument& document() const { return document_; }
    uint64_t revision() const { return revision_; }
    const std::filesystem::path& documentPath() const { return document_path_; }
    bool isOpen() const { return !document_path_.empty(); }

    static std::filesystem::path projectDocumentPath(const std::filesystem::path& project_root);

private:
    struct State {
        EventTemplateLibrary library;
        EventDocument document;
    };

    EventTemplateProjectResult executeReviewed(std::string operation_id,
                                               std::string label,
                                               State next,
                                               EventTemplateImpact impact,
                                               uint64_t expected_revision);
    bool writeState(const State& state, uint64_t revision, std::string& diagnostic) const;
    std::string recoverySnapshot() const;

    project::ProjectOperationCoordinator& coordinator_;
    EventTemplateLibrary library_;
    EventDocument document_;
    uint64_t revision_ = 0;
    std::filesystem::path document_path_;
};

} // namespace urpg::events
