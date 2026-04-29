#pragma once

#include "../global_state_hub.h"
#include <cstdint>
#include "audio_core.h"
#include <map>
#include <optional>
#include <string>
#include <vector>

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

    struct WeightedTrack {
        std::string assetId;
        int32_t weight = 1;
    };

    struct RandomRule {
        std::string hubKey;
        std::string hubValue;
        std::vector<WeightedTrack> tracks;
        uint32_t seed = 0;
        float fadeSeconds = 1.0f;
    };

    StateDrivenAudioResolver(AudioCore& core) : m_core(core) {
        // Subscribe to changes in state that might trigger BGM swaps
        // Updated Pattern matching to use global listener for multi-prefix rules
        m_subHandle = GlobalStateHub::getInstance().subscribe(
            "*",
            [this](const std::string& key, const GlobalStateHub::Value& value) { this->evaluateRules(key, value); });
    }

    ~StateDrivenAudioResolver() { GlobalStateHub::getInstance().unsubscribe(m_subHandle); }

    void addRule(const Rule& rule) { m_rules.push_back(rule); }
    void addRandomRule(const RandomRule& rule) { m_randomRules.push_back(rule); }

    static std::optional<std::string> selectWeightedTrack(const std::vector<WeightedTrack>& tracks, uint32_t seed) {
        int32_t totalWeight = 0;
        for (const auto& track : tracks) {
            if (!track.assetId.empty() && track.weight > 0) {
                totalWeight += track.weight;
            }
        }
        if (totalWeight <= 0) {
            return std::nullopt;
        }

        const auto roll = static_cast<int32_t>((seed * 1103515245U + 12345U) % static_cast<uint32_t>(totalWeight));
        int32_t cursor = 0;
        for (const auto& track : tracks) {
            if (track.assetId.empty() || track.weight <= 0) {
                continue;
            }
            cursor += track.weight;
            if (roll < cursor) {
                return track.assetId;
            }
        }
        return std::nullopt;
    }

  private:
    void evaluateRules(const std::string& key, const GlobalStateHub::Value& value) {
        // Handle both string and int values for state rules (common for map IDs)
        std::string valStr;
        if (std::holds_alternative<std::string>(value)) {
            valStr = std::get<std::string>(value);
        } else if (std::holds_alternative<int32_t>(value)) {
            valStr = std::to_string(std::get<int32_t>(value));
        } else {
            return;
        }

        for (const auto& rule : m_rules) {
            if (rule.hubKey == key && rule.hubValue == valStr) {
                m_core.playBGM(rule.bgmAssetId, rule.fadeSeconds);
                break;
            }
        }
        for (const auto& rule : m_randomRules) {
            if (rule.hubKey == key && rule.hubValue == valStr) {
                if (const auto selected = selectWeightedTrack(rule.tracks, rule.seed)) {
                    m_core.playBGM(*selected, rule.fadeSeconds);
                }
                break;
            }
        }
    }

    AudioCore& m_core;
    std::vector<Rule> m_rules;
    std::vector<RandomRule> m_randomRules;
    uint32_t m_subHandle = 0;
};

} // namespace urpg::audio
