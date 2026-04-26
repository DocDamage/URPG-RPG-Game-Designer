#pragma once

#include "battle_scene_state.h"
#include "presentation_schema.h"
#include "scene_adapters.h"
#include <cmath>

namespace urpg::presentation {

/**
 * @brief Concrete implementation of the BattleScene translator.
 * Section 10.2: BattleScene Integration
 */
class BattleSceneTranslatorImpl : public BattleSceneTranslator {
  public:
    static std::uint64_t ResolveParticipantCueId(const BattleParticipantState& participant) {
        if (participant.cueId != 0) {
            return participant.cueId;
        }

        constexpr std::uint64_t kEnemyCueDomainBit = 1ull << 63;
        const std::uint64_t parsedId = static_cast<std::uint64_t>(participant.actorId);
        if (parsedId == 0) {
            return 0;
        }
        return participant.isEnemy ? (parsedId | kEnemyCueDomainBit) : parsedId;
    }

    static const ActorPresentationProfile* FindActorProfile(const PresentationAuthoringData& data,
                                                            const BattleParticipantState& participant) {
        for (const auto& profile : data.actorProfiles) {
            if (profile.actorId == participant.classId) {
                return &profile;
            }
        }
        return nullptr;
    }

    static Vec3 ResolveParticipantPosition(const PresentationAuthoringData& data, const BattleSceneState& sceneState,
                                           const BattleParticipantState& participant) {
        const auto& formation = data.battleConfig.formation;
        const ActorPresentationProfile* profile = FindActorProfile(data, participant);

        float sideMultiplier = participant.isEnemy ? 1.0f : -1.0f;
        Vec3 pos = {0.0f, 0.0f, 0.0f};

        switch (formation.type) {
        case BattleFormation::LayoutType::Staged:
            pos.x = (5.0f + static_cast<float>(participant.formationIndex) * formation.spreadWidth) * sideMultiplier;
            pos.z = static_cast<float>(participant.formationIndex / 4) * formation.depthSpacing;
            break;

        case BattleFormation::LayoutType::Linear:
            pos.x = 6.0f * sideMultiplier;
            pos.z = static_cast<float>(participant.formationIndex) * formation.depthSpacing;
            break;

        case BattleFormation::LayoutType::Surround: {
            const auto participantCount = static_cast<float>(std::max<size_t>(sceneState.participants.size(), 1));
            const float angle = static_cast<float>(participant.formationIndex) * (3.14159f * 2.0f / participantCount);
            constexpr float kSurroundRadius = 8.0f;
            pos.x = std::cos(angle) * kSurroundRadius;
            pos.z = std::sin(angle) * kSurroundRadius;
            break;
        }
        }

        pos.y = profile != nullptr ? profile->anchorOffset.y : 0.0f;
        return pos;
    }

    static void ResolveParticipantAnchors(const PresentationAuthoringData& data, BattleSceneState& sceneState) {
        for (auto& participant : sceneState.participants) {
            participant.cueId = ResolveParticipantCueId(participant);
            participant.anchorPosition = ResolveParticipantPosition(data, sceneState, participant);
            participant.hasAnchorPosition = true;
        }
    }

    void Translate(const PresentationContext& context, const PresentationAuthoringData& data,
                   const BattleSceneState& sceneState, PresentationFrameIntent& outIntent) override {
        for (const auto& participant : sceneState.participants) {
            const ActorPresentationProfile* profile = FindActorProfile(data, participant);
            if (!profile)
                continue;

            const Vec3 pos = participant.hasAnchorPosition ? participant.anchorPosition
                                                           : ResolveParticipantPosition(data, sceneState, participant);

            if (context.activeMode == PresentationMode::Classic2D) {
                outIntent.AddActor(participant.actorId, Vec3{pos.x, pos.y, 0.0f}, *profile);
            } else {
                outIntent.AddActor(participant.actorId, pos, *profile);
            }
        }

        const auto& config = data.battleConfig;
        const auto& formation = config.formation;
        if (config.useDynamicCineCamera) {
            CameraProfile cineCam;
            cineCam.fov = 45.0f;
            cineCam.lookAtOffset = {0.0f, 1.5f, 0.0f};
            if (formation.type == BattleFormation::LayoutType::Surround) {
                cineCam.fov = 65.0f;
            }
            outIntent.SetCameraProfile(cineCam);
        }
    }

    // Unused base method
    void Translate(const PresentationContext&, const PresentationAuthoringData&, PresentationFrameIntent&) override {}
};

} // namespace urpg::presentation
