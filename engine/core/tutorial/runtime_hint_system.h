#pragma once

#include <nlohmann/json.hpp>

#include <set>
#include <string>
#include <vector>

namespace urpg::tutorial {

struct RuntimeHint {
    std::string id;
    std::string localization_key;
    std::string required_flag;
    bool once_only{true};
    bool accessibility_aware{false};
};

class RuntimeHintSystem {
public:
    void registerHint(RuntimeHint hint);
    [[nodiscard]] RuntimeHint nextHint(const std::set<std::string>& flags) const;
    void dismiss(const std::string& hint_id);
    [[nodiscard]] nlohmann::json toJson() const;
    [[nodiscard]] static RuntimeHintSystem fromJson(const nlohmann::json& json);

private:
    std::vector<RuntimeHint> hints_;
    std::set<std::string> dismissed_;
};

} // namespace urpg::tutorial
