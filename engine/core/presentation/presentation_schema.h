#pragma once

#include "presentation_types.h"
#include "capability_matrix.h"
#include <cstdint>
#include <string>
#include <vector>

namespace urpg::presentation {

/**
 * @brief Project-level presentation settings schema.
 */
struct ProjectPresentationSettings {
    uint32_t schemaVersion = 1;
    PresentationMode defaultMode = PresentationMode::Classic2D;
    CapabilityTier targetTier = CapabilityTier::Tier1_Standard;
    uint64_t frameArenaBudget = 2 * 1024 * 1024; 
};

/**
 * @brief Grid-based elevation data (ADR-003).
 */
struct ElevationGrid {
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<int8_t> levels;
    float stepHeight = 0.5f;
    
    float GetWorldHeight(uint32_t x, uint32_t y) const {
        if (x < width && y < height && !levels.empty()) {
            return static_cast<float>(levels[y * width + x]) * stepHeight;
        }
        return 0.0f;
    }
};

/**
 * @brief Prop instance placement (Section 12.1).
 */
struct PropInstance {
    std::string assetId;
    float posX, posY, posZ;
    float rotY;
    float scale = 1.0f;
};

/**
 * @brief Light profile schema (Section 12.1).
 */
struct LightProfile {
    enum class LightType { Directional, Point, Spot };
    uint32_t lightId = 0;
    LightType type = LightType::Point;
    Vec3 position = {0,0,0};
    float color[3] = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 10.0f;
};

/**
 * @brief Fog profile schema (Section 12.1).
 */
struct FogProfile {
    float density = 0.01f;
    float color[3] = {0.5f, 0.5f, 0.5f};
    float startDist = 0.0f;
    float endDist = 100.0f;
};

/**
 * @brief Post-FX profile schema (Section 12.1).
 */
struct PostFXProfile {
    float exposure = 1.0f;
    float bloomThreshold = 1.0f;
    float bloomIntensity = 0.0f;
    float saturation = 1.0f;
};

/**
 * @brief Spatial map overlay schema (Section 12.1).
 */
struct SpatialMapOverlay {
    uint32_t schemaVersion = 1;
    std::string mapId;
    ElevationGrid elevation;
    std::vector<PropInstance> props;
    std::vector<LightProfile> lights;
    FogProfile fog;
    PostFXProfile postFX;
};

/**
 * @brief Actor presentation profile (Section 12.1).
 */
struct ActorPresentationProfile {
    std::string actorId;
    float billboardPivot[2] = {0.5f, 0.0f};
    Vec3 anchorOffset = {0,0,0};
    bool useDepthSorting = true;
    float heightOffset = 0.0f;
};

/**
 * @brief Battle formation layout schema (Section 12.1).
 */
struct BattleFormation {
    enum class LayoutType { Staged, Surround, Linear };
    LayoutType type = LayoutType::Staged;
    float depthSpacing = 2.0f;
    float spreadWidth = 1.0f;
};

/**
 * @brief Battle presentation configuration (Section 12.1).
 */
struct BattlePresentationConfig {
    uint32_t schemaVersion = 1;
    BattleFormation formation;
    bool useDynamicCineCamera = true;
    float uiSafeZoneHeight = 0.2f; // Top/Bottom HUD reserved space
    float spreadWidth = 2.0f;      // Horizontal spacing between participants
    float depthSpacing = 1.5f;     // Side-view row spacing
};

/**
 * @brief Camera constraint and framing profile (Section 14.1).
 */
struct CameraProfile {
    std::string profileId;
    float fov = 60.0f;
    float nearClip = 0.1f;
    float farClip = 1000.0f;
    
    // Constraints
    float minPitch = -85.0f;
    float maxPitch = 85.0f;
    float minDistance = 1.0f;
    float maxDistance = 50.0f;
    
    // Framing
    Vec3 lookAtOffset = {0.0f, 1.0f, 0.0f}; // Target the head/chest
    float orthoSize = 5.0f; // For Classic2D fallback
};

/**
 * @brief UI/Dialogue readability configuration (Section 10.4).
 */
struct DialoguePresentationConfig {
    float contrastBgAlpha = 0.5f;   // Darken background for text
    bool pauseParticleSystems = true;
    bool dampenWorldAudio = true;
    bool enableBackgroundBlur = true;
    float sceneSaturationMultiplier = 0.5f; // Desaturate while talking
};

/**
 * @brief Runtime-ready form of all authored presentation schema.
 */
struct PresentationAuthoringData {
    ProjectPresentationSettings projectSettings;
    std::vector<SpatialMapOverlay> mapOverlays;
    std::vector<ActorPresentationProfile> actorProfiles;
    std::vector<CameraProfile> cameraProfiles;
    DialoguePresentationConfig dialogueDefaults;
    BattlePresentationConfig battleConfig;
    
    struct ValidationReport {
        bool isValid = true;
        std::vector<std::string> messages;
    } loadValidation;
};

/**
 * @brief Material response profile schema (Section 12.1).
 */
struct MaterialResponseProfile {
    std::string materialTag;
    float baseReflectivity = 0.04f;
    float metalness = 0.0f;
    float roughness = 0.5f;
};

/**
 * @brief Tier fallback declaration schema (Section 12.1).
 */
struct TierFallbackDeclaration {
    CapabilityTier targetTier;
    std::string featureId;
    FeatureStatus fallbackStatus;
};

} // namespace urpg::presentation
