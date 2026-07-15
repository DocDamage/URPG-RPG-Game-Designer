#include "editor/ui/editor_context_action.h"

#include "engine/core/editor/editor_panel_registry.h"

namespace urpg::editor {

EditorContextActionResult EditorContextActionStack::open(EditorContextAction action) {
    if (action.route.empty() || action.objectKind.empty() || action.objectId.empty() || action.projectRoot.empty()) {
        return {false, "context_action_missing_target",
                "A contextual editor requires a route, selected object, and open project.", std::move(action),
                "Select a map object in an open project before opening a contextual editor."};
    }
    const auto* entry = findEditorPanelRegistryEntry(action.route);
    if (entry == nullptr || !isRoutableEditorPanelExposure(entry->exposure)) {
        return {false, "context_action_route_unavailable",
                "The requested editor route is not available from the current creator workflow.", std::move(action),
                entry == nullptr ? "Choose a registered editor route."
                                 : entry->reason};
    }
    if (action.returnRoute.empty()) {
        action.returnRoute = "map";
    }
    stack_.push_back(action);
    return {true, "context_action_opened", "Opened the nested editor in the current Map context.", std::move(action), ""};
}

EditorContextActionResult EditorContextActionStack::returnToPrevious() {
    if (stack_.empty()) {
        return {false, "context_action_stack_empty", "There is no nested editor route to return from.", {}, "Open a contextual editor first."};
    }
    auto action = stack_.back();
    stack_.pop_back();
    return {true, "context_action_returned", "Returned to the preserved Map selection and viewport.", std::move(action), ""};
}

const EditorContextAction* EditorContextActionStack::active() const {
    return stack_.empty() ? nullptr : &stack_.back();
}

} // namespace urpg::editor
