#pragma once

#include "presentation_types.h"
#include "capability_matrix.h"
#include <functional>
#include <string>

namespace urpg::presentation {

/**
 * @brief Handles fallback decisions when features are unsupported or degraded.
 * Section 11: Fallback Policy
 */
class PresentationFallbackRouter {
public:
    using FallbackAction = std::function<void(const std::string& featureId, FeatureStatus status)>;

    PresentationFallbackRouter(const PresentationCapabilityMatrix& matrix)
        : m_matrix(matrix) {}

    /**
     * @brief Check if a feature is supported and execute fallback if needed.
     */
    void Route(CapabilityTier activeTier, const std::string& featureId, FallbackAction onFallback) {
        FeatureStatus status = m_matrix.GetFeatureStatus(activeTier, featureId);
        if (status != FeatureStatus::Supported) {
            onFallback(featureId, status);
        }
    }

private:
    const PresentationCapabilityMatrix& m_matrix;
};

} // namespace urpg::presentation
