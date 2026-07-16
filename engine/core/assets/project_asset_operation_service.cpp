#include "engine/core/assets/project_asset_operation_service.h"

#include <utility>

namespace urpg::assets {
namespace {

urpg::project::ProjectReferenceChangeKind referenceKind(const ProjectAssetOperationKind kind) {
    using ReferenceKind = urpg::project::ProjectReferenceChangeKind;
    switch (kind) {
    case ProjectAssetOperationKind::Relink: return ReferenceKind::Relink;
    case ProjectAssetOperationKind::Detach:
    case ProjectAssetOperationKind::Delete: return ReferenceKind::Delete;
    case ProjectAssetOperationKind::Replace:
    case ProjectAssetOperationKind::Deduplicate: return ReferenceKind::Replace;
    case ProjectAssetOperationKind::Rename: return ReferenceKind::Rename;
    case ProjectAssetOperationKind::Move: return ReferenceKind::Move;
    }
    return ReferenceKind::Replace;
}

std::string operationLabel(const ProjectAssetOperationKind kind) {
    switch (kind) {
    case ProjectAssetOperationKind::Relink: return "Relink Asset";
    case ProjectAssetOperationKind::Detach: return "Detach Asset";
    case ProjectAssetOperationKind::Replace: return "Replace Asset";
    case ProjectAssetOperationKind::Deduplicate: return "Deduplicate Asset";
    case ProjectAssetOperationKind::Rename: return "Rename Asset";
    case ProjectAssetOperationKind::Move: return "Move Asset";
    case ProjectAssetOperationKind::Delete: return "Delete Asset";
    }
    return "Change Asset";
}

} // namespace

const char* projectAssetOperationName(const ProjectAssetOperationKind kind) {
    switch (kind) {
    case ProjectAssetOperationKind::Relink: return "relink";
    case ProjectAssetOperationKind::Detach: return "detach";
    case ProjectAssetOperationKind::Replace: return "replace";
    case ProjectAssetOperationKind::Deduplicate: return "deduplicate";
    case ProjectAssetOperationKind::Rename: return "rename";
    case ProjectAssetOperationKind::Move: return "move";
    case ProjectAssetOperationKind::Delete: return "delete";
    }
    return "replace";
}

ProjectAssetOperationPreview ProjectAssetOperationService::preview(
    const urpg::project::ProjectReferenceIndex& index, const AssetLibrary& library,
    const ProjectAssetOperationRequest& request) const {
    ProjectAssetOperationPreview result;
    result.request = request;
    result.label = operationLabel(request.kind);
    if (request.operation_id.empty() || request.source_asset_id.empty()) {
        result.code = "project_asset_operation_invalid";
        result.message = "Asset operations require stable operation and source asset IDs.";
        return result;
    }
    if (request.kind == ProjectAssetOperationKind::Deduplicate) {
        const auto source = library.findAsset(request.source_asset_id);
        const auto replacement = library.findAsset(request.replacement_asset_id);
        if (!source || !replacement || source->sha256.empty() || source->sha256 != replacement->sha256) {
            result.code = "project_asset_deduplicate_hash_mismatch";
            result.message = "Deduplication requires two catalog assets with the same non-empty content hash.";
            return result;
        }
    }
    urpg::project::ProjectReferenceChangeRequest referenceRequest;
    referenceRequest.operation_id = request.operation_id;
    referenceRequest.kind = referenceKind(request.kind);
    referenceRequest.object_type = "asset";
    referenceRequest.source_id = request.source_asset_id;
    referenceRequest.replacement_id = request.replacement_asset_id;
    referenceRequest.destination_document = request.destination_document;
    result.reference_plan = urpg::project::previewProjectReferenceChange(index, referenceRequest);
    result.success = result.reference_plan.success;
    result.applicable = result.reference_plan.applicable;
    result.code = result.reference_plan.code;
    result.message = result.reference_plan.message;
    return result;
}

urpg::project::ProjectOperationResult ProjectAssetOperationService::execute(
    const ProjectAssetOperationPreview& preview,
    std::vector<urpg::project::ProjectOperationParticipant> participants) {
    if (!preview.success || !preview.applicable) {
        return {false, false, "project_asset_operation_preview_not_applicable",
                "The asset operation cannot execute until its impact preview is applicable.", {}};
    }
    return coordinator_.execute({preview.request.operation_id, preview.label, std::move(participants)});
}

urpg::project::ProjectOperationResult ProjectAssetOperationService::undoLast() { return coordinator_.undoLast(); }
urpg::project::ProjectOperationResult ProjectAssetOperationService::redoLast() { return coordinator_.redoLast(); }

} // namespace urpg::assets
