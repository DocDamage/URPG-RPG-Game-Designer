#pragma once

#include "presentation_types.h"
#include <map>
#include <string>

namespace urpg::presentation {

/**
 * @brief Represents a single feature's availability across tiers.
 * ADR-004: Capabilities and Fallback
 */
enum class FeatureStatus {
    Unsupported,
    Degraded,   // Supported but with reduced quality
    Supported
};

/**
 * @brief Matrix tracking which features are available at which levels.
 */
class PresentationCapabilityMatrix {
public:
    void SetFeatureStatus(CapabilityTier tier, const std::string& featureId, FeatureStatus status) {
        m_matrix[tier][featureId] = status;
    }

    FeatureStatus GetFeatureStatus(CapabilityTier tier, const std::string& featureId) const {
        auto itTier = m_matrix.find(tier);
        if (itTier != m_matrix.end()) {
            auto itFeature = itTier->second.find(featureId);
            if (itFeature != itTier->second.end()) {
                return itFeature->second;
            }
        }
        return FeatureStatus::Unsupported;
    }

private:
    std::map<CapabilityTier, std::map<std::string, FeatureStatus>> m_matrix;
};

} // namespace urpg::presentation
