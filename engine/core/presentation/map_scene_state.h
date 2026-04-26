#pragma once

#include <string>
#include <vector>

namespace urpg::presentation {

/**
 * @brief Representation of an actor's state in the MapScene.
 */
struct MapActorState {
    uint32_t actorId;
    std::string classId; // Links to ActorPresentationProfile
    float posX, posY;    // Grid coordinates
    bool isMoving;
    // ... animation state etc
};

/**
 * @brief Representation of the static map state.
 */
struct MapSceneState {
    std::string mapId;
    std::vector<MapActorState> actors;
};

} // namespace urpg::presentation
