#pragma once

#include "presentation_runtime.h"
#include <string>
#include <vector>

namespace urpg::presentation {

/**
 * @brief Representation of a menu or UI layer state.
 */
struct MenuSceneState {
    std::string menuId;
    float zDepth = 1.0f; // Environmental background depth
    bool drawBackground = true;
    bool blurBackground = false;
};

/**
 * @brief Translator for menu visual intent.
 */
class MenuSceneTranslator {
public:
    void Translate(const MenuSceneState& state, PresentationFrameIntent& intent) {
        // 1. Emit background intent (if any)
        if (state.drawBackground) {
            // Background is always Tier 0 compatible but spatial-capable
            // Using ShadowProxy as a placeholder for a "Z-depth background plane"
            intent.AddShadowProxy(0xBB, {0.0f, 0.0f, state.zDepth}, {0.0f, 0.0f, 0.0f});
        }

        // 2. Emit UI Post-FX safe-zones (ADR-007 compliance)
        if (state.blurBackground) {
             static PostFXProfile blurProfile = { 1.0f, 0.05f, 5.0f, 1.0f }; 
             intent.AddPostFX(blurProfile);
        }
    }
};

} // namespace urpg::presentation
