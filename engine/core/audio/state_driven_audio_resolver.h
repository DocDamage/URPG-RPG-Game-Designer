#pragma once

#include <string>
#include <map>
#include <vector>
#include "../global_state_hub.h"
#include "audio_core.h"

namespace urpg::audio {

/**
 * @brief Automates BGM/BGS transitions based on GlobalStateHub state tags.
 * 
 * Example: if "game.current_map_type" == "dungeon", switch BGM to "dungeon_theme".
 */
class StateDrivenAudioResolver {
public:
    struct Rule {
        std::string hubKey;
        std::string hubValue; // Simple string comparison for now
        std::string bgmAssetId;
        float fadeSeconds = 1.0f;
    };

    StateDrivenAudioResolver(AudioCore& core) : m_core(core) {
        // Subscribe to changes in state that might trigger BGM swaps
        m_subHandle = GlobalStateHub::getInstance().subscribe("map.*", [this](const std::string& key, const GlobalStateHub::Value& value) {
            this->evaluateRules(key, value);
        });
        
        m_subHandleBattle = GlobalStateHub::getInstance().subscribe("battle.*", [this](const std::string& key, const GlobalStateHub::Value& value) {
            this->evaluateRules(key, value);
        });
    }

    ~StateDrivenAudioResolver() {
        GlobalStateHub::getInstance().unsubscribe(m_subHandle);
        GlobalStateHub::getInstance().unsubscribe(m_subHandleBattle);
    }

    void addRule(const Rule& rule) {
        m_rules.push_back(rule);
    }

private:
    void evaluateRules(const std::string& key, const GlobalStateHub::Value& value) {
        if (!std::holds_alternative<std::string>(value)) return;
        const std::string& valStr = std::get<std::string>(value);

        for (const auto& rule : m_rules) {
            if (rule.hubKey == key && rule.hubValue == valStr) {
                m_core.playBGM(rule.bgmAssetId, rule.fadeSeconds);
                break;
            }
        }
    }

    AudioCore& m_core;
    std::vector<Rule> m_rules;
    uint32_t m_subHandle = 0;
    uint32_t m_subHandleBattle = 0;
};

} // namespace urpg::audio
