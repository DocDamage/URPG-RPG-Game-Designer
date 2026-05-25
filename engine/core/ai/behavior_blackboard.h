#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace urpg::ai {

class BehaviorBlackboard {
  public:
    void setBool(const std::string& key, bool value);
    [[nodiscard]] std::optional<bool> boolValue(const std::string& key) const;

    void setInt(const std::string& key, int32_t value);
    [[nodiscard]] std::optional<int32_t> intValue(const std::string& key) const;

    void setString(const std::string& key, std::string value);
    [[nodiscard]] std::optional<std::string> stringValue(const std::string& key) const;

    void startCooldown(const std::string& key, int32_t ticks);
    void tickCooldowns();
    [[nodiscard]] bool cooldownReady(const std::string& key) const;
    [[nodiscard]] int32_t cooldownRemaining(const std::string& key) const;

  private:
    std::unordered_map<std::string, bool> bools_;
    std::unordered_map<std::string, int32_t> ints_;
    std::unordered_map<std::string, std::string> strings_;
    std::unordered_map<std::string, int32_t> cooldowns_;
};

} // namespace urpg::ai
