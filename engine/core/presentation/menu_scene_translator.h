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
 * @brief In-world HUD / status overlay state emitted over the game scene.
 * Covers HP bars, status icons, turn indicators, and mini-map overlays.
 * S23-T04: Status scene render-command coverage.
 */
struct StatusHUDState {
    bool visible = true;
    uint32_t activeActorId = 0;    // Actor whose turn indicator is shown (0 = none)
    bool showMinimap = false;
    float minimapOpacity = 1.0f;
    size_t activeStatusIconCount = 0; // Number of status-effect icons currently displayed
};

/**
 * @brief Translator for menu visual intent.
 */
class MenuSceneStateTranslator {
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

/**
 * @brief Translator that emits HUD / status render commands over the game scene.
 * S23-T04: Status / HUD scene render-command coverage.
 *
 * Each active UI element becomes an overlay effect command so the render
 * backend can composite it in the UI pass without touching game state.
 */
class StatusHUDTranslator {
public:
    /**
     * @brief Appends HUD commands to an existing intent.
     * Does not emit if the state is invisible.
     */
    void Translate(const StatusHUDState& state, PresentationFrameIntent& intent) {
        if (!state.visible) return;

        // Turn indicator: DrawOverlayEffect with a dedicated owner domain (0xHUD0000...)
        if (state.activeActorId != 0) {
            constexpr uint64_t kTurnIndicatorDomain = 0x4855'4400'0000'0000ULL; // "HUD\0"
            intent.AddOverlayEffect(
                /*effectId=*/state.activeActorId,
                /*pos=*/{0.0f, 0.0f, 0.0f},
                /*ownerId=*/kTurnIndicatorDomain | state.activeActorId,
                /*durationSeconds=*/0.0f,
                /*scale=*/1.0f,
                /*intensity=*/1.0f,
                /*color=*/{1.0f, 1.0f, 0.5f, 1.0f});
        }

        // Status icons: one overlay command per visible icon
        for (size_t i = 0; i < state.activeStatusIconCount; ++i) {
            constexpr uint64_t kStatusIconDomain = 0x5354'4154'0000'0000ULL; // "STAT"
            intent.AddOverlayEffect(
                /*effectId=*/static_cast<uint32_t>(i + 1),
                /*pos=*/{static_cast<float>(i) * 0.5f, 0.0f, 0.0f},
                /*ownerId=*/kStatusIconDomain | static_cast<uint64_t>(i),
                /*durationSeconds=*/0.0f,
                /*scale=*/1.0f,
                /*intensity=*/0.8f,
                /*color=*/{1.0f, 1.0f, 1.0f, 1.0f});
        }

        // Minimap: a single overlay effect in its own domain
        if (state.showMinimap) {
            constexpr uint64_t kMinimapDomain = 0x4D4E'4D41'0000'0001ULL; // "MNMA" + 1
            intent.AddOverlayEffect(
                /*effectId=*/0xFFFF,
                /*pos=*/{0.0f, 0.0f, 0.0f},
                /*ownerId=*/kMinimapDomain,
                /*durationSeconds=*/0.0f,
                /*scale=*/1.0f,
                /*intensity=*/state.minimapOpacity,
                /*color=*/{1.0f, 1.0f, 1.0f, state.minimapOpacity});
        }
    }
};

} // namespace urpg::presentation
