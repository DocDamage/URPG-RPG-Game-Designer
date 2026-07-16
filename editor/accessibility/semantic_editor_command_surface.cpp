#include "editor/accessibility/semantic_editor_command_surface.h"

#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/map/project_world_graph.h"
#include "engine/core/map/tile_layer_document.h"
#include "engine/core/quest/quest_objective_graph.h"
#include "engine/core/ui/menu_authoring_document.h"

#include <algorithm>
#include <cctype>
#include <charconv>

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

std::optional<int> parseInt(std::string_view value) {
    int result = 0;
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size()) return std::nullopt;
    return result;
}

std::vector<SemanticEditorDiagnostic> dialogueDiagnostics(const dialogue::DialogueGraph& graph) {
    std::vector<SemanticEditorDiagnostic> result;
    auto diagnostics = graph.validate();
    const auto flow = graph.analyzeFlow();
    diagnostics.insert(diagnostics.end(), flow.begin(), flow.end());
    for (const auto& item : diagnostics)
        result.push_back({item.code, item.message, item.node_id, item.choice_id, true});
    return result;
}

std::vector<SemanticEditorDiagnostic> questDiagnostics(const quest::QuestObjectiveGraphDocument& document) {
    std::vector<SemanticEditorDiagnostic> result;
    auto diagnostics = document.validate();
    const auto flow = document.analyzeFlow();
    diagnostics.insert(diagnostics.end(), flow.begin(), flow.end());
    for (const auto& item : diagnostics)
        result.push_back({item.code, item.message, item.node_id, {}, true});
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
    std::vector<std::string> result;
    for (const auto& item : linkedDiagnostics()) result.push_back(item.code + ": " + item.message);
    return result;
}

std::vector<SemanticEditorDiagnostic> SemanticEditorCommandSurface::linkedDiagnostics() const {
    std::vector<SemanticEditorDiagnostic> result;
    const auto rows = orderedNodes();
    for (const auto& row : rows) {
        for (const auto& connection : row.connections) {
            if (std::none_of(rows.begin(), rows.end(), [&](const auto& target) { return target.id == connection; }))
                result.push_back({"semantic_connection_target_missing", "Connection target is missing.",
                                  row.id, connection, true});
        }
    }
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

SemanticEditorCommandResult SemanticEditorCommandSurface::focusDiagnostic(const std::size_t diagnostic_index) {
    const auto linked = linkedDiagnostics();
    if (diagnostic_index >= linked.size())
        return failure("semantic_diagnostic_missing", "The requested semantic diagnostic does not exist.");
    if (linked[diagnostic_index].object_id.empty())
        return failure("semantic_diagnostic_not_linked", "This diagnostic does not identify an editor object.");
    const auto selected = select(linked[diagnostic_index].object_id);
    if (!selected.applied)
        return failure("semantic_diagnostic_object_missing", "The diagnostic's editor object no longer exists.");
    return success("semantic_diagnostic_focused", "Focused the object linked to the semantic diagnostic.");
}

nlohmann::json SemanticEditorCommandSurface::renderSnapshot() const {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& row : orderedNodes()) {
        rows.push_back({{"id", row.id}, {"label", row.label}, {"kind", row.kind}, {"order", row.order},
                        {"properties", row.properties}, {"connections", row.connections},
                        {"selected", row.id == selected_id_}});
    }
    nlohmann::json linked_diagnostics = nlohmann::json::array();
    const auto ordered = orderedNodes();
    for (const auto& item : linkedDiagnostics()) {
        const bool focusable = !item.object_id.empty() &&
            std::any_of(ordered.begin(), ordered.end(), [&](const auto& row) { return row.id == item.object_id; });
        linked_diagnostics.push_back({{"code", item.code}, {"message", item.message},
                                      {"object_id", item.object_id},
                                      {"related_object_id", item.related_object_id},
                                      {"blocking", item.blocking}, {"focusable", focusable}});
    }
    return {{"bound", isBound()}, {"selected_id", selected_id_}, {"rows", std::move(rows)},
            {"diagnostics", diagnostics()}, {"linked_diagnostics", std::move(linked_diagnostics)},
            {"property_editing", canEditProperties()},
            {"connection_creation", canCreateConnections()},
            {"keyboard_commands", {"Home: first row", "End: last row", "Up: previous row",
                                    "Down: next row", "Enter: edit selected properties",
                                    "Ctrl+Enter: connect selected row"}},
            {"controller_commands", {"Left Shoulder: first row", "Right Shoulder: last row",
                                      "D-pad Up: previous row", "D-pad Down: next row",
                                      "South button: edit selected properties",
                                      "Right Trigger + South button: connect selected row"}},
            {"operation_routes", {{"tree_list_navigation", true},
                                   {"property_editing", canEditProperties()},
                                   {"connection_creation", canCreateConnections()},
                                   {"object_linked_diagnostics", true},
                                   {"keyboard_navigation", true},
                                   {"controller_navigation", true}}}};
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
            else if (key == "x") { const auto parsed = parseInt(value); if (!parsed) return false; node.layout.x = *parsed; }
            else if (key == "y") { const auto parsed = parseInt(value); if (!parsed) return false; node.layout.y = *parsed; }
            else if (key == "width") { const auto parsed = parseInt(value); if (!parsed) return false; node.layout.width = *parsed; }
            else if (key == "height") { const auto parsed = parseInt(value); if (!parsed) return false; node.layout.height = *parsed; }
            else if (key == "focus_order") { const auto parsed = parseInt(value); if (!parsed) return false; node.layout.focus_order = *parsed; }
            else if (key == "focus_next_id") node.focus_next_id = value;
            else return false;
            return document.updateNode(std::move(node));
        },
        [&document](std::string_view parent, std::string_view child) {
            const auto* source = document.findNode(child);
            if (source == nullptr || document.findNode(parent) == nullptr || parent == child) return false;
            auto node = *source;
            node.parent_id = parent;
            return document.updateNode(std::move(node));
        },
        [&document] {
            std::vector<SemanticEditorDiagnostic> result;
            for (const auto& item : ui::auditMenuAuthoringDocument(document, {}).issues)
                result.push_back({item.code, item.message, item.node_id, {}, item.blocking});
            return result;
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
            else if (key == "caption_localization_key")
                return graph.updateNodeMediaTrack(std::string(id), std::string(value), node->voice_takes,
                                                  node->caption_start_ms, node->caption_end_ms,
                                                  node->non_speech_cues);
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
        }, {},
        [&document] {
            std::vector<SemanticEditorDiagnostic> result;
            for (const auto& item : document.validateNavigation()) {
                std::string layer_id;
                for (const auto& layer : document.layers()) {
                    const auto tile = document.tileAt(layer.id, item.x, item.y);
                    if (tile.has_value() && *tile != 0 && (layer.collision || layer.navigation)) {
                        layer_id = layer.id;
                        break;
                    }
                }
                result.push_back({item.code, item.message, std::move(layer_id),
                                  "tile:" + std::to_string(item.x) + "," + std::to_string(item.y), true});
            }
            return result;
        });
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
            std::vector<SemanticEditorDiagnostic> result;
            for (const auto& item : graph.validate())
                result.push_back({item.code, item.message, item.map_id,
                                  item.route_id.empty() ? item.object_id : item.route_id, true});
            return result;
        });
    return surface;
}

} // namespace urpg::editor
