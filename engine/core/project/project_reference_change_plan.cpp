#include "engine/core/project/project_reference_change_plan.h"

#include <algorithm>

namespace urpg::project {
namespace {

bool requiresReplacement(const ProjectReferenceChangeKind kind) {
    return kind == ProjectReferenceChangeKind::Rename || kind == ProjectReferenceChangeKind::Replace ||
           kind == ProjectReferenceChangeKind::Relink;
}

} // namespace

ProjectReferenceChangePlan previewProjectReferenceChange(const ProjectReferenceIndex& index,
                                                         const ProjectReferenceChangeRequest& request) {
    ProjectReferenceChangePlan plan;
    plan.request = request;
    if (request.operation_id.empty() || request.object_type.empty() || request.source_id.empty() ||
        (requiresReplacement(request.kind) &&
         (request.replacement_id.empty() || request.replacement_id == request.source_id)) ||
        (request.kind == ProjectReferenceChangeKind::Move && request.destination_document.empty())) {
        plan.code = "project_reference_change_request_invalid";
        plan.message = "Reference changes require a stable operation, source object, and valid destination.";
        return plan;
    }

    if (request.kind == ProjectReferenceChangeKind::Delete) {
        plan.blocked_references = index.findUses(request.object_type, request.source_id).matches;
        std::copy_if(plan.blocked_references.begin(), plan.blocked_references.end(),
                     std::back_inserter(plan.package_impact), [](const auto& edge) { return edge.package_inclusion; });
        plan.success = true;
        plan.applicable = plan.blocked_references.empty();
        plan.code = plan.applicable ? "project_reference_delete_safe" : "project_reference_delete_blocked";
        plan.message = plan.applicable ? "No indexed inbound references block deletion."
                                       : "Deletion is blocked until every indexed inbound reference is resolved.";
        return plan;
    }

    if (request.kind == ProjectReferenceChangeKind::Move) {
        const auto references = index.findReferences(request.object_type, request.source_id).matches;
        if (references.empty()) {
            plan.code = "project_reference_move_source_not_found";
            plan.message = "No indexed source object is available to move.";
            return plan;
        }
        const auto originalDocument = references.front().document_path;
        if (std::any_of(references.begin(), references.end(), [&](const auto& edge) {
                return edge.document_path.lexically_normal() != originalDocument.lexically_normal();
            })) {
            plan.blocked_references = references;
            plan.success = true;
            plan.code = "project_reference_move_multiple_owners";
            plan.message = "Move preview is blocked because the source ID is emitted by multiple documents.";
            return plan;
        }
        for (const auto& edge : references) {
            auto after = edge;
            after.document_path = request.destination_document.lexically_normal();
            plan.updates.push_back({edge, std::move(after)});
        }
        plan.inverse_request = {request.operation_id + ".inverse", ProjectReferenceChangeKind::Move,
                                request.object_type, request.source_id, {}, originalDocument};
    } else {
        const auto uses = index.findUses(request.object_type, request.source_id).matches;
        for (const auto& edge : uses) {
            auto after = edge;
            after.target_id = request.replacement_id;
            plan.updates.push_back({edge, std::move(after)});
            if (edge.package_inclusion) plan.package_impact.push_back(edge);
        }
        plan.inverse_request = {request.operation_id + ".inverse", request.kind, request.object_type,
                                request.replacement_id, request.source_id, {}};
    }
    plan.success = true;
    plan.applicable = true;
    plan.code = "project_reference_change_preview_ready";
    plan.message = "Reference change preview is ready for typed owner validation.";
    return plan;
}

} // namespace urpg::project
