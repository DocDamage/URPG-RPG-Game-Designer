#pragma once

#include "presentation_types.h"
#include <string>
#include <vector>

namespace urpg::presentation {

/**
 * @brief Level of Detail (LOD) level for spatial items.
 */
enum class LODLevel {
    LOD0_High,   // Full detail, high-poly/high-res
    LOD1_Medium, // Simplified mesh/texture
    LOD2_Low,    // Proxy/Imposter
    LOD3_Culled  // Not rendered
};

/**
 * @brief Hint for the streaming system to manage asset lifetime.
 */
struct StreamingHint {
    std::string assetId;
    float priority = 1.0f;    // 0.0 to 1.0
    bool isEssential = false; // Cannot be evicted if true
};

/**
 * @brief Output container for streaming and LOD data generated during a frame.
 */
struct PresentationStreamingManifest {
    std::vector<StreamingHint> loadRequests;
    std::vector<std::string> evictionCandidates;
};

/**
 * @brief Policy for determining LOD based on distance and capability tier.
 */
struct LODPolicy {
    float lod1Distance = 20.0f;
    float lod2Distance = 50.0f;
    float cullDistance = 100.0f;
};

/**
 * @brief Utility to resolve streaming and LOD hints (Section 20).
 */
class PresentationStreamer {
  public:
    static LODLevel ResolveLOD(float distance, const LODPolicy& policy, CapabilityTier tier) {
        // Adjust distances based on tier (T0 is much more aggressive)
        float multiplier = (tier == CapabilityTier::Tier0_Baseline) ? 0.3f : 1.0f;

        if (distance > policy.cullDistance * multiplier)
            return LODLevel::LOD3_Culled;
        if (distance > policy.lod2Distance * multiplier)
            return LODLevel::LOD2_Low;
        if (distance > policy.lod1Distance * multiplier)
            return LODLevel::LOD1_Medium;

        return LODLevel::LOD0_High;
    }
};

} // namespace urpg::presentation
