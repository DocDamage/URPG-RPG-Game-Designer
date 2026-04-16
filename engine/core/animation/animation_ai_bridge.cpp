#include "animation_ai_bridge.h"
#include <sstream>
#include <regex>
#include <iostream>

namespace urpg::ai {

std::string AnimationKnowledgeBridge::generateAnimationPrompt(
    const AnimateableMetadata& actor,
    const std::string& description,
    float duration
) {
    std::stringstream ss;
    ss << "### Animation Copilot Phase ###\n";
    ss << "Actor: " << actor.actorId << " (" << actor.actorType << ")\n";
    ss << "Current Position: [" << actor.currentPosition.x.ToFloat() << ", " 
       << actor.currentPosition.y.ToFloat() << ", " << actor.currentPosition.z.ToFloat() << "]\n";
    ss << "Description of Motion: \"" << description << "\"\n";
    ss << "Target Duration: " << duration << " seconds\n\n";

    ss << "--- Instructions ---\n";
    ss << "Generate a sequence of keyframes in the following JSON format:\n";
    ss << "[{\"time\": T, \"x\": X, \"y\": Y, \"z\": Z}, ...]\n";
    ss << "Use relative positions and ensure the animation fits the duration.";
    
    return ss.str();
}

std::vector<urpg::AnimationKeyframe> AnimationKnowledgeBridge::parseKeyframes(const std::string& aiResponse) {
    std::vector<urpg::AnimationKeyframe> result;
    
    // Simplified regex-based parsing to extract structured keyframes from LLM text
    // In production, we'd use a full JSON parser (e.g. nlohmann::json)
    // but this illustrates the pattern for unstructured LLM extraction.
    std::regex keyframeRegex(R"(\{\s*"time":\s*([-+]?[0-9]*\.?[0-9]+),\s*"x":\s*([-+]?[0-9]*\.?[0-9]+),\s*"y":\s*([-+]?[0-9]*\.?[0-9]+),\s*"z":\s*([-+]?[0-9]*\.?[0-9]+)\s*\})");
    
    auto framesBegin = std::sregex_iterator(aiResponse.begin(), aiResponse.end(), keyframeRegex);
    auto framesEnd = std::sregex_iterator();

    for (std::sregex_iterator i = framesBegin; i != framesEnd; ++i) {
        std::smatch match = *i;
        
        urpg::AnimationKeyframe k;
        k.time = urpg::Fixed32::FromFloat(std::stof(match[1].str()));
        k.value.x = urpg::Fixed32::FromFloat(std::stof(match[2].str()));
        k.value.y = urpg::Fixed32::FromFloat(std::stof(match[3].str()));
        k.value.z = urpg::Fixed32::FromFloat(std::stof(match[4].str()));
        
        result.push_back(k);
    }

    return result;
}

} // namespace urpg::ai