#pragma once

#include "engine/core/audio/audio_core.h"
#include <string>
#include <vector>

namespace urpg::ai {

/**
 * @brief Metadata about the current scene's mood and audio state for the AI.
 */
struct AudioMetadata {
    std::string currentBGM;
    float bgmVolume;
    std::string sceneType; // "Town", "Dungeon", "Battle", "Cutscene"
    std::string mood;      // "Tense", "Mysterious", "Cheerful", "Sad"
};

/**
 * @brief Bridge between the Audio System and the AI Director.
 * Allows the AI to orchestrate BGM changes and sound effects based on game state.
 */
class AudioKnowledgeBridge {
  public:
    /**
     * @brief Generates a prompt for the AI to recommend audio changes.
     */
    static std::string generateAudioPrompt(const AudioMetadata& current, const std::string& targetEventDescription);

    /**
     * @brief Interprets the AI's orchestration command.
     */
    struct AudioCommand {
        std::string action; // "PLAY_BGM", "CROSSFADE", "PLAY_SE", "STOP"
        std::string assetId;
        float volume = 1.0f;
        float fadeTime = 1.0f;
    };

    static std::vector<AudioCommand> parseAudioCommands(const std::string& aiResponse);
};

} // namespace urpg::ai
