#include "engine/core/tutorial/runtime_hint_system.h"

namespace urpg::tutorial {

void RuntimeHintSystem::registerHint(RuntimeHint hint) {
    hints_.push_back(std::move(hint));
}

RuntimeHint RuntimeHintSystem::nextHint(const std::set<std::string>& flags) const {
    for (const auto& hint : hints_) {
        if ((!hint.once_only || !dismissed_.contains(hint.id)) && (hint.required_flag.empty() || flags.contains(hint.required_flag))) {
            return hint;
        }
    }
    return {};
}

void RuntimeHintSystem::dismiss(const std::string& hint_id) {
    dismissed_.insert(hint_id);
}

nlohmann::json RuntimeHintSystem::toJson() const {
    nlohmann::json json;
    json["hints"] = nlohmann::json::array();
    for (const auto& hint : hints_) {
        json["hints"].push_back({{"id", hint.id}, {"localizationKey", hint.localization_key}, {"requiredFlag", hint.required_flag}, {"onceOnly", hint.once_only}, {"accessibilityAware", hint.accessibility_aware}});
    }
    json["dismissed"] = dismissed_;
    return json;
}

RuntimeHintSystem RuntimeHintSystem::fromJson(const nlohmann::json& json) {
    RuntimeHintSystem system;
    for (const auto& hint : json.value("hints", nlohmann::json::array())) {
        system.registerHint({hint.value("id", ""), hint.value("localizationKey", ""), hint.value("requiredFlag", ""), hint.value("onceOnly", true), hint.value("accessibilityAware", false)});
    }
    system.dismissed_ = json.value("dismissed", std::set<std::string>{});
    return system;
}

} // namespace urpg::tutorial
