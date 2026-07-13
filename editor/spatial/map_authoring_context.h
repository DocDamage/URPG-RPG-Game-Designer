#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::editor {

enum class MapAuthoringDocumentOwner { GridParts, Perspective2D };

struct MapAuthoringSelection {
    std::string layerId;
    std::string objectId;
    std::string eventId;
    std::string partId;
    std::string viewportFocus;
    std::string activeTool = "select";
};

struct MapAuthoringValidationSummary {
    size_t diagnosticCount = 0;
    size_t blockingCount = 0;
    std::string nextAction = "Save the map to run validation.";
};

struct MapAuthoringHistoryEntry {
    std::string commandId;
    MapAuthoringDocumentOwner owner = MapAuthoringDocumentOwner::GridParts;
    bool affectsGridParts = false;
    bool affectsPerspective2D = false;
};

struct MapAuthoringContextSnapshot {
    std::filesystem::path projectRoot;
    std::string activeMapId;
    MapAuthoringSelection selection;
    MapAuthoringValidationSummary validation;
    std::string playtestState = "idle";
    std::string packageState = "draft";
    bool gridPartsDirty = false;
    bool perspective2DDirty = false;
    bool canUndo = false;
    bool canRedo = false;
    std::string historyOwner;
};

// Shared, UI-neutral context for both map representations. It coordinates
// lifecycle and history without converting either document.
class MapAuthoringContext {
  public:
    void setProjectRoot(std::filesystem::path projectRoot);
    void setActiveMapId(std::string mapId);
    void setSelection(MapAuthoringSelection selection);
    void setValidation(MapAuthoringValidationSummary validation);
    void setPlaytestState(std::string state);
    void setPackageState(std::string state);
    void setDocumentDirty(MapAuthoringDocumentOwner owner, bool dirty);
    void setChildHistoryAvailability(bool canUndo, bool canRedo, MapAuthoringDocumentOwner owner);
    void recordCommand(MapAuthoringHistoryEntry entry);
    bool undo(MapAuthoringHistoryEntry* restored = nullptr);
    bool redo(MapAuthoringHistoryEntry* restored = nullptr);
    void markSaved(MapAuthoringDocumentOwner owner);
    void clear();

    const MapAuthoringContextSnapshot& snapshot() const { return snapshot_; }
    const std::vector<MapAuthoringHistoryEntry>& history() const { return history_; }
    static const char* ownerName(MapAuthoringDocumentOwner owner);

  private:
    void syncHistoryState();
    void markAffectedDirty(const MapAuthoringHistoryEntry& entry);

    MapAuthoringContextSnapshot snapshot_;
    std::vector<MapAuthoringHistoryEntry> history_;
    std::vector<MapAuthoringHistoryEntry> redo_;
};

} // namespace urpg::editor
