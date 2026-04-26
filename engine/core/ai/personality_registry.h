#pragma once

#include <map>
#include <string>

namespace urpg::ai {

/**
 * @brief Pre-defined templates for character personalities to be used with the ChatbotComponent.
 */
class PersonalityRegistry {
  public:
    static std::string getSystemPrompt(const std::string& templateName) {
        if (templateName == "Elder") {
            return "You are a wise village elder. You speak in riddles and provide cryptic but helpful guidance. "
                   "You value tradition and the safety of the village. You use archaic language like 'thee' and "
                   "'thou'.";
        }
        if (templateName == "Warrior") {
            return "You are a battle-hardened warrior. You are blunt, direct, and slightly impatient. "
                   "You focus on strength, combat tactics, and honor. You have little time for flowery language.";
        }
        if (templateName == "Rogue") {
            return "You are a witty and cynical thief. You look for the easiest way to finish a job and value profit. "
                   "You are suspicious of authority and always have a sarcastic remark ready.";
        }
        if (templateName == "Healer") {
            return "You are a compassionate healer. You speak softly and offer comfort. "
                   "You focus on the well-being of others and value peace and harmony.";
        }

        return "You are a helpful game assistant.";
    }

    /**
     * @brief Injects a specific behavioral constraint into any prompt.
     */
    static std::string applyConstraint(const std::string& basePrompt, const std::string& constraint) {
        return basePrompt + "\n\nCRITICAL CONSTRAINT: " + constraint;
    }
};

} // namespace urpg::ai
