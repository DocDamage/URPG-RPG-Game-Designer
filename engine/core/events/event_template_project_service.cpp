#include "engine/core/events/event_template_project_service.h"

#include <fstream>

namespace urpg::events {
namespace {

constexpr std::string_view kSchema = "urpg.event_template_project.v1";

nlohmann::json stateJson(const EventTemplateLibrary& library, const EventDocument& document,
                         const uint64_t revision) {
    return {{"schema", kSchema},
            {"revision", revision},
            {"library", library.toJson()},
            {"event_document", document.toJson()}};
}

bool atomicWrite(const std::filesystem::path& target, const nlohmann::json& value,
                 std::string& diagnostic) {
    std::error_code error;
    std::filesystem::create_directories(target.parent_path(), error);
    if (error) {
        diagnostic = "event_template_project_directory_failed:" + error.message();
        return false;
    }
    auto temporary = target;
    temporary += ".tmp";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) {
            diagnostic = "event_template_project_temporary_open_failed";
            return false;
        }
        output << value.dump(2) << '\n';
        output.close();
        if (!output) {
            std::filesystem::remove(temporary, error);
            diagnostic = "event_template_project_temporary_flush_failed";
            return false;
        }
    }
    auto backup = target;
    backup += ".bak";
    const bool replacing = std::filesystem::exists(target);
    if (replacing) {
        std::filesystem::remove(backup, error);
        error.clear();
        std::filesystem::rename(target, backup, error);
        if (error) {
            std::filesystem::remove(temporary, error);
            diagnostic = "event_template_project_backup_failed:" + error.message();
            return false;
        }
    }
    std::filesystem::rename(temporary, target, error);
    if (error) {
        std::filesystem::remove(temporary, error);
        if (replacing) {
            std::error_code restore_error;
            std::filesystem::rename(backup, target, restore_error);
        }
        diagnostic = "event_template_project_publish_failed:" + error.message();
        return false;
    }
    if (replacing) std::filesystem::remove(backup, error);
    return true;
}

} // namespace

std::filesystem::path EventTemplateProjectService::projectDocumentPath(
    const std::filesystem::path& project_root) {
    return project_root / "content" / "events" / "template_project.json";
}

EventTemplateProjectResult EventTemplateProjectService::initialize(
    const std::filesystem::path& project_root, EventTemplateLibrary library,
    EventDocument document) {
    if (project_root.empty()) {
        return {false, "event_template_project_root_invalid", "A project root is required.", {}};
    }
    document_path_ = projectDocumentPath(project_root);
    State state{std::move(library), std::move(document)};
    std::string diagnostic;
    if (!writeState(state, 1, diagnostic)) {
        document_path_.clear();
        return {false, "event_template_project_initialize_failed", std::move(diagnostic), {}};
    }
    library_ = std::move(state.library);
    document_ = std::move(state.document);
    revision_ = 1;
    return {true, "event_template_project_initialized",
            "Initialized the durable event-template project owner.", {}};
}

EventTemplateProjectResult EventTemplateProjectService::open(
    const std::filesystem::path& project_root) {
    const auto path = projectDocumentPath(project_root);
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return {false, "event_template_project_missing", "The event-template project document is missing.", {}};
    }
    const auto json = nlohmann::json::parse(input, nullptr, false);
    if (!json.is_object() || json.value("schema", "") != kSchema ||
        !json.contains("library") || !json.contains("event_document")) {
        return {false, "event_template_project_invalid", "The event-template project document is malformed.", {}};
    }
    auto library = EventTemplateLibrary::fromJson(json["library"]);
    if (!library) {
        return {false, "event_template_project_library_invalid", "The template library is malformed.", {}};
    }
    auto document = EventDocument::fromJson(json["event_document"]);
    if (!document.validate().empty()) {
        return {false, "event_template_project_events_invalid", "The rendered event document is invalid.", {}};
    }
    for (const auto& [instance_id, instance] : library->instances()) {
        (void)instance_id;
        if (!document.commonEvents().contains(instance.common_event_id)) {
            return {false, "event_template_project_instance_target_missing",
                    "A template instance refers to a missing rendered common event.", {}};
        }
    }
    const auto revision = json.value("revision", uint64_t{0});
    if (revision == 0) {
        return {false, "event_template_project_revision_invalid", "The project revision is invalid.", {}};
    }
    document_path_ = path;
    library_ = std::move(*library);
    document_ = std::move(document);
    revision_ = revision;
    return {true, "event_template_project_opened", "Opened the durable event-template project owner.", {}};
}

EventTemplateImpact EventTemplateProjectService::previewUpdate(
    const EventTemplateDefinition& replacement) const {
    return library_.previewUpdate(replacement, document_);
}

EventTemplateImpact EventTemplateProjectService::previewDelete(
    const std::string& template_id, const std::string& replacement_template_id) const {
    return library_.previewDelete(template_id, replacement_template_id, document_);
}

EventTemplateProjectResult EventTemplateProjectService::applyReviewedUpdate(
    std::string operation_id, EventTemplateDefinition replacement,
    const uint64_t expected_revision) {
    auto next = State{library_, document_};
    EventTemplateImpact impact;
    if (!next.library.applyUpdate(std::move(replacement), next.document, &impact)) {
        return {false, impact.code.empty() ? "event_template_update_invalid" : impact.code,
                "The reviewed template update is no longer valid.", impact};
    }
    return executeReviewed(std::move(operation_id), "Update event template", std::move(next),
                           std::move(impact), expected_revision);
}

EventTemplateProjectResult EventTemplateProjectService::applyReviewedDelete(
    std::string operation_id, std::string template_id,
    std::string replacement_template_id, const uint64_t expected_revision) {
    auto next = State{library_, document_};
    EventTemplateImpact impact;
    if (!next.library.deleteTemplate(template_id, replacement_template_id, next.document, &impact)) {
        return {false, impact.code.empty() ? "event_template_delete_invalid" : impact.code,
                "The reviewed template deletion or replacement is no longer valid.", impact};
    }
    return executeReviewed(std::move(operation_id), "Delete or replace event template",
                           std::move(next), std::move(impact), expected_revision);
}

EventTemplateProjectResult EventTemplateProjectService::executeReviewed(
    std::string operation_id, std::string label, State next, EventTemplateImpact impact,
    const uint64_t expected_revision) {
    if (!isOpen()) {
        return {false, "event_template_project_not_open", "Open a project before applying a review.", impact};
    }
    const State before{library_, document_};
    project::ProjectOperationParticipant participant;
    participant.owner_id = "event_template_project";
    participant.expected_source_revision = expected_revision;
    participant.current_revision = [this] { return revision_; };
    participant.prepare = [](std::string& diagnostic) {
        diagnostic.clear();
        return true;
    };
    participant.commit = [this, next](std::string& diagnostic) mutable {
        const auto next_revision = revision_ + 1;
        if (!writeState(next, next_revision, diagnostic)) return false;
        library_ = next.library;
        document_ = next.document;
        revision_ = next_revision;
        return true;
    };
    participant.rollback = [this, before] {
        std::string ignored;
        const auto next_revision = revision_ + 1;
        if (writeState(before, next_revision, ignored)) {
            library_ = before.library;
            document_ = before.document;
            revision_ = next_revision;
        }
    };
    participant.inverse = [this, before](std::string& diagnostic) {
        const auto next_revision = revision_ + 1;
        if (!writeState(before, next_revision, diagnostic)) return false;
        library_ = before.library;
        document_ = before.document;
        revision_ = next_revision;
        return true;
    };
    participant.recovery_snapshot = [this] { return recoverySnapshot(); };
    participant.document_path = document_path_;

    project::ProjectOperationRequest request;
    request.operation_id = std::move(operation_id);
    request.label = std::move(label);
    request.participants.push_back(std::move(participant));
    const auto result = coordinator_.execute(request);
    return {result.success, result.code, result.message, std::move(impact)};
}

project::ProjectOperationResult EventTemplateProjectService::undoLast() {
    return coordinator_.undoLast();
}

project::ProjectOperationResult EventTemplateProjectService::redoLast() {
    return coordinator_.redoLast();
}

bool EventTemplateProjectService::writeState(const State& state, const uint64_t revision,
                                             std::string& diagnostic) const {
    if (document_path_.empty()) {
        diagnostic = "event_template_project_path_missing";
        return false;
    }
    return atomicWrite(document_path_, stateJson(state.library, state.document, revision), diagnostic);
}

std::string EventTemplateProjectService::recoverySnapshot() const {
    return stateJson(library_, document_, revision_).dump();
}

} // namespace urpg::events
