#pragma once

#include <string>
#include <vector>
#include "templates/arpg_core.h"
#include "templates/vn_core.h"
#include "templates/tactics_core.h"

// Note: This is an abstract abstraction of ImGui/UI integration
// To be implemented in the true Editor build.

namespace urpg::editor {

    /**
     * @brief Context-aware UI for the new 3.12-3.15 templates.
     */
    class TemplateInspector {
    public:
        /**
         * @brief Render the ARPG Combat Balance UI.
         */
        void drawARPGPanel(urpg::templates::ARPGCombatController& controller) {
            // Simplified logic: Representing ImGui::SliderFloat(...)
            float currentStamina = controller.getState() == urpg::templates::ARPGCombatController::CombatState::Idle ? 100.0f : 0.0f;
            // UI code here: SliderFloat("Stamina", &currentStamina, 0, 100);
        }

        /**
         * @brief Graph-based VN Editor visualization.
         */
        void drawVNNode(const urpg::templates::VNNode& node) {
            // UI code here: Text("Node: %s", node.id.c_str());
            // UI code here: InputText("Dialogue", &node.lineContent);
        }

        /**
         * @brief Tactics board editor (Cell placement / Stat modification).
         */
        void drawTacticsInspector(urpg::templates::UnitStats& stats) {
            // UI code here: InputInt("Move Range", &stats.moveRange);
            // UI code here: InputInt("Attack Range", &stats.attackRange);
        }
    };

} // namespace urpg::editor
