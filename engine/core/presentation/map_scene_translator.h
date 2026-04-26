#pragma once

#include "map_scene_state.h"
#include "presentation_schema.h"
#include "scene_adapters.h"
#include <cmath>

namespace urpg::presentation {

/**
 * @brief Concrete implementation of the MapScene translator.
 * Section 10.1: MapScene Integration
 */
class MapSceneTranslatorImpl : public MapSceneTranslator {
  public:
    static const SpatialMapOverlay* ResolveMapOverlay(const PresentationAuthoringData& data, const std::string& mapId) {
        for (const auto& overlay : data.mapOverlays) {
            if (overlay.mapId == mapId) {
                return &overlay;
            }
        }

        if (!data.mapOverlays.empty()) {
            return &data.mapOverlays.front();
        }

        return nullptr;
    }

    static const ActorPresentationProfile* ResolveActorProfile(const PresentationAuthoringData& data,
                                                               const std::string& classId) {
        for (const auto& profile : data.actorProfiles) {
            if (profile.actorId == classId) {
                return &profile;
            }
        }

        return nullptr;
    }

    /**
     * @brief Calculates the world position for an actor based on grid coordinates and elevation.
     */
    static Vec3 CalculateActorWorldPosition(const MapActorState& actor, const ElevationGrid& elevation) {

        // ADR-011: Anchoring Math
        // We use the floor of coordinates for the tile index to find base elevation,
        // then apply the fractional component if we want slope interpolation (not used for classic steps).

        int ix = static_cast<int>(std::floor(actor.posX));
        int iy = static_cast<int>(std::floor(actor.posY));

        float height =
            elevation.GetWorldHeight(static_cast<uint32_t>(ix >= 0 ? ix : 0), static_cast<uint32_t>(iy >= 0 ? iy : 0));

        // Apply grid scale (defaulting to 1.0f units per tile if not specified)
        // For URPG, 1 tile = 1.0 units usually.
        return Vec3{actor.posX, height, actor.posY};
    }

    void Translate(const PresentationContext& context, const PresentationAuthoringData& data,
                   const MapSceneState& sceneState, PresentationFrameIntent& outIntent) override {
        const SpatialMapOverlay* mapData = ResolveMapOverlay(data, sceneState.mapId);
        if (!mapData) {
            return;
        }

        std::vector<std::pair<uint32_t, Vec3>> visibleActorGroundPositions;
        visibleActorGroundPositions.reserve(sceneState.actors.size());

        // 1. Process Actors and Anchoring (Section 10.1)
        for (const auto& actor : sceneState.actors) {
            const ActorPresentationProfile* profile = ResolveActorProfile(data, actor.classId);
            if (!profile) {
                continue;
            }

            Vec3 spatialPos = CalculateActorWorldPosition(actor, mapData->elevation);
            spatialPos.y += profile->anchorOffset.y;
            visibleActorGroundPositions.push_back(
                {actor.actorId, CalculateActorWorldPosition(actor, mapData->elevation)});

            if (context.activeMode == PresentationMode::Classic2D) {
                outIntent.AddActor(actor.actorId, Vec3{actor.posX, actor.posY, 0.0f}, *profile);
            } else {
                outIntent.AddActor(actor.actorId, spatialPos, *profile);
            }
        }

        // 2. Process Props with a stable per-overlay ordering.
        for (size_t index = 0; index < mapData->props.size(); ++index) {
            const auto& prop = mapData->props[index];
            outIntent.AddProp(static_cast<uint32_t>(index + 1), Vec3{prop.posX, prop.posY, prop.posZ},
                              Vec3{0, prop.rotY, 0});
        }

        // 3. Environment intent is spatial-only.
        if (context.activeTier > CapabilityTier::Tier0_Baseline) {
            for (const auto& light : mapData->lights) {
                outIntent.AddLight(light);
            }
            outIntent.AddFog(mapData->fog);
            outIntent.AddPostFX(mapData->postFX);

            for (const auto& [actorId, groundPos] : visibleActorGroundPositions) {
                outIntent.AddShadowProxy(actorId, groundPos, Vec3{0, 0, 0});
            }
        }
    }

    // Unused base method for this specific adapter
    void Translate(const PresentationContext&, const PresentationAuthoringData&, PresentationFrameIntent&) override {}
};

} // namespace urpg::presentation
