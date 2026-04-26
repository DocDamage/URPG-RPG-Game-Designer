#pragma once

#include "presentation_context.h"
#include "presentation_schema.h"
#include "presentation_streaming.h"
#include "presentation_types.h"
#include "render_pass_manager.h"
#include <algorithm>
#include <array>

namespace urpg::presentation {

/**
 * @brief Represents a single command in the presentation intent.
 */
struct PresentationCommand {
    enum class Type {
        DrawActor,
        DrawProp,
        SetCamera,
        SetLight,
        SetFog,
        SetPostFX,
        DrawShadowProxy,
        SetEnvironment,
        DrawWorldEffect,
        DrawOverlayEffect
    } type;

    uint32_t id;
    Vec3 position;
    Vec3 rotation;
    const ActorPresentationProfile* actorProfile;
    const LightProfile* lightProfile = nullptr;
    const FogProfile* fogProfile = nullptr;
    const PostFXProfile* postFXProfile = nullptr;
    const CameraProfile* cameraProfile = nullptr;
    float blendWeight = 1.0f;
    uint64_t effectOwnerId = 0;
    float effectDurationSeconds = 0.0f;
    float effectScale = 1.0f;
    float effectIntensity = 1.0f;
    std::array<float, 4> effectColor = {1.0f, 1.0f, 1.0f, 1.0f};
    float effectOverlayEmphasis = 0.0f;
    LODLevel lodLevel = LODLevel::LOD0_High; // Section 20: LOD integration
};

/**
 * @brief Placeholder for frame-by-frame intent emission.
 * Section 9.2: PresentationFrameIntent
 */
struct PresentationFrameIntent {
    PresentationMode activeMode;
    CapabilityTier activeTier;

    std::vector<PresentationCommand> commands;
    std::vector<RenderPass> activePasses;
    PresentationStreamingManifest streamingManifest; // Added Section 20: Streaming Hints
    std::vector<FogProfile> resolvedFogProfiles;
    std::vector<PostFXProfile> resolvedPostFXProfiles;

    void AddActor(uint32_t id, Vec3 pos, const ActorPresentationProfile& profile, LODLevel lod = LODLevel::LOD0_High) {
        PresentationCommand cmd{PresentationCommand::Type::DrawActor, id, pos, {0, 0, 0}, &profile};
        cmd.lodLevel = lod;
        commands.push_back(cmd);
    }

    void AddProp(uint32_t id, Vec3 pos, Vec3 rot) {
        PresentationCommand cmd{PresentationCommand::Type::DrawProp, id, pos, rot, nullptr};
        commands.push_back(cmd);
    }

    void AddLight(const LightProfile& light) {
        PresentationCommand cmd{PresentationCommand::Type::SetLight, light.lightId, light.position, {0, 0, 0}, nullptr};
        cmd.lightProfile = &light;
        commands.push_back(cmd);
    }

    void AddFog(const FogProfile& fog, float blendWeight = 1.0f) {
        PresentationCommand cmd{PresentationCommand::Type::SetFog, 0, {0, 0, 0}, {0, 0, 0}, nullptr};
        cmd.fogProfile = &fog;
        cmd.blendWeight = blendWeight;
        commands.push_back(cmd);
    }

    void AddPostFX(const PostFXProfile& fx, float blendWeight = 1.0f) {
        PresentationCommand cmd{PresentationCommand::Type::SetPostFX, 0, {0, 0, 0}, {0, 0, 0}, nullptr};
        cmd.postFXProfile = &fx;
        cmd.blendWeight = blendWeight;
        commands.push_back(cmd);
    }

    void AddShadowProxy(uint32_t ownerId, Vec3 pos, Vec3 rot) {
        PresentationCommand cmd{PresentationCommand::Type::DrawShadowProxy, ownerId, pos, rot, nullptr};
        commands.push_back(cmd);
    }

    void AddWorldEffect(uint32_t effectId, Vec3 pos, uint64_t ownerId, float durationSeconds, float scale,
                        float intensity, const std::array<float, 4>& color, float overlayEmphasis = 0.0f) {
        PresentationCommand cmd{PresentationCommand::Type::DrawWorldEffect, effectId, pos, {0, 0, 0}, nullptr};
        cmd.effectOwnerId = ownerId;
        cmd.effectDurationSeconds = durationSeconds;
        cmd.effectScale = scale;
        cmd.effectIntensity = intensity;
        cmd.effectColor = color;
        cmd.effectOverlayEmphasis = overlayEmphasis;
        commands.push_back(cmd);
    }

    void AddOverlayEffect(uint32_t effectId, Vec3 pos, uint64_t ownerId, float durationSeconds, float scale,
                          float intensity, const std::array<float, 4>& color, float blendWeight = 1.0f,
                          float overlayEmphasis = 0.0f) {
        PresentationCommand cmd{PresentationCommand::Type::DrawOverlayEffect, effectId, pos, {0, 0, 0}, nullptr};
        cmd.effectOwnerId = ownerId;
        cmd.effectDurationSeconds = durationSeconds;
        cmd.effectScale = scale;
        cmd.effectIntensity = intensity;
        cmd.effectColor = color;
        cmd.effectOverlayEmphasis = overlayEmphasis;
        cmd.blendWeight = blendWeight;
        commands.push_back(cmd);
    }

    void SetCameraProfile(const CameraProfile& profile) {
        PresentationCommand cmd{
            PresentationCommand::Type::SetCamera, 0, profile.lookAtOffset, {profile.fov, 0.0f, 0.0f}, nullptr};
        cmd.cameraProfile = &profile;
        commands.push_back(cmd);
    }

    void AddPass(const RenderPass& pass) { activePasses.push_back(pass); }
};

/**
 * @brief Core entry point for the Presentation Core subsystem.
 *
 * This class owns the visual interpretation logic of the engine.
 * It is responsible for bridging Game Scene State to Render Intent.
 */
class PresentationRuntime {
  public:
    PresentationRuntime() = default;
    ~PresentationRuntime() = default;

    static FogProfile BlendFogProfiles(const std::vector<PresentationCommand>& commands) {
        FogProfile blended{};
        bool initialized = false;
        float totalWeight = 0.0f;

        for (const auto& cmd : commands) {
            if (cmd.type != PresentationCommand::Type::SetFog || !cmd.fogProfile || cmd.blendWeight <= 0.0f) {
                continue;
            }

            const float weight = cmd.blendWeight;
            if (!initialized) {
                blended = *cmd.fogProfile;
                totalWeight = weight;
                initialized = true;
                continue;
            }

            const float combinedWeight = totalWeight + weight;
            const float currentFactor = totalWeight / combinedWeight;
            const float newFactor = weight / combinedWeight;
            blended.density = blended.density * currentFactor + cmd.fogProfile->density * newFactor;
            blended.startDist = blended.startDist * currentFactor + cmd.fogProfile->startDist * newFactor;
            blended.endDist = blended.endDist * currentFactor + cmd.fogProfile->endDist * newFactor;
            for (size_t i = 0; i < 3; ++i) {
                blended.color[i] = blended.color[i] * currentFactor + cmd.fogProfile->color[i] * newFactor;
            }
            totalWeight = combinedWeight;
        }

        return blended;
    }

    static PostFXProfile BlendPostFXProfiles(const std::vector<PresentationCommand>& commands) {
        PostFXProfile blended{};
        bool initialized = false;
        float totalWeight = 0.0f;

        for (const auto& cmd : commands) {
            if (cmd.type != PresentationCommand::Type::SetPostFX || !cmd.postFXProfile || cmd.blendWeight <= 0.0f) {
                continue;
            }

            const float weight = cmd.blendWeight;
            if (!initialized) {
                blended = *cmd.postFXProfile;
                totalWeight = weight;
                initialized = true;
                continue;
            }

            const float combinedWeight = totalWeight + weight;
            const float currentFactor = totalWeight / combinedWeight;
            const float newFactor = weight / combinedWeight;
            blended.exposure = blended.exposure * currentFactor + cmd.postFXProfile->exposure * newFactor;
            blended.bloomThreshold =
                blended.bloomThreshold * currentFactor + cmd.postFXProfile->bloomThreshold * newFactor;
            blended.bloomIntensity =
                blended.bloomIntensity * currentFactor + cmd.postFXProfile->bloomIntensity * newFactor;
            blended.saturation = blended.saturation * currentFactor + cmd.postFXProfile->saturation * newFactor;
            totalWeight = combinedWeight;
        }

        return blended;
    }

    static void ResolveEnvironmentCommands(PresentationFrameIntent& intent) {
        bool hasFog = false;
        bool hasPostFX = false;
        std::vector<PresentationCommand> retained;
        retained.reserve(intent.commands.size());

        for (const auto& cmd : intent.commands) {
            if (cmd.type == PresentationCommand::Type::SetFog && cmd.fogProfile && cmd.blendWeight > 0.0f) {
                hasFog = true;
                continue;
            }
            if (cmd.type == PresentationCommand::Type::SetPostFX && cmd.postFXProfile && cmd.blendWeight > 0.0f) {
                hasPostFX = true;
                continue;
            }
            retained.push_back(cmd);
        }

        std::vector<PresentationCommand> resolved;
        resolved.reserve(retained.size() + 2);
        if (hasFog) {
            intent.resolvedFogProfiles.clear();
            intent.resolvedFogProfiles.push_back(BlendFogProfiles(intent.commands));
            PresentationCommand fogCmd{PresentationCommand::Type::SetFog, 0, {0, 0, 0}, {0, 0, 0}, nullptr};
            fogCmd.fogProfile = &intent.resolvedFogProfiles.back();
            resolved.push_back(fogCmd);
        }
        if (hasPostFX) {
            intent.resolvedPostFXProfiles.clear();
            intent.resolvedPostFXProfiles.push_back(BlendPostFXProfiles(intent.commands));
            PresentationCommand postFxCmd{PresentationCommand::Type::SetPostFX, 0, {0, 0, 0}, {0, 0, 0}, nullptr};
            postFxCmd.postFXProfile = &intent.resolvedPostFXProfiles.back();
            resolved.push_back(postFxCmd);
        }

        resolved.insert(resolved.end(), retained.begin(), retained.end());
        intent.commands = std::move(resolved);
    }

    /**
     * @brief The main spine function (Section 9.1).
     * ADR-009: Executed on the Game Thread only.
     */
    PresentationFrameIntent BuildPresentationFrame(const PresentationContext& context,
                                                   const PresentationAuthoringData& data);

    /**
     * @brief Access diagnostics collected during the last frame or load.
     */
    const std::vector<DiagnosticEntry>& GetDiagnostics() const { return m_diagnostics; }

    /**
     * @brief Clear diagnostics (usually at start of frame).
     */
    void ClearDiagnostics() { m_diagnostics.clear(); }

    /**
     * @brief Record a new diagnostic entry.
     */
    void EmitDiagnostic(DiagnosticSeverity severity, const std::string& message, const std::string& context = "") {
        m_diagnostics.push_back({severity, message, context});
    }

  private:
    std::vector<DiagnosticEntry> m_diagnostics;

    // Section 11: Capability and Fallback
    // Resolved internally during frame building
};

} // namespace urpg::presentation
