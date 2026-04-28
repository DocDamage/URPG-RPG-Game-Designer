#pragma once

#include "engine/core/ability/authored_ability_asset.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::ability {

struct AbilitySandboxDiagnostic {
    std::string code;
    std::string message;
    std::string target;
};

struct AbilitySandboxDocument {
    std::string id;
    AuthoredAbilityAsset ability;
    float source_mp = 100.0f;
    float source_attack = 10.0f;
    std::vector<std::string> source_tags;
    std::vector<std::string> required_tags;
    std::vector<std::string> blocking_tags;

    std::vector<AbilitySandboxDiagnostic> validate() const;
    nlohmann::json toJson() const;

    static AbilitySandboxDocument fromJson(const nlohmann::json& json);
};

struct AbilitySandboxResult {
    bool activation_allowed = false;
    bool activation_executed = false;
    std::string blocking_reason;
    float mp_before = 0.0f;
    float mp_after = 0.0f;
    float cooldown_after = 0.0f;
    float effect_attribute_before = 0.0f;
    float effect_attribute_after = 0.0f;
    size_t active_effect_count = 0;
    size_t active_tag_count = 0;
    std::vector<std::string> visible_tags;
    std::vector<AbilitySandboxDiagnostic> diagnostics;
};

AbilitySandboxResult RunAbilitySandbox(const AbilitySandboxDocument& document);

} // namespace urpg::ability
