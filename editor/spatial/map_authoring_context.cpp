#include "editor/spatial/map_authoring_context.h"

#include <utility>

namespace urpg::editor {

const char* MapAuthoringContext::ownerName(MapAuthoringDocumentOwner owner) {
    return owner == MapAuthoringDocumentOwner::GridParts ? "grid_parts" : "perspective_2d";
}

void MapAuthoringContext::setProjectRoot(std::filesystem::path projectRoot) { snapshot_.projectRoot = std::move(projectRoot); }
void MapAuthoringContext::setActiveMapId(std::string mapId) { snapshot_.activeMapId = std::move(mapId); }
void MapAuthoringContext::setSelection(MapAuthoringSelection selection) { snapshot_.selection = std::move(selection); }
void MapAuthoringContext::setValidation(MapAuthoringValidationSummary validation) { snapshot_.validation = std::move(validation); }
void MapAuthoringContext::setPlaytestState(std::string state) { snapshot_.playtestState = std::move(state); }
void MapAuthoringContext::setPackageState(std::string state) { snapshot_.packageState = std::move(state); }
void MapAuthoringContext::setDocumentDirty(MapAuthoringDocumentOwner owner, bool dirty) {
    if (owner == MapAuthoringDocumentOwner::GridParts) snapshot_.gridPartsDirty = dirty;
    else snapshot_.perspective2DDirty = dirty;
}

void MapAuthoringContext::setChildHistoryAvailability(const bool canUndo, const bool canRedo,
                                                      const MapAuthoringDocumentOwner owner) {
    snapshot_.canUndo = canUndo;
    snapshot_.canRedo = canRedo;
    snapshot_.historyOwner = (canUndo || canRedo) ? ownerName(owner) : "";
}

void MapAuthoringContext::markAffectedDirty(const MapAuthoringHistoryEntry& entry) {
    snapshot_.gridPartsDirty = snapshot_.gridPartsDirty || entry.affectsGridParts;
    snapshot_.perspective2DDirty = snapshot_.perspective2DDirty || entry.affectsPerspective2D;
}

void MapAuthoringContext::recordCommand(MapAuthoringHistoryEntry entry) {
    if (entry.commandId.empty()) return;
    if (!entry.affectsGridParts && !entry.affectsPerspective2D) {
        entry.affectsGridParts = entry.owner == MapAuthoringDocumentOwner::GridParts;
        entry.affectsPerspective2D = entry.owner == MapAuthoringDocumentOwner::Perspective2D;
    }
    markAffectedDirty(entry);
    history_.push_back(std::move(entry));
    redo_.clear();
    syncHistoryState();
}

bool MapAuthoringContext::undo(MapAuthoringHistoryEntry* restored) {
    if (history_.empty()) return false;
    auto entry = std::move(history_.back());
    history_.pop_back();
    if (restored) *restored = entry;
    redo_.push_back(std::move(entry));
    syncHistoryState();
    return true;
}

bool MapAuthoringContext::redo(MapAuthoringHistoryEntry* restored) {
    if (redo_.empty()) return false;
    auto entry = std::move(redo_.back());
    redo_.pop_back();
    markAffectedDirty(entry);
    if (restored) *restored = entry;
    history_.push_back(std::move(entry));
    syncHistoryState();
    return true;
}

void MapAuthoringContext::markSaved(MapAuthoringDocumentOwner owner) {
    if (owner == MapAuthoringDocumentOwner::GridParts) snapshot_.gridPartsDirty = false;
    else snapshot_.perspective2DDirty = false;
}

void MapAuthoringContext::clear() { snapshot_ = {}; history_.clear(); redo_.clear(); }

void MapAuthoringContext::syncHistoryState() {
    snapshot_.canUndo = !history_.empty();
    snapshot_.canRedo = !redo_.empty();
    snapshot_.historyOwner = history_.empty() ? "" : ownerName(history_.back().owner);
}

} // namespace urpg::editor
