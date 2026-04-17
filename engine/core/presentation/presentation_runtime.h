#pragma once

#include "presentation_types.h"
#include "presentation_context.h"
#include "presentation_schema.h"
#include "render_pass_manager.h"
#include "presentation_streaming.h"

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
        SetEnvironment
    } type;

    uint32_t id;
    Vec3 position;
    Vec3 rotation;
    const ActorPresentationProfile* actorProfile;
    const LightProfile* lightProfile = nullptr;
    const FogProfile* fogProfile = nullptr;
    const PostFXProfile* postFXProfile = nullptr;
    const CameraProfile* cameraProfile = nullptr;
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

    void AddActor(uint32_t id, Vec3 pos, const ActorPresentationProfile& profile, LODLevel lod = LODLevel::LOD0_High) {
        PresentationCommand cmd{PresentationCommand::Type::DrawActor, id, pos, {0,0,0}, &profile};
        cmd.lodLevel = lod;
        commands.push_back(cmd);
    }

    void AddProp(uint32_t id, Vec3 pos, Vec3 rot) {
        PresentationCommand cmd{PresentationCommand::Type::DrawProp, id, pos, rot, nullptr};
        commands.push_back(cmd);
    }

    void AddLight(const LightProfile& light) {
        PresentationCommand cmd{PresentationCommand::Type::SetLight, light.lightId, light.position, {0,0,0}, nullptr};
        cmd.lightProfile = &light;
        commands.push_back(cmd);
    }

    void AddFog(const FogProfile& fog) {
        PresentationCommand cmd{PresentationCommand::Type::SetFog, 0, {0,0,0}, {0,0,0}, nullptr};
        cmd.fogProfile = &fog;
        commands.push_back(cmd);
    }

    void AddPostFX(const PostFXProfile& fx) {
        PresentationCommand cmd{PresentationCommand::Type::SetPostFX, 0, {0,0,0}, {0,0,0}, nullptr};
        cmd.postFXProfile = &fx;
        commands.push_back(cmd);
    }

    void AddShadowProxy(uint32_t ownerId, Vec3 pos, Vec3 rot) {
        PresentationCommand cmd{PresentationCommand::Type::DrawShadowProxy, ownerId, pos, rot, nullptr};
        commands.push_back(cmd);
    }

    void SetCameraProfile(const CameraProfile& profile) {
        PresentationCommand cmd{PresentationCommand::Type::SetCamera, 0, profile.lookAtOffset, {profile.fov, 0.0f, 0.0f}, nullptr};
        cmd.cameraProfile = &profile;
        commands.push_back(cmd);
    }

    void AddPass(const RenderPass& pass) {
        activePasses.push_back(pass);
    }
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

    /**
     * @brief The main spine function (Section 9.1).
     * ADR-009: Executed on the Game Thread only.
     */
    PresentationFrameIntent BuildPresentationFrame(
        const PresentationContext& context,
        const PresentationAuthoringData& data) {
        
        PresentationFrameIntent intent;
        intent.activeMode = context.activeMode;
        intent.activeTier = context.activeTier;

        // 1. Resolve Global Environment (Fog, Post-FX)
        if (!data.mapOverlays.empty()) {
            const auto& overlay = data.mapOverlays[0]; 
            intent.AddFog(overlay.fog);
            intent.AddPostFX(overlay.postFX);
        }

        // 2. Resolve Camera Profile (Default to project-standard)
        CameraProfile defaultCam;
        defaultCam.fov = 60.0f;
        intent.SetCameraProfile(defaultCam);

        // 3. Dispatch to Scene Translators (Section 10)
        // Note: Real implementation would iterate over active scenes in context.
        // We ensure all spatial and UI intents are collected into a single frame buffer.
        
        for (const auto& overlay : data.mapOverlays) {
            // Placeholder: Translation of MapScene and BattleScene via their respective adapters
            // translator->Translate(context, data, sceneState, intent);
        }

        // Emit final sorted render passes based on intent commands
        intent.AddPass({"MainPass", RenderPassType::WorldSpatial, true, false, 0});
        intent.AddPass({"UIPass", RenderPassType::UserInterface, false, true, 1});

        return intent;
    }

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
