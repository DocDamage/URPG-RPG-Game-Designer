#pragma once

#include "presentation_runtime.h"
#include "presentation_schema.h"
#include <string>
#include <iostream>

namespace urpg::presentation {

/**
 * @brief State of current dialogue or message overlay.
 */
struct DialogueState {
    std::string text;
    bool isActive = false;
    bool requireHighContrast = true;
};

/**
 * @brief Refines environmental intent for dialogue high-readability (Section 10.4).
 */
class DialogueTranslator {
public:
    /**
     * @brief Injects readability overrides into the current intent.
     */
    void ApplyReadability(const DialogueState& state, 
                          const DialoguePresentationConfig& config, 
                          PresentationFrameIntent& intent) 
    {
        if (!state.isActive) return;

        // 1. Scene Desaturation (Visual Isolation)
        if (config.sceneSaturationMultiplier < 1.0f) {
            static PostFXProfile dialogueFX;
            dialogueFX.saturation = config.sceneSaturationMultiplier;
            dialogueFX.bloomIntensity = 0.5f; // Slight softening
            intent.AddPostFX(dialogueFX);
        }

        // 2. High Contrast Background Intent (Mocked as ShadowProxy infinity-plane)
        if (state.requireHighContrast && config.contrastBgAlpha > 0.0f) {
             // Position -1 behind the UI layer but in front of world
             intent.AddShadowProxy(0xDEF, {0,0,-1.0f}, {0,0,0});
        }

        std::cout << "[DIALOGUE] Readability applied. Saturation: " 
                  << config.sceneSaturationMultiplier << "\n";
    }
};

} // namespace urpg::presentation
