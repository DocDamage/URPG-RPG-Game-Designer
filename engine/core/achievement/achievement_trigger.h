#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::achievement {

/**
 * @brief Structured representation of an achievement unlock condition.
 *
 * Parses condition strings into event-type + parameter + target triples.
 * Supported formats:
 *   - "kill_count_10"          -> eventType="kill_count", target=10
 *   - "battle_victory"         -> eventType="battle_victory", target=1
 *   - "level_reach_5"          -> eventType="level_reach", target=5
 *   - "item_collect:item_id=3:count=5" -> eventType="item_collect", params{item_id="3"}, target=5
 */
struct AchievementTrigger {
    std::string eventType;
    std::unordered_map<std::string, std::string> params;
    uint32_t target = 1;

    /** @brief Parse a condition string into a structured trigger. */
    static AchievementTrigger parse(const std::string& condition);

    /** @brief Check if this trigger matches an emitted event. */
    bool matches(const std::string& eventType,
                 const std::unordered_map<std::string, std::string>& eventParams) const;

    /** @brief Extract target from legacy underscore-suffix format (e.g., "kill_count_10" -> 10). */
    static std::optional<uint32_t> parseLegacyTarget(const std::string& condition);
};

/**
 * @brief Gameplay event published to the achievement event bus.
 */
struct AchievementEvent {
    std::string eventType;
    std::unordered_map<std::string, std::string> params;
    uint32_t increment = 1;
};

/**
 * @brief Lightweight pub/sub bus for achievement-relevant gameplay events.
 */
class AchievementEventBus {
public:
    using Listener = std::function<void(const AchievementEvent&)>;

    void addListener(Listener listener);
    void emit(const AchievementEvent& event);

private:
    std::vector<Listener> listeners_;
};

/**
 * @brief Resolves achievement triggers against gameplay events and auto-reports progress.
 *
 * Binds to an AchievementRegistry and AchievementEventBus. When an event is emitted,
 * checks all registered achievements whose trigger matches the event and reports progress.
 */
class AchievementTriggerResolver {
public:
    void bindRegistry(class AchievementRegistry* registry);
    void bindEventBus(AchievementEventBus* bus);

    /** @brief Manually re-evaluate all triggers (e.g., after loading save data). */
    void refreshAll();

private:
    void onEvent(const AchievementEvent& event);

    AchievementRegistry* registry_ = nullptr;
    AchievementEventBus* bus_ = nullptr;
};

} // namespace urpg::achievement
