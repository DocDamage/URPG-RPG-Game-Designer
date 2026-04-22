#include "engine/core/presentation/presentation_bridge.h"

namespace urpg::presentation {

PresentationFrameIntent PresentationBridge::BuildFrameForActiveScene(
    const scene::SceneManager& sceneManager,
    const PresentationContext& context) {
    auto activeScene = sceneManager.getActiveScene();
    if (!activeScene) {
        resetBattleCueCursor();
        return PresentationFrameIntent{
            PresentationMode::Classic2D,
            CapabilityTier::Tier0_Baseline,
            {},
            {},
            {},
            {},
            {}
        };
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

void PresentationBridge::resetBattleCueCursor() {
    m_activeBattleScene = nullptr;
    m_battleCueCursor = {};
}

BattleSceneState PresentationBridge::BuildBattleSceneState(const scene::BattleScene& battleScene) {
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

} // namespace urpg::presentation
