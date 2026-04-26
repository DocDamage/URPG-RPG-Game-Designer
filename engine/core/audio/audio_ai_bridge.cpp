#include "audio_ai_bridge.h"
#include <algorithm>
#include <sstream>

namespace urpg::ai {

namespace {

std::string trim(std::string value) {
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

} // namespace

std::string AudioKnowledgeBridge::generateAudioPrompt(const AudioMetadata& current,
                                                      const std::string& targetEventDescription) {
    std::stringstream ss;
    ss << "### Audio Orchestrator Phase ###\n";
    ss << "Current BGM: " << (current.currentBGM.empty() ? "None" : current.currentBGM) << "\n";
    ss << "Scene Type: " << current.sceneType << "\n";
    ss << "Current Mood: " << current.mood << "\n";
    ss << "Recent Event Description: \"" << targetEventDescription << "\"\n\n";

    ss << "--- Instructions ---\n";
    ss << "Recommend an audio change using the following format:\n";
    ss << "[ACTION: PLAY_BGM|CROSSFADE|PLAY_SE|STOP, ASSET: AssetID, VOL: 0.0-1.0, FADE: Seconds]\n";
    ss << "Ensure the music matches the new scene mood and transitions smoothly.";

    return ss.str();
}

std::vector<AudioKnowledgeBridge::AudioCommand>
AudioKnowledgeBridge::parseAudioCommands(const std::string& aiResponse) {
    std::vector<AudioCommand> result;

    size_t search_pos = 0;
    while (true) {
        const size_t open = aiResponse.find('[', search_pos);
        if (open == std::string::npos) {
            break;
        }

        const size_t close = aiResponse.find(']', open + 1);
        if (close == std::string::npos) {
            break;
        }

        const std::string payload = aiResponse.substr(open + 1, close - open - 1);
        search_pos = close + 1;

        AudioCommand cmd;
        bool saw_action = false;
        bool saw_asset = false;
        bool saw_volume = false;
        bool saw_fade = false;

        size_t field_start = 0;
        while (field_start <= payload.size()) {
            size_t comma = payload.find(',', field_start);
            std::string field =
                trim(payload.substr(field_start, comma == std::string::npos ? std::string::npos : comma - field_start));
            field_start = (comma == std::string::npos) ? payload.size() + 1 : comma + 1;

            const size_t colon = field.find(':');
            if (colon == std::string::npos) {
                continue;
            }

            const std::string key = trim(field.substr(0, colon));
            const std::string value = trim(field.substr(colon + 1));

            if (key == "ACTION") {
                cmd.action = value;
                saw_action = true;
            } else if (key == "ASSET") {
                cmd.assetId = value;
                saw_asset = true;
            } else if (key == "VOL") {
                cmd.volume = std::stof(value);
                saw_volume = true;
            } else if (key == "FADE") {
                cmd.fadeTime = std::stof(value);
                saw_fade = true;
            }
        }

        if (saw_action && saw_asset && saw_volume && saw_fade) {
            result.push_back(std::move(cmd));
        }
    }

    return result;
}

} // namespace urpg::ai
