#pragma once

#include "engine/core/accessibility/inclusive_experience.h"

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::dialogue { class DialogueGraph; }
namespace urpg::map { class ProjectWorldGraph; class TileLayerDocument; }
namespace urpg::quest { class QuestObjectiveGraphDocument; }
namespace urpg::ui { class MenuAuthoringDocument; }

namespace urpg::editor {

enum class SemanticEditorNavigation { First, Previous, Next, Last };

struct SemanticEditorCommandResult {
    bool applied = false;
    std::string code;
    std::string message;
    std::vector<std::string> diagnostics;
};

struct SemanticEditorDiagnostic {
    std::string code;
    std::string message;
    std::string object_id;
    std::string related_object_id;
    bool blocking = false;
};

class SemanticEditorCommandSurface {
public:
    using SnapshotProvider = std::function<accessibility::SemanticEditorAlternative()>;
    using PropertyWriter = std::function<bool(std::string_view, std::string_view, std::string_view)>;
    using ConnectionWriter = std::function<bool(std::string_view, std::string_view)>;
    using DiagnosticProvider = std::function<std::vector<SemanticEditorDiagnostic>()>;

    void bind(SnapshotProvider snapshot_provider, PropertyWriter property_writer,
              ConnectionWriter connection_writer, DiagnosticProvider diagnostic_provider = {});
    void clear();
    void refresh();

    [[nodiscard]] bool isBound() const;
    [[nodiscard]] bool canEditProperties() const;
    [[nodiscard]] bool canCreateConnections() const;
    [[nodiscard]] const std::string& selectedId() const;
    [[nodiscard]] std::vector<accessibility::SemanticEditorNode> orderedNodes() const;
    [[nodiscard]] std::vector<std::string> diagnostics() const;
    [[nodiscard]] std::vector<SemanticEditorDiagnostic> linkedDiagnostics() const;

    SemanticEditorCommandResult select(std::string_view id);
    SemanticEditorCommandResult navigate(SemanticEditorNavigation navigation);
    SemanticEditorCommandResult setSelectedProperty(std::string_view key, std::string_view value);
    SemanticEditorCommandResult connectSelectedTo(std::string_view target_id);
    SemanticEditorCommandResult focusDiagnostic(std::size_t diagnostic_index);

    [[nodiscard]] nlohmann::json renderSnapshot() const;

private:
    SemanticEditorCommandResult failure(std::string code, std::string message) const;
    SemanticEditorCommandResult success(std::string code, std::string message) const;

    SnapshotProvider snapshot_provider_;
    PropertyWriter property_writer_;
    ConnectionWriter connection_writer_;
    DiagnosticProvider diagnostic_provider_;
    accessibility::SemanticEditorAlternative alternative_;
    std::string selected_id_;
};

SemanticEditorCommandSurface semanticCommandSurfaceForMenu(ui::MenuAuthoringDocument& document);
SemanticEditorCommandSurface semanticCommandSurfaceForDialogue(dialogue::DialogueGraph& graph);
SemanticEditorCommandSurface semanticCommandSurfaceForQuest(quest::QuestObjectiveGraphDocument& document);
SemanticEditorCommandSurface semanticCommandSurfaceForTileMap(map::TileLayerDocument& document);
SemanticEditorCommandSurface semanticCommandSurfaceForWorldMap(map::ProjectWorldGraph& graph);

} // namespace urpg::editor
