#pragma once

#include "editor/ui/menu_inspector_model.h"
#include "engine/core/ui/menu_scene_graph.h"
#include "engine/core/ui/menu_command_registry.h"
#include <string>
#include <optional>

namespace urpg::editor {

/**
 * @brief Editor panel for inspecting and diagnosing the active menu scene graph.
 * 
 * This panel follows the "Projection Model" pattern used across the URPG editor
 * (Message, Battle, Save). It binds to the runtime menu state and projects it 
 * into a flat, searchable list of command metadata and diagnostics.
 */
class MenuInspectorPanel {
public:
    MenuInspectorPanel() = default;

    /**
     * @brief Bind the panel to the active runtime menu components.
     * 
     * @param scene_graph The active menu scene graph to inspect.
     * @param registry The command registry used for route/state validation.
     * @param switches The current switch state for menu rule evaluation.
     * @param variables The current variable state for menu rule evaluation.
     */
    void bindRuntime(const urpg::ui::MenuSceneGraph& scene_graph,
                     const urpg::ui::MenuCommandRegistry& registry,
                     const urpg::ui::MenuCommandRegistry::SwitchState& switches,
                     const urpg::ui::MenuCommandRegistry::VariableState& variables);

    /**
     * @brief Clear existing runtime bindings and reset the model.
     */
    void clearRuntime();

    /**
     * @brief Access the underlying projection model.
     */
    MenuInspectorModel& getModel();
    const MenuInspectorModel& getModel() const;

    /**
     * @brief Control panel visibility.
     */
    void setVisible(bool visible);
    bool isVisible() const;

    /**
     * @brief Filter rows to show only those with diagnostic issues.
     */
    void setShowIssuesOnly(bool show_issues_only);

    /**
     * @brief Filter rows by a specific command ID.
     */
    void setCommandIdFilter(std::optional<std::string> command_id_filter);

    /**
     * @brief Render the panel (placeholder for ImGui logic).
     */
    void render();

    /**
     * @brief Force a re-projection of the runtime state into the model.
     */
    void refresh();

    /**
     * @brief Periodic update hook (usually calls refresh).
     */
    void update();

private:
    const urpg::ui::MenuSceneGraph* scene_graph_ = nullptr;
    const urpg::ui::MenuCommandRegistry* registry_ = nullptr;
    const urpg::ui::MenuCommandRegistry::SwitchState* switches_ = nullptr;
    const urpg::ui::MenuCommandRegistry::VariableState* variables_ = nullptr;

    MenuInspectorModel model_;
    bool visible_ = false;
    bool show_issues_only_ = false;
    std::optional<std::string> command_id_filter_;
};

} // namespace urpg::editor
