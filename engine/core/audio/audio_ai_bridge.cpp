#include "audio_ai_bridge.h"
#include <sstream>
#include <regex>

namespace urpg::ai {

std::string AudioKnowledgeBridge::generateAudioPrompt(
    const AudioMetadata& current,
    const std::string& targetEventDescription
) {
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

std::vector<AudioKnowledgeBridge::AudioCommand> AudioKnowledgeBridge::parseAudioCommands(const std::string& aiResponse) {
    std::vector<AudioCommand> result;
    
    // Pattern: [ACTION: PLAY_BGM, ASSET: Town_Cheerful, VOL: 0.8, FADE: 2.0]
    std::regex cmdRegex(R"(\[\s*ACTION:\s*(PLAY_BGM|CROSSFADE|PLAY_SE|STOP),\s*ASSET:\s*([a-zA-Z0-9_-]+),\s*VOL:\s*([0-9]*\.?[0-9]+),\s*FADE:\s*([0-9]*\.?[0-9]+)\s*\])");
    
    auto cmdsBegin = std::sregex_iterator(aiResponse.begin(), aiResponse.end(), cmdRegex);
    auto cmdsEnd = std::sregex_iterator();

    for (std::sregex_iterator i = cmdsBegin; i != cmdsEnd; ++i) {
        std::smatch match = *i;
        
        AudioCommand cmd;
        cmd.action = match[1].str();
        cmd.assetId = match[2].str();
        cmd.volume = std::stof(match[3].str());
        cmd.fadeTime = std::stof(match[4].str());
        
        result.push_back(cmd);
    }

    return result;
}

} // namespace urpg::ai