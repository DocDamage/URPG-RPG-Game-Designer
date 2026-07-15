#include "editor/project/editor_dirty_state_registry.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

bool EditorDirtyStateRegistry::registerSurface(EditorDirtySurface surface) {
    if (surface.document_id.empty() || !surface.save || find(surface.document_id) != nullptr) {
        return false;
    }
    surfaces_.push_back(std::move(surface));
    return true;
}

bool EditorDirtyStateRegistry::markDirty(const std::string& document_id, bool dirty) {
    auto* surface = find(document_id);
    if (surface == nullptr) {
        return false;
    }
    surface->dirty = dirty;
    return true;
}

bool EditorDirtyStateRegistry::isDirty(const std::string& document_id) const {
    const auto* surface = find(document_id);
    return surface != nullptr && surface->dirty;
}

std::vector<std::string> EditorDirtyStateRegistry::dirtyDocumentIds() const {
    std::vector<std::string> ids;
    for (const auto& surface : surfaces_) {
        if (surface.dirty) {
            ids.push_back(surface.document_id);
        }
    }
    return ids;
}

EditorDirtySaveResult EditorDirtyStateRegistry::save(const std::string& document_id) {
    auto* surface = find(document_id);
    if (surface == nullptr) {
        return {false, "dirty_surface_missing", "The requested dirty surface is not registered."};
    }
    const auto result = surface->save();
    surface->last_save_result = result;
    if (result.success) {
        surface->dirty = false;
    } else if (surface->focus) {
        surface->focus();
    }
    return result;
}

EditorNavigationGuardResult EditorDirtyStateRegistry::resolveNavigation(EditorNavigationDecision decision) {
    EditorNavigationGuardResult result;
    result.dirty_document_ids = dirtyDocumentIds();
    if (result.dirty_document_ids.empty() || decision == EditorNavigationDecision::Discard) {
        result.allowed = true;
        return result;
    }
    if (decision == EditorNavigationDecision::Cancel) {
        result.diagnostic = {false, "navigation_cancelled", "Navigation was cancelled because unsaved work remains."};
        return result;
    }
    for (const auto& document_id : result.dirty_document_ids) {
        // A single atomic save can own several registered surfaces (for
        // example the paired Grid Parts and Perspective 2D map documents).
        // Do not invoke a second, redundant save after that transaction has
        // already cleared another surface in this navigation request.
        if (!isDirty(document_id)) {
            continue;
        }
        const auto save_result = save(document_id);
        if (!save_result.success) {
            result.failed_document_id = document_id;
            result.diagnostic = save_result;
            return result;
        }
    }
    result.allowed = true;
    result.dirty_document_ids.clear();
    return result;
}

EditorDirtySurface* EditorDirtyStateRegistry::find(const std::string& document_id) {
    const auto it = std::find_if(surfaces_.begin(), surfaces_.end(), [&document_id](const auto& surface) {
        return surface.document_id == document_id;
    });
    return it == surfaces_.end() ? nullptr : &*it;
}

const EditorDirtySurface* EditorDirtyStateRegistry::find(const std::string& document_id) const {
    const auto it = std::find_if(surfaces_.begin(), surfaces_.end(), [&document_id](const auto& surface) {
        return surface.document_id == document_id;
    });
    return it == surfaces_.end() ? nullptr : &*it;
}

} // namespace urpg::editor
