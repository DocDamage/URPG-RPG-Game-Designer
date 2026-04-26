#pragma once

#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "presentation_types.h"
#include <string>

namespace urpg::presentation {

/**
 * @brief Represents detected hardware capabilities.
 */
struct HardwareProfile {
    std::string deviceName;
    size_t videoMemoryMB = 0;
    bool supportsComputeShaders = false;
    bool supportsRayTracing = false;
    uint32_t maxTextureSize = 2048;
    float shaderPerformanceScore = 0.0f; // 0.0 to 1.0
};

/**
 * @brief Multi-platform capability detection and auto-tiering logic.
 */
class TierDetector {
  public:
    /**
     * @brief Resolves the appropriate CapabilityTier based on a hardware profile.
     */
    static CapabilityTier AutoDetect(const HardwareProfile& profile) {
        urpg::diagnostics::RuntimeDiagnostics::info("presentation.tier_detector", "tier_detector.profile_analyzed",
                                                    "Analyzing: " + profile.deviceName);

        // Tier 3: High-end Desktop/Console
        if (profile.supportsRayTracing && profile.videoMemoryMB >= 8000 && profile.shaderPerformanceScore > 0.8f) {
            return CapabilityTier::Tier3_Full;
        }

        // Tier 2: Mid-range Desktop/Gaming Mobile
        if (profile.supportsComputeShaders && profile.videoMemoryMB >= 4000 && profile.shaderPerformanceScore > 0.5f) {
            return CapabilityTier::Tier2_Enhanced;
        }

        // Tier 1: Low-end PC/Standard Mobile
        if (profile.videoMemoryMB >= 2000 || profile.supportsComputeShaders) {
            return CapabilityTier::Tier1_Standard;
        }

        // Tier 0: Heritage/Legacy devices
        return CapabilityTier::Tier0_Baseline;
    }

    /**
     * @brief Dynamic Re-tiering logic based on runtime performance.
     * If the frame time is consistently too high, we may want to recommend a tier drop.
     */
    static CapabilityTier SuggestPerformanceAdjustment(CapabilityTier current, float avgFrameTimeMs) {
        if (avgFrameTimeMs > 33.3f && current != CapabilityTier::Tier0_Baseline) {
            // Under 30 FPS - suggest drop
            return static_cast<CapabilityTier>(static_cast<int>(current) - 1);
        }
        return current;
    }
};

} // namespace urpg::presentation
