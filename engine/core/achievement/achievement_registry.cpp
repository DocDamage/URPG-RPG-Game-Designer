#include "engine/core/achievement/achievement_registry.h"
#include "engine/core/achievement/achievement_trigger.h"

#include <algorithm>
#include <cstdlib>

namespace urpg::achievement {

void AchievementRegistry::registerAchievement(const AchievementDef& def) {
    m_definitions[def.id] = def;
    if (m_progress.find(def.id) == m_progress.end()) {
        AchievementProgress progress;
        if (def.target > 0) {
            progress.target = def.target;
        } else {
            progress.target = AchievementTrigger::parse(def.unlockCondition).target;
        }
        m_progress[def.id] = progress;
    }
}

std::optional<AchievementDef> AchievementRegistry::getAchievement(const std::string& id) const {
    auto it = m_definitions.find(id);
    if (it != m_definitions.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<AchievementDef> AchievementRegistry::getAllAchievements() const {
    std::vector<AchievementDef> result;
    result.reserve(m_definitions.size());
    for (const auto& pair : m_definitions) {
        result.push_back(pair.second);
    }
    std::sort(result.begin(), result.end(),
        [](const AchievementDef& a, const AchievementDef& b) {
            return a.id < b.id;
        });
    return result;
}

bool AchievementRegistry::reportProgress(const std::string& id, uint32_t increment) {
    auto defIt = m_definitions.find(id);
    if (defIt == m_definitions.end()) {
        return false;
    }

    auto& progress = m_progress[id];
    if (progress.unlocked) {
        return false;
    }

    progress.current += increment;
    if (progress.current >= progress.target) {
        progress.unlocked = true;
        progress.unlockTime = "deterministic_timestamp";
        return true;
    }
    return false;
}

std::optional<AchievementProgress> AchievementRegistry::getProgress(const std::string& id) const {
    auto it = m_progress.find(id);
    if (it != m_progress.end()) {
        return it->second;
    }
    return std::nullopt;
}

void AchievementRegistry::resetProgress(const std::string& id) {
    auto it = m_progress.find(id);
    if (it != m_progress.end()) {
        it->second.current = 0;
        it->second.unlocked = false;
        it->second.unlockTime = std::nullopt;
    }
}

nlohmann::json AchievementRegistry::saveToJson() const {
    nlohmann::json progressArray = nlohmann::json::array();
    for (const auto& pair : m_progress) {
        const auto& p = pair.second;
        nlohmann::json entry{
            {"id", pair.first},
            {"current", p.current},
            {"target", p.target},
            {"unlocked", p.unlocked}
        };
        if (p.unlockTime.has_value()) {
            entry["unlockTime"] = p.unlockTime.value();
        }
        progressArray.push_back(entry);
    }

    return nlohmann::json{
        {"version", "1.0.0"},
        {"progress", progressArray}
    };
}

void AchievementRegistry::loadFromJson(const nlohmann::json& json) {
    if (!json.contains("version") || !json["version"].is_string()) {
        return;
    }
    if (json["version"].get<std::string>() != "1.0.0") {
        return;
    }

    if (!json.contains("progress") || !json["progress"].is_array()) {
        return;
    }

    for (const auto& entry : json["progress"]) {
        if (!entry.is_object()) {
            continue;
        }
        if (!entry.contains("id") || !entry["id"].is_string()) {
            continue;
        }

        std::string id = entry["id"].get<std::string>();
        if (m_definitions.find(id) == m_definitions.end()) {
            continue;
        }

        AchievementProgress progress;
        if (entry.contains("current") && entry["current"].is_number_unsigned()) {
            progress.current = entry["current"].get<uint32_t>();
        }
        if (entry.contains("target") && entry["target"].is_number_unsigned()) {
            progress.target = entry["target"].get<uint32_t>();
        }
        if (entry.contains("unlocked") && entry["unlocked"].is_boolean()) {
            progress.unlocked = entry["unlocked"].get<bool>();
        }
        if (entry.contains("unlockTime") && entry["unlockTime"].is_string()) {
            progress.unlockTime = entry["unlockTime"].get<std::string>();
        }

        m_progress[id] = progress;
    }
}

nlohmann::json AchievementRegistry::exportTrophyPayload(const std::string& platform) const {
    const auto definitions = getAllAchievements();

    nlohmann::json trophies = nlohmann::json::array();
    size_t unlockedCount = 0;
    size_t secretCount = 0;

    for (const auto& def : definitions) {
        const auto progressIt = m_progress.find(def.id);
        AchievementProgress progress;
        if (progressIt != m_progress.end()) {
            progress = progressIt->second;
        }

        if (progress.unlocked) {
            ++unlockedCount;
        }
        if (def.secret) {
            ++secretCount;
        }

        nlohmann::json trophy{
            {"id", def.id},
            {"title", def.title},
            {"description", def.description},
            {"secret", def.secret},
            {"unlockCondition", def.unlockCondition},
            {"iconId", def.iconId},
            {"target", progress.target},
            {"progress", progress.current},
            {"unlocked", progress.unlocked}
        };
        if (progress.unlockTime.has_value()) {
            trophy["unlockTime"] = progress.unlockTime.value();
        }
        trophies.push_back(trophy);
    }

    return nlohmann::json{
        {"version", "1.0.0"},
        {"platform", platform.empty() ? "urpg-neutral" : platform},
        {"backendIntegration", "out-of-tree"},
        {"summary", {
            {"total", definitions.size()},
            {"unlocked", unlockedCount},
            {"secret", secretCount}
        }},
        {"trophies", trophies}
    };
}

} // namespace urpg::achievement
