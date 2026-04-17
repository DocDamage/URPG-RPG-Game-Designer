#pragma once

#include "scene_translator.h"

namespace urpg::presentation {

/**
 * @brief Adapter for MapScene presentation.
 * Section 10.1: MapScene contract
 */
class MapSceneTranslator : public PresentationSceneTranslator {
public:
    virtual void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        const class MapSceneState& sceneState, // Hypothetical scene state class
        PresentationFrameIntent& outIntent) = 0;
};

/**
 * @brief Adapter for BattleScene presentation.
 * Section 10.2: BattleScene contract
 */
class BattleSceneTranslator : public PresentationSceneTranslator {
public:
    virtual void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        const class BattleSceneState& sceneState,
        PresentationFrameIntent& outIntent) = 0;
};

/**
 * @brief Adapter for MenuScene presentation.
 * Section 10.4: MenuScene contract
 */
class MenuSceneTranslator : public PresentationSceneTranslator {
public:
    virtual void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        const class MenuSceneState& sceneState,
        PresentationFrameIntent& outIntent) = 0;
};

/**
 * @brief Adapter for Overlay/UI presentation.
 * Section 10.4: Message/UI overlays contract
 */
class OverlaySceneTranslator : public PresentationSceneTranslator {
public:
    virtual void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        const class OverlaySceneState& sceneState,
        PresentationFrameIntent& outIntent) = 0;
};

} // namespace urpg::presentation
