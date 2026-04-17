#pragma once

#include "presentation_runtime.h"

namespace urpg::presentation {

/**
 * @brief Base class for converting specific scene state to presentation intent.
 * Section 9.1: PresentationSceneTranslator base contract
 */
class PresentationSceneTranslator {
public:
    virtual ~PresentationSceneTranslator() = default;

    /**
     * @brief Translate scene family state to visual intent.
     */
    virtual void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        PresentationFrameIntent& outIntent) = 0;
};

} // namespace urpg::presentation
