#pragma once

#include "engine/core/presentation/presentation_runtime.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/presentation/presentation_context.h"
#include "engine/core/presentation/scene_adapters.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/scene/scene_manager.h"
#include <charconv>
#include <memory>

namespace urpg::presentation {

/**
 * @brief Integration bridge between the high-level SceneManager and the Presentation Core.
 * This class coordinates the transformation of GameScene state into Render Intent.
 */
class PresentationBridge {
public:
    PresentationBridge(std::shared_ptr<PresentationRuntime> runtime, 
                       std::shared_ptr<PresentationAuthoringData> authoringData)
        : m_runtime(runtime), m_authoringData(authoringData) {}

    /**
     * @brief Generates the visual intent for the currently active scene.
     */
    PresentationFrameIntent BuildFrameForActiveScene(const scene::SceneManager& sceneManager, 
                                                     const PresentationContext& context) {
        auto activeScene = sceneManager.getActiveScene();
        if (!activeScene) {
            resetBattleCueCursor();
            return PresentationFrameIntent{PresentationMode::Classic2D, CapabilityTier::Tier0_Baseline};
        }

        PresentationContext effectiveContext = context;
        if (activeScene->getType() == scene::SceneType::BATTLE) {
            if (const auto* battleScene = dynamic_cast<const scene::BattleScene*>(activeScene.get())) {
                effectiveContext.battleState = BuildBattleSceneState(*battleScene);
            }
        } else {
            resetBattleCueCursor();
        }

        return m_runtime->BuildPresentationFrame(effectiveContext, *m_authoringData);
    }

    /**
     * @brief Access the underlying runtime.
     */
    std::shared_ptr<PresentationRuntime> GetRuntime() const { return m_runtime; }

private:
    struct BattleCueCursor {
        uint32_t nextSequenceIndex = 0;
        size_t observedCueCount = 0;
    };

    void resetBattleCueCursor() {
        m_activeBattleScene = nullptr;
        m_battleCueCursor = {};
    }

    static uint32_t parseParticipantId(const std::string& id, uint32_t fallback) {
        uint32_t parsed = fallback;
        const auto* begin = id.data();
        const auto* end = begin + id.size();
        const auto result = std::from_chars(begin, end, parsed);
        if (result.ec != std::errc{} || result.ptr != end) {
            return fallback;
        }
        return parsed;
    }

    static std::uint64_t parseParticipantCueId(const std::string& id, bool isEnemy) {
        std::uint64_t parsed = 0;
        const auto* begin = id.data();
        const auto* end = begin + id.size();
        const auto result = std::from_chars(begin, end, parsed);
        if (result.ec != std::errc{} || result.ptr != end || parsed == 0) {
            return 0;
        }

        constexpr std::uint64_t kEnemyCueDomainBit = 1ull << 63;
        return isEnemy ? (parsed | kEnemyCueDomainBit) : parsed;
    }

    BattleSceneState BuildBattleSceneState(const scene::BattleScene& battleScene) {
        BattleSceneState state;
        state.battleArenaId = battleScene.getName();

        if (m_activeBattleScene != &battleScene) {
            m_activeBattleScene = &battleScene;
            m_battleCueCursor = {};
        }

        const auto& sceneCues = battleScene.effectCues();
        if (!sceneCues.empty()) {
            if (sceneCues.size() < m_battleCueCursor.observedCueCount) {
                m_battleCueCursor.nextSequenceIndex = 0;
            }

            for (const auto& cue : sceneCues) {
                if (cue.sequenceIndex >= m_battleCueCursor.nextSequenceIndex) {
                    state.effectCues.push_back(cue);
                }
            }

            m_battleCueCursor.nextSequenceIndex = sceneCues.back().sequenceIndex + 1;
        }
        m_battleCueCursor.observedCueCount = sceneCues.size();

        uint32_t actorFormationIndex = 0;
        uint32_t enemyFormationIndex = 0;
        uint32_t fallbackId = 1;
        for (const auto& participant : battleScene.getParticipants()) {
            BattleParticipantState presentationParticipant;
            presentationParticipant.actorId = parseParticipantId(participant.id, fallbackId++);
            presentationParticipant.classId = participant.id;
            presentationParticipant.formationIndex =
                static_cast<int32_t>(participant.isEnemy ? enemyFormationIndex++ : actorFormationIndex++);
            presentationParticipant.isEnemy = participant.isEnemy;
            presentationParticipant.currentHPPulse =
                participant.maxHp > 0 ? static_cast<float>(participant.hp) / static_cast<float>(participant.maxHp) : 0.0f;
            presentationParticipant.cueId = parseParticipantCueId(participant.id, participant.isEnemy);
            state.participants.push_back(std::move(presentationParticipant));
        }

        return state;
    }

    std::shared_ptr<PresentationRuntime> m_runtime;
    std::shared_ptr<PresentationAuthoringData> m_authoringData;
    const scene::BattleScene* m_activeBattleScene = nullptr;
    BattleCueCursor m_battleCueCursor;
};

} // namespace urpg::presentation
