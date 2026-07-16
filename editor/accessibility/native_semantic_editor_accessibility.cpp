#include "editor/accessibility/native_semantic_editor_accessibility.h"

#include <charconv>
#include <iterator>
#include <optional>

namespace urpg::editor {
namespace {

struct ParsedIndex {
    std::size_t value = 0;
    std::string_view remainder;
};

std::optional<ParsedIndex> parseIndex(std::string_view value) {
    const auto separator = value.find('.');
    const auto token = value.substr(0, separator);
    std::size_t result = 0;
    const auto parsed = std::from_chars(token.data(), token.data() + token.size(), result);
    if (token.empty() || parsed.ec != std::errc{} || parsed.ptr != token.data() + token.size()) return std::nullopt;
    return ParsedIndex{result, separator == std::string_view::npos ? std::string_view{} : value.substr(separator + 1)};
}

std::string prefixFor(const std::string_view domain) {
    return "semantic." + std::string(domain) + ".";
}

bool synchronizeSelection(SemanticEditorCommandSurface& surface, const std::string_view selected_id) {
    if (!selected_id.empty() && surface.select(selected_id).applied) return true;
    return !surface.selectedId().empty();
}

NativeAccessibilityNode node(std::string id, std::string name, std::string description,
                             std::string value, std::string default_action,
                             const NativeAccessibilityRole role, const bool enabled,
                             const bool focusable, const bool selected = false,
                             const bool editable = false) {
    return {std::move(id), std::move(name), std::move(description), std::move(value),
            std::move(default_action), role, enabled, focusable, selected, editable};
}

} // namespace

void appendNativeSemanticEditorAccessibility(NativeAccessibilitySnapshot& tree, const std::string_view domain,
                                             const std::string_view label, SemanticEditorCommandSurface& surface,
                                             const std::string_view selected_id) {
    if (!surface.isBound()) return;
    (void)synchronizeSelection(surface, selected_id);
    const auto rows = surface.orderedNodes();
    const auto prefix = prefixFor(domain);
    const auto active_id = surface.selectedId();
    tree.nodes.push_back(node(prefix + "summary", std::string(label) + " semantic editor",
                              "Ordered non-canvas route with property, connection, and diagnostic actions.",
                              std::to_string(rows.size()) + " rows", {}, NativeAccessibilityRole::Group,
                              true, false));
    tree.nodes.push_back(node(prefix + "navigate.first", "First " + std::string(label) + " row",
                              "Select the first semantic row.", {}, "Select", NativeAccessibilityRole::Button,
                              !rows.empty(), !rows.empty()));
    tree.nodes.push_back(node(prefix + "navigate.previous", "Previous " + std::string(label) + " row",
                              "Select the previous semantic row with wrapping.", {}, "Select",
                              NativeAccessibilityRole::Button, !rows.empty(), !rows.empty()));
    tree.nodes.push_back(node(prefix + "navigate.next", "Next " + std::string(label) + " row",
                              "Select the next semantic row with wrapping.", {}, "Select",
                              NativeAccessibilityRole::Button, !rows.empty(), !rows.empty()));
    tree.nodes.push_back(node(prefix + "navigate.last", "Last " + std::string(label) + " row",
                              "Select the last semantic row.", {}, "Select", NativeAccessibilityRole::Button,
                              !rows.empty(), !rows.empty()));

    for (std::size_t row_index = 0; row_index < rows.size(); ++row_index) {
        const auto& row = rows[row_index];
        const bool selected = row.id == active_id;
        tree.nodes.push_back(node(prefix + "node." + std::to_string(row_index), row.label,
                                  "Semantic " + row.kind + " row " + row.id + ".",
                                  "Order " + std::to_string(row.order) + "; " +
                                      std::to_string(row.connections.size()) + " connections",
                                  "Select", NativeAccessibilityRole::ListItem, true, true, selected));
        if (!selected) continue;
        std::size_t property_index = 0;
        for (const auto& [key, value] : row.properties) {
            tree.nodes.push_back(node(prefix + "property." + std::to_string(row_index) + "." +
                                          std::to_string(property_index++),
                                      key, "Property for selected " + std::string(label) + " row " + row.id + ".",
                                      value, surface.canEditProperties() ? "Edit" : std::string{},
                                      NativeAccessibilityRole::TextField, true, surface.canEditProperties(),
                                      false, surface.canEditProperties()));
        }
        if (surface.canCreateConnections()) {
            for (std::size_t target_index = 0; target_index < rows.size(); ++target_index) {
                if (target_index == row_index) continue;
                tree.nodes.push_back(node(prefix + "connect." + std::to_string(row_index) + "." +
                                              std::to_string(target_index),
                                          "Connect " + row.label + " to " + rows[target_index].label,
                                          "Create an owner-backed semantic connection.", {}, "Connect",
                                          NativeAccessibilityRole::Button, true, true));
            }
        }
    }

    const auto diagnostics = surface.linkedDiagnostics();
    for (std::size_t index = 0; index < diagnostics.size(); ++index) {
        const auto& diagnostic = diagnostics[index];
        const bool linked = !diagnostic.object_id.empty();
        tree.nodes.push_back(node(prefix + "diagnostic." + std::to_string(index),
                                  diagnostic.code + ": " + diagnostic.message,
                                  "Object " + diagnostic.object_id +
                                      (diagnostic.related_object_id.empty() ? std::string{} :
                                       "; related " + diagnostic.related_object_id),
                                  diagnostic.blocking ? "Blocking" : "Advisory",
                                  linked ? "Focus" : std::string{},
                                  NativeAccessibilityRole::Diagnostic, true, linked));
    }
}

NativeSemanticEditorCommandResult activateNativeSemanticEditorAccessibilityNode(
    const std::string_view domain, const std::string_view node_id, SemanticEditorCommandSurface& surface,
    std::string* selected_id) {
    if (selected_id == nullptr || !surface.isBound()) return {};
    (void)synchronizeSelection(surface, *selected_id);
    const auto prefix = prefixFor(domain);
    if (!node_id.starts_with(prefix)) return {};
    const auto command = node_id.substr(prefix.size());
    SemanticEditorCommandResult result;
    if (command == "navigate.first") result = surface.navigate(SemanticEditorNavigation::First);
    else if (command == "navigate.previous") result = surface.navigate(SemanticEditorNavigation::Previous);
    else if (command == "navigate.next") result = surface.navigate(SemanticEditorNavigation::Next);
    else if (command == "navigate.last") result = surface.navigate(SemanticEditorNavigation::Last);
    else if (command.starts_with("node.")) {
        const auto index = parseIndex(command.substr(5));
        const auto rows = surface.orderedNodes();
        if (!index || index->value >= rows.size()) return {};
        result = surface.select(rows[index->value].id);
    } else if (command.starts_with("connect.")) {
        const auto source = parseIndex(command.substr(8));
        if (!source) return {};
        const auto target = parseIndex(source->remainder);
        const auto rows = surface.orderedNodes();
        if (!target || source->value >= rows.size() || target->value >= rows.size()) return {};
        result = surface.select(rows[source->value].id);
        if (result.applied) result = surface.connectSelectedTo(rows[target->value].id);
        if (result.applied) *selected_id = surface.selectedId();
        return {result.applied, result.applied};
    } else if (command.starts_with("diagnostic.")) {
        const auto index = parseIndex(command.substr(11));
        if (!index) return {};
        result = surface.focusDiagnostic(index->value);
    } else {
        return {};
    }
    if (result.applied) *selected_id = surface.selectedId();
    return {result.applied, false};
}

NativeSemanticEditorCommandResult setNativeSemanticEditorAccessibilityValue(
    const std::string_view domain, const std::string_view node_id, const std::string_view value,
    SemanticEditorCommandSurface& surface, std::string* selected_id) {
    if (selected_id == nullptr || !surface.isBound()) return {};
    (void)synchronizeSelection(surface, *selected_id);
    const auto prefix = prefixFor(domain) + "property.";
    if (!node_id.starts_with(prefix)) return {};
    const auto row_index = parseIndex(node_id.substr(prefix.size()));
    if (!row_index) return {};
    const auto property_index = parseIndex(row_index->remainder);
    const auto rows = surface.orderedNodes();
    if (!property_index || row_index->value >= rows.size() ||
        property_index->value >= rows[row_index->value].properties.size()) return {};
    auto property = rows[row_index->value].properties.begin();
    std::advance(property, static_cast<std::ptrdiff_t>(property_index->value));
    if (!surface.select(rows[row_index->value].id).applied) return {};
    const auto result = surface.setSelectedProperty(property->first, value);
    if (result.applied) *selected_id = surface.selectedId();
    return {result.applied, result.applied};
}

} // namespace urpg::editor
