#include "animation_ai_bridge.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace urpg::ai {

namespace {

std::string trim(std::string value) {
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

} // namespace

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

    auto appendFrame = [&result](float time, float x, float y, float z) {
        urpg::AnimationKeyframe keyframe;
        keyframe.time = urpg::Fixed32::FromFloat(time);
        keyframe.value.x = urpg::Fixed32::FromFloat(x);
        keyframe.value.y = urpg::Fixed32::FromFloat(y);
        keyframe.value.z = urpg::Fixed32::FromFloat(z);
        result.push_back(keyframe);
    };

    size_t search_pos = 0;
    while (true) {
        const size_t marker = aiResponse.find("[KEYFRAME:", search_pos);
        if (marker == std::string::npos) {
            break;
        }
        const size_t close = aiResponse.find(']', marker);
        if (close == std::string::npos) {
            break;
        }

        const std::string payload = aiResponse.substr(marker + 1, close - marker - 1);
        search_pos = close + 1;

        const size_t comma = payload.find(',');
        const size_t pos_marker = payload.find("POS:");
        if (comma == std::string::npos || pos_marker == std::string::npos) {
            continue;
        }

        const std::string time_str = trim(payload.substr(std::string("KEYFRAME:").size(), comma - std::string("KEYFRAME:").size()));
        const std::string pos_str = trim(payload.substr(pos_marker + std::string("POS:").size()));

        const size_t first_sep = pos_str.find(':');
        const size_t second_sep = pos_str.find(':', first_sep == std::string::npos ? std::string::npos : first_sep + 1);
        if (first_sep == std::string::npos || second_sep == std::string::npos) {
            continue;
        }

        appendFrame(
            std::stof(time_str),
            std::stof(trim(pos_str.substr(0, first_sep))),
            std::stof(trim(pos_str.substr(first_sep + 1, second_sep - first_sep - 1))),
            std::stof(trim(pos_str.substr(second_sep + 1)))
        );
    }

    if (!result.empty()) {
        return result;
    }

    search_pos = 0;
    while (true) {
        const size_t open = aiResponse.find('{', search_pos);
        if (open == std::string::npos) {
            break;
        }
        const size_t close = aiResponse.find('}', open + 1);
        if (close == std::string::npos) {
            break;
        }

        const std::string payload = aiResponse.substr(open + 1, close - open - 1);
        search_pos = close + 1;

        float time = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        bool saw_time = false;
        bool saw_x = false;
        bool saw_y = false;
        bool saw_z = false;

        size_t field_start = 0;
        while (field_start <= payload.size()) {
            size_t comma = payload.find(',', field_start);
            std::string field = trim(payload.substr(field_start, comma == std::string::npos ? std::string::npos : comma - field_start));
            field_start = (comma == std::string::npos) ? payload.size() + 1 : comma + 1;

            const size_t colon = field.find(':');
            if (colon == std::string::npos) {
                continue;
            }

            std::string key = trim(field.substr(0, colon));
            const std::string value = trim(field.substr(colon + 1));
            key.erase(std::remove(key.begin(), key.end(), '"'), key.end());

            if (key == "time") {
                time = std::stof(value);
                saw_time = true;
            } else if (key == "x") {
                x = std::stof(value);
                saw_x = true;
            } else if (key == "y") {
                y = std::stof(value);
                saw_y = true;
            } else if (key == "z") {
                z = std::stof(value);
                saw_z = true;
            }
        }

        if (saw_time && saw_x && saw_y && saw_z) {
            appendFrame(time, x, y, z);
        }
    }

    std::sort(result.begin(), result.end(), [](const urpg::AnimationKeyframe& lhs, const urpg::AnimationKeyframe& rhs) {
        return lhs.time < rhs.time;
    });

    return result;
}

} // namespace urpg::ai
