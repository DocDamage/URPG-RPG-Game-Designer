#pragma once

#include <string>
#include <vector>
#include <memory>
#include "gameplay/ability_system.h"

namespace urpg::templates {

    /**
     * @brief Core definition for an Action RPG combat participant.
     * Part of Wave 3.12 Template Expansion.
     */
    struct ARPGCombatStats {
        float moveSpeed = 4.0f;
        float dodgeCooldown = 1.5f;
        float stamina = 100.0f;
        float staminaRecoveryRate = 10.0f;
        bool isInvulnerable = false;
    };

    /**
     * @brief Controller for Real-time Combat in ARPG template.
     * Manages combo-chaining and frame-perfect dodging/parrying.
     */
    class ARPGCombatController {
    public:
        enum class CombatState { Idle, Attacking, Dodging, Recoiling };

        void update(float deltaTime) {
            if (m_stats.stamina < 100.0f) {
                m_stats.stamina += m_stats.staminaRecoveryRate * deltaTime;
            }

            if (m_comboTimeout > 0.0f) {
                m_comboTimeout -= deltaTime;
                if (m_comboTimeout <= 0.0f) m_currentComboStep = 0;
            }
        }

        /**
         * @brief Execute a dodge. Uses stamina and grants i-frames.
         */
        bool tryDodge() {
            if (m_stats.stamina >= 20.0f && m_state != CombatState::Dodging) {
                m_stats.stamina -= 20.0f;
                m_state = CombatState::Dodging;
                m_stats.isInvulnerable = true;
                return true;
            }
            return false;
        }

        /**
         * @brief Advance weapon combo.
         */
        void executeAttack() {
            m_state = CombatState::Attacking;
            m_currentComboStep++;
            m_comboTimeout = 1.2f; // Window to chain next hit
        }

        CombatState getState() const { return m_state; }
        int getComboStep() const { return m_currentComboStep; }

    private:
        ARPGCombatStats m_stats;
        CombatState m_state = CombatState::Idle;
        int m_currentComboStep = 0;
        float m_comboTimeout = 0.0f;
    };

} // namespace urpg::templates
