#pragma once

#include "engine/core/animation/animation_components.h"
#include <string>
#include <vector>

namespace urpg::ai {

/**
 * @brief Representation of an actor's animate-able properties for the AI.
 */
struct AnimateableMetadata {
    std::string actorId;
    std::string actorType; // "Hero", "NPC", "Light", "Camera"
    Vector3 currentPosition;
    Vector3 currentRotation;
};

/**
 * @brief Bridge between the Animation System and the AI Creator.
 * Allows the AI to generate keyframes based on natural language descriptions (e.g., "Make him jump").
 */
class AnimationKnowledgeBridge {
public:
    /**
     * @brief Generates a prompt for keyframe generation.
     */
    static std::string generateAnimationPrompt(
        const AnimateableMetadata& actor,
        const std::string& description,
        float duration
    );

    /**
     * @brief Parses the LLM's response into native animation tracks.
     */
    static std::vector<urpg::AnimationKeyframe> parseKeyframes(const std::string& aiResponse);
};

} // namespace urpg::ai