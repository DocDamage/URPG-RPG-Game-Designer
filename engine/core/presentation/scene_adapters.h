#pragma once

#include "scene_translator.h"

namespace urpg::presentation {

struct MapSceneState;
struct BattleSceneState;
struct MenuSceneState;
struct OverlaySceneState;

/**
 * @brief Adapter for MapScene presentation.
 * Section 10.1: MapScene contract
 */
class MapSceneTranslator : public PresentationSceneTranslator {
public:
    using PresentationSceneTranslator::Translate;

    virtual void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        const MapSceneState& sceneState,
        PresentationFrameIntent& outIntent) = 0;
};

/**
 * @brief Adapter for BattleScene presentation.
 * Section 10.2: BattleScene contract
 */
class BattleSceneTranslator : public PresentationSceneTranslator {
public:
    using PresentationSceneTranslator::Translate;

    virtual void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        const BattleSceneState& sceneState,
        PresentationFrameIntent& outIntent) = 0;
};

/**
 * @brief Adapter for MenuScene presentation.
 * Section 10.4: MenuScene contract
 */
class MenuSceneTranslator : public PresentationSceneTranslator {
public:
    using PresentationSceneTranslator::Translate;

    virtual void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        const MenuSceneState& sceneState,
        PresentationFrameIntent& outIntent) = 0;
};

/**
 * @brief Adapter for Overlay/UI presentation.
 * Section 10.4: Message/UI overlays contract
 */
class OverlaySceneTranslator : public PresentationSceneTranslator {
public:
    using PresentationSceneTranslator::Translate;

    virtual void Translate(
        const PresentationContext& context,
        const PresentationAuthoringData& data,
        const OverlaySceneState& sceneState,
        PresentationFrameIntent& outIntent) = 0;
};

} // namespace urpg::presentation
