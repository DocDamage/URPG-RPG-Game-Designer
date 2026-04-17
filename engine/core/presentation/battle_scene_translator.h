#pragma once

#include "scene_adapters.h"
#include "battle_scene_state.h"
#include "presentation_schema.h"

namespace urpg::presentation {

/**
 * @brief Concrete implementation of the BattleScene translator.
 * Section 10.2: BattleScene Integration
 */
class BattleSceneTranslatorImpl : public BattleSceneTranslator {
public:
    void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        const BattleSceneState& sceneState,
        PresentationFrameIntent& outIntent) override {
        
        const auto& config = data.battleConfig;
        const auto& formation = config.formation;

        // 1. Process Participants and Staged Positioning
        for (const auto& participant : sceneState.participants) {
            // Find Actor Profile
            const ActorPresentationProfile* profile = nullptr;
            for (const auto& p : data.actorProfiles) {
                if (p.actorId == participant.classId) {
                    profile = &p;
                    break;
                }
            }

            if (!profile) continue;

            // ADR-005: Staged spatial composition
            // Side multiplier handles Heroes vs Enemies
            float sideMultiplier = participant.isEnemy ? 1.0f : -1.0f;
            
            // Calculate base position within formation based on LayoutType
            Vec3 pos = {0,0,0};
            
            switch (formation.type) {
                case BattleFormation::LayoutType::Staged:
                    // Classic side-by-side rows
                    pos.x = (5.0f + (float)participant.formationIndex * formation.spreadWidth) * sideMultiplier;
                    pos.z = (float)(participant.formationIndex / 4) * formation.depthSpacing;
                    break;
                    
                case BattleFormation::LayoutType::Linear:
                    // Single file line
                    pos.x = 6.0f * sideMultiplier;
                    pos.z = (float)participant.formationIndex * formation.depthSpacing;
                    break;
                    
                case BattleFormation::LayoutType::Surround:
                    // Circular arrangement around center
                    {
                        float angle = (float)participant.formationIndex * (3.14159f * 2.0f / (float)sceneState.participants.size());
                        float radius = 8.0f;
                        pos.x = std::cos(angle) * radius;
                        pos.z = std::sin(angle) * radius;
                    }
                    break;
            }

            pos.y = profile->anchorOffset.y; // Standard 2D billboard grounding

            // Tier-based Actor Emission
            if (context.activeMode == PresentationMode::Classic2D) {
                // Flatten to 2D staging area
                outIntent.AddActor(participant.actorId, Vec3{pos.x, pos.y, 0.0f}, *profile);
            } else {
                outIntent.AddActor(participant.actorId, pos, *profile);
            }
        }

        // 2. Battle Camera Intent (ADR-003: Cine-Cam)
        if (config.useDynamicCineCamera) {
            CameraProfile cineCam;
            cineCam.fov = 45.0f; // Tighter FOV for cinematic battles
            cineCam.lookAtOffset = {0.0f, 1.5f, 0.0f};
            
            // Dynamic framing based on formation
            if (formation.type == BattleFormation::LayoutType::Surround) {
                cineCam.fov = 65.0f; // Wider view for surrounded encounters
            }
            
            outIntent.SetCameraProfile(cineCam);
        }

        // 3. UI Safe Zone Enforcement (Section 10.2)
        // Ensure no world spatial elements bleed into HUD reserved regions
        // This is primarily handled via RenderPass priority and UI pass isolation
    }

    // Unused base method
    void Translate(const PresentationContext&, const PresentationAuthoringData&, PresentationFrameIntent&) override {}
};

} // namespace urpg::presentation
