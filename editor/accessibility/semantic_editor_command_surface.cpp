#include "editor/accessibility/semantic_editor_command_surface.h"

#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/map/project_world_graph.h"
#include "engine/core/map/tile_layer_document.h"
#include "engine/core/quest/quest_objective_graph.h"
#include "engine/core/ui/menu_authoring_document.h"

#include <algorithm>
#include <cctype>

namespace urpg::editor {
namespace {

std::optional<bool> parseBool(std::string_view value) {
    std::string normalized(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char item) { return static_cast<char>(std::tolower(item)); });
    if (normalized == "true" || normalized == "1" || normalized == "yes" || normalized == "on") return true;
    if (normalized == "false" || normalized == "0" || normalized == "no" || normalized == "off") return false;
    return std::nullopt;
}

std::vector<std::string> dialogueDiagnostics(const dialogue::DialogueGraph& graph) {
    std::vector<std::string> result;
    auto diagnostics = graph.validate();
    const auto flow = graph.analyzeFlow();
    diagnostics.insert(diagnostics.end(), flow.begin(), flow.end());
    for (const auto& item : diagnostics) result.push_back(item.code + ": " + item.message);
    return result;
}

std::vector<std::string> questDiagnostics(const quest::QuestObjectiveGraphDocument& document) {
    std::vector<std::string> result;
    auto diagnostics = document.validate();
    const auto flow = document.analyzeFlow();
    diagnostics.insert(diagnostics.end(), flow.begin(), flow.end());
    for (const auto& item : diagnostics) result.push_back(item.code + ": " + item.message);
    return result;
}

} // namespace

void SemanticEditorCommandSurface::bind(SnapshotProvider snapshot_provider, PropertyWriter property_writer,
                                        ConnectionWriter connection_writer,
                                        DiagnosticProvider diagnostic_provider) {
    snapshot_provider_ = std::move(snapshot_provider);
    property_writer_ = std::move(property_writer);
    connection_writer_ = std::move(connection_writer);
    diagnostic_provider_ = std::move(diagnostic_provider);
    selected_id_.clear();
    refresh();
}

void SemanticEditorCommandSurface::clear() {
    snapshot_provider_ = {};
    property_writer_ = {};
    connection_writer_ = {};
    diagnostic_provider_ = {};
    alternative_ = {};
    selected_id_.clear();
}

void SemanticEditorCommandSurface::refresh() {
    if (!snapshot_provider_) {
        alternative_ = {};
        selected_id_.clear();
        return;
    }
    alternative_ = snapshot_provider_();
    const auto rows = alternative_.orderedNodes();
    if (rows.empty()) {
        selected_id_.clear();
        return;
    }
    const auto selected = std::find_if(rows.begin(), rows.end(),
                                       [&](const auto& row) { return row.id == selected_id_; });
    if (selected == rows.end()) selected_id_ = rows.front().id;
}

bool SemanticEditorCommandSurface::isBound() const { return static_cast<bool>(snapshot_provider_); }
bool SemanticEditorCommandSurface::canEditProperties() const { return static_cast<bool>(property_writer_); }
bool SemanticEditorCommandSurface::canCreateConnections() const { return static_cast<bool>(connection_writer_); }
const std::string& SemanticEditorCommandSurface::selectedId() const { return selected_id_; }

std::vector<accessibility::SemanticEditorNode> SemanticEditorCommandSurface::orderedNodes() const {
    return alternative_.orderedNodes();
}

std::vector<std::string> SemanticEditorCommandSurface::diagnostics() const {
    auto result = alternative_.diagnostics();
    if (diagnostic_provider_) {
        auto owner = diagnostic_provider_();
        result.insert(result.end(), owner.begin(), owner.end());
    }
    return result;
}

SemanticEditorCommandResult SemanticEditorCommandSurface::failure(std::string code, std::string message) const {
    return {false, std::move(code), std::move(message), diagnostics()};
}

SemanticEditorCommandResult SemanticEditorCommandSurface::success(std::string code, std::string message) const {
    return {true, std::move(code), std::move(message), diagnostics()};
}

SemanticEditorCommandResult SemanticEditorCommandSurface::select(std::string_view id) {
    const auto rows = orderedNodes();
    if (std::none_of(rows.begin(), rows.end(), [&](const auto& row) { return row.id == id; }))
        return failure("semantic_selection_missing", "The requested semantic row does not exist.");
    selected_id_ = std::string(id);
    return success("semantic_selection_changed", "Semantic editor selection changed.");
}

SemanticEditorCommandResult SemanticEditorCommandSurface::navigate(SemanticEditorNavigation navigation) {
    const auto rows = orderedNodes();
    if (rows.empty()) return failure("semantic_navigation_empty", "The semantic editor has no rows.");
    auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.id == selected_id_; });
    std::size_t index = found == rows.end() ? 0 : static_cast<std::size_t>(std::distance(rows.begin(), found));
    switch (navigation) {
    case SemanticEditorNavigation::First: index = 0; break;
    case SemanticEditorNavigation::Last: index = rows.size() - 1; break;
    case SemanticEditorNavigation::Previous: index = index == 0 ? rows.size() - 1 : index - 1; break;
    case SemanticEditorNavigation::Next: index = (index + 1) % rows.size(); break;
    }
    selected_id_ = rows[index].id;
    return success("semantic_navigation_applied", "Semantic editor navigation applied.");
}

SemanticEditorCommandResult SemanticEditorCommandSurface::setSelectedProperty(std::string_view key,
                                                                               std::string_view value) {
    if (selected_id_.empty()) return failure("semantic_selection_required", "Select a row before editing properties.");
    if (!property_writer_)
        return failure("semantic_property_unsupported", "This semantic owner does not expose property editing.");
    if (key.empty() || !property_writer_(selected_id_, key, value))
        return failure("semantic_property_rejected", "The semantic property change was rejected by its document owner.");
    refresh();
    return success("semantic_property_applied", "Semantic property updated through its document owner.");
}

SemanticEditorCommandResult SemanticEditorCommandSurface::connectSelectedTo(std::string_view target_id) {
    if (selected_id_.empty()) return failure("semantic_selection_required", "Select a source row before connecting.");
    if (!connection_writer_)
        return failure("semantic_connection_unsupported", "This semantic owner does not define connections.");
    if (target_id.empty() || !connection_writer_(selected_id_, target_id))
        return failure("semantic_connection_rejected", "The semantic connection was rejected by its document owner.");
    refresh();
    return success("semantic_connection_applied", "Semantic connection created through its document owner.");
}

nlohmann::json SemanticEditorCommandSurface::renderSnapshot() const {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& row : orderedNodes()) {
        rows.push_back({{"id", row.id}, {"label", row.label}, {"kind", row.kind}, {"order", row.order},
                        {"properties", row.properties}, {"connections", row.connections},
                        {"selected", row.id == selected_id_}});
    }
    return {{"bound", isBound()}, {"selected_id", selected_id_}, {"rows", std::move(rows)},
            {"diagnostics", diagnostics()}, {"property_editing", canEditProperties()},
            {"connection_creation", canCreateConnections()},
            {"keyboard_commands", {"Home: first row", "End: last row", "Up: previous row",
                                    "Down: next row", "Enter: edit selected properties",
                                    "Ctrl+Enter: connect selected row"}}};
}

SemanticEditorCommandSurface semanticCommandSurfaceForMenu(ui::MenuAuthoringDocument& document) {
    SemanticEditorCommandSurface surface;
    surface.bind(
        [&document] { return accessibility::semanticAlternativeForMenu(document); },
        [&document](std::string_view id, std::string_view key, std::string_view value) {
            const auto* source = document.findNode(id);
            if (source == nullptr) return false;
            auto node = *source;
            if (key == "label") node.label = value;
            else if (key == "accessible_label") node.accessible_label = value;
            else if (key == "visible") { const auto parsed = parseBool(value); if (!parsed) return false; node.visible = *parsed; }
            else if (key == "enabled") { const auto parsed = parseBool(value); if (!parsed) return false; node.enabled = *parsed; }
            else return false;
            return document.updateNode(std::move(node));
        },
        [&document](std::string_view parent, std::string_view child) {
            const auto* source = document.findNode(child);
            if (source == nullptr || document.findNode(parent) == nullptr || parent == child) return false;
            auto node = *source;
            node.parent_id = parent;
            return document.updateNode(std::move(node));
        });
    return surface;
}

SemanticEditorCommandSurface semanticCommandSurfaceForDialogue(dialogue::DialogueGraph& graph) {
    SemanticEditorCommandSurface surface;
    surface.bind(
        [&graph] { return accessibility::semanticAlternativeForDialogue(graph); },
        [&graph](std::string_view id, std::string_view key, std::string_view value) {
            const auto* node = graph.findNode(std::string(id));
            if (node == nullptr) return false;
            auto speaker_id = node->speaker_id;
            auto speaker_name = node->speaker_name;
            auto localization_key = node->localization_key;
            auto text_preview = node->text_preview;
            auto ending = node->ending;
            if (key == "speaker_id") speaker_id = value;
            else if (key == "speaker_name") speaker_name = value;
            else if (key == "localization_key") localization_key = value;
            else if (key == "text_preview") text_preview = value;
            else if (key == "ending") { const auto parsed = parseBool(value); if (!parsed) return false; ending = *parsed; }
            else return false;
            return graph.updateNode(std::string(id), std::move(speaker_id), std::move(speaker_name),
                                    std::move(localization_key), std::move(text_preview), ending);
        },
        [&graph](std::string_view from, std::string_view to) {
            const auto* source = graph.findNode(std::string(from));
            if (source == nullptr || graph.findNode(std::string(to)) == nullptr || from == to) return false;
            if (std::any_of(source->choices.begin(), source->choices.end(),
                            [&](const auto& choice) { return choice.target_node_id == to; })) return false;
            const auto choice_id = "semantic_" + std::string(from) + "_to_" + std::string(to);
            return graph.addChoice(std::string(from), {choice_id, "Continue", std::string(to), {}, {}, {}});
        },
        [&graph] { return dialogueDiagnostics(graph); });
    return surface;
}

SemanticEditorCommandSurface semanticCommandSurfaceForQuest(quest::QuestObjectiveGraphDocument& document) {
    SemanticEditorCommandSurface surface;
    surface.bind(
        [&document] { return accessibility::semanticAlternativeForQuest(document); },
        [&document](std::string_view id, std::string_view key, std::string_view value) {
            const auto found = std::find_if(document.nodes.begin(), document.nodes.end(),
                                            [&](const auto& node) { return node.id == id; });
            if (found == document.nodes.end()) return false;
            if (key == "title") found->title = value;
            else if (key == "type") found->type = value;
            else if (key == "objective_id") found->objective_id = value;
            else if (key == "localization_key") found->localization_key = value;
            else return false;
            return true;
        },
        [&document](std::string_view from, std::string_view to) {
            return document.connect(std::string(from), std::string(to));
        },
        [&document] { return questDiagnostics(document); });
    return surface;
}

SemanticEditorCommandSurface semanticCommandSurfaceForTileMap(map::TileLayerDocument& document) {
    SemanticEditorCommandSurface surface;
    surface.bind(
        [&document] { return accessibility::semanticAlternativeForTileMap(document); },
        [&document](std::string_view id, std::string_view key, std::string_view value) {
            const auto found = std::find_if(document.layers().begin(), document.layers().end(),
                                            [&](const auto& layer) { return layer.id == id; });
            if (found == document.layers().end()) return false;
            auto layer = *found;
            if (key == "visible") { const auto parsed = parseBool(value); if (!parsed) return false; layer.visible = *parsed; }
            else if (key == "locked") { const auto parsed = parseBool(value); if (!parsed) return false; layer.locked = *parsed; }
            else if (key == "collision") { const auto parsed = parseBool(value); if (!parsed) return false; layer.collision = *parsed; }
            else if (key == "navigation") { const auto parsed = parseBool(value); if (!parsed) return false; layer.navigation = *parsed; }
            else return false;
            return document.updateLayer(std::move(layer));
        }, {});
    return surface;
}

SemanticEditorCommandSurface semanticCommandSurfaceForWorldMap(map::ProjectWorldGraph& graph) {
    SemanticEditorCommandSurface surface;
    surface.bind(
        [&graph] { return accessibility::semanticAlternativeForWorldMap(graph); },
        [&graph](std::string_view id, std::string_view key, std::string_view value) {
            const auto found = std::find_if(graph.maps().begin(), graph.maps().end(),
                                            [&](const auto& map) { return map.id == id; });
            if (found == graph.maps().end() || key != "label" || value.empty()) return false;
            auto map = *found;
            map.label = value;
            return graph.updateMap(std::move(map));
        },
        [&graph](std::string_view from, std::string_view to) {
            const auto source = std::find_if(graph.maps().begin(), graph.maps().end(),
                                             [&](const auto& map) { return map.id == from; });
            const auto target = std::find_if(graph.maps().begin(), graph.maps().end(),
                                             [&](const auto& map) { return map.id == to; });
            if (source == graph.maps().end() || target == graph.maps().end() || from == to ||
                source->exits.empty() || target->entrances.empty()) return false;
            const auto route_id = "semantic_" + std::string(from) + "_to_" + std::string(to);
            return graph.addRoute({route_id, source->label + " to " + target->label, std::string(from),
                                   source->exits.front().id, std::string(to), target->entrances.front().id, {}});
        },
        [&graph] {
            std::vector<std::string> result;
            for (const auto& item : graph.validate()) result.push_back(item.code + ": " + item.message);
            return result;
        });
    return surface;
}

} // namespace urpg::editor
